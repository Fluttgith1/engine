// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_GL_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_GL_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <glib-object.h>
#include <stdint.h>
#include "fl_texture.h"

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(FlTextureGL, fl_texture_gl, FL, TEXTURE_GL, FlTexture)

/**
 * FlTextureGL:
 *
 * #FlTextureGL is an abstract class that represents an OpenGL texture.
 *
 * If you want to render textures in other OpenGL context, create and use the
 * #GdkGLContext by calling gdk_window_create_gl_context () with the #GdkWindow
 * of #FlView. The context will be shared with the one used by Flutter.
 */

struct _FlTextureGLClass {
  FlTextureClass parent_class;

  /**
   * Virtual method called when Flutter populates this texture. The OpenGL
   * context used by Flutter has been already set.
   * @texture: an #FlTexture.
   * @target: texture target (example GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE).
   * @name: (out): name of texture.
   * @width: (inout): width of the texture in pixels.
   * @height: (inout): height of the texture in pixels.
   * @error: (allow-none): #GError location to store the error occurring, or
   * %NULL to ignore.
   *
   * Returns: %TRUE on success.
   */
  gboolean (*populate)(FlTextureGL* texture,
                       uint32_t* target,
                       uint32_t* name,
                       uint32_t* width,
                       uint32_t* height,
                       GError** error);
};

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_H_
