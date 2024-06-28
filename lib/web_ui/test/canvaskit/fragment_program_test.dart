// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:convert';
import 'dart:typed_data';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';

import 'package:ui/src/engine.dart';
import 'package:ui/ui.dart' as ui;

import '../common/test_initialization.dart';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

/// This is generated by impellerc, and included here as a source file to
/// simplify the build.
const String kJsonIPLR = r'''
{
  "sksl": {
    "entrypoint": "ink_sparkle_fragment_main",
    "shader": "// This SkSL shader is autogenerated by spirv-cross.\n\nfloat4 flutter_FragCoord;\n\nuniform vec4 u_color;\nuniform float u_alpha;\nuniform vec4 u_sparkle_color;\nuniform float u_sparkle_alpha;\nuniform float u_blur;\nuniform vec2 u_center;\nuniform float u_radius_scale;\nuniform float u_max_radius;\nuniform vec2 u_resolution_scale;\nuniform vec2 u_noise_scale;\nuniform float u_noise_phase;\nuniform vec2 u_circle1;\nuniform vec2 u_circle2;\nuniform vec2 u_circle3;\nuniform vec2 u_rotation1;\nuniform vec2 u_rotation2;\nuniform vec2 u_rotation3;\n\nvec4 fragColor;\n\nmat2 FLT_flutter_local_rotate2d(vec2 rad)\n{\n    return mat2(vec2(rad.x, -rad.y), vec2(rad.y, rad.x));\n}\n\nfloat FLT_flutter_local_soft_circle(vec2 uv, vec2 xy, float radius, float blur)\n{\n    float blur_half = blur * 0.5;\n    float d = distance(uv, xy);\n    return 1.0 - smoothstep(1.0 - blur_half, 1.0 + blur_half, d / radius);\n}\n\nfloat FLT_flutter_local_circle_grid(vec2 resolution, inout vec2 p, vec2 xy, vec2 rotation, float cell_diameter)\n{\n    vec2 param = rotation;\n    p = (FLT_flutter_local_rotate2d(param) * (xy - p)) + xy;\n    p = mod(p, vec2(cell_diameter)) / resolution;\n    float cell_uv = (cell_diameter / resolution.y) * 0.5;\n    float r = 0.64999997615814208984375 * cell_uv;\n    vec2 param_1 = p;\n    vec2 param_2 = vec2(cell_uv);\n    float param_3 = r;\n    float param_4 = r * 50.0;\n    return FLT_flutter_local_soft_circle(param_1, param_2, param_3, param_4);\n}\n\nfloat FLT_flutter_local_turbulence(vec2 uv)\n{\n    vec2 uv_scale = uv * vec2(0.800000011920928955078125);\n    vec2 param = vec2(0.800000011920928955078125);\n    vec2 param_1 = uv_scale;\n    vec2 param_2 = u_circle1;\n    vec2 param_3 = u_rotation1;\n    float param_4 = 0.17000000178813934326171875;\n    float _290 = FLT_flutter_local_circle_grid(param, param_1, param_2, param_3, param_4);\n    float g1 = _290;\n    vec2 param_5 = vec2(0.800000011920928955078125);\n    vec2 param_6 = uv_scale;\n    vec2 param_7 = u_circle2;\n    vec2 param_8 = u_rotation2;\n    float param_9 = 0.20000000298023223876953125;\n    float _302 = FLT_flutter_local_circle_grid(param_5, param_6, param_7, param_8, param_9);\n    float g2 = _302;\n    vec2 param_10 = vec2(0.800000011920928955078125);\n    vec2 param_11 = uv_scale;\n    vec2 param_12 = u_circle3;\n    vec2 param_13 = u_rotation3;\n    float param_14 = 0.2750000059604644775390625;\n    float _315 = FLT_flutter_local_circle_grid(param_10, param_11, param_12, param_13, param_14);\n    float g3 = _315;\n    float v = (((g1 * g1) + g2) - g3) * 0.5;\n    return clamp(0.449999988079071044921875 + (0.800000011920928955078125 * v), 0.0, 1.0);\n}\n\nfloat FLT_flutter_local_soft_ring(vec2 uv, vec2 xy, float radius, float thickness, float blur)\n{\n    vec2 param = uv;\n    vec2 param_1 = xy;\n    float param_2 = radius + thickness;\n    float param_3 = blur;\n    float circle_outer = FLT_flutter_local_soft_circle(param, param_1, param_2, param_3);\n    vec2 param_4 = uv;\n    vec2 param_5 = xy;\n    float param_6 = max(radius - thickness, 0.0);\n    float param_7 = blur;\n    float circle_inner = FLT_flutter_local_soft_circle(param_4, param_5, param_6, param_7);\n    return clamp(circle_outer - circle_inner, 0.0, 1.0);\n}\n\nfloat FLT_flutter_local_triangle_noise(inout vec2 n)\n{\n    n = fract(n * vec2(5.398700237274169921875, 5.442100048065185546875));\n    n += vec2(dot(n.yx, n + vec2(21.5351009368896484375, 14.3136997222900390625)));\n    float xy = n.x * n.y;\n    return (fract(xy * 95.43070220947265625) + fract(xy * 75.0496063232421875)) - 1.0;\n}\n\nfloat FLT_flutter_local_threshold(float v, float l, float h)\n{\n    return step(l, v) * (1.0 - step(h, v));\n}\n\nfloat FLT_flutter_local_sparkle(vec2 uv, float t)\n{\n    vec2 param = uv;\n    float _213 = FLT_flutter_local_triangle_noise(param);\n    float n = _213;\n    float param_1 = n;\n    float param_2 = 0.0;\n    float param_3 = 0.0500000007450580596923828125;\n    float s = FLT_flutter_local_threshold(param_1, param_2, param_3);\n    float param_4 = n + sin(3.1415927410125732421875 * (t + 0.3499999940395355224609375));\n    float param_5 = 0.100000001490116119384765625;\n    float param_6 = 0.1500000059604644775390625;\n    s += FLT_flutter_local_threshold(param_4, param_5, param_6);\n    float param_7 = n + sin(3.1415927410125732421875 * (t + 0.699999988079071044921875));\n    float param_8 = 0.20000000298023223876953125;\n    float param_9 = 0.25;\n    s += FLT_flutter_local_threshold(param_7, param_8, param_9);\n    float param_10 = n + sin(3.1415927410125732421875 * (t + 1.0499999523162841796875));\n    float param_11 = 0.300000011920928955078125;\n    float param_12 = 0.3499999940395355224609375;\n    s += FLT_flutter_local_threshold(param_10, param_11, param_12);\n    return clamp(s, 0.0, 1.0) * 0.550000011920928955078125;\n}\n\nvoid FLT_main()\n{\n    vec2 p = flutter_FragCoord.xy;\n    vec2 uv_1 = p * u_resolution_scale;\n    vec2 density_uv = uv_1 - mod(p, u_noise_scale);\n    float radius = u_max_radius * u_radius_scale;\n    vec2 param_13 = uv_1;\n    float turbulence = FLT_flutter_local_turbulence(param_13);\n    vec2 param_14 = p;\n    vec2 param_15 = u_center;\n    float param_16 = radius;\n    float param_17 = 0.0500000007450580596923828125 * u_max_radius;\n    float param_18 = u_blur;\n    float ring = FLT_flutter_local_soft_ring(param_14, param_15, param_16, param_17, param_18);\n    vec2 param_19 = density_uv;\n    float param_20 = u_noise_phase;\n    float sparkle = ((FLT_flutter_local_sparkle(param_19, param_20) * ring) * turbulence) * u_sparkle_alpha;\n    vec2 param_21 = p;\n    vec2 param_22 = u_center;\n    float param_23 = radius;\n    float param_24 = u_blur;\n    float wave_alpha = (FLT_flutter_local_soft_circle(param_21, param_22, param_23, param_24) * u_alpha) * u_color.w;\n    vec4 wave_color = vec4(u_color.xyz * wave_alpha, wave_alpha);\n    vec4 sparkle_color = vec4(u_sparkle_color.xyz * u_sparkle_color.w, u_sparkle_color.w);\n    fragColor = mix(wave_color, sparkle_color, vec4(sparkle));\n}\n\nhalf4 main(float2 iFragCoord)\n{\n      flutter_FragCoord = float4(iFragCoord, 0, 0);\n      FLT_main();\n      return fragColor;\n}\n",
    "stage": 1,
    "uniforms": [
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 11,
        "name": "u_circle1",
        "rows": 2,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 14,
        "name": "u_rotation1",
        "rows": 2,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 12,
        "name": "u_circle2",
        "rows": 2,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 15,
        "name": "u_rotation2",
        "rows": 2,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 13,
        "name": "u_circle3",
        "rows": 2,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 16,
        "name": "u_rotation3",
        "rows": 2,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 8,
        "name": "u_resolution_scale",
        "rows": 2,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 9,
        "name": "u_noise_scale",
        "rows": 2,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 7,
        "name": "u_max_radius",
        "rows": 1,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 6,
        "name": "u_radius_scale",
        "rows": 1,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 5,
        "name": "u_center",
        "rows": 2,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 4,
        "name": "u_blur",
        "rows": 1,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 10,
        "name": "u_noise_phase",
        "rows": 1,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 3,
        "name": "u_sparkle_alpha",
        "rows": 1,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 1,
        "name": "u_alpha",
        "rows": 1,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 0,
        "name": "u_color",
        "rows": 4,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 2,
        "name": "u_sparkle_color",
        "rows": 4,
        "type": 10
      }
    ]
  }
}
''';

/// Generated by impellerc from
/// ```cpp
/// #include <flutter/runtime_effect.glsl>
/// uniform vec2 uSize;
/// uniform float[10] uFloats;
/// uniform float uLoneFloat;
/// uniform vec2[3] uVectors;
/// uniform vec3 uLoneVector;
/// uniform mat4[2] uMatrices;
/// uniform mat4 uLoneMatrix;
/// ...
/// ```
const String kJsonArrayIPLR = r'''
{
  "sksl": {
    "entrypoint": "array_shader_fragment_main",
    "shader": "// This SkSL shader is autogenerated by spirv-cross.\n\nfloat4 flutter_FragCoord;\n\nuniform vec2 uSize;\nuniform float uFloats[10];\nuniform float uLoneFloat;\nuniform vec2 uVectors[3];\nuniform vec3 uLoneVector;\nuniform mat4 uMatrices[2];\nuniform mat4 uLoneMatrix;\n\nvec4 fragColor;\n\nvec2 FLT_flutter_local_FlutterFragCoord()\n{\n    return flutter_FragCoord.xy;\n}\n\nvoid FLT_main()\n{\n    vec2 pos = FLT_flutter_local_FlutterFragCoord();\n    vec2 uv = pos / uSize;\n    float barWidth = 0.100000001490116119384765625;\n    float barIndex = floor(uv.x / barWidth);\n    for (int i = 0; i < 10; i++)\n    {\n        if (i == int(barIndex))\n        {\n            float barHeight = uFloats[i];\n            if (uv.y > (1.0 - barHeight))\n            {\n                fragColor = vec4(0.3300000131130218505859375, 0.0900000035762786865234375, 0.0900000035762786865234375, 1.0);\n            }\n            else\n            {\n                fragColor = vec4(1.0);\n            }\n        }\n    }\n}\n\nhalf4 main(float2 iFragCoord)\n{\n      flutter_FragCoord = float4(iFragCoord, 0, 0);\n      FLT_main();\n      return fragColor;\n}\n",
    "stage": 1,
    "uniforms": [
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 0,
        "name": "uSize",
        "rows": 2,
        "type": 10
      },
      {
        "array_elements": 10,
        "bit_width": 32,
        "columns": 1,
        "location": 1,
        "name": "uFloats",
        "rows": 1,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 11,
        "name": "uLoneFloat",
        "rows": 1,
        "type": 10
      },
      {
        "array_elements": 3,
        "bit_width": 32,
        "columns": 1,
        "location": 12,
        "name": "uVectors",
        "rows": 2,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 1,
        "location": 15,
        "name": "uLoneVector",
        "rows": 3,
        "type": 10
      },
      {
        "array_elements": 2,
        "bit_width": 32,
        "columns": 4,
        "location": 16,
        "name": "uMatrices",
        "rows": 4,
        "type": 10
      },
      {
        "array_elements": 0,
        "bit_width": 32,
        "columns": 4,
        "location": 18,
        "name": "uLoneMatrix",
        "rows": 4,
        "type": 10
      }
    ]
  }
}
''';

void testMain() {
  setUpAll(() async {
    await bootstrapAndRunApp();
  });

  test('FragmentProgram can be created from JSON IPLR bundle', () {
    final Uint8List data = utf8.encode(kJsonIPLR);
    final CkFragmentProgram program = CkFragmentProgram.fromBytes('test', data);

    expect(program.effect, isNotNull);
    expect(program.floatCount, 32);
    expect(program.textureCount, 0);
    expect(program.uniforms, hasLength(17));
    expect(program.name, 'test');

    {
      final CkFragmentShader shader =
          program.fragmentShader() as CkFragmentShader;

      shader.setFloat(0, 4);
      expect(
        reason: 'SkShaders are created lazily',
        shader.ref,
        isNull,
      );

      final SkShader skShader = shader.getSkShader(ui.FilterQuality.none);
      final UniqueRef<SkShader> ref = shader.ref!;
      expect(skShader, same(ref.nativeObject));
      expect(ref.isDisposed, false);

      shader.dispose();
      expect(ref.isDisposed, true);
      expect(shader.ref, isNull);
      expect(shader.debugDisposed, true);
    }

    {
      final CkFragmentShader shader =
          program.fragmentShader() as CkFragmentShader;
      shader.setFloat(0, 5);

      final SkShader skShader1 = shader.getSkShader(ui.FilterQuality.none);
      final UniqueRef<SkShader> ref1 = shader.ref!;

      final SkShader skShader2 = shader.getSkShader(ui.FilterQuality.none);
      final UniqueRef<SkShader> ref2 = shader.ref!;
      expect(ref1, isNot(same(ref2)));
      expect(
        reason:
            'getSkShader creates a new shader every time. Old references should be disposed of.',
        ref1.isDisposed,
        true,
      );
      expect(ref2.isDisposed, false);

      expect(
        reason:
            'Fragment shaders are mutable, so a new instance is created with every application.',
        skShader1,
        isNot(same(skShader2)),
      );

      shader.dispose();
      expect(shader.debugDisposed, true);
      expect(
        reason:
            'The last remaining SkShader reference should be disposed of when the FragmentShader itself is disposed of.',
        ref2.isDisposed,
        true,
      );
    }
  });

  test(
      'FragmentProgram can be created from JSON IPLR bundle with arrays and matrices',
      () {
    final Uint8List data = utf8.encode(kJsonArrayIPLR);
    final CkFragmentProgram program = CkFragmentProgram.fromBytes('test', data);

    expect(program.effect, isNotNull);
    expect(program.floatCount, 70,
        reason: 'Columns, rows and array elements should be accounted for');
    expect(program.textureCount, 0);
    expect(program.uniforms, hasLength(7));
    expect(program.name, 'test');
  });
}
