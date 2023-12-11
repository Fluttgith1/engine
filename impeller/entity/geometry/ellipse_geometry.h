// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/entity/geometry/geometry.h"

namespace impeller {

// Geometry class that can generate vertices (with or without texture
// coordinates) for filled ellipses. Generating vertices for a stroked
// ellipse would require a lot more work since the line width must be
// applied perpendicular to the distorted ellipse shape.
class EllipseGeometry final : public Geometry {
 public:
  explicit EllipseGeometry(Rect bounds);

  ~EllipseGeometry() = default;

  // |Geometry|
  bool CoversArea(const Matrix& transform, const Rect& rect) const override;

  // |Geometry|
  bool IsAxisAlignedRect() const override;

 private:
  // |Geometry|
  GeometryResult GetPositionBuffer(const ContentContext& renderer,
                                   const Entity& entity,
                                   RenderPass& pass) const override;

  // |Geometry|
  GeometryVertexType GetVertexType() const override;

  // |Geometry|
  std::optional<Rect> GetCoverage(const Matrix& transform) const override;

  // |Geometry|
  GeometryResult GetPositionUVBuffer(Rect texture_coverage,
                                     Matrix effect_transform,
                                     const ContentContext& renderer,
                                     const Entity& entity,
                                     RenderPass& pass) const override;

  Rect bounds_;

  EllipseGeometry(const EllipseGeometry&) = delete;

  EllipseGeometry& operator=(const EllipseGeometry&) = delete;
};

}  // namespace impeller
