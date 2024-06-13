// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

import 'package:ui/src/engine/vector_math.dart';
import 'package:ui/ui.dart' as ui;

import '../util.dart';
import 'canvaskit_api.dart';
import 'color_filter.dart';
import 'native_memory.dart';

typedef SkImageFilterBorrow = void Function(SkImageFilter);

/// An [ImageFilter] that can create a managed skia [SkImageFilter] object.
///
/// Concrete subclasses of this interface must provide efficient implementation
/// of [operator==], to avoid re-creating the underlying skia filters
/// whenever possible.
///
/// Currently implemented by [CkImageFilter] and [CkColorFilter].
abstract class CkManagedSkImageFilterConvertible implements ui.ImageFilter {
  void imageFilter(SkImageFilterBorrow borrow);

  Matrix4 get transform;
}

/// The CanvasKit implementation of [ui.ImageFilter].
///
/// Currently only supports `blur`, `matrix`, and ColorFilters.
abstract class CkImageFilter implements CkManagedSkImageFilterConvertible {
  factory CkImageFilter.blur(
      {required double sigmaX,
      required double sigmaY,
      required ui.TileMode tileMode}) = _CkBlurImageFilter;
  factory CkImageFilter.color({required CkColorFilter colorFilter}) =
      CkColorFilterImageFilter;
  factory CkImageFilter.matrix(
      {required Float64List matrix,
      required ui.FilterQuality filterQuality}) = _CkMatrixImageFilter;
  factory CkImageFilter.dilate(
      {required double radiusX,
      required double radiusY}) = _CkDilateImageFilter;
  factory CkImageFilter.erode(
      {required double radiusX, required double radiusY}) = _CkErodeImageFilter;
  factory CkImageFilter.compose(
      {required CkImageFilter outer,
      required CkImageFilter inner}) = _CkComposeImageFilter;

  CkImageFilter._();

  @override
  Matrix4 get transform => Matrix4.identity();
}

class CkColorFilterImageFilter extends CkImageFilter {
  CkColorFilterImageFilter({required this.colorFilter}) : super._() {
    final SkImageFilter skImageFilter = colorFilter.initRawImageFilter();
    _ref = UniqueRef<SkImageFilter>(this, skImageFilter, 'ImageFilter.color');
  }

  final CkColorFilter colorFilter;

  late final UniqueRef<SkImageFilter> _ref;

  @override
  void imageFilter(SkImageFilterBorrow borrow) {
    borrow(_ref.nativeObject);
  }

  void dispose() {
    _ref.dispose();
  }

  @override
  int get hashCode => colorFilter.hashCode;

  @override
  bool operator ==(Object other) {
    if (runtimeType != other.runtimeType) {
      return false;
    }
    return other is CkColorFilterImageFilter &&
        other.colorFilter == colorFilter;
  }

  @override
  String toString() => colorFilter.toString();
}

class _CkBlurImageFilter extends CkImageFilter {
  _CkBlurImageFilter(
      {required this.sigmaX, required this.sigmaY, required this.tileMode})
      : super._() {
    /// Return the identity matrix when both sigmaX and sigmaY are 0. Replicates
    /// effect of applying no filter
    final SkImageFilter skImageFilter;
    if (sigmaX == 0 && sigmaY == 0) {
      skImageFilter = canvasKit.ImageFilter.MakeMatrixTransform(
          toSkMatrixFromFloat32(Matrix4.identity().storage),
          toSkFilterOptions(ui.FilterQuality.none),
          null);
    } else {
      skImageFilter = canvasKit.ImageFilter.MakeBlur(
        sigmaX,
        sigmaY,
        toSkTileMode(tileMode),
        null,
      );
    }
    _ref = UniqueRef<SkImageFilter>(this, skImageFilter, 'ImageFilter.blur');
  }

  final double sigmaX;
  final double sigmaY;
  final ui.TileMode tileMode;

  late final UniqueRef<SkImageFilter> _ref;

  @override
  void imageFilter(SkImageFilterBorrow borrow) {
    borrow(_ref.nativeObject);
  }

  @override
  bool operator ==(Object other) {
    if (runtimeType != other.runtimeType) {
      return false;
    }
    return other is _CkBlurImageFilter &&
        other.sigmaX == sigmaX &&
        other.sigmaY == sigmaY &&
        other.tileMode == tileMode;
  }

  @override
  int get hashCode => Object.hash(sigmaX, sigmaY, tileMode);

  @override
  String toString() {
    return 'ImageFilter.blur($sigmaX, $sigmaY, ${tileModeString(tileMode)})';
  }
}

class _CkMatrixImageFilter extends CkImageFilter {
  _CkMatrixImageFilter(
      {required Float64List matrix, required this.filterQuality})
      : matrix = Float64List.fromList(matrix),
        _transform = Matrix4.fromFloat32List(toMatrix32(matrix)),
        super._() {
    final SkImageFilter skImageFilter =
        canvasKit.ImageFilter.MakeMatrixTransform(
      toSkMatrixFromFloat64(matrix),
      toSkFilterOptions(filterQuality),
      null,
    );
    _ref = UniqueRef<SkImageFilter>(this, skImageFilter, 'ImageFilter.matrix');
  }

  final Float64List matrix;
  final ui.FilterQuality filterQuality;
  final Matrix4 _transform;

  late final UniqueRef<SkImageFilter> _ref;

  @override
  void imageFilter(SkImageFilterBorrow borrow) {
    borrow(_ref.nativeObject);
  }

  @override
  bool operator ==(Object other) {
    if (other.runtimeType != runtimeType) {
      return false;
    }
    return other is _CkMatrixImageFilter &&
        other.filterQuality == filterQuality &&
        listEquals<double>(other.matrix, matrix);
  }

  @override
  int get hashCode => Object.hash(filterQuality, Object.hashAll(matrix));

  @override
  String toString() => 'ImageFilter.matrix($matrix, $filterQuality)';

  @override
  Matrix4 get transform => _transform;
}

class _CkDilateImageFilter extends CkImageFilter {
  _CkDilateImageFilter({required this.radiusX, required this.radiusY})
      : super._() {
    final SkImageFilter skImageFilter = canvasKit.ImageFilter.MakeDilate(
      radiusX,
      radiusY,
      null,
    );
    _ref = UniqueRef<SkImageFilter>(this, skImageFilter, 'ImageFilter.dilate');
  }

  final double radiusX;
  final double radiusY;

  late final UniqueRef<SkImageFilter> _ref;

  @override
  void imageFilter(SkImageFilterBorrow borrow) {
    borrow(_ref.nativeObject);
  }

  @override
  bool operator ==(Object other) {
    if (runtimeType != other.runtimeType) {
      return false;
    }
    return other is _CkDilateImageFilter &&
        other.radiusX == radiusX &&
        other.radiusY == radiusY;
  }

  @override
  int get hashCode => Object.hash(radiusX, radiusY);

  @override
  String toString() {
    return 'ImageFilter.dilate($radiusX, $radiusY)';
  }
}

class _CkErodeImageFilter extends CkImageFilter {
  _CkErodeImageFilter({required this.radiusX, required this.radiusY})
      : super._() {
    final SkImageFilter skImageFilter = canvasKit.ImageFilter.MakeErode(
      radiusX,
      radiusY,
      null,
    );
    _ref = UniqueRef<SkImageFilter>(this, skImageFilter, 'ImageFilter.erode');
  }

  final double radiusX;
  final double radiusY;

  late final UniqueRef<SkImageFilter> _ref;

  @override
  void imageFilter(SkImageFilterBorrow borrow) {
    borrow(_ref.nativeObject);
  }

  @override
  bool operator ==(Object other) {
    if (runtimeType != other.runtimeType) {
      return false;
    }
    return other is _CkErodeImageFilter &&
        other.radiusX == radiusX &&
        other.radiusY == radiusY;
  }

  @override
  int get hashCode => Object.hash(radiusX, radiusY);

  @override
  String toString() {
    return 'ImageFilter.erode($radiusX, $radiusY)';
  }
}

class _CkComposeImageFilter extends CkImageFilter {
  _CkComposeImageFilter({required this.outer, required this.inner})
      : super._() {
    outer.imageFilter((SkImageFilter outerFilter) {
      inner.imageFilter((SkImageFilter innerFilter) {
        final SkImageFilter skImageFilter = canvasKit.ImageFilter.MakeCompose(
          outerFilter,
          innerFilter,
        );
        _ref = UniqueRef<SkImageFilter>(
            this, skImageFilter, 'ImageFilter.compose');
      });
    });
  }

  final CkImageFilter outer;
  final CkImageFilter inner;

  late final UniqueRef<SkImageFilter> _ref;

  @override
  void imageFilter(SkImageFilterBorrow borrow) {
    borrow(_ref.nativeObject);
  }

  @override
  bool operator ==(Object other) {
    if (runtimeType != other.runtimeType) {
      return false;
    }
    return other is _CkComposeImageFilter &&
        other.outer == outer &&
        other.inner == inner;
  }

  @override
  int get hashCode => Object.hash(outer, inner);

  @override
  String toString() {
    return 'ImageFilter.compose($outer, $inner)';
  }
}
