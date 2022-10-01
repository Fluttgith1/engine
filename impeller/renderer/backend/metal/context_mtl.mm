// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/metal/context_mtl.h"

#include <Foundation/Foundation.h>

#include "flutter/fml/file.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/paths.h"
#include "impeller/base/platform/darwin/work_queue_darwin.h"
#include "impeller/renderer/backend/metal/sampler_library_mtl.h"
#include "impeller/renderer/sampler_descriptor.h"

namespace impeller {

ContextMTL::ContextMTL(id<MTLDevice> device,
                       NSArray<id<MTLLibrary>>* shader_libraries)
    : device_(device) {
  // Validate device.
  if (!device_) {
    VALIDATION_LOG << "Could not setup valid Metal device.";
    return;
  }

  // Setup the shader library.
  {
    if (shader_libraries == nil) {
      VALIDATION_LOG << "Shader libraries were null.";
      return;
    }

    // std::make_shared disallowed because of private friend ctor.
    auto library = std::shared_ptr<ShaderLibraryMTL>(
        new ShaderLibraryMTL(shader_libraries));
    if (!library->IsValid()) {
      VALIDATION_LOG << "Could not create valid Metal shader library.";
      return;
    }
    shader_library_ = std::move(library);
  }

  // Setup command queue.
  {
    command_queue_ = device_.newCommandQueue;
    if (!command_queue_) {
      VALIDATION_LOG << "Could not setup the command queue.";
      return;
    }
    command_queue_.label = @"Impeller Command Queue";
  }

  // Setup the pipeline library.
  {
    pipeline_library_ =
        std::shared_ptr<PipelineLibraryMTL>(new PipelineLibraryMTL(device_));
  }

  // Setup the sampler library.
  {
    sampler_library_ =
        std::shared_ptr<SamplerLibraryMTL>(new SamplerLibraryMTL(device_));
  }

  // Setup the resource allocator.
  {
    resource_allocator_ = std::shared_ptr<AllocatorMTL>(
        new AllocatorMTL(device_, "Impeller Permanents Allocator"));
    if (!resource_allocator_) {
      VALIDATION_LOG << "Could not setup the resource allocator.";
      return;
    }
  }

  // Setup the work queue.
  {
    work_queue_ = WorkQueueDarwin::Create();
    if (!work_queue_) {
      VALIDATION_LOG << "Could not setup the work queue.";
      return;
    }
  }

  is_valid_ = true;
}

static NSArray<id<MTLLibrary>>* MTLShaderLibraryFromFilePaths(
    id<MTLDevice> device,
    const std::vector<std::string>& libraries_paths) {
  NSMutableArray<id<MTLLibrary>>* found_libraries = [NSMutableArray array];
  for (const auto& library_path : libraries_paths) {
    if (!fml::IsFile(library_path)) {
      VALIDATION_LOG << "Shader library does not exist at path '"
                     << library_path << "'";
      return nil;
    }
    NSError* shader_library_error = nil;
    auto library = [device newLibraryWithFile:@(library_path.c_str())
                                        error:&shader_library_error];
    if (!library) {
      FML_LOG(ERROR) << "Could not create shader library: "
                     << shader_library_error.localizedDescription.UTF8String;
      return nil;
    }
    [found_libraries addObject:library];
  }
  return found_libraries;
}

static NSArray<id<MTLLibrary>>* MTLShaderLibraryFromFileData(
    id<MTLDevice> device,
    const std::vector<std::shared_ptr<fml::Mapping>>& libraries_data,
    const std::string& label) {
  NSMutableArray<id<MTLLibrary>>* found_libraries = [NSMutableArray array];
  for (const auto& library_data : libraries_data) {
    if (library_data == nullptr) {
      FML_LOG(ERROR) << "Shader library data was null.";
      return nil;
    }

    __block auto data = library_data;

    auto dispatch_data =
        ::dispatch_data_create(library_data->GetMapping(),  // buffer
                               library_data->GetSize(),     // size
                               dispatch_get_main_queue(),   // queue
                               ^() {
                                 // We just need a reference.
                                 data.reset();
                               }  // destructor
        );
    if (!dispatch_data) {
      FML_LOG(ERROR) << "Could not wrap shader data in dispatch data.";
      return nil;
    }

    NSError* shader_library_error = nil;
    auto library = [device newLibraryWithData:dispatch_data
                                        error:&shader_library_error];
    if (!library) {
      FML_LOG(ERROR) << "Could not create shader library: "
                     << shader_library_error.localizedDescription.UTF8String;
      return nil;
    }
    [found_libraries addObject:library];
  }
  return found_libraries;
}

static id<MTLDevice> CreateMetalDevice() {
  return ::MTLCreateSystemDefaultDevice();
}

std::shared_ptr<Context> ContextMTL::Create(
    const std::vector<std::string>& shader_library_paths) {
  auto device = CreateMetalDevice();
  auto context = std::shared_ptr<ContextMTL>(new ContextMTL(
      device, MTLShaderLibraryFromFilePaths(device, shader_library_paths)));
  if (!context->IsValid()) {
    FML_LOG(ERROR) << "Could not create Metal context.";
    return nullptr;
  }
  return context;
}

std::shared_ptr<Context> ContextMTL::Create(
    const std::vector<std::shared_ptr<fml::Mapping>>& shader_libraries_data,
    const std::string& label) {
  auto device = CreateMetalDevice();
  auto context = std::shared_ptr<ContextMTL>(new ContextMTL(
      device,
      MTLShaderLibraryFromFileData(device, shader_libraries_data, label)));
  if (!context->IsValid()) {
    FML_LOG(ERROR) << "Could not create Metal context.";
    return nullptr;
  }
  return context;
}

ContextMTL::~ContextMTL() = default;

// |Context|
bool ContextMTL::IsValid() const {
  return is_valid_;
}

// |Context|
std::shared_ptr<ShaderLibrary> ContextMTL::GetShaderLibrary() const {
  return shader_library_;
}

// |Context|
std::shared_ptr<PipelineLibrary> ContextMTL::GetPipelineLibrary() const {
  return pipeline_library_;
}

// |Context|
std::shared_ptr<SamplerLibrary> ContextMTL::GetSamplerLibrary() const {
  return sampler_library_;
}

// |Context|
std::shared_ptr<CommandBuffer> ContextMTL::CreateCommandBuffer() const {
  return CreateCommandBufferInQueue(command_queue_);
}

// |Context|
std::shared_ptr<WorkQueue> ContextMTL::GetWorkQueue() const {
  return work_queue_;
}

std::shared_ptr<CommandBuffer> ContextMTL::CreateCommandBufferInQueue(
    id<MTLCommandQueue> queue) const {
  if (!IsValid()) {
    return nullptr;
  }

  auto buffer = std::shared_ptr<CommandBufferMTL>(
      new CommandBufferMTL(weak_from_this(), queue));
  if (!buffer->IsValid()) {
    return nullptr;
  }
  return buffer;
}

std::shared_ptr<Allocator> ContextMTL::GetResourceAllocator() const {
  return resource_allocator_;
}

id<MTLDevice> ContextMTL::GetMTLDevice() const {
  return device_;
}

// |Context|
bool ContextMTL::SupportsOffscreenMSAA() const {
  return true;
}

// |Context|
bool ContextMTL::StartCapturingFrames(bool saveLocally) const {
  if (!device_ || !command_queue_) {
    return false;
  }

  MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
  if (captureManager.isCapturing) {
    return false;
  }

  if (@available(iOS 13.0, macOS 10.15, *)) {
    MTLCaptureDescriptor* desc = [[MTLCaptureDescriptor alloc] init];
    desc.captureObject = device_;
    return [captureManager startCaptureWithDescriptor:desc error:nil];
  }

  [captureManager startCaptureWithDevice:device_];
  return captureManager.isCapturing;
}

// |Context|
bool ContextMTL::StopCapturingFrames() const {
  if (!device_ || !command_queue_) {
    return false;
  }

  MTLCaptureManager* captureManager = [MTLCaptureManager sharedCaptureManager];
  if (!captureManager.isCapturing) {
    return false;
  }

  [captureManager stopCapture];
  return !captureManager.isCapturing;
}

}  // namespace impeller
