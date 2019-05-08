// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of engine;

/// Converts [path] to SVG path syntax to be used as "d" attribute in path
/// element.
void pathToSvg(ui.Path path, StringBuffer sb,
    {double offsetX = 0, double offsetY = 0}) {
  for (Subpath subPath in path.subpaths) {
    for (PathCommand command in subPath.commands) {
      switch (command.type) {
        case PathCommandTypes.moveTo:
          MoveTo moveTo = command;
          sb.write('M ${moveTo.x + offsetX} ${moveTo.y + offsetY}');
          break;
        case PathCommandTypes.lineTo:
          LineTo lineTo = command;
          sb.write('L ${lineTo.x + offsetX} ${lineTo.y + offsetY}');
          break;
        case PathCommandTypes.bezierCurveTo:
          BezierCurveTo curve = command;
          sb.write('C ${curve.x1 + offsetX} ${curve.y1 + offsetY} '
              '${curve.x2 + offsetX} ${curve.y2 + offsetY} ${curve.x3 + offsetX} ${curve.y3 + offsetY}');
          break;
        case PathCommandTypes.quadraticCurveTo:
          QuadraticCurveTo quadraticCurveTo = command;
          sb.write(
              'Q ${quadraticCurveTo.x1 + offsetX} ${quadraticCurveTo.y1 + offsetY} '
              '${quadraticCurveTo.x2 + offsetX} ${quadraticCurveTo.y2 + offsetY}');
          break;
        case PathCommandTypes.close:
          sb.write('Z');
          break;
        case PathCommandTypes.ellipse:
          Ellipse ellipse = command;
          // Handle edge case where start and end points are the same by drawing
          // 2 half arcs.
          if ((ellipse.endAngle - ellipse.startAngle) % (2 * math.pi) == 0.0) {
            _writeEllipse(
                sb,
                ellipse.x + offsetX,
                ellipse.y + offsetY,
                ellipse.radiusX,
                ellipse.radiusY,
                ellipse.rotation,
                -math.pi,
                0,
                ellipse.anticlockwise,
                moveToStartPoint: true);
            _writeEllipse(
                sb,
                ellipse.x + offsetX,
                ellipse.y + offsetY,
                ellipse.radiusX,
                ellipse.radiusY,
                ellipse.rotation,
                0,
                math.pi,
                ellipse.anticlockwise);
          } else {
            _writeEllipse(
                sb,
                ellipse.x + offsetX,
                ellipse.y + offsetY,
                ellipse.radiusX,
                ellipse.radiusY,
                ellipse.rotation,
                ellipse.startAngle,
                ellipse.endAngle,
                ellipse.anticlockwise);
          }
          break;
        case PathCommandTypes.rRect:
          RRectCommand rrectCommand = command;
          ui.RRect rrect = rrectCommand.rrect;
          var left = rrect.left + offsetX;
          var right = rrect.right + offsetX;
          var top = rrect.top + offsetY;
          var bottom = rrect.bottom + offsetY;
          if (left > right) {
            left = right;
            right = rrect.left + offsetX;
          }
          if (top > bottom) {
            top = bottom;
            bottom = rrect.top + offsetY;
          }
          var trRadiusX = rrect.trRadiusX.abs();
          var tlRadiusX = rrect.tlRadiusX.abs();
          var trRadiusY = rrect.trRadiusY.abs();
          var tlRadiusY = rrect.tlRadiusY.abs();
          var blRadiusX = rrect.blRadiusX.abs();
          var brRadiusX = rrect.brRadiusX.abs();
          var blRadiusY = rrect.blRadiusY.abs();
          var brRadiusY = rrect.brRadiusY.abs();

          sb.write('L ${left + trRadiusX} $top ');
          // Top side and top-right corner
          sb.write('M ${right - trRadiusX} $top ');
          _writeEllipse(sb, right - trRadiusX, top + trRadiusY, trRadiusX,
              trRadiusY, 0, 1.5 * math.pi, 2.0 * math.pi, false);
          // Right side and bottom-right corner
          sb.write('L $right ${bottom - brRadiusY} ');
          _writeEllipse(sb, right - brRadiusX, bottom - brRadiusY, brRadiusX,
              brRadiusY, 0, 0, 0.5 * math.pi, false);
          // Bottom side and bottom-left corner
          sb.write('L ${left + blRadiusX} $bottom ');
          _writeEllipse(sb, left + blRadiusX, bottom - blRadiusY, blRadiusX,
              blRadiusY, 0, 0.5 * math.pi, math.pi, false);
          // Left side and top-left corner
          sb.write('L $left ${top + tlRadiusY} ');
          _writeEllipse(
            sb,
            left + tlRadiusX,
            top + tlRadiusY,
            tlRadiusX,
            tlRadiusY,
            0,
            math.pi,
            1.5 * math.pi,
            false,
          );
          break;
        case PathCommandTypes.rect:
          RectCommand rectCommand = command;
          bool horizontalSwap = rectCommand.width < 0;
          final left = offsetX +
              (horizontalSwap
                  ? rectCommand.x - rectCommand.width
                  : rectCommand.x);
          final width = horizontalSwap ? -rectCommand.width : rectCommand.width;
          bool verticalSwap = rectCommand.height < 0;
          final top = offsetY +
              (verticalSwap
                  ? rectCommand.y - rectCommand.height
                  : rectCommand.y);
          final height =
              verticalSwap ? -rectCommand.height : rectCommand.height;
          sb.write('M $left $top ');
          sb.write('L ${left + width} $top ');
          sb.write('L ${left + width} ${top + height} ');
          sb.write('L $left ${top + height} ');
          sb.write('L $left $top ');
          break;
        default:
          throw new UnimplementedError('Unknown path command $command');
      }
    }
  }
}

// See https://www.w3.org/TR/SVG/implnote.html B.2.3. Conversion from center to
// endpoint parameterization.
void _writeEllipse(
    StringBuffer sb,
    double cx,
    double cy,
    double radiusX,
    double radiusY,
    double rotation,
    double startAngle,
    double endAngle,
    bool antiClockwise,
    {bool moveToStartPoint = false}) {
  double cosRotation = math.cos(rotation);
  double sinRotation = math.sin(rotation);
  double x = math.cos(startAngle) * radiusX;
  double y = math.sin(startAngle) * radiusY;

  double startPx = cx + (cosRotation * x - sinRotation * y);
  double startPy = cy + (sinRotation * x + cosRotation * y);

  double xe = math.cos(endAngle) * radiusX;
  double ye = math.sin(endAngle) * radiusY;

  double endPx = cx + (cosRotation * xe - sinRotation * ye);
  double endPy = cy + (sinRotation * xe + cosRotation * ye);

  double delta = endAngle - startAngle;
  bool largeArc = delta.abs() > math.pi;

  double rotationDeg = rotation / math.pi * 180.0;
  if (moveToStartPoint) {
    sb.write('M $startPx $startPy ');
  }
  sb.write('A $radiusX $radiusY ${rotationDeg} '
      '${largeArc ? 1 : 0} ${antiClockwise ? 0 : 1} $endPx $endPy');
}
