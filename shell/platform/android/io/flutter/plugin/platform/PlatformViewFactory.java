// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.plugin.platform;

import android.content.Context;
import android.support.annotation.StyleRes;

import io.flutter.plugin.common.MessageCodec;

public abstract class PlatformViewFactory {
    private final MessageCodec<Object> createArgsCodec;

    /**
     *
     * @param createArgsCodec the codec used to decode the args parameter of {@link #create}.
     */
    public PlatformViewFactory(MessageCodec<Object> createArgsCodec) {
        this.createArgsCodec = createArgsCodec;
    }

    /**
     * Creates a new Android view to be embedded in the Flutter hierarchy.
     *
     * @param context the context to be used when creating the view, this is different than FlutterView's context.
     * @param viewId unique identifier for the created instance, this value is known on the Dart side.
     * @param args arguments sent from the Flutter app. The bytes for this value are decoded using the createArgsCodec
     *             argument passed to the constructor. This is null if createArgsCodec was null, or no arguments were
     *             sent from the Flutter app.
     */
    public abstract PlatformView create(Context context, int viewId, Object args);

    /**
     * Used to customize the Theme of Presentation. This is useful when need a translucent AndroidView.
     * Return 0 by default, will use the theme defined by com.android.internal.R.attr.presentationTheme attribute.
     * @return A style resource describing the theme to use for the window.
     */
    @StyleRes
    protected int getPresentationTheme() {
        return 0;
    }

    /**
     * Returns the codec to be used for decoding the args parameter of {@link #create}.
     */
    public final MessageCodec<Object> getCreateArgsCodec() {
        return createArgsCodec;
    }
}
