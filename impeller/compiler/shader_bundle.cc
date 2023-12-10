// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/compiler/shader_bundle.h"
#include "impeller/compiler/compiler.h"
#include "impeller/compiler/reflector.h"
#include "impeller/compiler/source_options.h"
#include "impeller/compiler/types.h"

#include "impeller/compiler/utilities.h"
#include "impeller/shader_bundle/shader_bundle_flatbuffers.h"
#include "third_party/json/include/nlohmann/json.hpp"

namespace impeller {
namespace compiler {

static std::optional<ShaderBundleConfig> ParseShaderBundleConfig(
    std::string& json_config) {
  auto json = nlohmann::json::parse(json_config);
  if (!json.is_object()) {
    std::cerr << "The shader bundle must be a JSON object." << std::endl;
    return std::nullopt;
  }

  ShaderBundleConfig bundle;
  for (auto& [shader_name, shader_value] : json.items()) {
    if (bundle.find(shader_name) != bundle.end()) {
      std::cerr << "Duplicate shader \"" << shader_name << "\"." << std::endl;
      return std::nullopt;
    }
    if (!shader_value.is_object()) {
      std::cerr << "Invalid shader entry \"" << shader_name
                << "\": Entry is not a JSON object." << std::endl;
      return std::nullopt;
    }

    ShaderConfig shader;

    if (!shader_value.contains("file")) {
      std::cerr << "Invalid shader entry \"" << shader_name
                << "\": Missing required \"file\" field. \"" << std::endl;
      return std::nullopt;
    }
    shader.source_file_name = shader_value["file"];

    if (!shader_value.contains("type")) {
      std::cerr << "Invalid shader entry \"" << shader_name
                << "\": Missing required \"type\" field. \"" << std::endl;
      return std::nullopt;
    }
    shader.type = SourceTypeFromString(shader_value["type"]);
    if (shader.type == SourceType::kUnknown) {
      std::cerr << "Invalid shader entry \"" << shader_name
                << "\": Shader type \"" << shader_value["type"]
                << "\" is unknown. \"" << std::endl;
      return std::nullopt;
    }

    shader.language = shader_value.contains("language")
                          ? ToSourceLanguage(shader_value["language"])
                          : SourceLanguage::kGLSL;
    if (shader.language == SourceLanguage::kUnknown) {
      std::cerr << "Invalid shader entry \"" << shader_name
                << "\": Unknown language type \"" << shader_value["language"]
                << "\"." << std::endl;
      return std::nullopt;
    }

    shader.entry_point = shader_value.contains("entry_point")
                             ? shader_value["entry_point"]
                             : "main";

    bundle[shader_name] = shader;
  }

  return bundle;
}

static std::unique_ptr<fb::ShaderT> GenerateShaderFB(
    SourceOptions& options,
    const std::string& shader_name,
    const ShaderConfig& shader_config) {
  auto result = std::make_unique<fb::ShaderT>();
  result->name = shader_name;

  std::shared_ptr<fml::FileMapping> source_file_mapping =
      fml::FileMapping::CreateReadOnly(shader_config.source_file_name);
  if (!source_file_mapping) {
    std::cerr << "Could not open file for bundled shader \"" << shader_name
              << "\"." << std::endl;
    return nullptr;
  }

  /// Override options.
  options.type = shader_config.type;
  options.source_language = shader_config.language;
  options.entry_point_name = EntryPointFunctionNameFromSourceName(
      shader_config.source_file_name, options.type, options.source_language,
      shader_config.entry_point);

  Reflector::Options reflector_options;
  reflector_options.target_platform = options.target_platform;
  reflector_options.entry_point_name = options.entry_point_name;
  reflector_options.shader_name = shader_name;

  Compiler compiler(source_file_mapping, options, reflector_options);
  if (!compiler.IsValid()) {
    std::cerr << "Compilation failed for bundled shader \"" << shader_name
              << "\"." << std::endl;
    std::cerr << compiler.GetErrorMessages() << std::endl;
    return nullptr;
  }

  auto reflector = compiler.GetReflector();
  if (reflector == nullptr) {
    std::cerr << "Could not create reflector for bundled shader \""
              << shader_name << "\"." << std::endl;
    return nullptr;
  }

  auto stage_data = reflector->GetRuntimeStageData();
  if (!stage_data) {
    std::cerr << "Runtime stage information was nil for bundled shader \""
              << shader_name << "\"." << std::endl;
    return nullptr;
  }

  result->shader = stage_data->CreateFlatbuffer();
  if (!result->shader) {
    std::cerr << "Failed to create flatbuffer for bundled shader \""
              << shader_name << "\"." << std::endl;
    return nullptr;
  }

  // TODO(bdero): Vertex attributes.

  return result;
}

/// Parses the given JSON shader bundle configuration and invokes the compiler
/// multiple times to produce a shader bundle file.
bool GenerateShaderBundle(Switches& switches, SourceOptions& options) {
  // --------------------------------------------------------------------------
  /// 1. Parse the bundle configuration.
  ///

  std::optional<ShaderBundleConfig> bundle_config =
      ParseShaderBundleConfig(switches.shader_bundle);
  if (!bundle_config) {
    return false;
  }

  // --------------------------------------------------------------------------
  /// 2. Build the deserialized shader bundle.
  ///

  fb::ShaderBundleT shader_bundle;

  for (const auto& [shader_name, shader_config] : bundle_config.value()) {
    std::unique_ptr<fb::ShaderT> shader =
        GenerateShaderFB(options, shader_name, shader_config);
    if (!shader) {
      return false;
    }
    shader_bundle.shaders.push_back(std::move(shader));
  }

  // --------------------------------------------------------------------------
  /// 3. Serialize the shader bundle and write to disk.
  ///

  auto builder = std::make_shared<flatbuffers::FlatBufferBuilder>();
  builder->Finish(fb::ShaderBundle::Pack(*builder.get(), &shader_bundle),
                  fb::ShaderBundleIdentifier());
  auto mapping = std::make_shared<fml::NonOwnedMapping>(
      builder->GetBufferPointer(), builder->GetSize(),
      [builder](auto, auto) {});

  auto sl_file_name = std::filesystem::absolute(
      std::filesystem::current_path() / switches.sl_file_name);

  if (!fml::WriteAtomically(*switches.working_directory,         //
                            Utf8FromPath(sl_file_name).c_str(),  //
                            *mapping                             //
                            )) {
    std::cerr << "Could not write file to " << switches.sl_file_name
              << std::endl;
    return false;
  }
  // Tools that consume the runtime stage data expect the access mode to
  // be 0644.
  if (!SetPermissiveAccess(sl_file_name)) {
    return false;
  }

  return true;
}

}  // namespace compiler
}  // namespace impeller
