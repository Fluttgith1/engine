// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/fl_method_codec_private.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_method_response.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_method_codec.h"
#include "gtest/gtest.h"

#include <string.h>

struct _FlutterEngine {
  bool running;
  FlutterPlatformMessageCallback platform_message_callback;
  FlutterTaskRunnerPostTaskCallback platform_post_task_callback;
  void* user_data;

  _FlutterEngine(FlutterPlatformMessageCallback platform_message_callback,
                 FlutterTaskRunnerPostTaskCallback platform_post_task_callback,
                 void* user_data)
      : running(false),
        platform_message_callback(platform_message_callback),
        platform_post_task_callback(platform_post_task_callback),
        user_data(user_data) {}
};

struct _FlutterPlatformMessageResponseHandle {
  FlutterDataCallback data_callback;
  void* user_data;
  std::string channel;
  bool released;

  // Constructor for a response handle generated by the engine.
  _FlutterPlatformMessageResponseHandle(std::string channel)
      : data_callback(nullptr),
        user_data(nullptr),
        channel(channel),
        released(false) {}

  // Constructor for a response handle generated by the shell.
  _FlutterPlatformMessageResponseHandle(FlutterDataCallback data_callback,
                                        void* user_data)
      : data_callback(data_callback), user_data(user_data), released(false) {}
};

struct _FlutterTaskRunner {
  uint64_t task;
  std::string channel;
  const FlutterPlatformMessageResponseHandle* response_handle;
  uint8_t* message;
  size_t message_size;

  _FlutterTaskRunner(
      uint64_t task,
      const std::string& channel,
      const FlutterPlatformMessageResponseHandle* response_handle,
      const uint8_t* message,
      size_t message_size)
      : task(task),
        channel(channel),
        response_handle(response_handle),
        message_size(message_size) {
    this->message = static_cast<uint8_t*>(malloc(message_size));
    memcpy(this->message, message, message_size);
  }
  ~_FlutterTaskRunner() {
    if (response_handle != nullptr) {
      EXPECT_TRUE(response_handle->released);
      delete response_handle;
    }
    free(message);
  }
};

// Send a response from the engine.
static void send_response(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    const std::string& channel,
    const FlutterPlatformMessageResponseHandle* response_handle,
    const uint8_t* message,
    size_t message_size) {
  if (response_handle == nullptr)
    return;

  FlutterTask task;
  task.runner = new _FlutterTaskRunner(1234, channel, response_handle, message,
                                       message_size);
  task.task = task.runner->task;
  engine->platform_post_task_callback(task, 0, engine->user_data);
}

// Send a message from the engine.
static void send_message(FLUTTER_API_SYMBOL(FlutterEngine) engine,
                         const std::string& channel,
                         const uint8_t* message,
                         size_t message_size) {
  FlutterTask task;
  task.runner =
      new _FlutterTaskRunner(1234, channel, nullptr, message, message_size);
  task.task = task.runner->task;
  engine->platform_post_task_callback(task, 0, engine->user_data);
}

static void invoke_method(FLUTTER_API_SYMBOL(FlutterEngine) engine,
                          const std::string& channel,
                          const gchar* name,
                          FlValue* args) {
  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message = fl_method_codec_encode_method_call(
      FL_METHOD_CODEC(codec), name, args, &error);
  EXPECT_NE(message, nullptr);
  EXPECT_EQ(error, nullptr);

  FlutterTask task;
  task.runner = new _FlutterTaskRunner(
      1234, channel, nullptr,
      static_cast<const uint8_t*>(g_bytes_get_data(message, nullptr)),
      g_bytes_get_size(message));
  task.task = task.runner->task;
  engine->platform_post_task_callback(task, 0, engine->user_data);
}

FlutterEngineResult FlutterEngineCreateAOTData(
    const FlutterEngineAOTDataSource* source,
    FlutterEngineAOTData* data_out) {
  *data_out = nullptr;
  return kSuccess;
}

FlutterEngineResult FlutterEngineCollectAOTData(FlutterEngineAOTData data) {
  return kSuccess;
}

FlutterEngineResult FlutterEngineRun(size_t version,
                                     const FlutterRendererConfig* config,
                                     const FlutterProjectArgs* args,
                                     void* user_data,
                                     FLUTTER_API_SYMBOL(FlutterEngine) *
                                         engine_out) {
  EXPECT_NE(config, nullptr);
  EXPECT_NE(args, nullptr);
  EXPECT_NE(user_data, nullptr);
  EXPECT_NE(engine_out, nullptr);

  FlutterEngineResult result =
      FlutterEngineInitialize(version, config, args, user_data, engine_out);
  if (result != kSuccess)
    return result;
  return FlutterEngineRunInitialized(*engine_out);
}

FlutterEngineResult FlutterEngineShutdown(FLUTTER_API_SYMBOL(FlutterEngine)
                                              engine) {
  delete engine;
  return kSuccess;
}

FlutterEngineResult FlutterEngineInitialize(size_t version,
                                            const FlutterRendererConfig* config,
                                            const FlutterProjectArgs* args,
                                            void* user_data,
                                            FLUTTER_API_SYMBOL(FlutterEngine) *
                                                engine_out) {
  EXPECT_NE(config, nullptr);

  EXPECT_NE(args, nullptr);
  EXPECT_NE(args->platform_message_callback, nullptr);
  EXPECT_NE(args->custom_task_runners, nullptr);
  EXPECT_NE(args->custom_task_runners->platform_task_runner, nullptr);
  EXPECT_NE(args->custom_task_runners->platform_task_runner->post_task_callback,
            nullptr);

  EXPECT_NE(user_data, nullptr);

  EXPECT_EQ(config->type, kOpenGL);

  *engine_out = new _FlutterEngine(
      args->platform_message_callback,
      args->custom_task_runners->platform_task_runner->post_task_callback,
      user_data);
  return kSuccess;
}

FlutterEngineResult FlutterEngineDeinitialize(FLUTTER_API_SYMBOL(FlutterEngine)
                                                  engine) {
  return kSuccess;
}

FlutterEngineResult FlutterEngineRunInitialized(
    FLUTTER_API_SYMBOL(FlutterEngine) engine) {
  engine->running = true;
  return kSuccess;
}

FlutterEngineResult FlutterEngineSendWindowMetricsEvent(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    const FlutterWindowMetricsEvent* event) {
  EXPECT_TRUE(engine->running);
  return kSuccess;
}

FlutterEngineResult FlutterEngineSendPointerEvent(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    const FlutterPointerEvent* events,
    size_t events_count) {
  return kSuccess;
}

FLUTTER_EXPORT
FlutterEngineResult FlutterEngineSendPlatformMessage(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    const FlutterPlatformMessage* message) {
  EXPECT_TRUE(engine->running);

  if (strcmp(message->channel, "test/echo") == 0) {
    // Responds with the same message received.
    send_response(engine, message->channel, message->response_handle,
                  message->message, message->message_size);
  } else if (strcmp(message->channel, "test/send-message") == 0) {
    // Triggers the engine to send a message.
    send_response(engine, message->channel, message->response_handle, nullptr,
                  0);
    send_message(engine, "test/messages", message->message,
                 message->message_size);
  } else if (strcmp(message->channel, "test/standard-method") == 0) {
    g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
    g_autoptr(GBytes) m = g_bytes_new(message->message, message->message_size);
    g_autofree gchar* name = nullptr;
    g_autoptr(FlValue) args = nullptr;
    g_autoptr(GError) error = nullptr;
    EXPECT_TRUE(fl_method_codec_decode_method_call(FL_METHOD_CODEC(codec), m,
                                                   &name, &args, &error));
    EXPECT_EQ(error, nullptr);

    g_autoptr(GBytes) response = nullptr;
    if (strcmp(name, "Echo") == 0) {
      // Returns args as a success result.
      response = fl_method_codec_encode_success_envelope(FL_METHOD_CODEC(codec),
                                                         args, &error);
      EXPECT_EQ(error, nullptr);
    } else if (strcmp(name, "Error") == 0) {
      // Returns an error result.
      const gchar* code = nullptr;
      const gchar* message = nullptr;
      FlValue* details = nullptr;
      if (fl_value_get_length(args) >= 2) {
        FlValue* code_value = fl_value_get_list_value(args, 0);
        EXPECT_EQ(fl_value_get_type(code_value), FL_VALUE_TYPE_STRING);
        code = fl_value_get_string(code_value);
        FlValue* message_value = fl_value_get_list_value(args, 1);
        message = fl_value_get_type(message_value) == FL_VALUE_TYPE_STRING
                      ? fl_value_get_string(message_value)
                      : nullptr;
      }
      if (fl_value_get_length(args) >= 3)
        details = fl_value_get_list_value(args, 2);
      response = fl_method_codec_encode_error_envelope(
          FL_METHOD_CODEC(codec), code, message, details, &error);
      EXPECT_EQ(error, nullptr);
    } else if (strcmp(name, "InvokeMethod") == 0) {
      // Gets the engine to call the shell.
      if (fl_value_get_length(args) == 3) {
        FlValue* channel_value = fl_value_get_list_value(args, 0);
        EXPECT_EQ(fl_value_get_type(channel_value), FL_VALUE_TYPE_STRING);
        const gchar* channel = fl_value_get_string(channel_value);
        FlValue* name_value = fl_value_get_list_value(args, 1);
        EXPECT_EQ(fl_value_get_type(name_value), FL_VALUE_TYPE_STRING);
        const gchar* name = fl_value_get_string(name_value);
        FlValue* method_args = fl_value_get_list_value(args, 2);
        invoke_method(engine, channel, name, method_args);
      }
      response = fl_method_codec_encode_success_envelope(FL_METHOD_CODEC(codec),
                                                         nullptr, &error);
      EXPECT_EQ(error, nullptr);
    } else {
      // Returns "not implemented".
      response = g_bytes_new(nullptr, 0);
    }

    send_response(
        engine, message->channel, message->response_handle,
        static_cast<const uint8_t*>(g_bytes_get_data(response, nullptr)),
        g_bytes_get_size(response));
  } else if (strcmp(message->channel, "test/nullptr-response") == 0) {
    // Sends a null response.
    send_response(engine, message->channel, message->response_handle, nullptr,
                  0);
  } else if (strcmp(message->channel, "test/failure") == 0) {
    // Generates an internal error.
    return kInternalInconsistency;
  }

  return kSuccess;
}

FlutterEngineResult FlutterPlatformMessageCreateResponseHandle(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    FlutterDataCallback data_callback,
    void* user_data,
    FlutterPlatformMessageResponseHandle** response_out) {
  EXPECT_TRUE(engine->running);
  EXPECT_NE(data_callback, nullptr);
  EXPECT_NE(user_data, nullptr);

  _FlutterPlatformMessageResponseHandle* handle =
      new _FlutterPlatformMessageResponseHandle(data_callback, user_data);

  *response_out = handle;
  return kSuccess;
}

FlutterEngineResult FlutterPlatformMessageReleaseResponseHandle(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    FlutterPlatformMessageResponseHandle* response) {
  EXPECT_NE(engine, nullptr);
  EXPECT_NE(response, nullptr);

  EXPECT_TRUE(engine->running);

  EXPECT_FALSE(response->released);
  response->released = true;

  return kSuccess;
}

FlutterEngineResult FlutterEngineSendPlatformMessageResponse(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    const FlutterPlatformMessageResponseHandle* handle,
    const uint8_t* data,
    size_t data_length) {
  EXPECT_NE(engine, nullptr);
  EXPECT_NE(handle, nullptr);

  EXPECT_TRUE(engine->running);

  // Send a message so the shell can check the responses received.
  if (handle->channel != "test/responses")
    send_message(engine, "test/responses", data, data_length);

  EXPECT_FALSE(handle->released);

  delete handle;

  return kSuccess;
}

FlutterEngineResult FlutterEngineRunTask(FLUTTER_API_SYMBOL(FlutterEngine)
                                             engine,
                                         const FlutterTask* task) {
  EXPECT_NE(engine, nullptr);
  EXPECT_NE(task, nullptr);
  EXPECT_NE(task->runner, nullptr);

  FlutterTaskRunner runner = task->runner;
  EXPECT_NE(runner, nullptr);
  const FlutterPlatformMessageResponseHandle* response_handle =
      runner->response_handle;
  if (response_handle != nullptr) {
    EXPECT_NE(response_handle->data_callback, nullptr);
    response_handle->data_callback(runner->message, runner->message_size,
                                   response_handle->user_data);
  } else {
    _FlutterPlatformMessageResponseHandle* handle =
        new _FlutterPlatformMessageResponseHandle(runner->channel);

    FlutterPlatformMessage message;
    message.struct_size = sizeof(FlutterPlatformMessage);
    message.channel = runner->channel.c_str();
    message.message = runner->message;
    message.message_size = runner->message_size;
    message.response_handle = handle;
    engine->platform_message_callback(&message, engine->user_data);
  }

  delete runner;

  return kSuccess;
}

bool FlutterEngineRunsAOTCompiledDartCode() {
  return false;
}

FlutterEngineResult FlutterEngineUpdateLocales(FLUTTER_API_SYMBOL(FlutterEngine)
                                                   engine,
                                               const FlutterLocale** locales,
                                               size_t locales_count) {
  return kSuccess;
}

FlutterEngineResult FlutterEngineRegisterExternalTexture(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    int64_t texture_identifier) {
  return kSuccess;
}

FlutterEngineResult FlutterEngineMarkExternalTextureFrameAvailable(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    int64_t texture_identifier) {
  return kSuccess;
}

FlutterEngineResult FlutterEngineUnregisterExternalTexture(
    FLUTTER_API_SYMBOL(FlutterEngine) engine,
    int64_t texture_identifier) {
  return kSuccess;
}
