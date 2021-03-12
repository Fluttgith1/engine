// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_view.h"

#include "flutter/shell/platform/linux/fl_view_private.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <vector>

#include "flutter/shell/platform/linux/fl_accessibility_plugin.h"
#include "flutter/shell/platform/linux/fl_clipping_view.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/fl_gesture_helper.h"
#include "flutter/shell/platform/linux/fl_key_event_plugin.h"
#include "flutter/shell/platform/linux/fl_mouse_cursor_plugin.h"
#include "flutter/shell/platform/linux/fl_platform_plugin.h"
#include "flutter/shell/platform/linux/fl_platform_views_plugin.h"
#include "flutter/shell/platform/linux/fl_plugin_registrar_private.h"
#include "flutter/shell/platform/linux/fl_renderer_gl.h"
#include "flutter/shell/platform/linux/fl_text_input_plugin.h"
#include "flutter/shell/platform/linux/fl_view_accessible.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_engine.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_plugin_registry.h"

static constexpr int kMicrosecondsPerMillisecond = 1000;

struct _FlView {
  GtkContainer parent_instance;

  // Project being run.
  FlDartProject* project;

  // Rendering output.
  FlRenderer* renderer;

  // Engine running @project.
  FlEngine* engine;

  // Pointer button state recorded for sending status updates.
  int64_t button_state;

  // Flutter system channel handlers.
  FlAccessibilityPlugin* accessibility_plugin;
  FlKeyEventPlugin* key_event_plugin;
  FlMouseCursorPlugin* mouse_cursor_plugin;
  FlPlatformPlugin* platform_plugin;
  FlTextInputPlugin* text_input_plugin;
  FlPlatformViewsPlugin* platform_views_plugin;

  FlGestureHelper* gesture_helper;

  GList* gl_area_list;
  GList* used_area_list;

  std::map<GtkWidget*, FlClippingView*>* current_clipping_views;
  std::map<GtkWidget*, FlClippingView*>* last_clipping_views;

  GtkWidget* event_box;

  std::vector<GtkWidget*>* children_list;
  std::vector<GtkWidget*>* pending_children_list;
};

enum { PROP_FLUTTER_PROJECT = 1, PROP_LAST };

static void fl_view_plugin_registry_iface_init(
    FlPluginRegistryInterface* iface);

G_DEFINE_TYPE_WITH_CODE(
    FlView,
    fl_view,
    GTK_TYPE_CONTAINER,
    G_IMPLEMENT_INTERFACE(fl_plugin_registry_get_type(),
                          fl_view_plugin_registry_iface_init))

static void fl_view_update_semantics_node_cb(FlEngine* engine,
                                             const FlutterSemanticsNode* node,
                                             gpointer user_data) {
  FlView* self = FL_VIEW(user_data);

  fl_accessibility_plugin_handle_update_semantics_node(
      self->accessibility_plugin, node);
}

// Converts a GDK button event into a Flutter event and sends it to the engine.
static gboolean fl_view_send_pointer_button_event(FlView* self,
                                                  GdkEventButton* event) {
  int64_t button;
  switch (event->button) {
    case 1:
      button = kFlutterPointerButtonMousePrimary;
      break;
    case 2:
      button = kFlutterPointerButtonMouseMiddle;
      break;
    case 3:
      button = kFlutterPointerButtonMouseSecondary;
      break;
    default:
      return FALSE;
  }
  int old_button_state = self->button_state;
  FlutterPointerPhase phase = kMove;
  if (event->type == GDK_BUTTON_PRESS) {
    // Drop the event if Flutter already thinks the button is down.
    if ((self->button_state & button) != 0) {
      return FALSE;
    }
    self->button_state ^= button;

    phase = old_button_state == 0 ? kDown : kMove;
  } else if (event->type == GDK_BUTTON_RELEASE) {
    // Drop the event if Flutter already thinks the button is up.
    if ((self->button_state & button) == 0) {
      return FALSE;
    }
    self->button_state ^= button;

    phase = self->button_state == 0 ? kUp : kMove;
  }

  if (self->engine == nullptr) {
    return FALSE;
  }

  gint scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(self));
  fl_engine_send_mouse_pointer_event(
      self->engine, phase, event->time * kMicrosecondsPerMillisecond,
      event->x * scale_factor, event->y * scale_factor, 0, 0,
      self->button_state);

  return TRUE;
}

// Updates the engine with the current window metrics.
static void fl_view_geometry_changed(FlView* self) {
  GtkAllocation allocation;
  gtk_widget_get_allocation(GTK_WIDGET(self), &allocation);
  gint scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(self));
  fl_engine_send_window_metrics_event(
      self->engine, allocation.width * scale_factor,
      allocation.height * scale_factor, scale_factor);
}

// Implements FlPluginRegistry::get_registrar_for_plugin.
static FlPluginRegistrar* fl_view_get_registrar_for_plugin(
    FlPluginRegistry* registry,
    const gchar* name) {
  FlView* self = FL_VIEW(registry);

  return fl_plugin_registrar_new(self,
                                 fl_engine_get_binary_messenger(self->engine));
}

static void fl_view_plugin_registry_iface_init(
    FlPluginRegistryInterface* iface) {
  iface->get_registrar_for_plugin = fl_view_get_registrar_for_plugin;
}

static gboolean event_box_button_release_event(GtkWidget* widget,
                                               GdkEventButton* event,
                                               FlView* view);

static gboolean event_box_button_press_event(GtkWidget* widget,
                                             GdkEventButton* event,
                                             FlView* view);

static gboolean event_box_scroll_event(GtkWidget* widget,
                                       GdkEventScroll* event,
                                       FlView* view);

static gboolean event_box_motion_notify_event(GtkWidget* widget,
                                              GdkEventMotion* event,
                                              FlView* view);

static void fl_view_constructed(GObject* object) {
  FlView* self = FL_VIEW(object);

  self->renderer = FL_RENDERER(fl_renderer_gl_new());
  self->engine = fl_engine_new(self->project, self->renderer);
  self->gesture_helper = fl_gesture_helper_new();

  fl_engine_set_update_semantics_node_handler(
      self->engine, fl_view_update_semantics_node_cb, self, nullptr);

  // Create system channel handlers.
  FlBinaryMessenger* messenger = fl_engine_get_binary_messenger(self->engine);
  self->accessibility_plugin = fl_accessibility_plugin_new(self);
  self->text_input_plugin = fl_text_input_plugin_new(messenger, self);
  self->key_event_plugin =
      fl_key_event_plugin_new(messenger, self->text_input_plugin);
  self->mouse_cursor_plugin = fl_mouse_cursor_plugin_new(messenger, self);
  self->platform_plugin = fl_platform_plugin_new(messenger);
  self->platform_views_plugin =
      fl_platform_views_plugin_new(messenger, self->gesture_helper);

  self->children_list = new std::vector<GtkWidget*>();
  self->pending_children_list = new std::vector<GtkWidget*>();
  self->current_clipping_views = new std::map<GtkWidget*, FlClippingView*>();
  self->last_clipping_views = new std::map<GtkWidget*, FlClippingView*>();

  self->event_box = gtk_event_box_new();
  gtk_widget_set_parent(self->event_box, GTK_WIDGET(self));
  gtk_widget_show(self->event_box);
  gtk_widget_add_events(self->event_box,
                        GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK |
                            GDK_SMOOTH_SCROLL_MASK);

  g_signal_connect(self->event_box, "button-press-event",
                   G_CALLBACK(event_box_button_press_event), self);
  g_signal_connect(self->event_box, "button-release-event",
                   G_CALLBACK(event_box_button_release_event), self);
  g_signal_connect(self->event_box, "scroll-event",
                   G_CALLBACK(event_box_scroll_event), self);
  g_signal_connect(self->event_box, "motion-notify-event",
                   G_CALLBACK(event_box_motion_notify_event), self);
}

static void fl_view_set_property(GObject* object,
                                 guint prop_id,
                                 const GValue* value,
                                 GParamSpec* pspec) {
  FlView* self = FL_VIEW(object);

  switch (prop_id) {
    case PROP_FLUTTER_PROJECT:
      g_set_object(&self->project,
                   static_cast<FlDartProject*>(g_value_get_object(value)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void fl_view_get_property(GObject* object,
                                 guint prop_id,
                                 GValue* value,
                                 GParamSpec* pspec) {
  FlView* self = FL_VIEW(object);

  switch (prop_id) {
    case PROP_FLUTTER_PROJECT:
      g_value_set_object(value, self->project);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void fl_view_notify(GObject* object, GParamSpec* pspec) {
  FlView* self = FL_VIEW(object);

  if (strcmp(pspec->name, "scale-factor") == 0) {
    fl_view_geometry_changed(self);
  }

  if (G_OBJECT_CLASS(fl_view_parent_class)->notify != nullptr) {
    G_OBJECT_CLASS(fl_view_parent_class)->notify(object, pspec);
  }
}

static void fl_view_dispose(GObject* object) {
  FlView* self = FL_VIEW(object);

  if (self->engine != nullptr) {
    fl_engine_set_update_semantics_node_handler(self->engine, nullptr, nullptr,
                                                nullptr);
  }

  g_clear_object(&self->project);
  g_clear_object(&self->renderer);
  g_clear_object(&self->engine);
  g_clear_object(&self->accessibility_plugin);
  g_clear_object(&self->key_event_plugin);
  g_clear_object(&self->mouse_cursor_plugin);
  g_clear_object(&self->platform_plugin);
  g_clear_object(&self->text_input_plugin);
  g_clear_object(&self->platform_views_plugin);
  g_clear_object(&self->gesture_helper);
  g_list_free_full(self->gl_area_list, g_object_unref);
  self->gl_area_list = nullptr;

  delete self->children_list;
  delete self->pending_children_list;
  delete self->current_clipping_views;
  delete self->last_clipping_views;

  G_OBJECT_CLASS(fl_view_parent_class)->dispose(object);
}

// Implements GtkWidget::realize.
static void fl_view_realize(GtkWidget* widget) {
  FlView* self = FL_VIEW(widget);
  g_autoptr(GError) error = nullptr;

  GtkAllocation allocation;
  gtk_widget_get_allocation(widget, &allocation);

  gtk_widget_set_realized(widget, TRUE);

  GdkWindowAttr attributes;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual(widget);
  attributes.event_mask =
      gtk_widget_get_events(widget) | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK;
  gint attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
  GdkWindow* window = gdk_window_new(gtk_widget_get_parent_window(widget),
                                     &attributes, attributes_mask);
  gtk_widget_set_window(widget, window);
  gtk_widget_register_window(widget, window);

  if (!fl_renderer_start(self->renderer, self, &error)) {
    g_warning("Failed to start Flutter renderer: %s", error->message);
    return;
  }

  if (!fl_engine_start(self->engine, &error)) {
    g_warning("Failed to start Flutter engine: %s", error->message);
    return;
  }
}

// Implements GtkWidget::size-allocate.
static void fl_view_size_allocate(GtkWidget* widget,
                                  GtkAllocation* allocation) {
  FlView* self = FL_VIEW(widget);

  gtk_widget_set_allocation(widget, allocation);

  if (gtk_widget_get_has_window(widget)) {
    if (gtk_widget_get_realized(widget))
      gdk_window_move_resize(gtk_widget_get_window(widget), allocation->x,
                             allocation->y, allocation->width,
                             allocation->height);
  }

  GtkAllocation default_child_allocation = {
      .x = 0,
      .y = 0,
      .width = allocation->width,
      .height = allocation->height,
  };

  for (GtkWidget* child : *self->children_list) {
    if (!gtk_widget_get_visible(child))
      continue;

    GtkAllocation child_allocation = default_child_allocation;
    if (!gtk_widget_get_has_window(widget)) {
      child_allocation.x += allocation->x;
      child_allocation.y += allocation->y;
    }

    gtk_widget_size_allocate(child, &child_allocation);
  }

  GtkAllocation event_box_allocation = default_child_allocation;
  if (!gtk_widget_get_has_window(self->event_box)) {
    event_box_allocation.x += allocation->x;
    event_box_allocation.y += allocation->y;
  }
  gtk_widget_size_allocate(self->event_box, &event_box_allocation);

  fl_view_geometry_changed(self);
}

struct _ReorderData {
  GdkWindow* parent_window;
  GdkWindow* last_window;
};

static void fl_view_reorder_forall(GtkWidget* widget, gpointer user_data) {
  _ReorderData* data = reinterpret_cast<_ReorderData*>(user_data);
  GdkWindow* window = gtk_widget_get_window(widget);
  if (window && window != data->parent_window) {
    if (data->last_window) {
      gdk_window_restack(window, data->last_window, TRUE);
    }
    data->last_window = window;
  }
}

static gboolean event_box_button_press_event(GtkWidget* widget,
                                             GdkEventButton* event,
                                             FlView* view) {
  // Flutter doesn't handle double and triple click events.
  if (event->type == GDK_DOUBLE_BUTTON_PRESS ||
      event->type == GDK_TRIPLE_BUTTON_PRESS) {
    return FALSE;
  }

  fl_gesture_helper_button_press(view->gesture_helper,
                                 reinterpret_cast<GdkEvent*>(event));

  if (!gtk_widget_has_focus(GTK_WIDGET(view))) {
    gtk_widget_grab_focus(GTK_WIDGET(view));
  }

  return fl_view_send_pointer_button_event(view, event);
}

static gboolean event_box_button_release_event(GtkWidget* widget,
                                               GdkEventButton* event,
                                               FlView* view) {
  fl_gesture_helper_button_release(view->gesture_helper,
                                   reinterpret_cast<GdkEvent*>(event));
  return fl_view_send_pointer_button_event(view, event);
}

static gboolean event_box_scroll_event(GtkWidget* widget,
                                       GdkEventScroll* event,
                                       FlView* view) {
  // TODO(robert-ancell): Update to use GtkEventControllerScroll when we can
  // depend on GTK 3.24.

  gdouble scroll_delta_x = 0.0, scroll_delta_y = 0.0;
  if (event->direction == GDK_SCROLL_SMOOTH) {
    scroll_delta_x = event->delta_x;
    scroll_delta_y = event->delta_y;
  } else {
    // We currently skip non-smooth scroll events due to the X11 events being
    // delivered directly to the FlView X window and bypassing the GtkWindow
    // handling.
    // This causes both smooth and non-smooth events to be received (i.e.
    // duplication).
    // https://github.com/flutter/flutter/issues/73823
    return FALSE;
  }

  fl_gesture_helper_scroll(view->gesture_helper,
                           reinterpret_cast<GdkEvent*>(event));

  // The multiplier is taken from the Chromium source
  // (ui/events/x/events_x_utils.cc).
  const int kScrollOffsetMultiplier = 53;
  scroll_delta_x *= kScrollOffsetMultiplier;
  scroll_delta_y *= kScrollOffsetMultiplier;

  gint scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(view));
  fl_engine_send_mouse_pointer_event(
      view->engine, view->button_state != 0 ? kMove : kHover,
      event->time * kMicrosecondsPerMillisecond, event->x * scale_factor,
      event->y * scale_factor, scroll_delta_x, scroll_delta_y,
      view->button_state);

  return TRUE;
}

static gboolean event_box_motion_notify_event(GtkWidget* widget,
                                              GdkEventMotion* event,
                                              FlView* view) {
  if (view->engine == nullptr) {
    return FALSE;
  }

  fl_gesture_helper_button_motion(view->gesture_helper,
                                  reinterpret_cast<GdkEvent*>(event));

  gint scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(view));
  fl_engine_send_mouse_pointer_event(
      view->engine, view->button_state != 0 ? kMove : kHover,
      event->time * kMicrosecondsPerMillisecond, event->x * scale_factor,
      event->y * scale_factor, 0, 0, view->button_state);

  return TRUE;
}

// Implements GtkWidget::key_press_event.
static gboolean fl_view_key_press_event(GtkWidget* widget, GdkEventKey* event) {
  FlView* self = FL_VIEW(widget);

  return fl_key_event_plugin_send_key_event(self->key_event_plugin, event);
}

// Implements GtkWidget::key_release_event.
static gboolean fl_view_key_release_event(GtkWidget* widget,
                                          GdkEventKey* event) {
  FlView* self = FL_VIEW(widget);

  return fl_key_event_plugin_send_key_event(self->key_event_plugin, event);
}

static void fl_view_put(FlView* self, GtkWidget* widget) {
  gtk_widget_set_parent(widget, GTK_WIDGET(self));
  self->children_list->push_back(widget);
}

static void fl_view_add(GtkContainer* container, GtkWidget* widget) {
  fl_view_put(FL_VIEW(container), widget);
}

static void fl_view_remove(GtkContainer* container, GtkWidget* widget) {
  FlView* self = FL_VIEW(container);
  for (auto child = self->children_list->begin();
       child != self->children_list->end(); child++) {
    if (*child == widget) {
      g_object_ref(widget);  // children_list does not own widgets.
      gtk_widget_unparent(widget);
      self->children_list->erase(child);
      break;
    }
  }

  if (widget == GTK_WIDGET(self->event_box)) {
    g_clear_object(&self->event_box);
  }
}

static void fl_view_forall(GtkContainer* container,
                           gboolean include_internals,
                           GtkCallback callback,
                           gpointer callback_data) {
  FlView* self = FL_VIEW(container);
  for (GtkWidget* child : *self->children_list) {
    (*callback)(child, callback_data);
  }

  if (include_internals) {
    (*callback)(self->event_box, callback_data);
  }
}

static GType fl_view_child_type(GtkContainer* container) {
  return GTK_TYPE_WIDGET;
}

static void fl_view_set_child_property(GtkContainer* container,
                                       GtkWidget* child,
                                       guint property_id,
                                       const GValue* value,
                                       GParamSpec* pspec) {}

static void fl_view_get_child_property(GtkContainer* container,
                                       GtkWidget* child,
                                       guint property_id,
                                       GValue* value,
                                       GParamSpec* pspec) {}

static void fl_view_class_init(FlViewClass* klass) {
  GObjectClass* object_class = G_OBJECT_CLASS(klass);
  object_class->constructed = fl_view_constructed;
  object_class->set_property = fl_view_set_property;
  object_class->get_property = fl_view_get_property;
  object_class->notify = fl_view_notify;
  object_class->dispose = fl_view_dispose;

  GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);
  widget_class->realize = fl_view_realize;
  widget_class->size_allocate = fl_view_size_allocate;
  widget_class->key_press_event = fl_view_key_press_event;
  widget_class->key_release_event = fl_view_key_release_event;

  GtkContainerClass* container_class = GTK_CONTAINER_CLASS(klass);
  container_class->add = fl_view_add;
  container_class->remove = fl_view_remove;
  container_class->forall = fl_view_forall;
  container_class->child_type = fl_view_child_type;
  container_class->set_child_property = fl_view_set_child_property;
  container_class->get_child_property = fl_view_get_child_property;

  g_object_class_install_property(
      G_OBJECT_CLASS(klass), PROP_FLUTTER_PROJECT,
      g_param_spec_object(
          "flutter-project", "flutter-project", "Flutter project in use",
          fl_dart_project_get_type(),
          static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
                                   G_PARAM_STATIC_STRINGS)));

  gtk_widget_class_set_accessible_type(GTK_WIDGET_CLASS(klass),
                                       fl_view_accessible_get_type());
}

static void fl_view_init(FlView* self) {
  gtk_widget_set_can_focus(GTK_WIDGET(self), TRUE);
}

G_MODULE_EXPORT FlView* fl_view_new(FlDartProject* project) {
  return static_cast<FlView*>(
      g_object_new(fl_view_get_type(), "flutter-project", project, nullptr));
}

G_MODULE_EXPORT FlEngine* fl_view_get_engine(FlView* view) {
  g_return_val_if_fail(FL_IS_VIEW(view), nullptr);
  return view->engine;
}

FlPlatformViewsPlugin* fl_view_get_platform_views_plugin(FlView* self) {
  g_return_val_if_fail(FL_IS_VIEW(self), FALSE);
  return self->platform_views_plugin;
}

void fl_view_begin_frame(FlView* view) {
  g_return_if_fail(FL_IS_VIEW(view));
  FlView* self = FL_VIEW(view);

  self->used_area_list = self->gl_area_list;
  self->pending_children_list->clear();
}

static void fl_view_add_pending_child(FlView* self, GtkWidget* child) {
  self->pending_children_list->push_back(child);
}

void fl_view_add_gl_area(FlView* view,
                         GdkGLContext* context,
                         FlBackingStoreProvider* texture) {
  g_return_if_fail(FL_IS_VIEW(view));

  FlGLArea* area;
  if (view->used_area_list) {
    area = reinterpret_cast<FlGLArea*>(view->used_area_list->data);
    view->used_area_list = view->used_area_list->next;
  } else {
    area = FL_GL_AREA(fl_gl_area_new(context));
    view->gl_area_list = g_list_append(view->gl_area_list, area);
  }

  gtk_widget_show(GTK_WIDGET(area));
  fl_view_add_pending_child(view, GTK_WIDGET(area));
  fl_gl_area_queue_render(area, texture);
}

void fl_view_add_widget(FlView* view,
                        GtkWidget* widget,
                        GdkRectangle* geometry,
                        GPtrArray* mutations) {
  FlClippingView* clipping_view;
  if (view->last_clipping_views->count(widget)) {
    clipping_view = view->last_clipping_views->at(widget);
    view->last_clipping_views->erase(widget);
  } else {
    clipping_view = FL_CLIPPING_VIEW(fl_clipping_view_new());
  }
  view->current_clipping_views->insert({widget, clipping_view});
  fl_clipping_view_reset(clipping_view, widget, geometry, mutations);

  gtk_widget_show_all(GTK_WIDGET(clipping_view));
  fl_view_add_pending_child(view, GTK_WIDGET(clipping_view));
}

void fl_view_end_frame(FlView* self) {
  for (GtkWidget* pending_child : *self->pending_children_list) {
    auto child = std::find(self->children_list->begin(),
                           self->children_list->end(), pending_child);

    if (child != self->children_list->end()) {
      // existing child
      *child = nullptr;
    } else {
      // newly added child
      gtk_widget_set_parent(pending_child, GTK_WIDGET(self));
    }
  }

  for (GtkWidget* child : *self->children_list) {
    if (child) {
      // removed child
      g_object_ref(child);
      gtk_widget_unparent(child);
    }
  }

  self->children_list->clear();
  std::swap(self->children_list, self->pending_children_list);

  for (auto& it : *self->last_clipping_views) {
    g_object_unref(it.second);
  }
  self->last_clipping_views->clear();
  std::swap(self->last_clipping_views, self->current_clipping_views);

  struct _ReorderData data = {
      .parent_window = gtk_widget_get_window(GTK_WIDGET(view)),
      .last_window = nullptr,
  };

  gtk_container_forall(GTK_CONTAINER(view), fl_view_reorder_forall, &data);

  gtk_widget_queue_draw(GTK_WIDGET(self));
}
