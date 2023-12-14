// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_METAL_DEVICE_BUFFER_MTL_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_METAL_DEVICE_BUFFER_MTL_H_

#include <Metal/Metal.h>

#include "flutter/fml/macros.h"
#include "impeller/base/backend_cast.h"
#include "impeller/core/device_buffer.h"

namespace impeller {

class DeviceBufferMTL final : public DeviceBuffer,
                              public BackendCast<DeviceBufferMTL, Buffer> {
 public:
  DeviceBufferMTL();

  // |DeviceBuffer|
  ~DeviceBufferMTL() override;

  id<MTLBuffer> GetMTLBuffer() const;

 private:
  friend class AllocatorMTL;

  const id<MTLBuffer> buffer_;
  const MTLStorageMode storage_mode_;

  DeviceBufferMTL(DeviceBufferDescriptor desc,
                  id<MTLBuffer> buffer,
                  MTLStorageMode storage_mode);

  // |DeviceBuffer|
  uint8_t* OnGetContents() const override;

  // |DeviceBuffer|
  std::shared_ptr<Texture> AsTexture(Allocator& allocator,
                                     const TextureDescriptor& descriptor,
                                     uint16_t row_bytes) const override;

  // |DeviceBuffer|
  bool OnCopyHostBuffer(const uint8_t* source,
                        Range source_range,
                        size_t offset) override;

  // |DeviceBuffer|
  bool SetLabel(const std::string& label) override;

  // |DeviceBuffer|
  bool SetLabel(const std::string& label, Range range) override;

  DeviceBufferMTL(const DeviceBufferMTL&) = delete;

  DeviceBufferMTL& operator=(const DeviceBufferMTL&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_METAL_DEVICE_BUFFER_MTL_H_
