# The dependencies referenced by the Flutter Engine.
#
# This file is referenced by the .gclient file at the root of the checkout.
# To preview changes to the dependencies, update this file and run
# `gclient sync`.
#
# When adding a new dependency, please update the top-level .gitignore file
# to list the dependency's destination directory.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'swiftshader_git': 'https://swiftshader.googlesource.com',
  'dart_git': 'https://dart.googlesource.com',
  'flutter_git': 'https://flutter.googlesource.com',
  'fuchsia_git': 'https://fuchsia.googlesource.com',
  'github_git': 'https://github.com',
  'skia_git': 'https://skia.googlesource.com',
  'llvm_git': 'https://llvm.googlesource.com',
  # OCMock is for testing only so there is no google clone
  'ocmock_git': 'https://github.com/erikdoe/ocmock.git',
  'skia_revision': '936433124f938c06d5b1609d534cd9b693edd71c',

  # WARNING: DO NOT EDIT canvaskit_cipd_instance MANUALLY
  # See `lib/web_ui/README.md` for how to roll CanvasKit to a new version.
  'canvaskit_cipd_instance': 'yrsfF-vTvu4jzBBm1o6tDl70dky-l4G29Dnj75UvRDgC',

  # Do not download the Emscripten SDK by default.
  # This prevents us from downloading the Emscripten toolchain for builds
  # which do not build for the web. This toolchain is needed to build CanvasKit
  # for the web engine.
  'download_emsdk': False,

  # For experimental features some dependencies may only be avaialable in the master/main
  # channels. This variable is being set when CI is checking out the repository.
  'release_candidate': False,


  # As Dart does, we use Fuchsia's GN and Clang toolchain. These revision
  # should be kept up to date with the revisions pulled by Dart.
  # The list of revisions for these tools comes from Fuchsia, here:
  # https://fuchsia.googlesource.com/integration/+/HEAD/toolchain
  # If there are problems with the toolchain, contact fuchsia-toolchain@.
  'clang_version': 'git_revision:bca75abc01f303512da409cf25a1d267b89b7276',

  # When updating the Dart revision, ensure that all entries that are
  # dependencies of Dart are also updated to match the entries in the
  # Dart SDK's DEPS file for that revision of Dart. The DEPS file for
  # Dart is: https://github.com/dart-lang/sdk/blob/main/DEPS.
  # You can use //tools/dart/create_updated_flutter_deps.py to produce
  # updated revision list of existing dependencies.
  'dart_revision': 'c106633d931cde83ee9b2f6e16e00646a4f5145f',

  # WARNING: DO NOT EDIT MANUALLY
  # The lines between blank lines above and below are generated by a script. See create_updated_flutter_deps.py
  'dart_boringssl_gen_rev': 'ced85ef0a00bbca77ce5a91261a5f2ae61b1e62f',
  'dart_boringssl_rev': '87f316d7748268eb56f2dc147bd593254ae93198',
  'dart_browser_launcher_rev': '981ca8847dd2b0fe022f9e742045cfb8f214d35f',
  'dart_clock_rev': '2507a228773c5e877fc9e3330080b234aad965c0',
  'dart_collection_rev': '414ffa1bc8ba18bd608bbf916d95715311d89ac1',
  'dart_devtools_rev': '95d292626da26505b02417735f77e8922783b477',
  'dart_protobuf_rev': '2d6c6037cee6c5f683e8f38e598443f9bec74b94',
  'dart_pub_rev': '9bf4289d6fd5d6872a8929d6312bbd7098f3ea9c',
  'dart_root_certificates_rev': '692f6d6488af68e0121317a9c2c9eb393eb0ee50',
  'dart_watcher_rev': 'e00c0ea769e32821d91c0880da8eb736839a6e6d',
  'dart_webdev_rev': '9c4428472b04f50748ea4871829897cff43455a3',
  'dart_webkit_inspection_protocol_rev': '57522d6b29d94903b765c757079d906555d5a171',
  'dart_yaml_edit_rev': '01589b3ce447b03aed991db49f1ec6445ad5476d',
  'dart_zlib_rev': '27c2f474b71d0d20764f86f60ef8b00da1a16cda',

  'ocmock_rev': 'c4ec0e3a7a9f56cfdbd0aa01f4f97bb4b75c5ef8', # v3.7.1

  # Download a prebuilt Dart SDK by default
  'download_dart_sdk': True,

  # Checkout Android dependencies only on platforms where we build for Android targets.
  'download_android_deps': 'host_cpu == "x64" and (host_os == "mac" or host_os == "linux")',

  # Checkout Windows dependencies only if we are building on Windows.
  'download_windows_deps' : 'host_os == "win"',

  # Checkout Linux dependencies only when building on Linux.
  'download_linux_deps': 'host_os == "linux"',

  # Downloads the fuchsia SDK as listed in fuchsia_sdk_path var. This variable
  # is currently only used for the Fuchsia LSC process and is not intended for
  # local development.
  'download_fuchsia_sdk': False,
  'fuchsia_sdk_path': '',

  # An LLVM backend needs LLVM binaries and headers. To avoid build time
  # increases we can use prebuilts. We don't want to download this on every
  # CQ/CI bot nor do we want the average Dart developer to incur that cost.
  # So by default we will not download prebuilts. This varible is needed in
  # the flutter engine to ensure that Dart gn has access to it as well.
  "checkout_llvm": False,

  # Setup Git hooks by default.
  "setup_githooks": True,
}

gclient_gn_args_file = 'src/third_party/dart/build/config/gclient_args.gni'
gclient_gn_args = [
  'checkout_llvm'
]

# Only these hosts are allowed for dependencies in this DEPS file.
# If you need to add a new host, contact chrome infrastructure team.
allowed_hosts = [
  'chromium.googlesource.com',
  'flutter.googlesource.com',
  'fuchsia.googlesource.com',
  'github.com',
  'skia.googlesource.com',
]

deps = {
  'src': 'https://github.com/flutter/buildroot.git' + '@' + '6af51ff4b86270cc61517bff3fff5c3bb11492e1',

   # Fuchsia compatibility
   #
   # The dependencies in this section should match the layout in the Fuchsia gn
   # build. Eventually, we'll manage these dependencies together with Fuchsia
   # and not have to specific specific hashes.

  'src/third_party/rapidjson':
   Var('fuchsia_git') + '/third_party/rapidjson' + '@' + 'ef3564c5c8824989393b87df25355baf35ff544b',

  'src/third_party/harfbuzz':
   Var('flutter_git') + '/third_party/harfbuzz' + '@' + 'd40d15e994ed60d32bcfc9ab87004dfb028dfbd6',

  'src/third_party/libcxx':
   Var('llvm_git') + '/libcxx' + '@' + '7524ef50093a376f334a62a7e5cebf5d238d4c99',

  'src/third_party/libcxxabi':
   Var('fuchsia_git') + '/third_party/libcxxabi' + '@' + '74d1e602c76350f0760bf6907910e4f3a4fccffe',

  'src/third_party/glfw':
   Var('fuchsia_git') + '/third_party/glfw' + '@' + '78e6a0063d27ed44c2c4805606309744f6fb29fc',

  'src/third_party/shaderc':
   Var('github_git') + '/google/shaderc.git' + '@' + '948660cccfbbc303d2590c7f44a4cee40b66fdd6',

  'src/third_party/vulkan-deps':
   Var('chromium_git') + '/vulkan-deps' + '@' + '23b710f1a0b3c44d51035c6400a554415f95d9c6',

  'src/third_party/flatbuffers':
   Var('github_git') + '/google/flatbuffers.git' + '@' + '0a80646371179f8a7a5c1f42c31ee1d44dcf6709',

  'src/third_party/icu':
   Var('chromium_git') + '/chromium/deps/icu.git' + '@' + '12de966fcbe1d1a48dba310aee63807856ffeee8',

  'src/third_party/khronos':
   Var('chromium_git') + '/chromium/src/third_party/khronos.git' + '@' + '676d544d2b8f48903b7da9fceffaa534a5613978',

   'src/third_party/gtest-parallel':
   Var('chromium_git') + '/external/github.com/google/gtest-parallel' + '@' + '38191e2733d7cbaeaef6a3f1a942ddeb38a2ad14',

  'src/third_party/benchmark':
   Var('github_git') + '/google/benchmark' + '@' + '431abd149fd76a072f821913c0340137cc755f36',

  'src/third_party/googletest':
   Var('github_git') + '/google/googletest' + '@' + '054a986a8513149e8374fc669a5fe40117ca6b41',

  'src/third_party/boringssl':
   Var('github_git') + '/dart-lang/boringssl_gen.git' + '@' + Var('dart_boringssl_gen_rev'),

  'src/third_party/yapf':
  Var('github_git') + '/google/yapf' + '@' + '212c5b5ad8e172d2d914ae454c121c89cccbcb35',

  'src/third_party/boringssl/src':
   'https://boringssl.googlesource.com/boringssl.git' + '@' + Var('dart_boringssl_rev'),

  'src/third_party/dart':
   Var('dart_git') + '/sdk.git' + '@' + Var('dart_revision'),

  # WARNING: Unused Dart dependencies in the list below till "WARNING:" marker are removed automatically - see create_updated_flutter_deps.py.

  'src/third_party/dart/third_party/devtools':
   {'packages': [{'version': 'git_revision:95d292626da26505b02417735f77e8922783b477', 'package': 'dart/third_party/flutter/devtools'}], 'dep_type': 'cipd'},

  'src/third_party/dart/third_party/pkg/args':
   Var('dart_git') + '/args.git@73e8d3b55cbedc9765f8e266f3422d8914f8e62a',

  'src/third_party/dart/third_party/pkg/async':
   Var('dart_git') + '/async.git@f3ed5f690e2ec9dbe1bfc5184705575b4f6480e5',

  'src/third_party/dart/third_party/pkg/bazel_worker':
   Var('dart_git') + '/bazel_worker.git@9710de6c9c70b1b583183db9d9721ba64e5a16fe',

  'src/third_party/dart/third_party/pkg/boolean_selector':
   Var('dart_git') + '/boolean_selector.git@1d3565e2651d16566bb556955b96ea75018cbd0c',

  'src/third_party/dart/third_party/pkg/browser_launcher':
   Var('dart_git') + '/browser_launcher.git' + '@' + Var('dart_browser_launcher_rev'),

  'src/third_party/dart/third_party/pkg/cli_util':
   Var('dart_git') + '/cli_util.git@b0adbba89442b2ea6fef39c7a82fe79cb31e1168',

  'src/third_party/dart/third_party/pkg/clock':
   Var('dart_git') + '/clock.git' + '@' + Var('dart_clock_rev'),

  'src/third_party/dart/third_party/pkg/collection':
   Var('dart_git') + '/collection.git' + '@' + Var('dart_collection_rev'),

  'src/third_party/dart/third_party/pkg/convert':
   Var('dart_git') + '/convert.git@7145da14f9cd730e80fb4c6a10108fcfd205e8e7',

  'src/third_party/dart/third_party/pkg/crypto':
   Var('dart_git') + '/crypto.git@223e0a62c0f762fd2b510f753861445b52e14fc3',

  'src/third_party/dart/third_party/pkg/csslib':
   Var('dart_git') + '/csslib.git@ba2eb2d80530eedefadaade338a09c2dd60410f3',

  'src/third_party/dart/third_party/pkg/dart_style':
   Var('dart_git') + '/dart_style.git@d7b73536a8079331c888b7da539b80e6825270ea',

  'src/third_party/dart/third_party/pkg/dartdoc':
   Var('dart_git') + '/dartdoc.git@adc5a45ddafb57e3a600ee7d9e88dc81061d1410',

  'src/third_party/dart/third_party/pkg/ffi':
   Var('dart_git') + '/ffi.git@18b2b549d55009ff594600b04705ff6161681e07',

  'src/third_party/dart/third_party/pkg/file':
   Var('dart_git') + '/external/github.com/google/file.dart@0132eeedea2933513bf230513a766a8baeab0c4f',

  'src/third_party/dart/third_party/pkg/fixnum':
   Var('dart_git') + '/fixnum.git@164712f6547cdfb2709b752188186baf31fd1730',

  'src/third_party/dart/third_party/pkg/glob':
   Var('dart_git') + '/glob.git@1d51fcc172e5adfbae6e82c3f8f119774cb2fca2',

  'src/third_party/dart/third_party/pkg/html':
   Var('dart_git') + '/html.git@8243e967caad9932c13971af3b2a7c8f028383d5',

  'src/third_party/dart/third_party/pkg/http':
   Var('dart_git') + '/http.git@843c5ecb1ea2233ba7b7049833b5801b149fba86',

  'src/third_party/dart/third_party/pkg/http_multi_server':
   Var('dart_git') + '/http_multi_server.git@20bf079c8955d1250a45afb9cb096472a724a551',

  'src/third_party/dart/third_party/pkg/http_parser':
   Var('dart_git') + '/http_parser.git@eaa63304c333316acd114e3be7ed701d7d7ba32c',

  'src/third_party/dart/third_party/pkg/json_rpc_2':
   Var('dart_git') + '/json_rpc_2.git@805e6536dd961d66f6b8cd46d8f3e61774f957c9',

  'src/third_party/dart/third_party/pkg/linter':
   Var('dart_git') + '/linter.git@1ddc70948d94f2449fec69a95e3ceb7b6b6c8348',

  'src/third_party/dart/third_party/pkg/logging':
   Var('dart_git') + '/logging.git@f6979e3bc3b6e1847a08335b7eb6304e18986195',

  'src/third_party/dart/third_party/pkg/markdown':
   Var('dart_git') + '/markdown.git@e3f4bd28c9e61b522f75f291d4d6cfcfeccd83ee',

  'src/third_party/dart/third_party/pkg/matcher':
   Var('dart_git') + '/matcher.git@1a7fcae0d7af1604781afabe61fd35d9b404d8ed',

  'src/third_party/dart/third_party/pkg/mime':
   Var('dart_git') + '/mime.git@0a75a41445eb642674a0a271eecde78cb025ee60',

  'src/third_party/dart/third_party/pkg/mockito':
   Var('dart_git') + '/mockito.git@25d25dab6b57ac710c0be0e759def7505b352ea7',

  'src/third_party/dart/third_party/pkg/oauth2':
   Var('dart_git') + '/oauth2.git@199ebf15cbd5b07958438184f32e41c4447a57bf',

  'src/third_party/dart/third_party/pkg/package_config':
   Var('dart_git') + '/package_config.git@cff98c90acc457a3b0750f0a7da0e351a35e5d0c',

  'src/third_party/dart/third_party/pkg/path':
   Var('dart_git') + '/path.git@7a0ed40280345b1c11df4c700c71e590738f4257',

  'src/third_party/dart/third_party/pkg/pool':
   Var('dart_git') + '/pool.git@fa84ddd0e39f45bf3f09dcc5d6b9fbdda7820fef',

  'src/third_party/dart/third_party/pkg/protobuf':
   Var('dart_git') + '/protobuf.git' + '@' + Var('dart_protobuf_rev'),

  'src/third_party/dart/third_party/pkg/pub':
   Var('dart_git') + '/pub.git' + '@' + Var('dart_pub_rev'),

  'src/third_party/dart/third_party/pkg/pub_semver':
   Var('dart_git') + '/pub_semver.git@5c0b4bfd5ca57fe16f1319c581dc8c882e9b8cb2',

  'src/third_party/dart/third_party/pkg/shelf':
   Var('dart_git') + '/shelf.git@8f8f3703efd241f9cf6b18e36e0067ca74c47fd8',

  'src/third_party/dart/third_party/pkg/source_map_stack_trace':
   Var('dart_git') + '/source_map_stack_trace.git@72dbf21a33293b2b8434d0a9751e36f9463981ac',

  'src/third_party/dart/third_party/pkg/source_maps':
   Var('dart_git') + '/source_maps.git@e93565b43a7b6b367789de8ffba969c4ebeeb317',

  'src/third_party/dart/third_party/pkg/source_span':
   Var('dart_git') + '/source_span.git@24151fd80e4557a626f81f2bc0d6a2ebde172cae',

  'src/third_party/dart/third_party/pkg/sse':
   Var('dart_git') + '/sse.git@2df072848a6090d3ed67f30c69e86ec4d6b96cd6',

  'src/third_party/dart/third_party/pkg/stack_trace':
   Var('dart_git') + '/stack_trace.git@17f09c2c6845bb31c7c385acecce5befb8527a13',

  'src/third_party/dart/third_party/pkg/stream_channel':
   Var('dart_git') + '/stream_channel.git@8e0d7ef1f4a3fb97fbd82e11cd539093f58511f3',

  'src/third_party/dart/third_party/pkg/string_scanner':
   Var('dart_git') + '/string_scanner.git@c637deb8d998b72a5807afbd06aba8370db725c0',

  'src/third_party/dart/third_party/pkg/term_glyph':
   Var('dart_git') + '/term_glyph.git@741efdedf9da62ee66a06c295d36fa28f8780e24',

  'src/third_party/dart/third_party/pkg/test':
   Var('dart_git') + '/test.git@fb4ccaf6c68fcc1d208c5c53a52d8e0e718bdffe',

  'src/third_party/dart/third_party/pkg/test_reflective_loader':
   Var('dart_git') + '/test_reflective_loader.git@8d0de01bbe852fea1f8e33aba907abcba50a8a1e',

  'src/third_party/dart/third_party/pkg/typed_data':
   Var('dart_git') + '/typed_data.git@bb10b64f9a56b8fb49307d4465474bf1c1309f6d',

  'src/third_party/dart/third_party/pkg/usage':
   Var('dart_git') + '/usage.git@1d3c31e780af665fb796a27898a441fcb7d263db',

  'src/third_party/dart/third_party/pkg/watcher':
   Var('dart_git') + '/watcher.git' + '@' + Var('dart_watcher_rev'),

  'src/third_party/dart/third_party/pkg/web_socket_channel':
   Var('dart_git') + '/web_socket_channel.git@99dbdc5769e19b9eeaf69449a59079153c6a8b1f',

  'src/third_party/dart/third_party/pkg/webdev':
   Var('dart_git') + '/webdev.git' + '@' + Var('dart_webdev_rev'),

  'src/third_party/dart/third_party/pkg/webkit_inspection_protocol':
   Var('dart_git') + '/external/github.com/google/webkit_inspection_protocol.dart.git' + '@' + Var('dart_webkit_inspection_protocol_rev'),

  'src/third_party/dart/third_party/pkg/yaml':
   Var('dart_git') + '/yaml.git@fda5b15692ccfa0feb7793a27fe3829b3d0f77fa',

  'src/third_party/dart/third_party/pkg/yaml_edit':
   Var('dart_git') + '/yaml_edit.git' + '@' + Var('dart_yaml_edit_rev'),

  'src/third_party/dart/tools/sdks':
   {'packages': [{'version': 'version:2.17.0', 'package': 'dart/dart-sdk/${{platform}}'}], 'dep_type': 'cipd'},

  # WARNING: end of dart dependencies list that is cleaned up automatically - see create_updated_flutter_deps.py.

  # Prebuilt Dart SDK of the same revision as the Dart SDK source checkout
  'src/flutter/prebuilts/linux-x64/dart-sdk': {
    'packages': [
      {
        'package': 'flutter/dart-sdk/linux-amd64',
        'version': 'git_revision:'+Var('dart_revision')
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "linux" and download_dart_sdk'
  },
  'src/flutter/prebuilts/linux-arm64/dart-sdk': {
    'packages': [
      {
        'package': 'flutter/dart-sdk/linux-arm64',
        'version': 'git_revision:'+Var('dart_revision')
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "linux" and download_dart_sdk'
  },
  'src/flutter/prebuilts/macos-x64/dart-sdk': {
    'packages': [
      {
        'package': 'flutter/dart-sdk/mac-amd64',
        'version': 'git_revision:'+Var('dart_revision')
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "mac" and download_dart_sdk'
  },
  'src/flutter/prebuilts/macos-arm64/dart-sdk': {
    'packages': [
      {
        'package': 'flutter/dart-sdk/mac-arm64',
        'version': 'git_revision:'+Var('dart_revision')
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "mac" and download_dart_sdk'
  },
  'src/flutter/prebuilts/windows-x64/dart-sdk': {
    'packages': [
      {
        'package': 'flutter/dart-sdk/windows-amd64',
        'version': 'git_revision:'+Var('dart_revision')
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "win" and download_dart_sdk'
  },
  'src/flutter/prebuilts/windows-arm64/dart-sdk': {
    'packages': [
      {
        'package': 'flutter/dart-sdk/windows-arm64',
        'version': 'git_revision:'+Var('dart_revision')
      }
    ],
    'dep_type': 'cipd',
    'condition': 'host_os == "win" and download_dart_sdk and not release_candidate'
  },

  'src/third_party/colorama/src':
   Var('chromium_git') + '/external/colorama.git' + '@' + '799604a1041e9b3bc5d2789ecbd7e8db2e18e6b8',

  'src/third_party/expat':
   Var('chromium_git') + '/external/github.com/libexpat/libexpat.git' + '@' + 'a28238bdeebc087071777001245df1876a11f5ee',

  'src/third_party/freetype2':
   Var('flutter_git') + '/third_party/freetype2' + '@' + '3bea2761290a1cbe7d8f75c1c5a7ad727f826a66',

  'src/third_party/root_certificates':
   Var('dart_git') + '/root_certificates.git' + '@' + Var('dart_root_certificates_rev'),

  'src/third_party/skia':
   Var('skia_git') + '/skia.git' + '@' +  Var('skia_revision'),

  'src/third_party/ocmock':
   Var('ocmock_git') + '@' +  Var('ocmock_rev'),

  'src/third_party/libjpeg-turbo':
   Var('fuchsia_git') + '/third_party/libjpeg-turbo' + '@' + '0fb821f3b2e570b2783a94ccd9a2fb1f4916ae9f',

  'src/third_party/libpng':
   Var('flutter_git') + '/third_party/libpng' + '@' + '134cf139cb24d802ee6ad5fc51bccff3221c2b49',

  'src/third_party/libwebp':
   Var('chromium_git') + '/webm/libwebp.git' + '@' + '7dfde712a477e420968732161539011e0fd446cf', # 1.2.0

  'src/third_party/wuffs':
   Var('skia_git') + '/external/github.com/google/wuffs-mirror-release-c.git' + '@' + '600cd96cf47788ee3a74b40a6028b035c9fd6a61',

  'src/third_party/fontconfig/src':
   Var('chromium_git') + '/external/fontconfig.git' + '@' + 'c336b8471877371f0190ba06f7547c54e2b890ba',

  'src/third_party/fontconfig':
   Var('flutter_git') + '/third_party/fontconfig' + '@' + '81c83d510ae3aa75589435ce32a5de05139aacb0',

  'src/third_party/libxml':
   Var('flutter_git') + '/third_party/libxml' + '@' + 'a143e452b5fc7d872813eeadc8db421694058098',

  'src/third_party/zlib':
   Var('chromium_git') + '/chromium/src/third_party/zlib.git' + '@' + Var('dart_zlib_rev'),

  'src/third_party/inja':
   Var('flutter_git') + '/third_party/inja' + '@' + '88bd6112575a80d004e551c98cf956f88ff4d445',

  'src/third_party/libtess2':
   Var('flutter_git') + '/third_party/libtess2' + '@' + 'fc52516467dfa124bdd967c15c7cf9faf02a34ca',

  'src/third_party/sqlite':
   Var('flutter_git') + '/third_party/sqlite' + '@' + '0f61bd2023ba94423b4e4c8cfb1a23de1fe6a21c',

  'src/third_party/pyyaml':
   Var('fuchsia_git') + '/third_party/pyyaml.git' + '@' + '25e97546488eee166b1abb229a27856cecd8b7ac',

   # Upstream Khronos Vulkan Headers are part of vulkan-deps
   # Downstream Fuchsia Vulkan Headers (v1.2.198)
  'src/third_party/fuchsia-vulkan':
   Var('fuchsia_git') + '/third_party/Vulkan-Headers.git' + '@' + '32640ad82ef648768c706c9bf828b77123a09bc2',

   'src/third_party/swiftshader':
   Var('swiftshader_git') + '/SwiftShader.git' + '@' + 'bea8d2471bd912220ba59032e0738f3364632657',

   'src/third_party/angle':
   Var('chromium_git') + '/angle/angle.git' + '@' + '3faaded8234b31dea24c929e40e33089a34a9aa5',

   'src/third_party/vulkan_memory_allocator':
   Var('chromium_git') + '/external/github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator' + '@' + '7de5cc00de50e71a3aab22dea52fbb7ff4efceb6',

   'src/third_party/abseil-cpp':
   Var('chromium_git') + '/chromium/src/third_party/abseil-cpp.git' + '@' + '2d8c1340f0350828f1287c4eaeebefcf317bcfc9',

   # Dart packages
  'src/third_party/pkg/archive':
  Var('github_git') + '/brendan-duncan/archive.git' + '@' + '9de7a0544457c6aba755ccb65abb41b0dc1db70d', # 3.1.2

  'src/third_party/pkg/equatable':
  Var('github_git') + '/felangel/equatable.git' + '@' + '0ba67c72db8bed75877fc1caafa74112ee0bd921', # 2.0.2

  'src/third_party/pkg/file':
  Var('github_git') + '/google/file.dart.git' + '@' + '427bb20ccc852425d67f2880da2a9b4707c266b4', # 6.1.0

  'src/third_party/pkg/flutter_packages':
  Var('github_git') + '/flutter/packages.git' + '@' + 'a19eca7fe2660c71acf5928a275deda1da318c50', # various

  'src/third_party/pkg/gcloud':
  Var('github_git') + '/dart-lang/gcloud.git' + '@' + '92a33a9d95ea94a4354b052a28b98088d660e0e7', # 0.8.0-dev

  'src/third_party/pkg/googleapis':
  Var('github_git') + '/google/googleapis.dart.git' + '@' + '07f01b7aa6985e4cafd0fd4b98724841bc9e85a1', # various

  'src/third_party/pkg/platform':
  Var('github_git') + '/google/platform.dart.git' + '@' + '1ffad63428bbd1b3ecaa15926bacfb724023648c', # 3.1.0

  'src/third_party/pkg/process':
  Var('github_git') + '/google/process.dart.git' + '@' + '0c9aeac86dcc4e3a6cf760b76fed507107e244d5', # 4.2.1

  'src/third_party/pkg/process_runner':
  Var('github_git') + '/google/process_runner.git' + '@' + 'd632ea0bfd814d779fcc53a361ed33eaf3620a0b', # 4.0.1

  'src/third_party/pkg/quiver':
  Var('github_git') + '/google/quiver-dart.git' + '@' + '66f473cca1332496e34a783ba4527b04388fd561', # 2.1.5

  'src/third_party/pkg/vector_math':
  Var('github_git') + '/google/vector_math.dart.git' + '@' + '0a5fd95449083d404df9768bc1b321b88a7d2eef', # 2.1.0

  'src/third_party/imgui':
  Var('github_git') + '/ocornut/imgui.git' + '@' + '29d462ebce0275345a6ce4621d8fff0ded57c9e5',

  'src/third_party/gradle': {
    'packages': [
      {
        'version': 'version:7.0.2',
        'package': 'flutter/gradle'
      }
    ],
    'condition': 'download_android_deps',
    'dep_type': 'cipd'
  },

  'src/third_party/android_tools/trace_to_text': {
    'packages': [
      {
        'version': 'git_tag:v20.1',
        'package': 'perfetto/trace_to_text/${{platform}}'
      }
    ],
    'condition': 'download_android_deps',
    'dep_type': 'cipd'
  },

   'src/third_party/android_tools/google-java-format': {
     'packages': [
       {
        'package': 'flutter/android/google-java-format',
        'version': 'version:1.7-1'
       }
     ],
     # We want to be able to format these as part of CI, and the CI step that
     # checks formatting runs without downloading the rest of the Android build
     # tooling. Therefore unlike all the other Android-related tools, we want to
     # download this every time.
     'dep_type': 'cipd',
   },

  'src/third_party/android_tools': {
     'packages': [
       {
        'package': 'flutter/android/sdk/all/${{platform}}',
        'version': 'version:32v1'
       }
     ],
     'condition': 'download_android_deps',
     'dep_type': 'cipd',
   },

  'src/third_party/android_embedding_dependencies': {
     'packages': [
       {
        'package': 'flutter/android/embedding_bundle',
        'version': 'last_updated:2021-11-23T12:31:07-0800'
       }
     ],
     'condition': 'download_android_deps',
     'dep_type': 'cipd',
   },

  'src/third_party/web_dependencies': {
     'packages': [
       {
         'package': 'flutter/web/canvaskit_bundle',
         'version': Var('canvaskit_cipd_instance')
       }
     ],
     'dep_type': 'cipd',
   },

  'src/third_party/java/openjdk': {
     'packages': [
       {
        'package': 'flutter/java/openjdk/${{platform}}',
        'version': 'version:11'
       }
     ],
     'condition': 'download_android_deps',
     'dep_type': 'cipd',
   },

  'src/flutter/third_party/gn': {
    'packages': [
      {
        'package': 'gn/gn/${{platform}}',
        'version': 'git_revision:b79031308cc878488202beb99883ec1f2efd9a6d'
      },
    ],
    'dep_type': 'cipd',
  },

  'src/buildtools/emsdk': {
   'url': Var('skia_git') + '/external/github.com/emscripten-core/emsdk.git' + '@' + 'fc645b7626ebf86530dbd82fbece74d457e7ae07',
   'condition': 'download_emsdk',
  },

  # Clang on mac and linux are expected to typically be the same revision.
  # They are separated out so that the autoroller can more easily manage them.
  'src/buildtools/mac-x64/clang': {
    'packages': [
      {
        'package': 'fuchsia/third_party/clang/mac-amd64',
        'version': '2UxMJ4z6houS9ZJc_5ncts3m-r-zU0wT06pSgY6R3cgC'
      }
    ],
    'condition': 'host_os == "mac"',
    'dep_type': 'cipd',
  },

  'src/buildtools/linux-x64/clang': {
    'packages': [
      {
        'package': 'fuchsia/third_party/clang/linux-amd64',
        'version': 'ugk-KfeqO9fhSfhBFRG4Z-56Kr2AQVSEbku9AEUdotYC'
      }
    ],
    'condition': 'host_os == "linux"',
    'dep_type': 'cipd',
  },

  'src/buildtools/windows-x64/clang': {
    'packages': [
      {
        'package': 'fuchsia/third_party/clang/windows-amd64',
        'version': 'ydMHECzTQcNpE43445qb_qSRkNHWRzUkf3w7GkEv3bgC'
      }
    ],
    'condition': 'download_windows_deps',
    'dep_type': 'cipd',
  },

   # Get the SDK from https://chrome-infra-packages.appspot.com/p/fuchsia/sdk/core at the 'latest' tag
   # Get the toolchain from https://chrome-infra-packages.appspot.com/p/fuchsia/clang at the 'goma' tag

   'src/fuchsia/sdk/mac': {
     'packages': [
       {
        'package': 'fuchsia/sdk/core/mac-amd64',
        'version': 'm2ohKWpxNr6OfF35m9x-tMmf3uZXkLJntKJxDBPXQRsC'
       }
     ],
     'condition': 'host_os == "mac" and not download_fuchsia_sdk',
     'dep_type': 'cipd',
   },
   'src/fuchsia/sdk/linux': {
     'packages': [
       {
        'package': 'fuchsia/sdk/core/linux-amd64',
        'version': 'bf53fjQIIvsbEVNhfmqAX7sFPpfw8ib7VIQoJIsdJJEC'
       }
     ],
     'condition': 'host_os == "linux" and not download_fuchsia_sdk',
     'dep_type': 'cipd',
   },
}

recursedeps = [
  'src/third_party/vulkan-deps',
]

hooks = [
  {
    # Generate the Dart SDK's .dart_tool/package_confg.json file.
    'name': 'Generate .dart_tool/package_confg.json',
    'pattern': '.',
    'action': ['python3', 'src/third_party/dart/tools/generate_package_config.py'],
  },
  {
    # Update the Windows toolchain if necessary.
    'name': 'win_toolchain',
    'condition': 'download_windows_deps',
    'pattern': '.',
    'action': ['python3', 'src/build/vs_toolchain.py', 'update'],
  },
  {
    # Ensure that we don't accidentally reference any .pyc files whose
    # corresponding .py files have already been deleted.
    'name': 'remove_stale_pyc_files',
    'pattern': 'src/tools/.*\\.py',
    'action': [
        'python3',
        'src/tools/remove_stale_pyc_files.py',
        'src/tools',
    ],
  },
  {
    'name': 'dia_dll',
    'pattern': '.',
    'condition': 'download_windows_deps',
    'action': [
      'python3',
      'src/flutter/tools/dia_dll.py',
    ],
  },
  {
    'name': 'linux_sysroot_x64',
    'pattern': '.',
    'condition': 'download_linux_deps',
    'action': [
      'python3',
      'src/build/linux/sysroot_scripts/install-sysroot.py',
      '--arch=x64'],
  },
  {
    'name': 'linux_sysroot_arm64',
    'pattern': '.',
    'condition': 'download_linux_deps',
    'action': [
      'python3',
      'src/build/linux/sysroot_scripts/install-sysroot.py',
      '--arch=arm64'],
  },
  {
    'name': 'pub get --offline',
    'pattern': '.',
    'action': [
      'python3',
      'src/flutter/tools/pub_get_offline.py',
    ]
  },
  {
    'name': 'Download Fuchsia SDK',
    'pattern': '.',
    'condition': 'download_fuchsia_sdk',
    'action': [
      'python3',
      'src/flutter/tools/download_fuchsia_sdk.py',
      '--fail-loudly',
      '--verbose',
      '--host-os',
      Var('host_os'),
      '--fuchsia-sdk-path',
      Var('fuchsia_sdk_path'),
    ]
  },
  {
    'name': 'Activate Emscripten SDK',
    'pattern': '.',
    'condition': 'download_emsdk',
    'action': [
      'python3',
      'src/flutter/tools/activate_emsdk.py',
    ]
  },
  {
    'name': 'Setup githooks',
    'pattern': '.',
    'condition': 'setup_githooks',
    'action': [
      'python3',
      'src/flutter/tools/githooks/setup.py',
    ]
  }
]
