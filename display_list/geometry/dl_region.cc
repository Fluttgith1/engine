// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/geometry/dl_region.h"

#include "flutter/fml/logging.h"

namespace flutter {

DlRegion::SpanBuffer::SpanBuffer(DlRegion::SpanBuffer&& m)
    : capacity_(m.capacity_), size_(m.size_), spans_(m.spans_) {
  m.size_ = 0;
  m.capacity_ = 0;
  m.spans_ = nullptr;
};

DlRegion::SpanBuffer::~SpanBuffer() {
  free(spans_);
}

void DlRegion::SpanBuffer::reserve(size_t capacity) {
  if (capacity_ < capacity) {
    spans_ = static_cast<Span*>(std::realloc(spans_, capacity * sizeof(Span)));
    capacity_ = capacity;
  }
}

DlRegion::SpanChunkHandle DlRegion::SpanBuffer::storeChunk(const Span* begin,
                                                           const Span* end) {
  size_t chunk_size = end - begin;
  size_t min_capacity = size_ + chunk_size + 1;
  if (capacity_ < min_capacity) {
    size_t new_capacity = std::max(min_capacity, capacity_ * 2);
    new_capacity = std::max(new_capacity, size_t(512));
    reserve(new_capacity);
  }
  SpanChunkHandle res = size_;
  size_ += chunk_size + 1;
  spans_[res].left = chunk_size;

  auto* dst = spans_ + res + 1;
  memmove(dst, begin, chunk_size * sizeof(Span));

  return res;
}

size_t DlRegion::SpanBuffer::getChunkSize(SpanChunkHandle handle) const {
  FML_DCHECK(handle < size_);
  return spans_[handle].left;
}

void DlRegion::SpanBuffer::getSpans(SpanChunkHandle handle,
                                    const DlRegion::Span*& begin,
                                    const DlRegion::Span*& end) const {
  FML_DCHECK(handle < size_);
  auto& info = spans_[handle];
  begin = spans_ + handle + 1;
  end = begin + info.left;
}

DlRegion::DlRegion(const std::vector<SkIRect>& rects) {
  addRects(rects);
}

DlRegion::DlRegion() {}

bool DlRegion::spansEqual(SpanLine& line,
                          const Span* begin,
                          const Span* end) const {
  const Span *our_begin, *our_end;
  span_buffer_.getSpans(line.chunk_handle, our_begin, our_end);
  size_t our_size = our_end - our_begin;
  size_t their_size = end - begin;
  if (our_size != their_size) {
    return false;
  }

  return memcmp(our_begin, begin, our_size * sizeof(Span)) == 0;
}

DlRegion::SpanLine DlRegion::makeLine(int32_t top,
                                      int32_t bottom,
                                      const SpanVec& v) {
  return makeLine(top, bottom, v.data(), v.data() + v.size());
}

DlRegion::SpanLine DlRegion::makeLine(int32_t top,
                                      int32_t bottom,
                                      const Span* begin,
                                      const Span* end) {
  auto handle = span_buffer_.storeChunk(begin, end);
  return {top, bottom, handle};
}

size_t DlRegion::mergeLines(std::vector<Span>& res,
                            const SpanBuffer& a_buffer,
                            SpanChunkHandle a_handle,
                            const SpanBuffer& b_buffer,
                            SpanChunkHandle b_handle) {
  const Span *begin1, *end1;
  a_buffer.getSpans(a_handle, begin1, end1);

  const Span *begin2, *end2;
  b_buffer.getSpans(b_handle, begin2, end2);

  size_t min_size = (end1 - begin1) + (end2 - begin2);
  if (res.size() < min_size) {
    res.resize(min_size);
  }
  Span* end = res.data();

  while (true) {
    if (begin1->right < begin2->left - 1) {
      *end = *begin1;
      ++begin1;
      ++end;
      if (begin1 == end1) {
        break;
      }
    } else if (begin2->right < begin1->left) {
      *end = *begin2;
      ++begin2;
      ++end;
      if (begin2 == end2) {
        break;
      }
    } else {
      break;
    }
  }

  Span currentSpan{0, 0};
  while (begin1 != end1 && begin2 != end2) {
    if (currentSpan.left == currentSpan.right) {
      if (begin1->right < begin2->left - 1) {
        *end = *begin1;
        ++begin1;
        ++end;
      } else if (begin2->right < begin1->left) {
        *end = *begin2;
        ++begin2;
        ++end;
      } else if (begin1->left == begin2->left) {
        currentSpan.left = begin1->left;
        currentSpan.right = std::max(begin1->right, begin2->right);
        ++begin1;
        ++begin2;
      } else if (begin1->left < begin2->left) {
        currentSpan.left = begin1->left;
        currentSpan.right = begin1->right;
        ++begin1;
      } else {
        currentSpan.left = begin2->left;
        currentSpan.right = begin2->right;
        ++begin2;
      }
    } else if (currentSpan.right >= begin1->left) {
      currentSpan.right = std::max(currentSpan.right, begin1->right);
      ++begin1;
    } else if (currentSpan.right >= begin2->left) {
      currentSpan.right = std::max(currentSpan.right, begin2->right);
      ++begin2;
    } else {
      *end = currentSpan;
      ++end;
      currentSpan.left = currentSpan.right = 0;
    }
  }

  if (currentSpan.left != currentSpan.right) {
    while (begin1 != end1 && currentSpan.right >= begin1->left) {
      currentSpan.right = std::max(currentSpan.right, begin1->right);
      ++begin1;
    }
    while (begin2 != end2 && currentSpan.right >= begin2->left) {
      currentSpan.right = std::max(currentSpan.right, begin2->right);
      ++begin2;
    }

    *end = currentSpan;
    ++end;
  }

  FML_DCHECK(begin1 == end1 || begin2 == end2);

  while (begin1 != end1) {
    *end = *begin1;
    ++begin1;
    ++end;
  }

  while (begin2 != end2) {
    *end = *begin2;
    ++begin2;
    ++end;
  }

  return end - res.data();
}

void DlRegion::addRects(const std::vector<SkIRect>& unsorted_rects) {
  size_t count = unsorted_rects.size();
  std::vector<const SkIRect*> rects(count);
  for (size_t i = 0; i < count; i++) {
    rects[i] = &unsorted_rects[i];
    bounds_.join(unsorted_rects[i]);
  }
  std::sort(rects.begin(), rects.end(), [](const SkIRect* a, const SkIRect* b) {
    if (a->top() < b->top()) {
      return true;
    }
    if (a->top() > b->top()) {
      return false;
    }
    return a->left() < b->left();
  });

  size_t active_end = 0;
  size_t next_rect = 0;
  int32_t cur_y = std::numeric_limits<int32_t>::min();
  SpanVec working_spans;

#ifdef DlRegion_DO_STATS
  size_t active_rect_count = 0;
  size_t span_count = 0;
  int pass_count = 0;
  int line_count = 0;
#endif

  while (next_rect < count || active_end > 0) {
    // First prune passed rects out of the active list
    size_t preserve_end = 0;
    for (size_t i = 0; i < active_end; i++) {
      const SkIRect* r = rects[i];
      if (r->bottom() > cur_y) {
        rects[preserve_end++] = r;
      }
    }
    active_end = preserve_end;

    // If we have no active rects any more, jump to the top of the
    // next available input rect.
    if (active_end == 0) {
      if (next_rect >= count) {
        // No active rects and no more rects to bring in. We are done.
        break;
      }
      cur_y = rects[next_rect]->top();
    }

    // Next, insert any new rects we've reached into the active list
    while (next_rect < count) {
      const SkIRect* r = rects[next_rect];
      if (r->isEmpty()) {
        continue;
      }
      if (r->top() > cur_y) {
        break;
      }
      // We now know that we will be inserting this rect into active list
      next_rect++;
      size_t insert_at = active_end++;
      while (insert_at > 0) {
        const SkIRect* ir = rects[insert_at - 1];
        if (ir->left() <= r->left()) {
          break;
        }
        rects[insert_at--] = ir;
      }
      rects[insert_at] = r;
    }

    // We either preserved some rects in the active list or added more from
    // the remaining input rects, or we would have exited the loop above.
    FML_DCHECK(active_end != 0);
    working_spans.clear();
    FML_DCHECK(working_spans.empty());

#ifdef DlRegion_DO_STATS
    active_rect_count += active_end;
    pass_count++;
#endif

    // [start_x, end_x) always represents a valid span to be inserted
    // [cur_y, end_y) is the intersecting range over which all spans are valid
    int32_t start_x = rects[0]->left();
    int32_t end_x = rects[0]->right();
    int32_t end_y = rects[0]->bottom();
    for (size_t i = 1; i < active_end; i++) {
      const SkIRect* r = rects[i];
      if (r->left() > end_x) {
        working_spans.emplace_back(start_x, end_x);
        start_x = r->left();
        end_x = r->right();
      } else if (end_x < r->right()) {
        end_x = r->right();
      }
      if (end_y > r->bottom()) {
        end_y = r->bottom();
      }
    }
    working_spans.emplace_back(start_x, end_x);

    // end_y must not pass by the top of the next input rect
    if (next_rect < count && end_y > rects[next_rect]->top()) {
      end_y = rects[next_rect]->top();
    }

    // If all of the rules above work out, we should never collapse the
    // current range of Y coordinates to empty
    FML_DCHECK(end_y > cur_y);

    if (!lines_.empty() && lines_.back().bottom == cur_y &&
        spansEqual(lines_.back(), working_spans.data(),
                   working_spans.data() + working_spans.size())) {
      lines_.back().bottom = end_y;
    } else {
#ifdef DlRegion_DO_STATS
      span_count += working_spans.size();
      line_count++;
#endif
      lines_.push_back(makeLine(cur_y, end_y, working_spans));
    }
    cur_y = end_y;
  }

#ifdef DlRegion_DO_STATS
  double span_avg = ((double)span_count) / line_count;
  double active_avg = ((double)active_rect_count) / pass_count;
  FML_LOG(ERROR) << lines_.size() << " lines for " << count
                 << " input rects, avg " << span_avg
                 << " spans per line and avg " << active_avg
                 << " active rects per loop";
#endif
}

DlRegion DlRegion::MakeUnion(const DlRegion& a, const DlRegion& b) {
  DlRegion res;

  res.span_buffer_.reserve(a.span_buffer_.capacity() +
                           b.span_buffer_.capacity());
  res.bounds_ = a.bounds_;
  res.bounds_.join(b.bounds_);

  auto& lines = res.lines_;
  lines.reserve(a.lines_.size() + b.lines_.size());

  auto append_spans = [&](int32_t top, int32_t bottom, const Span* begin,
                          const Span* end) {
    if (lines.empty()) {
      lines.push_back(res.makeLine(top, bottom, begin, end));
    } else {
      if (lines.back().bottom == top &&
          res.spansEqual(lines.back(), begin, end)) {
        lines.back().bottom = bottom;
      } else {
        lines.push_back(res.makeLine(top, bottom, begin, end));
      }
    }
  };

  auto append_line = [&](int32_t top, int32_t bottom, const SpanBuffer& buffer,
                         SpanChunkHandle chunk_handle) {
    const Span *begin, *end;
    buffer.getSpans(chunk_handle, begin, end);
    append_spans(top, bottom, begin, end);
  };

  auto a_lines = a.lines_;
  auto b_lines = b.lines_;

  auto a_it = a_lines.begin();
  auto b_it = b_lines.begin();

  auto& a_buffer = a.span_buffer_;
  auto& b_buffer = b.span_buffer_;

  std::vector<Span> tmp;

  while (a_it != a_lines.end() && b_it != b_lines.end()) {
    if (a_it->bottom <= b_it->top) {
      append_line(a_it->top, a_it->bottom, a_buffer, a_it->chunk_handle);
      ++a_it;
    } else if (b_it->bottom <= a_it->top) {
      append_line(b_it->top, b_it->bottom, b_buffer, b_it->chunk_handle);
      ++b_it;
    } else {
      if (a_it->top < b_it->top) {
        append_line(a_it->top, b_it->top, a_buffer, a_it->chunk_handle);
        a_it->top = b_it->top;
        if (a_it->top == b_it->bottom) {
          ++a_it;
        }
      } else if (b_it->top < a_it->top) {
        append_line(b_it->top, a_it->top, b_buffer, b_it->chunk_handle);
        b_it->top = a_it->top;
        if (b_it->top == a_it->bottom) {
          ++b_it;
        }
      } else {
        auto new_bottom = std::min(a_it->bottom, b_it->bottom);
        FML_DCHECK(a_it->top == b_it->top);
        FML_DCHECK(new_bottom > a_it->top);
        FML_DCHECK(new_bottom > b_it->top);
        auto size = mergeLines(tmp, a_buffer, a_it->chunk_handle, b_buffer,
                               b_it->chunk_handle);
        append_spans(a_it->top, new_bottom, tmp.data(), tmp.data() + size);
        a_it->top = b_it->top = new_bottom;
        if (a_it->top == a_it->bottom) {
          ++a_it;
        }
        if (b_it->top == b_it->bottom) {
          ++b_it;
        }
      }
    }
  }

  FML_DCHECK(a_it == a_lines.end() || b_it == b_lines.end());

  while (a_it != a_lines.end()) {
    append_line(a_it->top, a_it->bottom, a_buffer, a_it->chunk_handle);
    ++a_it;
  }

  while (b_it != b_lines.end()) {
    append_line(b_it->top, b_it->bottom, b_buffer, b_it->chunk_handle);
    ++b_it;
  }

  return res;
}

std::vector<SkIRect> DlRegion::getRects(bool deband) const {
  std::vector<SkIRect> rects;
  size_t rect_count = 0;
  size_t previous_span_end = 0;
  for (const auto& line : lines_) {
    rect_count += span_buffer_.getChunkSize(line.chunk_handle);
  }
  rects.reserve(rect_count);

  for (const auto& line : lines_) {
    const Span *span_begin, *span_end;
    span_buffer_.getSpans(line.chunk_handle, span_begin, span_end);
    for (const auto* span = span_begin; span < span_end; ++span) {
      SkIRect rect{span->left, line.top, span->right, line.bottom};
      if (deband) {
        auto iter = rects.begin() + previous_span_end;
        // If there is rectangle previously in rects on which this one is a
        // vertical continuation, remove the previous rectangle and expand
        // this one vertically to cover the area.
        while (iter != rects.begin()) {
          --iter;
          if (iter->bottom() < rect.top()) {
            // Went all the way to previous span line.
            break;
          } else if (iter->left() == rect.left() &&
                     iter->right() == rect.right()) {
            FML_DCHECK(iter->bottom() == rect.top());
            rect.fTop = iter->fTop;
            rects.erase(iter);
            --previous_span_end;
            break;
          }
        }
      }
      rects.push_back(rect);
    }
    previous_span_end = rects.size();
  }
  return rects;
}

DlRegion::~DlRegion() {}

bool DlRegion::isComplex() const {
  return lines_.size() > 1 ||
         (lines_.size() == 1 &&
          span_buffer_.getChunkSize(lines_.front().chunk_handle) > 1);
}

bool DlRegion::intersects(const SkIRect& rect) const {
  if (isEmpty()) {
    return false;
  }

  auto bounds_intersect = SkIRect::Intersects(bounds_, rect);

  if (!isComplex()) {
    return bounds_intersect;
  }

  if (!bounds_intersect) {
    return false;
  }

  auto it = std::upper_bound(
      lines_.begin(), lines_.end(), rect.fTop,
      [](int32_t i, const SpanLine& line) { return i < line.bottom; });

  while (it != lines_.end() && it->top < rect.fBottom) {
    FML_DCHECK(rect.fTop < it->bottom || it->top < rect.fBottom);
    const Span *begin, *end;
    span_buffer_.getSpans(it->chunk_handle, begin, end);
    while (begin != end && begin->left < rect.fRight) {
      if (begin->right > rect.fLeft && begin->left < rect.fRight) {
        return true;
      }
      ++begin;
    }
    ++it;
  }

  return false;
}

bool DlRegion::spansIntersect(const Span* begin1,
                              const Span* end1,
                              const Span* begin2,
                              const Span* end2) {
  while (begin1 != end1 && begin2 != end2) {
    if (begin1->right <= begin2->left) {
      ++begin1;
    } else if (begin2->right <= begin1->left) {
      ++begin2;
    } else {
      return true;
    }
  }
  return false;
}

bool DlRegion::intersects(const DlRegion& region) const {
  if (isEmpty() || region.isEmpty()) {
    return false;
  }

  auto our_complex = isComplex();
  auto their_complex = region.isComplex();
  auto bounds_intersect = SkIRect::Intersects(bounds_, region.bounds_);

  if (!our_complex && !their_complex) {
    return bounds_intersect;
  }

  if (!bounds_intersect) {
    return false;
  }

  if (!our_complex) {
    return region.intersects(bounds_);
  }

  if (!their_complex) {
    return intersects(region.bounds_);
  }

  auto ours = lines_.begin();
  auto theirs = region.lines_.begin();

  while (ours != lines_.end() && theirs != region.lines_.end()) {
    if (ours->bottom <= theirs->top) {
      ++ours;
    } else if (theirs->bottom <= ours->top) {
      ++theirs;
    } else {
      FML_DCHECK(ours->top < theirs->bottom || theirs->top < ours->bottom);
      const Span *ours_begin, *ours_end;
      span_buffer_.getSpans(ours->chunk_handle, ours_begin, ours_end);
      const Span *theirs_begin, *theirs_end;
      region.span_buffer_.getSpans(theirs->chunk_handle, theirs_begin,
                                   theirs_end);
      if (spansIntersect(ours_begin, ours_end, theirs_begin, theirs_end)) {
        return true;
      }
      if (ours->bottom < theirs->bottom) {
        ++ours;
      } else {
        ++theirs;
      }
    }
  }
  return false;
}

}  // namespace flutter
