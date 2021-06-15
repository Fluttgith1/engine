// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_DISPLAY_LIST_H_
#define FLUTTER_FLOW_DISPLAY_LIST_H_

#include "third_party/skia/include/core/SkBlurTypes.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/core/SkVertices.h"

// The Flutter DisplayList mechanism is used in place of the Skia SkPicture
// mechanism. It encapsulates a sequence of rendering operations into a
// persistent list that can be replayed upon request by calling its
// dispatch() method with a Dispatcher object that responds to the various
// rendering methods encapsulated therein. The mechanism is inspired by
// the SkLiteDL class that is not directly supported by Skia, but has been
// recommended as a basis for custom display lists for a number of their
// customers.
//
// This file contains the definitions for:
// DisplayList: the base class that holds the information about the
//              sequence of operations and can dispatch them to a Dispatcher
// Dispatcher: a pure virtual interface which can be implemented to field
//             the requests for purposes such as sending them to an SkCanvas
//             or detecting various rendering optimization scenarios
// DisplayListBuilder: a class for constructing a DisplayList from the same
//                     calls defined in the Dispatcher
//
// Other files include various class definitions for dealing with display
// lists, such as:
// display_list_canvas.h: classes to interact between SkCanvas and DisplayList
//                        (SkCanvas->DisplayList adapter and vice versa)
//
// display_list_utils.h: various utility classes to ease implementing
//                       a Dispatcher, including NOP implementations of
//                       the attribute, clip, and transform methods,
//                       classes to track attributes, clips, and transforms
//                       and a class to compute the bounds of a DisplayList
//                       Any class implementing Dispatcher can inherit from
//                       these utility classes to simplify its creation

namespace flutter {

#define FOR_EACH_DISPLAY_LIST_OP(V) \
  V(SetAA)                          \
  V(SetDither)                      \
  V(SetInvertColors)                \
                                    \
  V(SetCaps)                        \
  V(SetJoins)                       \
                                    \
  V(SetDrawStyle)                   \
  V(SetStrokeWidth)                 \
  V(SetMiterLimit)                  \
                                    \
  V(SetColor)                       \
  V(SetBlendMode)                   \
                                    \
  V(SetFilterQuality)               \
                                    \
  V(SetShader)                      \
  V(ClearShader)                    \
  V(SetColorFilter)                 \
  V(ClearColorFilter)               \
  V(SetImageFilter)                 \
  V(ClearImageFilter)               \
                                    \
  V(ClearMaskFilter)                \
  V(SetMaskFilter)                  \
  V(SetMaskBlurFilterNormal)        \
  V(SetMaskBlurFilterSolid)         \
  V(SetMaskBlurFilterOuter)         \
  V(SetMaskBlurFilterInner)         \
                                    \
  V(Save)                           \
  V(SaveLayer)                      \
  V(SaveLayerBounds)                \
  V(Restore)                        \
                                    \
  V(Translate)                      \
  V(Scale)                          \
  V(Rotate)                         \
  V(Skew)                           \
  V(Transform2x3)                   \
  V(Transform3x3)                   \
                                    \
  V(ClipRect)                       \
  V(ClipRRect)                      \
  V(ClipPath)                       \
                                    \
  V(DrawPaint)                      \
  V(DrawColor)                      \
                                    \
  V(DrawLine)                       \
  V(DrawRect)                       \
  V(DrawOval)                       \
  V(DrawCircle)                     \
  V(DrawRRect)                      \
  V(DrawDRRect)                     \
  V(DrawArc)                        \
  V(DrawPath)                       \
                                    \
  V(DrawPoints)                     \
  V(DrawLines)                      \
  V(DrawPolygon)                    \
  V(DrawVertices)                   \
                                    \
  V(DrawImage)                      \
  V(DrawImageRect)                  \
  V(DrawImageNine)                  \
  V(DrawImageLattice)               \
  V(DrawAtlas)                      \
  V(DrawAtlasColored)               \
  V(DrawAtlasCulled)                \
  V(DrawAtlasColoredCulled)         \
                                    \
  V(DrawSkPicture)                  \
  V(DrawDisplayList)                \
  V(DrawTextBlob)                   \
  /* V(DrawShadowRec) */            \
                                    \
  V(DrawShadow)

#define DL_OP_TO_ENUM_VALUE(name) name,
enum DisplayListOpType { FOR_EACH_DISPLAY_LIST_OP(DL_OP_TO_ENUM_VALUE) };
#undef DL_OP_TO_ENUM_VALUE

class Dispatcher;
class DisplayListBuilder;

// The base class that contains a sequence of rendering operations
// for dispatch to a Dispatcher. These objects must be instantiated
// through an instance of DisplayListBuilder::build().
class DisplayList : public SkRefCnt {
 public:
  static const SkSamplingOptions NearestSampling;
  static const SkSamplingOptions LinearSampling;
  static const SkSamplingOptions MipmapSampling;
  static const SkSamplingOptions CubicSampling;

  DisplayList()
      : ptr_(nullptr),
        used_(0),
        opCount_(0),
        uniqueID_(0),
        bounds_({0, 0, 0, 0}),
        boundsCull_({0, 0, 0, 0}) {}

  ~DisplayList();

  void dispatch(Dispatcher& ctx) const { dispatch(ctx, ptr_, ptr_ + used_); }

  void renderTo(SkCanvas* canvas) const;

  size_t bytes() const { return used_; }
  int opCount() const { return opCount_; }
  uint32_t uniqueID() const { return uniqueID_; }

  const SkRect& bounds() {
    if (bounds_.width() < 0.0) {
      // computeBounds() will leave the variable with a
      // non-negative width and height
      computeBounds();
    }
    return bounds_;
  }

  bool equals(const DisplayList& other) const;

 private:
  DisplayList(uint8_t* ptr, size_t used, int opCount, const SkRect& cullRect);

  uint8_t* ptr_;
  size_t used_;
  int opCount_;

  uint32_t uniqueID_;
  SkRect bounds_;

  // Only used for drawPaint() and drawColor()
  SkRect boundsCull_;

  void computeBounds();
  void dispatch(Dispatcher& ctx, uint8_t* ptr, uint8_t* end) const;

  friend class DisplayListBuilder;
};

// The pure virtual interface for interacting with a display list.
// This interface represents the methods used to build a list
// through the DisplayListBuilder and also the methods that will
// be invoked through the DisplayList::dispatch() method.
class Dispatcher {
 public:
  // MaxDrawPointsCount * sizeof(SkPoint) must be less than 1 << 32
  static constexpr int MaxDrawPointsCount = ((1 << 29) - 1);

  virtual void setAA(bool aa) = 0;
  virtual void setDither(bool dither) = 0;
  virtual void setInvertColors(bool invert) = 0;
  virtual void setCaps(SkPaint::Cap cap) = 0;
  virtual void setJoins(SkPaint::Join join) = 0;
  virtual void setDrawStyle(SkPaint::Style style) = 0;
  virtual void setStrokeWidth(SkScalar width) = 0;
  virtual void setMiterLimit(SkScalar limit) = 0;
  virtual void setColor(SkColor color) = 0;
  virtual void setBlendMode(SkBlendMode mode) = 0;
  virtual void setFilterQuality(SkFilterQuality quality) = 0;
  virtual void setShader(const sk_sp<SkShader> shader) = 0;
  virtual void setImageFilter(const sk_sp<SkImageFilter> filter) = 0;
  virtual void setColorFilter(const sk_sp<SkColorFilter> filter) = 0;
  virtual void setMaskFilter(const sk_sp<SkMaskFilter> filter) = 0;
  virtual void setMaskBlurFilter(SkBlurStyle style, SkScalar sigma) = 0;

  virtual void save() = 0;
  virtual void restore() = 0;
  virtual void saveLayer(const SkRect* bounds, bool restoreWithPaint) = 0;

  virtual void translate(SkScalar tx, SkScalar ty) = 0;
  virtual void scale(SkScalar sx, SkScalar sy) = 0;
  virtual void rotate(SkScalar degrees) = 0;
  virtual void skew(SkScalar sx, SkScalar sy) = 0;
  virtual void transform2x3(SkScalar mxx,
                            SkScalar mxy,
                            SkScalar mxt,
                            SkScalar myx,
                            SkScalar myy,
                            SkScalar myt) = 0;
  virtual void transform3x3(SkScalar mxx,
                            SkScalar mxy,
                            SkScalar mxt,
                            SkScalar myx,
                            SkScalar myy,
                            SkScalar myt,
                            SkScalar px,
                            SkScalar py,
                            SkScalar pt) = 0;

  virtual void clipRect(const SkRect& rect, bool isAA, SkClipOp clip_op) = 0;
  virtual void clipRRect(const SkRRect& rrect, bool isAA, SkClipOp clip_op) = 0;
  virtual void clipPath(const SkPath& path, bool isAA, SkClipOp clip_op) = 0;

  virtual void drawPaint() = 0;
  virtual void drawColor(SkColor color, SkBlendMode mode) = 0;
  virtual void drawLine(const SkPoint& p0, const SkPoint& p1) = 0;
  virtual void drawRect(const SkRect& rect) = 0;
  virtual void drawOval(const SkRect& bounds) = 0;
  virtual void drawCircle(const SkPoint& center, SkScalar radius) = 0;
  virtual void drawRRect(const SkRRect& rrect) = 0;
  virtual void drawDRRect(const SkRRect& outer, const SkRRect& inner) = 0;
  virtual void drawPath(const SkPath& path) = 0;
  virtual void drawArc(const SkRect& bounds,
                       SkScalar start,
                       SkScalar sweep,
                       bool useCenter) = 0;
  virtual void drawPoints(SkCanvas::PointMode mode,
                          uint32_t count,
                          const SkPoint pts[]) = 0;
  virtual void drawVertices(const sk_sp<SkVertices> vertices,
                            SkBlendMode mode) = 0;
  virtual void drawImage(const sk_sp<SkImage> image,
                         const SkPoint point,
                         const SkSamplingOptions& sampling) = 0;
  virtual void drawImageRect(const sk_sp<SkImage> image,
                             const SkRect& src,
                             const SkRect& dst,
                             const SkSamplingOptions& sampling) = 0;
  virtual void drawImageNine(const sk_sp<SkImage> image,
                             const SkIRect& center,
                             const SkRect& dst,
                             SkFilterMode filter) = 0;
  virtual void drawImageLattice(const sk_sp<SkImage> image,
                                const SkCanvas::Lattice& lattice,
                                const SkRect& dst,
                                SkFilterMode filter) = 0;
  virtual void drawAtlas(const sk_sp<SkImage> atlas,
                         const SkRSXform xform[],
                         const SkRect tex[],
                         const SkColor colors[],
                         int count,
                         SkBlendMode mode,
                         const SkSamplingOptions& sampling,
                         const SkRect* cullRect) = 0;
  virtual void drawPicture(const sk_sp<SkPicture> picture) = 0;
  virtual void drawDisplayList(const sk_sp<DisplayList> display_list) = 0;
  virtual void drawTextBlob(const sk_sp<SkTextBlob> blob,
                            SkScalar x,
                            SkScalar y) = 0;
  // Unfortunately SkDrawShadowRec requires including an internal Skia header
  // virtual void drawShadowRec(const SkPath&, const SkDrawShadowRec&) = 0;
  virtual void drawShadow(const SkPath& path,
                          const SkColor color,
                          const SkScalar elevation,
                          bool occludes) = 0;
};

// The primary class used to build a display list. The list of methods
// here matches the list of methods invoked during dispatch().
// If there is some code that already renders to an SkCanvas object,
// those rendering commands can be captured into a DisplayList using
// the DisplayListCanvasRecorder class.
class DisplayListBuilder final : public virtual Dispatcher, public SkRefCnt {
 public:
  DisplayListBuilder(const SkRect& cull = SkRect::MakeEmpty());
  ~DisplayListBuilder();

  void setAA(bool aa) override;
  void setDither(bool dither) override;
  void setInvertColors(bool invert) override;
  void setCaps(SkPaint::Cap cap) override;
  void setJoins(SkPaint::Join join) override;
  void setDrawStyle(SkPaint::Style style) override;
  void setStrokeWidth(SkScalar width) override;
  void setMiterLimit(SkScalar limit) override;
  void setColor(SkColor color) override;
  void setBlendMode(SkBlendMode mode) override;
  void setFilterQuality(SkFilterQuality quality) override;
  void setShader(sk_sp<SkShader> shader) override;
  void setImageFilter(sk_sp<SkImageFilter> filter) override;
  void setColorFilter(sk_sp<SkColorFilter> filter) override;
  void setMaskFilter(sk_sp<SkMaskFilter> filter) override;
  void setMaskBlurFilter(SkBlurStyle style, SkScalar sigma) override;

  void save() override;
  void restore() override;
  void saveLayer(const SkRect* bounds, bool restoreWithPaint) override;

  void translate(SkScalar tx, SkScalar ty) override;
  void scale(SkScalar sx, SkScalar sy) override;
  void rotate(SkScalar degrees) override;
  void skew(SkScalar sx, SkScalar sy) override;
  void transform2x3(SkScalar mxx,
                    SkScalar mxy,
                    SkScalar mxt,
                    SkScalar myx,
                    SkScalar myy,
                    SkScalar myt) override;
  void transform3x3(SkScalar mxx,
                    SkScalar mxy,
                    SkScalar mxt,
                    SkScalar myx,
                    SkScalar myy,
                    SkScalar myt,
                    SkScalar px,
                    SkScalar py,
                    SkScalar pt) override;

  void clipRect(const SkRect& rect, bool isAA, SkClipOp clip_op) override;
  void clipRRect(const SkRRect& rrect, bool isAA, SkClipOp clip_op) override;
  void clipPath(const SkPath& path, bool isAA, SkClipOp clip_op) override;

  void drawPaint() override;
  void drawColor(SkColor color, SkBlendMode mode) override;
  void drawLine(const SkPoint& p0, const SkPoint& p1) override;
  void drawRect(const SkRect& rect) override;
  void drawOval(const SkRect& bounds) override;
  void drawCircle(const SkPoint& center, SkScalar radius) override;
  void drawRRect(const SkRRect& rrect) override;
  void drawDRRect(const SkRRect& outer, const SkRRect& inner) override;
  void drawPath(const SkPath& path) override;
  void drawArc(const SkRect& bounds,
               SkScalar start,
               SkScalar sweep,
               bool useCenter) override;
  void drawPoints(SkCanvas::PointMode mode,
                  uint32_t count,
                  const SkPoint pts[]) override;
  void drawVertices(const sk_sp<SkVertices> vertices,
                    SkBlendMode mode) override;
  void drawImage(const sk_sp<SkImage> image,
                 const SkPoint point,
                 const SkSamplingOptions& sampling) override;
  void drawImageRect(const sk_sp<SkImage> image,
                     const SkRect& src,
                     const SkRect& dst,
                     const SkSamplingOptions& sampling) override;
  void drawImageNine(const sk_sp<SkImage> image,
                     const SkIRect& center,
                     const SkRect& dst,
                     SkFilterMode filter) override;
  void drawImageLattice(const sk_sp<SkImage> image,
                        const SkCanvas::Lattice& lattice,
                        const SkRect& dst,
                        SkFilterMode filter) override;
  void drawAtlas(const sk_sp<SkImage> atlas,
                 const SkRSXform xform[],
                 const SkRect tex[],
                 const SkColor colors[],
                 int count,
                 SkBlendMode mode,
                 const SkSamplingOptions& sampling,
                 const SkRect* cullRect) override;
  void drawPicture(const sk_sp<SkPicture> picture) override;
  void drawDisplayList(const sk_sp<DisplayList> display_list) override;
  void drawTextBlob(const sk_sp<SkTextBlob> blob,
                    SkScalar x,
                    SkScalar y) override;
  // void drawShadowRec(const SkPath&, const SkDrawShadowRec&) override;
  void drawShadow(const SkPath& path,
                  const SkColor color,
                  const SkScalar elevation,
                  bool occludes) override;

  sk_sp<DisplayList> build();

 private:
  SkAutoTMalloc<uint8_t> storage_;
  size_t used_ = 0;
  size_t allocated_ = 0;
  int opCount_ = 0;
  int saveLevel_ = 0;

  SkRect cull_;

  template <typename T, typename... Args>
  void* push(size_t extra, Args&&... args);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_DISPLAY_LIST_H_
