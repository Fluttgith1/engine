// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_DISPLAY_LIST_DL_DISPATCHER_H_
#define FLUTTER_IMPELLER_DISPLAY_LIST_DL_DISPATCHER_H_

#include "display_list/utils/dl_receiver_utils.h"
#include "flutter/display_list/dl_op_receiver.h"
#include "impeller/aiks/canvas_type.h"
#include "impeller/aiks/experimental_canvas.h"
#include "impeller/aiks/paint.h"
#include "impeller/entity/contents/content_context.h"

namespace impeller {

class DlDispatcherBase : public flutter::DlOpReceiver {
 public:
  Picture EndRecordingAsPicture();

  // |flutter::DlOpReceiver|
  bool PrefersImpellerPaths() const override { return true; }

  // |flutter::DlOpReceiver|
  void setAntiAlias(bool aa) override;

  // |flutter::DlOpReceiver|
  void setDrawStyle(flutter::DlDrawStyle style) override;

  // |flutter::DlOpReceiver|
  void setColor(flutter::DlColor color) override;

  // |flutter::DlOpReceiver|
  void setStrokeWidth(SkScalar width) override;

  // |flutter::DlOpReceiver|
  void setStrokeMiter(SkScalar limit) override;

  // |flutter::DlOpReceiver|
  void setStrokeCap(flutter::DlStrokeCap cap) override;

  // |flutter::DlOpReceiver|
  void setStrokeJoin(flutter::DlStrokeJoin join) override;

  // |flutter::DlOpReceiver|
  void setColorSource(const flutter::DlColorSource* source) override;

  // |flutter::DlOpReceiver|
  void setColorFilter(const flutter::DlColorFilter* filter) override;

  // |flutter::DlOpReceiver|
  void setInvertColors(bool invert) override;

  // |flutter::DlOpReceiver|
  void setBlendMode(flutter::DlBlendMode mode) override;

  // |flutter::DlOpReceiver|
  void setPathEffect(const flutter::DlPathEffect* effect) override;

  // |flutter::DlOpReceiver|
  void setMaskFilter(const flutter::DlMaskFilter* filter) override;

  // |flutter::DlOpReceiver|
  void setImageFilter(const flutter::DlImageFilter* filter) override;

  // |flutter::DlOpReceiver|
  void save() override;

  // |flutter::DlOpReceiver|
  void saveLayer(const SkRect& bounds,
                 const flutter::SaveLayerOptions options,
                 const flutter::DlImageFilter* backdrop) override;

  // |flutter::DlOpReceiver|
  void restore() override;

  // |flutter::DlOpReceiver|
  void translate(SkScalar tx, SkScalar ty) override;

  // |flutter::DlOpReceiver|
  void scale(SkScalar sx, SkScalar sy) override;

  // |flutter::DlOpReceiver|
  void rotate(SkScalar degrees) override;

  // |flutter::DlOpReceiver|
  void skew(SkScalar sx, SkScalar sy) override;

  // |flutter::DlOpReceiver|
  void transform2DAffine(SkScalar mxx,
                         SkScalar mxy,
                         SkScalar mxt,
                         SkScalar myx,
                         SkScalar myy,
                         SkScalar myt) override;

  // |flutter::DlOpReceiver|
  void transformFullPerspective(SkScalar mxx,
                                SkScalar mxy,
                                SkScalar mxz,
                                SkScalar mxt,
                                SkScalar myx,
                                SkScalar myy,
                                SkScalar myz,
                                SkScalar myt,
                                SkScalar mzx,
                                SkScalar mzy,
                                SkScalar mzz,
                                SkScalar mzt,
                                SkScalar mwx,
                                SkScalar mwy,
                                SkScalar mwz,
                                SkScalar mwt) override;

  // |flutter::DlOpReceiver|
  void transformReset() override;

  // |flutter::DlOpReceiver|
  void clipRect(const SkRect& rect, ClipOp clip_op, bool is_aa) override;

  // |flutter::DlOpReceiver|
  void clipRRect(const SkRRect& rrect, ClipOp clip_op, bool is_aa) override;

  // |flutter::DlOpReceiver|
  void clipPath(const SkPath& path, ClipOp clip_op, bool is_aa) override;

  // |flutter::DlOpReceiver|
  void clipPath(const CacheablePath& cache,
                ClipOp clip_op,
                bool is_aa) override;

  // |flutter::DlOpReceiver|
  void drawColor(flutter::DlColor color, flutter::DlBlendMode mode) override;

  // |flutter::DlOpReceiver|
  void drawPaint() override;

  // |flutter::DlOpReceiver|
  void drawLine(const SkPoint& p0, const SkPoint& p1) override;

  // |flutter::DlOpReceiver|
  void drawRect(const SkRect& rect) override;

  // |flutter::DlOpReceiver|
  void drawOval(const SkRect& bounds) override;

  // |flutter::DlOpReceiver|
  void drawCircle(const SkPoint& center, SkScalar radius) override;

  // |flutter::DlOpReceiver|
  void drawRRect(const SkRRect& rrect) override;

  // |flutter::DlOpReceiver|
  void drawDRRect(const SkRRect& outer, const SkRRect& inner) override;

  // |flutter::DlOpReceiver|
  void drawPath(const SkPath& path) override;

  // |flutter::DlOpReceiver|
  void drawPath(const CacheablePath& cache) override;

  // |flutter::DlOpReceiver|
  void drawArc(const SkRect& oval_bounds,
               SkScalar start_degrees,
               SkScalar sweep_degrees,
               bool use_center) override;

  // |flutter::DlOpReceiver|
  void drawPoints(PointMode mode,
                  uint32_t count,
                  const SkPoint points[]) override;

  // |flutter::DlOpReceiver|
  void drawVertices(const flutter::DlVertices* vertices,
                    flutter::DlBlendMode dl_mode) override;

  // |flutter::DlOpReceiver|
  void drawImage(const sk_sp<flutter::DlImage> image,
                 const SkPoint point,
                 flutter::DlImageSampling sampling,
                 bool render_with_attributes) override;

  // |flutter::DlOpReceiver|
  void drawImageRect(const sk_sp<flutter::DlImage> image,
                     const SkRect& src,
                     const SkRect& dst,
                     flutter::DlImageSampling sampling,
                     bool render_with_attributes,
                     SrcRectConstraint constraint) override;

  // |flutter::DlOpReceiver|
  void drawImageNine(const sk_sp<flutter::DlImage> image,
                     const SkIRect& center,
                     const SkRect& dst,
                     flutter::DlFilterMode filter,
                     bool render_with_attributes) override;

  // |flutter::DlOpReceiver|
  void drawAtlas(const sk_sp<flutter::DlImage> atlas,
                 const SkRSXform xform[],
                 const SkRect tex[],
                 const flutter::DlColor colors[],
                 int count,
                 flutter::DlBlendMode mode,
                 flutter::DlImageSampling sampling,
                 const SkRect* cull_rect,
                 bool render_with_attributes) override;

  // |flutter::DlOpReceiver|
  void drawDisplayList(const sk_sp<flutter::DisplayList> display_list,
                       SkScalar opacity) override;

  // |flutter::DlOpReceiver|
  void drawTextBlob(const sk_sp<SkTextBlob> blob,
                    SkScalar x,
                    SkScalar y) override;

  // |flutter::DlOpReceiver|
  void drawTextFrame(const std::shared_ptr<impeller::TextFrame>& text_frame,
                     SkScalar x,
                     SkScalar y) override;

  // |flutter::DlOpReceiver|
  void drawShadow(const SkPath& path,
                  const flutter::DlColor color,
                  const SkScalar elevation,
                  bool transparent_occluder,
                  SkScalar dpr) override;

  // |flutter::DlOpReceiver|
  void drawShadow(const CacheablePath& cache,
                  const flutter::DlColor color,
                  const SkScalar elevation,
                  bool transparent_occluder,
                  SkScalar dpr) override;

  virtual Canvas& GetCanvas() = 0;

 private:
  Paint paint_;
  Matrix initial_matrix_;

  static const Path& GetOrCachePath(const CacheablePath& cache);

  static void SimplifyOrDrawPath(Canvas& canvas,
                                 const CacheablePath& cache,
                                 const Paint& paint);
};

class DlDispatcher : public DlDispatcherBase {
 public:
  DlDispatcher();

  explicit DlDispatcher(IRect cull_rect);

  explicit DlDispatcher(Rect cull_rect);

  ~DlDispatcher() = default;

 private:
  Canvas canvas_;

  Canvas& GetCanvas() override;
};

class ExperimentalDlDispatcher : public DlDispatcherBase {
 public:
  ExperimentalDlDispatcher(ContentContext& renderer,
                           RenderTarget& render_target,
                           IRect cull_rect);

  ~ExperimentalDlDispatcher() = default;

  void FinishRecording() { canvas_.EndReplay(); }

 private:
  ExperimentalCanvas canvas_;

  Canvas& GetCanvas() override;
};

/// Performs a first pass over the display list to collect all text frames.
class TextFrameDispatcher : public flutter::IgnoreAttributeDispatchHelper,
                            public flutter::IgnoreClipDispatchHelper,
                            public flutter::IgnoreDrawDispatchHelper {
 public:
  TextFrameDispatcher(const ContentContext& renderer,
                      const Matrix& initial_matrix)
      : renderer_(renderer), matrix_(initial_matrix) {
    renderer.GetLazyGlyphAtlas()->ResetTextFrames();
  }

  void save() override { stack_.emplace_back(matrix_); }

  void saveLayer(const SkRect& bounds,
                 const flutter::SaveLayerOptions options,
                 const flutter::DlImageFilter* backdrop) override {
    save();
  }

  void restore() override {
    matrix_ = stack_.back();
    stack_.pop_back();
  }

  void translate(SkScalar tx, SkScalar ty) override {
    matrix_ = matrix_.Translate({tx, ty});
  }

  void scale(SkScalar sx, SkScalar sy) override {
    matrix_ = matrix_.Scale({sx, sy, 1.0f});
  }

  void rotate(SkScalar degrees) override {
    matrix_ = matrix_ * Matrix::MakeRotationZ(Degrees(degrees));
  }

  void skew(SkScalar sx, SkScalar sy) override {
    matrix_ = matrix_ * Matrix::MakeSkew(sx, sy);
  }

  // clang-format off
  // 2x3 2D affine subset of a 4x4 transform in row major order
  void transform2DAffine(SkScalar mxx, SkScalar mxy, SkScalar mxt,
                         SkScalar myx, SkScalar myy, SkScalar myt) override {
    matrix_ = matrix_ * Matrix::MakeColumn(
        mxx,  myx,  0.0f, 0.0f,
        mxy,  myy,  0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        mxt,  myt,  0.0f, 1.0f
    );
  }

  // full 4x4 transform in row major order
  void transformFullPerspective(
      SkScalar mxx, SkScalar mxy, SkScalar mxz, SkScalar mxt,
      SkScalar myx, SkScalar myy, SkScalar myz, SkScalar myt,
      SkScalar mzx, SkScalar mzy, SkScalar mzz, SkScalar mzt,
      SkScalar mwx, SkScalar mwy, SkScalar mwz, SkScalar mwt) override {
    matrix_ = matrix_ * Matrix::MakeColumn(
        mxx, myx, mzx, mwx,
        mxy, myy, mzy, mwy,
        mxz, myz, mzz, mwz,
        mxt, myt, mzt, mwt
    );
  }
  // clang-format on

  void transformReset() override { matrix_ = Matrix(); }

  void drawTextFrame(const std::shared_ptr<impeller::TextFrame>& text_frame,
                     SkScalar x,
                     SkScalar y) override {
    renderer_.GetLazyGlyphAtlas()->AddTextFrame(*text_frame,
                                                matrix_.GetMaxBasisLengthXY());
  }

  void drawDisplayList(const sk_sp<flutter::DisplayList> display_list,
                       SkScalar opacity) override {
    save();
    display_list->Dispatch(*this);
    restore();
  }

 private:
  const ContentContext& renderer_;
  Matrix matrix_;
  std::vector<Matrix> stack_;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_DISPLAY_LIST_DL_DISPATCHER_H_
