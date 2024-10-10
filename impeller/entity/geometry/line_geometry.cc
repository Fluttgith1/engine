// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/geometry/line_geometry.h"
#include "impeller/core/formats.h"
#include "impeller/entity/geometry/geometry.h"
#include "impeller/geometry/path.h"

namespace impeller {

LineGeometry::LineGeometry(Point p0, Point p1, Scalar width, Cap cap)
    : p0_(p0), p1_(p1), width_(width), cap_(cap) {
  FML_DCHECK(width >= 0);
}

std::pair<Scalar, bool> LineGeometry::ComputePixelHalfWidth(Scalar max_basis,
                                                            Scalar width) {
  Scalar min_size = kMinStrokeSize / max_basis;
  return std::make_pair(std::max(width, min_size) * 0.5f, width <= min_size);
}

Vector2 LineGeometry::ComputeAlongVector(Scalar max_basis,
                                         bool allow_zero_length) const {
  auto [stroke_half_width, _] = ComputePixelHalfWidth(max_basis, width_);
  if (stroke_half_width < kEhCloseEnough) {
    return {};
  }

  auto along = p1_ - p0_;
  Scalar length = along.GetLength();
  if (length < kEhCloseEnough) {
    if (!allow_zero_length) {
      // We won't enclose any pixels unless the endpoints are extended
      return {};
    }
    return {stroke_half_width, 0};
  } else {
    return along * stroke_half_width / length;
  }
}

bool LineGeometry::ComputeCorners(Point corners[4],
                                  Scalar max_basis,
                                  bool extend_endpoints) const {
  auto along = ComputeAlongVector(max_basis, extend_endpoints);
  if (along.IsZero()) {
    return false;
  }

  auto across = Vector2(along.y, -along.x);
  corners[0] = p0_ - across;
  corners[1] = p1_ - across;
  corners[2] = p0_ + across;
  corners[3] = p1_ + across;
  if (extend_endpoints) {
    corners[0] -= along;
    corners[1] += along;
    corners[2] -= along;
    corners[3] += along;
  }
  return true;
}

Scalar LineGeometry::ComputeAlphaCoverage(const Matrix& entity) const {
  return Geometry::ComputeStrokeAlphaCoverage(entity, width_);
}

GeometryResult LineGeometry::GetPositionBuffer(const ContentContext& renderer,
                                               const Entity& entity,
                                               RenderPass& pass) const {
  using VT = SolidFillVertexShader::PerVertexData;

  Scalar max_basis = entity.GetTransform().GetMaxBasisLengthXY();
  auto& host_buffer = renderer.GetTransientsBuffer();

  if (max_basis == 0) {
    return {};
  }

  auto [radius, is_harline] = ComputePixelHalfWidth(max_basis, width_);

  // This is a harline stroke and can be drawn directly with line primitives,
  // which avoids extra tessellation work, cap/joins, and overdraw prevention.
  // TODO(jonahwilliams): round and square caps would require us to extend the
  // primitive by a half pixel in each direction.
  if (is_harline && cap_ == Cap::kButt) {
    Point points[2] = {p0_, p1_};
    return GeometryResult{
        .type = PrimitiveType::kLineStrip,
        .vertex_buffer = {.vertex_buffer = host_buffer.Emplace(
                              points, sizeof(points), alignof(Point)),
                          .vertex_count = 2,
                          .index_type = IndexType::kNone},
        .transform = entity.GetShaderTransform(pass),
    };
  }

  if (cap_ == Cap::kRound) {
    auto& transform = entity.GetTransform();
    std::shared_ptr<Tessellator> tessellator = renderer.GetTessellator();
    auto generator = tessellator->RoundCapLine(transform, p0_, p1_, radius);
    return ComputePositionGeometry(renderer, generator, entity, pass);
  }

  Point corners[4];
  if (!ComputeCorners(corners, max_basis, cap_ == Cap::kSquare)) {
    return kEmptyResult;
  }

  size_t count = 4;
  BufferView vertex_buffer = host_buffer.Emplace(
      count * sizeof(VT), alignof(VT), [&corners](uint8_t* buffer) {
        auto vertices = reinterpret_cast<VT*>(buffer);
        for (auto& corner : corners) {
          *vertices++ = {
              .position = corner,
          };
        }
      });

  return GeometryResult{
      .type = PrimitiveType::kTriangleStrip,
      .vertex_buffer =
          {
              .vertex_buffer = vertex_buffer,
              .vertex_count = count,
              .index_type = IndexType::kNone,
          },
      .transform = entity.GetShaderTransform(pass),
  };
}

std::optional<Rect> LineGeometry::GetCoverage(const Matrix& transform) const {
  Point corners[4];
  if (!ComputeCorners(corners, transform.GetMaxBasisLengthXY(),
                      cap_ != Cap::kButt)) {
    return {};
  }

  for (int i = 0; i < 4; i++) {
    corners[i] = transform * corners[i];
  }
  return Rect::MakePointBounds(std::begin(corners), std::end(corners));
}

bool LineGeometry::CoversArea(const Matrix& transform, const Rect& rect) const {
  if (!transform.IsTranslationScaleOnly() || !IsAxisAlignedRect()) {
    return false;
  }
  auto coverage = GetCoverage(transform);
  return coverage.has_value() ? coverage->Contains(rect) : false;
}

bool LineGeometry::IsAxisAlignedRect() const {
  return cap_ != Cap::kRound && (p0_.x == p1_.x || p0_.y == p1_.y);
}

}  // namespace impeller
