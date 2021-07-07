// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_registrar.h"
#include "flutter/shell/platform/linux/fl_texture_registrar_private.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_pixel_buffer_texture.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_gl.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"
#include "gtest/gtest.h"

#include <epoxy/gl.h>

#include <gmodule.h>

static constexpr uint32_t BUFFER_WIDTH = 4u;
static constexpr uint32_t BUFFER_HEIGHT = 4u;
static constexpr uint32_t REAL_BUFFER_WIDTH = 2u;
static constexpr uint32_t REAL_BUFFER_HEIGHT = 2u;

G_DECLARE_FINAL_TYPE(FlTestRegistrarTexture,
                     fl_test_registrar_texture,
                     FL,
                     TEST_REGISTRAR_TEXTURE,
                     FlTextureGL)

/// A simple texture.
struct _FlTestRegistrarTexture {
  FlTextureGL parent_instance;
};

G_DEFINE_TYPE(FlTestRegistrarTexture,
              fl_test_registrar_texture,
              fl_texture_gl_get_type())

static gboolean fl_test_registrar_texture_populate(FlTextureGL* texture,
                                                   uint32_t* target,
                                                   uint32_t* format,
                                                   uint32_t* width,
                                                   uint32_t* height,
                                                   GError** error) {
  EXPECT_TRUE(FL_IS_TEST_REGISTRAR_TEXTURE(texture));

  EXPECT_EQ(*width, BUFFER_WIDTH);
  EXPECT_EQ(*height, BUFFER_HEIGHT);
  *target = GL_TEXTURE_2D;
  *format = GL_R8;
  *width = REAL_BUFFER_WIDTH;
  *height = REAL_BUFFER_HEIGHT;

  return TRUE;
}

static void fl_test_registrar_texture_class_init(
    FlTestRegistrarTextureClass* klass) {
  FL_TEXTURE_GL_CLASS(klass)->populate = fl_test_registrar_texture_populate;
}

static void fl_test_registrar_texture_init(FlTestRegistrarTexture* self) {}

static FlTestRegistrarTexture* fl_test_registrar_texture_new() {
  return FL_TEST_REGISTRAR_TEXTURE(
      g_object_new(fl_test_registrar_texture_get_type(), nullptr));
}

// Test that registering a texture works.
TEST(FlTextureRegistrarTest, RegisterTexture) {
  g_autoptr(FlEngine) engine = make_mock_engine();
  g_autoptr(FlTextureRegistrar) registrar = fl_texture_registrar_new(engine);
  g_autoptr(FlTexture) texture = FL_TEXTURE(fl_test_registrar_texture_new());
  int64_t id = fl_texture_registrar_register_texture(registrar, texture);

  EXPECT_EQ(fl_texture_registrar_get_texture(registrar, id), texture);

  fl_texture_registrar_unregister_texture(registrar, id);

  EXPECT_EQ(fl_texture_registrar_get_texture(registrar, id), nullptr);
}

// Test that marking a texture frame available works.
TEST(FlTextureRegistrarTest, MarkTextureFrameAvailable) {
  g_autoptr(FlEngine) engine = make_mock_engine();
  g_autoptr(FlTextureRegistrar) registrar = fl_texture_registrar_new(engine);
  g_autoptr(FlTexture) texture = FL_TEXTURE(fl_test_registrar_texture_new());
  int64_t id = fl_texture_registrar_register_texture(registrar, texture);
  fl_texture_registrar_mark_texture_frame_available(registrar, id);
}

// Test that populating an OpenGL texture works.
TEST(FlTextureRegistrarTest, PopulateTexture) {
  g_autoptr(FlEngine) engine = make_mock_engine();
  g_autoptr(FlTextureRegistrar) registrar = fl_texture_registrar_new(engine);
  g_autoptr(FlTexture) texture = FL_TEXTURE(fl_test_registrar_texture_new());
  int64_t id = fl_texture_registrar_register_texture(registrar, texture);
  FlutterOpenGLTexture opengl_texture;
  g_autoptr(GError) error = nullptr;
  EXPECT_TRUE(fl_texture_registrar_populate_gl_external_texture(
      registrar, id, BUFFER_WIDTH, BUFFER_HEIGHT, &opengl_texture, &error));
  EXPECT_EQ(error, nullptr);
  EXPECT_EQ(opengl_texture.width, REAL_BUFFER_WIDTH);
  EXPECT_EQ(opengl_texture.height, REAL_BUFFER_HEIGHT);
}
