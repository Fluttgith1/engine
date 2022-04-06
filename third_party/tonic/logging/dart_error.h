// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_TONIC_LOGGING_DART_ERROR_H_
#define LIB_TONIC_LOGGING_DART_ERROR_H_

#include "third_party/dart/runtime/include/dart_api.h"

#include "tonic/dart_persistent_value.h"

namespace tonic {

namespace DartError {
using UnhandledExceptionReporter =
    std::function<void(const std::string&, const std::string&)>;

extern const char kInvalidArgument[];
}  // namespace DartError

/// Check if a Dart_Handle is an error or exception.
///
/// If it is an error or exception, this method will return true.
///
/// If it is an unhandled error or exception, first a call will be made to the
/// Dart code set in |SetUnhandledErrorHandler|. If that closure returns false
/// or throws an exception, a fallback is made to the closure in
/// |SetUnhandledExceptionReporter| to log details of the exception and stack.
/// If the |SetUnhandledErrorHandler| callback throws an exception, the
/// |SetUnhandledExceptionReporter| will be called with at least two separate
/// exceptions and stacktraces: one for the original exception, and one for the
/// exception thrown in the callback.
///
/// The fallback behavior matches the behavior of Flutter applications before
/// the introduction of PlatformDispatcher.onError.
///
/// Dart has errors that are not considered unhandled excpetions, such as
/// Dart_* API usage errors. In these cases, `Dart_IsUnhandledException` returns
/// false but `Dart_IsError` returns true. Such errors are logged to stderr or
/// some similar mechanism provided by the platform such as logcat on Android.
/// Depending on which type of error occurs, the process may crash and the Dart
/// isolate may be unusable. Errors that fall into this category include
/// compilation errors, Dart API errors, and unwind errors that will terminate
/// the Dart VM.
///
/// Historically known as LogIfError.
bool CheckAndHandleError(Dart_Handle handle);

/// Provides a Dart_Handle to a top level static field closure to invoke when
/// an unhandled exception occurs.
///
/// The signature of this field must match that of _onError in hooks.dart,
/// namely `bool _onError(Object error, StackTrace? stackTrace)`.
///
/// This handler will be invoked when an unhandled exception occurs. If it
/// returns false or throws an exception, a fallback handler will be invoked. By
/// default, UIDartState registers its ReportUnhandledError and will either call
/// a closure provided in the Shell Settings or simply dump the error to stderr
/// or some similar platform specific mechanism.
///
/// This must be called from the UI thread for the dart_state.
///
/// See also:
///  - CheckAndHandleError
///  - SetUnhandledExceptionReporter
void SetUnhandledErrorHandler(DartState* dart_state, Dart_Handle value);

/// The fallback mechanism to log errors if |SetUnhandledErrorHandler| has
/// provided a null value or a closure that returns false.
///
/// Normally, UIDartState registers with this method in its constructor.
void SetUnhandledExceptionReporter(
    DartError::UnhandledExceptionReporter reporter);

enum DartErrorHandleType {
  kNoError,
  kUnknownErrorType,
  kApiErrorType,
  kCompilationErrorType,
};

DartErrorHandleType GetErrorHandleType(Dart_Handle handle);

int GetErrorExitCode(Dart_Handle handle);

}  // namespace tonic

#endif  // LIB_TONIC_DART_ERROR_H_
