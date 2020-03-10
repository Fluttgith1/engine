/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "flutter/testing/testing.h"
#include "platform_view_rtree.h"

#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"

namespace flutter {
namespace testing {

TEST(PlatformViewRTree, NoIntersection) {
    auto r_tree = sk_make_sp<PlatformViewRTree>();
    auto rtree_factory = PlatformViewRTreeFactory(r_tree);
    auto recorder = std::make_unique<SkPictureRecorder>();
    auto recording_canvas = recorder->beginRecording(SkRect::MakeIWH(1000, 1000), &rtree_factory);

    auto rect_paint = SkPaint();
    rect_paint.setColor(SkColors::kCyan);
    rect_paint.setStyle(SkPaint::Style::kFill_Style);

    // If no rect is intersected with the query rect, then the result vector is empty.
    recording_canvas->drawRect(SkRect::MakeLTRB(20, 20, 40, 40), rect_paint);
    recorder->finishRecordingAsPicture();

    auto hits = r_tree->searchRects(SkRect::MakeLTRB(40, 40, 80, 80));
    ASSERT_TRUE(hits.empty());
}

TEST(PlatformViewRTree, SingleRectIntersection) {
    auto r_tree = sk_make_sp<PlatformViewRTree>();
    auto rtree_factory = PlatformViewRTreeFactory(r_tree);
    auto recorder = std::make_unique<SkPictureRecorder>();
    auto recording_canvas = recorder->beginRecording(SkRect::MakeIWH(1000, 1000), &rtree_factory);

    auto rect_paint = SkPaint();
    rect_paint.setColor(SkColors::kCyan);
    rect_paint.setStyle(SkPaint::Style::kFill_Style);

    // Given a single rect A that intersects with the query rect,
    // the result vector contains this rect.
    recording_canvas->drawRect(SkRect::MakeLTRB(120, 120, 160, 160), rect_paint);

    recorder->finishRecordingAsPicture();

    auto hits = r_tree->searchRects(SkRect::MakeLTRB(140, 140, 150, 150));
    ASSERT_EQ(1UL, hits.size());
    ASSERT_EQ(*hits.begin(), SkRect::MakeLTRB(120, 120, 160, 160));
}

TEST(PlatformViewRTree, IgnoresNonDrawingRecords) {
    auto r_tree = sk_make_sp<PlatformViewRTree>();
    auto rtree_factory = PlatformViewRTreeFactory(r_tree);
    auto recorder = std::make_unique<SkPictureRecorder>();
    auto recording_canvas = recorder->beginRecording(SkRect::MakeIWH(1000, 1000), &rtree_factory);

    auto rect_paint = SkPaint();
    rect_paint.setColor(SkColors::kCyan);
    rect_paint.setStyle(SkPaint::Style::kFill_Style);

    // Creates two non drawing records.
    recording_canvas->translate(100, 100);
    // The result vector should only contain the clipping rect.
    recording_canvas->clipRect(SkRect::MakeLTRB(40, 40, 50, 50), SkClipOp::kIntersect);
    recording_canvas->drawRect(SkRect::MakeLTRB(20, 20, 80, 80), rect_paint);

    recorder->finishRecordingAsPicture();

    // The rtree has a translate, a clip and a rect record.
    ASSERT_EQ(3, r_tree->getCount());

    auto hits = r_tree->searchRects(SkRect::MakeLTRB(0, 0, 1000, 1000));
    ASSERT_EQ(1UL, hits.size());
    ASSERT_EQ(*hits.begin(), SkRect::MakeLTRB(120, 120, 180, 180));
}

TEST(PlatformViewRTree, MultipleRectIntersection) {
    auto r_tree = sk_make_sp<PlatformViewRTree>();
    auto rtree_factory = PlatformViewRTreeFactory(r_tree);
    auto recorder = std::make_unique<SkPictureRecorder>();
    auto recording_canvas = recorder->beginRecording(SkRect::MakeIWH(1000, 1000), &rtree_factory);

    auto rect_paint = SkPaint();
    rect_paint.setColor(SkColors::kCyan);
    rect_paint.setStyle(SkPaint::Style::kFill_Style);

    // Given the A, B that intersect with the query rect,
    // there should be A and B in the result vector since
    // they don't intersect with each other.
    //
    //  +-----+   +-----+
    //  |  A  |   |  B  |
    //  +-----+   +-----+
    // A
    recording_canvas->drawRect(SkRect::MakeLTRB(100, 100, 200, 200), rect_paint);
    // B
    recording_canvas->drawRect(SkRect::MakeLTRB(300, 100, 400, 200), rect_paint);

    recorder->finishRecordingAsPicture();

    auto hits = r_tree->searchRects(SkRect::MakeLTRB(0, 0, 1000, 1050));
    ASSERT_EQ(2UL, hits.size());
    ASSERT_EQ(*hits.begin(), SkRect::MakeLTRB(100, 100, 200, 200));
    ASSERT_EQ(*std::next(hits.begin(), 1), SkRect::MakeLTRB(300, 100, 400, 200));
}

TEST(PlatformViewRTree, JoinRectsWhenIntersectedCase1) {
    auto r_tree = sk_make_sp<PlatformViewRTree>();
    auto rtree_factory = PlatformViewRTreeFactory(r_tree);
    auto recorder = std::make_unique<SkPictureRecorder>();
    auto recording_canvas = recorder->beginRecording(SkRect::MakeIWH(1000, 1000), &rtree_factory);

    auto rect_paint = SkPaint();
    rect_paint.setColor(SkColors::kCyan);
    rect_paint.setStyle(SkPaint::Style::kFill_Style);

    // Given the A, and B rects, which intersect with the query rect,
    // the result vector contains the rect resulting from the union of A and B.
    //
    // +-----+
    // |  A  |
    // |   +-----+
    // |   |  C  |
    // |   +-----+
    // |     |
    // +-----+

    // A
    recording_canvas->drawRect(SkRect::MakeLTRB(100, 100, 150, 150), rect_paint);
    // B
    recording_canvas->drawRect(SkRect::MakeLTRB(125, 125, 175, 175), rect_paint);

    recorder->finishRecordingAsPicture();

    auto hits = r_tree->searchRects(SkRect::MakeXYWH(120, 120, 126, 126));
    ASSERT_EQ(1UL, hits.size());
    ASSERT_EQ(*hits.begin(), SkRect::MakeLTRB(100, 100, 175, 175));
}

TEST(PlatformViewRTree, JoinRectsWhenIntersectedCase2) {
    auto r_tree = sk_make_sp<PlatformViewRTree>();
    auto rtree_factory = PlatformViewRTreeFactory(r_tree);
    auto recorder = std::make_unique<SkPictureRecorder>();
    auto recording_canvas = recorder->beginRecording(SkRect::MakeIWH(1000, 1000), &rtree_factory);

    auto rect_paint = SkPaint();
    rect_paint.setColor(SkColors::kCyan);
    rect_paint.setStyle(SkPaint::Style::kFill_Style);

    // Given the A, B, and C rects that intersect with the query rect,
    // there should be only C in the result vector,
    // since A and B are contained in C.
    //
    // +---------------------+
    // | C                   |
    // |  +-----+   +-----+  |
    // |  |  A  |   |  B  |  |
    // |  +-----+   +-----+  |
    // +---------------------+
    //              +-----+
    //              |  D  |
    //              +-----+

    // A
    recording_canvas->drawRect(SkRect::MakeLTRB(100, 100, 200, 200), rect_paint);
    // B
    recording_canvas->drawRect(SkRect::MakeLTRB(300, 100, 400, 200), rect_paint);
    // C
    recording_canvas->drawRect(SkRect::MakeLTRB(50, 50, 500, 250), rect_paint);
    // D
    recording_canvas->drawRect(SkRect::MakeLTRB(280, 100, 280, 320), rect_paint);

    recorder->finishRecordingAsPicture();

    auto hits = r_tree->searchRects(SkRect::MakeLTRB(30, 30, 550, 270));
    ASSERT_EQ(1UL, hits.size());
    ASSERT_EQ(*hits.begin(), SkRect::MakeLTRB(50, 50, 500, 250));
}

TEST(PlatformViewRTree, JoinRectsWhenIntersectedCase3) {
    auto r_tree = sk_make_sp<PlatformViewRTree>();
    auto rtree_factory = PlatformViewRTreeFactory(r_tree);
    auto recorder = std::make_unique<SkPictureRecorder>();
    auto recording_canvas = recorder->beginRecording(SkRect::MakeIWH(1000, 1000), &rtree_factory);

    auto rect_paint = SkPaint();
    rect_paint.setColor(SkColors::kCyan);
    rect_paint.setStyle(SkPaint::Style::kFill_Style);

    // Given the A, B, C and D rects that intersect with the query rect,
    // the result vector contains a single rect, which is the union of
    // these four rects.
    //
    // +------------------------------+
    // | D                            |
    // |  +-----+   +-----+   +-----+ |
    // |  |  A  |   |  B  |   |  C  | |
    // |  +-----+   +-----+   |     | |
    // +----------------------|     |-+
    //                        +-----+
    //              +-----+
    //              |  E  |
    //              +-----+

    // A
    recording_canvas->drawRect(SkRect::MakeLTRB(100, 100, 200, 200), rect_paint);
    // B
    recording_canvas->drawRect(SkRect::MakeLTRB(300, 100, 400, 200), rect_paint);
    // C
    recording_canvas->drawRect(SkRect::MakeLTRB(500, 100, 600, 300), rect_paint);
    // D
    recording_canvas->drawRect(SkRect::MakeLTRB(50, 50, 620, 250), rect_paint);
    // E
    recording_canvas->drawRect(SkRect::MakeLTRB(280, 100, 280, 320), rect_paint);

    recorder->finishRecordingAsPicture();

    auto hits = r_tree->searchRects(SkRect::MakeLTRB(30, 30, 550, 270));
    ASSERT_EQ(1UL, hits.size());
    ASSERT_EQ(*hits.begin(), SkRect::MakeLTRB(50, 50, 620, 300));
}

}  // namespace testing
}  // namespace flutter
