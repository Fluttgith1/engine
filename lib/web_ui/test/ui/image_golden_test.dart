// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:js_interop';
import 'dart:math';
import 'dart:typed_data';

import 'package:test/bootstrap/browser.dart';
import 'package:test/test.dart';
import 'package:ui/src/engine.dart';

import 'package:ui/ui.dart' as ui;
import 'package:web_engine_tester/golden_tester.dart';

import '../common/fake_asset_manager.dart';
import '../common/test_initialization.dart';
import 'utils.dart';

const String kGlitchShaderSksl = r'''
{"sksl": {
  "shader": "// This SkSL shader is autogenerated by spirv-cross.\n\nfloat4 flutter_FragCoord;\n\nuniform vec2 uResolution;\nuniform float uTime;\nuniform shader uTex;\nuniform half2 uTex_size;\n\nvec4 oColor;\n\nvec2 FLT_flutter_local_FlutterFragCoord()\n{\n    return flutter_FragCoord.xy;\n}\n\nfloat FLT_flutter_local_cubicPulse(float c, float w, inout float x)\n{\n    x = abs(x - c);\n    if (x > w)\n    {\n        return 0.0;\n    }\n    x /= w;\n    return 1.0 - ((x * x) * (3.0 - (2.0 * x)));\n}\n\nfloat FLT_flutter_local_twoSin(inout float x)\n{\n    x = (6.4899997711181640625 * x) - 0.64999997615814208984375;\n    float t = ((-0.699999988079071044921875) * sin(6.80000019073486328125 * x)) + (1.39999997615814208984375 * sin(2.900000095367431640625 * x));\n    t = (t / 4.099999904632568359375) + 0.5;\n    return t;\n}\n\nfloat FLT_flutter_local_hash_1d(float v)\n{\n    float u = 50.0 * sin(v * 3000.0);\n    return (2.0 * fract((2.0 * u) * u)) - 1.0;\n}\n\nvoid FLT_main()\n{\n    vec2 uv = vec2(FLT_flutter_local_FlutterFragCoord()) / uResolution;\n    float param = 0.5;\n    float param_1 = 0.0500000007450580596923828125;\n    float param_2 = fract(uTime / 4.0);\n    float _127 = FLT_flutter_local_cubicPulse(param, param_1, param_2);\n    float t_2 = _127;\n    float param_3 = fract(uTime / 5.0);\n    float _134 = FLT_flutter_local_twoSin(param_3);\n    float t_1 = _134;\n    float glitchScale = mix(0.0, 8.0, t_1 + t_2);\n    float aberrationSize = mix(0.0, 5.0, t_1 + t_2);\n    float param_4 = uv.y;\n    float h = FLT_flutter_local_hash_1d(param_4);\n    float hs = sign(h);\n    h = max(h, 0.0);\n    h *= h;\n    h = floor(h + float(0.5)) * hs;\n    uv += (vec2(h * glitchScale, 0.0) / uResolution);\n    vec2 redOffset = vec2(aberrationSize, 0.0) / uResolution;\n    vec2 greenOffset = vec2(0.0) / uResolution;\n    vec2 blueOffset = vec2(-aberrationSize, 0.0) / uResolution;\n    vec2 redUv = uv + redOffset;\n    vec2 greenUv = uv + greenOffset;\n    vec2 blueUv = uv + blueOffset;\n    vec2 ra = uTex.eval(uTex_size *  redUv).xw;\n    vec2 ga = uTex.eval(uTex_size *  greenUv).yw;\n    vec2 ba = uTex.eval(uTex_size *  blueUv).zw;\n    ra.x /= ra.y;\n    ga.x /= ga.y;\n    ba.x /= ba.y;\n    float alpha = max(ra.y, max(ga.y, ba.y));\n    oColor = vec4(ra.x, ga.x, ba.x, 1.0) * alpha;\n}\n\nhalf4 main(float2 iFragCoord)\n{\n      flutter_FragCoord = float4(iFragCoord, 0, 0);\n      FLT_main();\n      return oColor;\n}\n",
  "entrypoint": "main",
  "stage": 1,
  "uniforms": [
    {
      "array_elements": 0,
      "bit_width": 32,
      "columns": 1,
      "location": 0,
      "name": "uResolution",
      "rows": 2,
      "type": 10
    },
    {
      "array_elements": 0,
      "bit_width": 32,
      "columns": 1,
      "location": 1,
      "name": "uTime",
      "rows": 1,
      "type": 10
    },
    {
      "array_elements": 0,
      "bit_width": 0,
      "columns": 1,
      "location": 2,
      "name": "uTex",
      "rows": 1,
      "type": 12
    }
  ]
}}
''';

void main() {
  internalBootstrapBrowserTest(() => testMain);
}

Future<void> testMain() async {
  setUpUnitTests(
    withImplicitView: true,
    setUpTestViewDimensions: false,
  );

  late FakeAssetScope assetScope;
  setUp(() {
    assetScope = fakeAssetManager.pushAssetScope();
    assetScope.setAsset(
      'glitch_shader',
      ByteData.sublistView(utf8.encode(kGlitchShaderSksl))
    );
  });

  tearDown(() {
    fakeAssetManager.popAssetScope(assetScope);
  });

  const ui.Rect drawRegion = ui.Rect.fromLTWH(0, 0, 300, 300);
  const ui.Rect imageRegion = ui.Rect.fromLTWH(0, 0, 150, 150);

  // Emits a set of rendering tests for an image
  // `imageGenerator` should produce an image that is 150x150 pixels.
  void emitImageTests(String name, Future<ui.Image> Function() imageGenerator) {
    group(name, () {
      final List<ui.Image> images = <ui.Image>[];

      Future<ui.Image> generateImage() async {
        final ui.Image image = await imageGenerator();
        images.add(image);
        return image;
      }

      tearDown(() {
        for (final ui.Image image in images) {
          image.dispose();
        }
        images.clear();
      });

      test('drawImage', () async {
        final ui.Image image = await generateImage();

        final ui.PictureRecorder recorder = ui.PictureRecorder();
        final ui.Canvas canvas = ui.Canvas(recorder, drawRegion);
        canvas.drawImage(image, ui.Offset.zero, ui.Paint()..filterQuality = ui.FilterQuality.none);
        canvas.drawImage(image, const ui.Offset(150, 0), ui.Paint()..filterQuality = ui.FilterQuality.low);
        canvas.drawImage(image, const ui.Offset(0, 150), ui.Paint()..filterQuality = ui.FilterQuality.medium);
        canvas.drawImage(image, const ui.Offset(150, 150), ui.Paint()..filterQuality = ui.FilterQuality.high);

        await drawPictureUsingCurrentRenderer(recorder.endRecording());

        await matchGoldenFile('${name}_canvas_drawImage.png', region: drawRegion);
      });

      test('drawImageRect', () async {
        final ui.Image image = await generateImage();

        final ui.PictureRecorder recorder = ui.PictureRecorder();
        final ui.Canvas canvas = ui.Canvas(recorder, drawRegion);
        const ui.Rect srcRect = ui.Rect.fromLTRB(50, 50, 100, 100);
        canvas.drawImageRect(
          image,
          srcRect,
          const ui.Rect.fromLTRB(0, 0, 150, 150),
          ui.Paint()..filterQuality = ui.FilterQuality.none
        );
        canvas.drawImageRect(
          image,
          srcRect,
          const ui.Rect.fromLTRB(150, 0, 300, 150),
          ui.Paint()..filterQuality = ui.FilterQuality.low
        );
        canvas.drawImageRect(
          image,
          srcRect,
          const ui.Rect.fromLTRB(0, 150, 150, 300),
          ui.Paint()..filterQuality = ui.FilterQuality.medium
        );
        canvas.drawImageRect(
          image,
          srcRect,
          const ui.Rect.fromLTRB(150, 150, 300, 300),
          ui.Paint()..filterQuality = ui.FilterQuality.high
        );

        await drawPictureUsingCurrentRenderer(recorder.endRecording());

        await matchGoldenFile('${name}_canvas_drawImageRect.png', region: drawRegion);
      });

      test('drawImageNine', () async {
        final ui.Image image = await generateImage();

        final ui.PictureRecorder recorder = ui.PictureRecorder();
        final ui.Canvas canvas = ui.Canvas(recorder, drawRegion);
        canvas.drawImageNine(
          image,
          const ui.Rect.fromLTRB(50, 50, 100, 100),
          drawRegion,
          ui.Paint()
        );

        await drawPictureUsingCurrentRenderer(recorder.endRecording());

        await matchGoldenFile('${name}_canvas_drawImageNine.png', region: drawRegion);
      });

      test('image_shader_cubic_rotated', () async {
        final ui.PictureRecorder recorder = ui.PictureRecorder();
        final ui.Canvas canvas = ui.Canvas(recorder, drawRegion);
        final Float64List matrix = Matrix4.rotationZ(pi / 6).toFloat64();
        Future<void> drawOvalWithShader(ui.Rect rect, ui.FilterQuality quality) async {
          final ui.Image image = await generateImage();
          final ui.ImageShader shader = ui.ImageShader(
            image,
            ui.TileMode.repeated,
            ui.TileMode.repeated,
            matrix,
            filterQuality: quality,
          );
          canvas.drawOval(
            rect,
            ui.Paint()..shader = shader
          );
        }

        // Draw image shader with all four qualities.
        await drawOvalWithShader(const ui.Rect.fromLTRB(0, 0, 150, 100), ui.FilterQuality.none);
        await drawOvalWithShader(const ui.Rect.fromLTRB(150, 0, 300, 100), ui.FilterQuality.low);

        // Note that for images that skia handles lazily (ones created via
        // `createImageFromImageBitmap` or `instantiateImageCodecFromUrl`)
        // there is a skia bug that this just renders a black oval instead of
        // actually texturing it with the image.
        // See https://g-issues.skia.org/issues/338095525
        await drawOvalWithShader(const ui.Rect.fromLTRB(0, 100, 150, 200), ui.FilterQuality.medium);
        await drawOvalWithShader(const ui.Rect.fromLTRB(150, 100, 300, 200), ui.FilterQuality.high);

        await drawPictureUsingCurrentRenderer(recorder.endRecording());
        await matchGoldenFile('${name}_image_shader_cubic_rotated.png', region: drawRegion);
      });

      test('fragment_shader_sampler', () async {
        final ui.Image image = await generateImage();

        final ui.FragmentProgram program = await renderer.createFragmentProgram('glitch_shader');
        final ui.FragmentShader shader = program.fragmentShader();

        // Resolution
        shader.setFloat(0, 300);
        shader.setFloat(1, 300);

        // Time
        shader.setFloat(2, 2);

        // Image
        shader.setImageSampler(0, image);

        final ui.PictureRecorder recorder = ui.PictureRecorder();
        final ui.Canvas canvas = ui.Canvas(recorder, drawRegion);
        canvas.drawCircle(const ui.Offset(150, 150), 100, ui.Paint()..shader = shader);

        await drawPictureUsingCurrentRenderer(recorder.endRecording());

        await matchGoldenFile('${name}_fragment_shader_sampler.png', region: drawRegion);
      }, skip: isHtml); // HTML doesn't support fragment shaders

      test('drawVertices with image shader', () async {
        final ui.Image image = await generateImage();

        final Float64List matrix = Matrix4.rotationZ(pi / 6).toFloat64();
        final ui.ImageShader shader = ui.ImageShader(
          image,
          ui.TileMode.decal,
          ui.TileMode.decal,
          matrix,
          filterQuality: ui.FilterQuality.medium,
        );

        // Draw an octagon
        const List<ui.Offset> vertexValues = <ui.Offset>[
          ui.Offset(50, 0),
          ui.Offset(100, 0),
          ui.Offset(150, 50),
          ui.Offset(150, 100),
          ui.Offset(100, 150),
          ui.Offset(50, 150),
          ui.Offset(0, 100),
          ui.Offset(0, 50),
        ];
        final ui.Vertices vertices = ui.Vertices(
          ui.VertexMode.triangles,
          vertexValues,
          textureCoordinates: vertexValues,
          indices: <int>[
            0, 1, 2, //
            0, 2, 3, //
            0, 3, 4, //
            0, 4, 5, //
            0, 5, 6, //
            0, 6, 7, //
          ],
        );

        final ui.PictureRecorder recorder = ui.PictureRecorder();
        final ui.Canvas canvas = ui.Canvas(recorder, drawRegion);
        canvas.drawVertices(vertices, ui.BlendMode.srcOver, ui.Paint()..shader = shader);

        await drawPictureUsingCurrentRenderer(recorder.endRecording());

        await matchGoldenFile('${name}_drawVertices_imageShader.png', region: drawRegion);
      });

      test('toByteData_rgba', () async {
        final ui.Image image = await generateImage();

        final ByteData? rgbaData = await image.toByteData();
        expect(rgbaData, isNotNull);
        expect(rgbaData!.lengthInBytes, isNonZero);
      });

      test('toByteData_png', () async {
        final ui.Image image = await generateImage();

        final ByteData? pngData = await image.toByteData(format: ui.ImageByteFormat.png);
        expect(pngData, isNotNull);
        expect(pngData!.lengthInBytes, isNonZero);
      }, skip: isHtml); // https://github.com/flutter/flutter/issues/126611
    });
  }

  emitImageTests('picture_toImage', () {
    final ui.PictureRecorder recorder = ui.PictureRecorder();
    final ui.Canvas canvas = ui.Canvas(recorder, imageRegion);
    for (int y = 0; y < 15; y++) {
      for (int x = 0; x < 15; x++) {
        final ui.Offset center = ui.Offset(x * 10 + 5, y * 10 + 5);
        final ui.Color color = ui.Color.fromRGBO(
          (center.dx * 256 / 150).round(),
          (center.dy * 256 / 150).round(), 0, 1);
        canvas.drawCircle(center, 5, ui.Paint()..color = color);
      }
    }
    return recorder.endRecording().toImage(150, 150);
  });

  Uint8List generatePixelData(
    int width,
    int height,
    ui.Color Function(double, double) generator
  ) {
    final Uint8List data = Uint8List(width * height * 4);
    int outputIndex = 0;
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        final ui.Color pixelColor = generator(
          (2.0 * x / width) - 1.0,
          (2.0 * y / height) - 1.0,
        );
        data[outputIndex++] = pixelColor.red;
        data[outputIndex++] = pixelColor.green;
        data[outputIndex++] = pixelColor.blue;
        data[outputIndex++] = pixelColor.alpha;
      }
    }
    return data;
  }

  emitImageTests('decodeImageFromPixels_unscaled', () {
    final Uint8List pixels = generatePixelData(150, 150, (double x, double y) {
      final double r = sqrt(x * x + y * y);
      final double theta = atan2(x, y);
      return ui.Color.fromRGBO(
        (255 * (sin(r * 10.0) + 1.0) / 2.0).round(),
        (255 * (sin(theta * 10.0) + 1.0) / 2.0).round(),
        0,
        1,
      );
    });
    final Completer<ui.Image> completer = Completer<ui.Image>();
    ui.decodeImageFromPixels(pixels, 150, 150, ui.PixelFormat.rgba8888, completer.complete);
    return completer.future;
  });

  // https://github.com/flutter/flutter/issues/126603
  if (!isHtml) {
    emitImageTests('decodeImageFromPixels_scaled', () {
      final Uint8List pixels = generatePixelData(50, 50, (double x, double y) {
        final double r = sqrt(x * x + y * y);
        final double theta = atan2(x, y);
        return ui.Color.fromRGBO(
          (255 * (sin(r * 10.0) + 1.0) / 2.0).round(),
          (255 * (sin(theta * 10.0) + 1.0) / 2.0).round(),
          0,
          1,
        );
      });
      final Completer<ui.Image> completer = Completer<ui.Image>();
      ui.decodeImageFromPixels(
        pixels,
        50,
        50,
        ui.PixelFormat.rgba8888,
        completer.complete,
        targetWidth: 150,
        targetHeight: 150,
      );
      return completer.future;
    });
  }

  emitImageTests('codec_uri', () async {
    final ui.Codec codec = await renderer.instantiateImageCodecFromUrl(
      Uri(path: '/test_images/mandrill_128.png')
    );
    expect(codec.frameCount, 1);

    final ui.FrameInfo info = await codec.getNextFrame();
    return info.image;
  });

  // This API doesn't work in headless Firefox due to requiring WebGL
  // See https://github.com/flutter/flutter/issues/109265
  if (!isFirefox) {
    emitImageTests('svg_image_bitmap', () async {
      final DomBlob svgBlob = createDomBlob(<String>[
  '''
  <svg xmlns="http://www.w3.org/2000/svg" width="150" height="150">
    <path d="M25,75  A50,50 0 1,0 125 75 L75,25 Z" stroke="blue" stroke-width="10" fill="red"></path>
  </svg>
  '''
      ], <String, String>{'type': 'image/svg+xml'});
      final String url = domWindow.URL.createObjectURL(svgBlob);
      final DomHTMLImageElement image = createDomHTMLImageElement();
      final Completer<void> completer = Completer<void>();
      late final DomEventListener loadListener;
      loadListener = createDomEventListener((DomEvent event) {
        completer.complete();
        image.removeEventListener('load', loadListener);
      });
      image.addEventListener('load', loadListener);
      image.src = url;
      await completer.future;

      final DomImageBitmap bitmap = await createImageBitmap(image as JSObject);

      expect(bitmap.width.toDartInt, 150);
      expect(bitmap.height.toDartInt, 150);
      final ui.Image uiImage = await renderer.createImageFromImageBitmap(bitmap);

      if (isSkwasm) {
        // Skwasm transfers the bitmap to the web worker, so it should be disposed/consumed.
        expect(bitmap.width.toDartInt, 0);
        expect(bitmap.height.toDartInt, 0);
      }
      return uiImage;
    });
  }

  emitImageTests('codec_list_resized', () async {
    final ByteBuffer data = await httpFetchByteBuffer('/test_images/mandrill_128.png');
    final ui.Codec codec = await renderer.instantiateImageCodec(
      data.asUint8List(),
      targetWidth: 150,
      targetHeight: 150,
    );
    expect(codec.frameCount, 1);

    final ui.FrameInfo info = await codec.getNextFrame();
    return info.image;
  });
}
