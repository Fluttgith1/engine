// Copyright 2023 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
part of dart.ui;

class PlatformIsolate {
  static Future<Isolate> spawn<T>(
      void entryPoint(T message), T message,
      {/*bool paused = false,  // TODO: Support these params.
      bool errorsAreFatal = true,
      SendPort? onExit,
      SendPort? onError,*/
      String? debugName}) {
    final isolateCompleter = Completer<Isolate>();
    final isolateReadyPort = RawReceivePort();
    isolateReadyPort.handler = (readyMessage) {
      isolateReadyPort.close();
      print("PlatformIsolate has been spawned: $readyMessage");

      if (readyMessage is _PlatformIsolateReadyMessage) {
        readyMessage.entryPointPort.send((entryPoint, message));
        isolateCompleter.complete(new Isolate(
            readyMessage.controlPort,
            pauseCapability: readyMessage.pauseCapability,
            terminateCapability: readyMessage.terminateCapability));
      } else if (readyMessage is String) {
        // We encountered an error while starting the new isolate.
        isolateCompleter.completeError(new IsolateSpawnException(
            'Unable to spawn isolate: $readyMessage'));
      } else {
        // This shouldn't happen.
        isolateCompleter.completeError(new IsolateSpawnException(
            "Internal error: unexpected format for ready message: "
            "'$readyMessage'"));
      }
    };
    _spawn(_platformIsolateMain<T>, isolateReadyPort.sendPort,
        debugName ?? "PlatformIsolate");
    return isolateCompleter.future;
  }

  static void _platformIsolateMain<T>(SendPort isolateReadyPort) {
    final entryPointPort = RawReceivePort();
    entryPointPort.handler = (entryPointAndMessage) {
      print("Received entrypoint: $entryPointAndMessage");
      print("    Are we on the platform thread? ${isRunningOnPlatformThread()}");
      entryPointPort.close();
      final (void Function(T) entryPoint, T message) = entryPointAndMessage;
      entryPoint(message);
    };
    final isolate = Isolate.current;
    isolateReadyPort.send(_PlatformIsolateReadyMessage(
        isolate.controlPort, isolate.pauseCapability,
        isolate.terminateCapability, entryPointPort.sendPort));
  }

  @Native<Void Function(Handle, Handle, Handle)>(symbol: 'PlatformIsolateNativeApi::Spawn')
  external static void _spawn(
      Function entryPoint, SendPort isolateReadyPort, String debugName);

  static Future<R> run<R>(FutureOr<R> computation(), {String? debugName}) {
    final resultCompleter = Completer<R>();
    final resultPort = RawReceivePort();
    resultPort.handler = (
        (R? result, Object? remoteError, Object? remoteStack)? response) {
      resultPort.close();
      if (response == null) {
        // onExit handler message, isolate terminated without sending result.
        resultCompleter.completeError(
            RemoteError("Computation ended without result", ""),
            StackTrace.empty);
        return;
      }
      final (result, remoteError, remoteStack) = response;
      if (result == null) {
        if (remoteStack is StackTrace) {
          // Typed error.
          resultCompleter.completeError(remoteError!, remoteStack);
        } else {
          // onError handler message, uncaught async error.
          // Both values are strings, so calling `toString` is efficient.
          final error =
              RemoteError(remoteError!.toString(), remoteStack!.toString());
          resultCompleter.completeError(error, error.stackTrace);
        }
      } else {
        resultCompleter.complete(result);
      }
    };
    try {
      PlatformIsolate.spawn(_remoteRun, (computation, resultPort.sendPort));
    } on Object {
      // Sending the computation failed synchronously.
      // This is not expected to happen, but if it does,
      // the synchronous error is respected and rethrown synchronously.
      resultPort.close();
      rethrow;
    }
    return resultCompleter.future;
  }

  static void _remoteRun<R>(
      (FutureOr<R> Function() computation, SendPort resultPort) args) async {
    final (computation, resultPort) = args;
    late final result;
    try {
      final potentiallyAsyncResult = computation();
      if (potentiallyAsyncResult is Future<R>) {
        result = await potentiallyAsyncResult;
      } else {
        result = potentiallyAsyncResult;
      }
    } catch (e, s) {
      // If sending fails, the error becomes an uncaught error.
      //Isolate.exit(resultPort, (e, s));
      resultPort.send((null, e, s));
    }
    //Isolate.exit(resultPort, (result));
    resultPort.send((result, null, null));
  }

  // Using this function to verify we're on the platform thread for prototyping.
  // TODO: Need to figure out a better way of doing this.
  @Native<Bool Function()>(symbol: 'PlatformIsolateNativeApi::IsRunningOnPlatformThread')
  external static bool isRunningOnPlatformThread();
}

class _PlatformIsolateReadyMessage {
  final SendPort controlPort;
  final Capability? pauseCapability;
  final Capability? terminateCapability;
  final SendPort entryPointPort;

  _PlatformIsolateReadyMessage(
      this.controlPort, this.pauseCapability, this.terminateCapability,
      this.entryPointPort);
}
