// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#ifndef FLUTTER_FLOW_EMBEDDED_VIEWS_H_
#define FLUTTER_FLOW_EMBEDDED_VIEWS_H_

#include <vector>

#include "flutter/fml/memory/ref_counted.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkSize.h"

namespace flutter {

enum MutatorType { clip_rect, clip_rrect, clip_path, transform };

class Mutator {
 public:
  void setType(const MutatorType type) { type_ = type; }
  void setRect(const SkRect& rect) { rect_ = rect; }
  void setRRect(const SkRRect& rrect) { rrect_ = rrect; }
  void setMatrix(const SkMatrix& matrix) { matrix_ = matrix; }

  MutatorType type() const { return type_; }
  SkRect rect() const { return rect_; }
  SkRRect rrect() const { return rrect_; }
  SkPath path() const { return path_; }
  SkMatrix matrix() const { return matrix_; }

  bool operator==(const Mutator& other) const {
    if (type_ != other.type_) {
      return false;
    }
    if (type_ == clip_rect && rect_ == other.rect_) {
      return true;
    }
    if (type_ == clip_rrect && rrect_ == other.rrect_) {
      return true;
    }
    if (type_ == clip_path && path_ == other.path_) {
      return true;
    }
    if (type_ == transform && matrix_ == other.matrix_) {
      return true;
    }

    return false;
  }

  bool operator!=(const Mutator& other) const { return !operator==(other); }

  bool isClipType() {
    return type_ == clip_rect || type_ == clip_rrect || type_ == clip_path;
  }

 private:
  MutatorType type_;
  SkRect rect_;
  SkRRect rrect_;
  SkPath path_;
  SkMatrix matrix_;
};  // Mutator

// A stack of mutators that can be applied to an embedded platform view.
//
// The stack may include mutators like transforms and clips, each mutator
// applies to all the mutators that are below it in the stack and to the
// embedded view.
//
// For example consider the following stack: [T1, T2, T3], where T1 is the top
// of the stack and T3 is the bottom of the stack. Applying this mutators stack
// to a platform view P1 will result in T1(T2(T2(P1))).
class MutatorsStack {
 public:
  void pushClipRect(const SkRect& rect);
  void pushClipRRect(const SkRRect& rrect);
  void pushClipPath(const SkPath& path);

  void pushTransform(const SkMatrix& matrix);

  // Removes the `Mutator` on the top of the stack
  // and destroys it.
  void pop();

  // Returns the iterator points to the top of the stack..
  const std::vector<std::unique_ptr<Mutator>>::const_reverse_iterator top();
  // Returns an iterator pointing to the bottom of the stack.
  const std::vector<std::unique_ptr<Mutator>>::const_reverse_iterator bottom();

  bool operator==(const MutatorsStack& other) const {
    if (vector_.size() != other.vector_.size()) {
      return false;
    }

    for (size_t i = 0; i < vector_.size(); i++) {
      if (*(vector_[i].get()) != *(other.vector_[i].get())) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const MutatorsStack& other) const {
    return !operator==(other);
  }

 private:
  std::vector<std::unique_ptr<Mutator>> vector_;
};  // MutatorsStack

class EmbeddedViewParams {
 public:
  SkPoint offsetPixels;
  SkSize sizePoints;
  MutatorsStack* mutatorsStack;

  bool operator==(const EmbeddedViewParams& other) const {
    return offsetPixels == other.offsetPixels &&
           sizePoints == other.sizePoints &&
           *mutatorsStack == *(other.mutatorsStack);
  }
};

// This is only used on iOS when running in a non headless mode,
// in this case ExternalViewEmbedder is a reference to the
// FlutterPlatformViewsController which is owned by FlutterViewController.
class ExternalViewEmbedder {
 public:
  ExternalViewEmbedder() = default;

  virtual void BeginFrame(SkISize frame_size) = 0;

  virtual void PrerollCompositeEmbeddedView(int view_id) = 0;

  virtual std::vector<SkCanvas*> GetCurrentCanvases() = 0;

  // Must be called on the UI thread.
  virtual SkCanvas* CompositeEmbeddedView(int view_id,
                                          const EmbeddedViewParams& params) = 0;

  virtual bool SubmitFrame(GrContext* context);

  virtual ~ExternalViewEmbedder() = default;

  FML_DISALLOW_COPY_AND_ASSIGN(ExternalViewEmbedder);

};  // ExternalViewEmbedder

}  // namespace flutter

#endif  // FLUTTER_FLOW_EMBEDDED_VIEWS_H_
