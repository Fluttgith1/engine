// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/testing/mock_im_context.h"

using namespace flutter::testing;

G_DECLARE_FINAL_TYPE(FlMockIMContext,
                     fl_mock_im_context,
                     FL,
                     MOCK_IM_CONTEXT,
                     GtkIMContext)

struct _FlMockIMContext {
  GtkIMContext parent_instance;
  MockIMContext* mock;
};

G_DEFINE_TYPE(FlMockIMContext, fl_mock_im_context, GTK_TYPE_IM_CONTEXT)

static void fl_mock_im_context_set_client_widget(GtkIMContext* context,
                                                 GtkWidget* widget) {
  FlMockIMContext* self = FL_MOCK_IM_CONTEXT(context);
  self->mock->gtk_im_context_set_client_widget(context, widget);
}

static void fl_mock_im_context_get_preedit_string(GtkIMContext* context,
                                                  char** str,
                                                  PangoAttrList** attrs,
                                                  int* cursor_pos) {
  FlMockIMContext* self = FL_MOCK_IM_CONTEXT(context);
  self->mock->gtk_im_context_get_preedit_string(context, str, attrs,
                                                cursor_pos);
}

static gboolean fl_mock_im_context_filter_keypress(GtkIMContext* context,
                                                   GdkEvent* event) {
  FlMockIMContext* self = FL_MOCK_IM_CONTEXT(context);
  return self->mock->gtk_im_context_filter_keypress(context, event);
}

static void fl_mock_im_context_focus_in(GtkIMContext* context) {
  FlMockIMContext* self = FL_MOCK_IM_CONTEXT(context);
  self->mock->gtk_im_context_focus_in(context);
}

static void fl_mock_im_context_focus_out(GtkIMContext* context) {
  FlMockIMContext* self = FL_MOCK_IM_CONTEXT(context);
  self->mock->gtk_im_context_focus_out(context);
}

static void fl_mock_im_context_reset(GtkIMContext* context) {
  FlMockIMContext* self = FL_MOCK_IM_CONTEXT(context);
  self->mock->gtk_im_context_reset(context);
}

static void fl_mock_im_context_set_cursor_location(GtkIMContext* context,
                                                   GdkRectangle* area) {
  FlMockIMContext* self = FL_MOCK_IM_CONTEXT(context);
  self->mock->gtk_im_context_set_cursor_location(context, area);
}

static void fl_mock_im_context_set_use_preedit(GtkIMContext* context,
                                               gboolean use_preedit) {
  FlMockIMContext* self = FL_MOCK_IM_CONTEXT(context);
  self->mock->gtk_im_context_set_use_preedit(context, use_preedit);
}

static void fl_mock_im_context_set_surrounding_with_selection(
    GtkIMContext* context,
    const char* text,
    int len,
    int cursor_index,
    int anchor_index) {
  FlMockIMContext* self = FL_MOCK_IM_CONTEXT(context);
  self->mock->gtk_im_context_set_surrounding_with_selection(
      context, text, len, cursor_index, anchor_index);
}

static gboolean fl_mock_im_context_get_surrounding_with_selection(
    GtkIMContext* context,
    char** text,
    int* cursor_index,
    int* anchor_index) {
  FlMockIMContext* self = FL_MOCK_IM_CONTEXT(context);
  return self->mock->gtk_im_context_get_surrounding_with_selection(
      context, text, cursor_index, anchor_index);
}

static void fl_mock_im_context_class_init(FlMockIMContextClass* klass) {
  GtkIMContextClass* im_context_class = GTK_IM_CONTEXT_CLASS(klass);
  im_context_class->set_client_widget = fl_mock_im_context_set_client_widget;
  im_context_class->get_preedit_string = fl_mock_im_context_get_preedit_string;
  im_context_class->filter_keypress = fl_mock_im_context_filter_keypress;
  im_context_class->focus_in = fl_mock_im_context_focus_in;
  im_context_class->focus_out = fl_mock_im_context_focus_out;
  im_context_class->reset = fl_mock_im_context_reset;
  im_context_class->set_cursor_location =
      fl_mock_im_context_set_cursor_location;
  im_context_class->set_use_preedit = fl_mock_im_context_set_use_preedit;
  im_context_class->set_surrounding_with_selection =
      fl_mock_im_context_set_surrounding_with_selection;
  im_context_class->get_surrounding_with_selection =
      fl_mock_im_context_get_surrounding_with_selection;
}

static void fl_mock_im_context_init(FlMockIMContext* self) {}

static GtkIMContext* fl_mock_im_context_new(MockIMContext* mock) {
  FlMockIMContext* self =
      FL_MOCK_IM_CONTEXT(g_object_new(fl_mock_im_context_get_type(), nullptr));
  self->mock = mock;
  return GTK_IM_CONTEXT(self);
}

MockIMContext::~MockIMContext() {
  if (FL_IS_MOCK_IM_CONTEXT(instance_)) {
    g_clear_object(&instance_);
  }
}

MockIMContext::operator GtkIMContext*() {
  if (instance_ == nullptr) {
    instance_ = fl_mock_im_context_new(this);
  }
  return instance_;
}
