// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.view;

import android.graphics.Rect;
import android.os.Build;
import android.util.Log;
import android.util.SparseArray;
import android.view.View;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.accessibility.AccessibilityNodeProvider;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;

/**
 * Facilitates embedding of platform views in the accessibility tree generated by the accessibility bridge.
 *
 * Embedding is done by mirroring the accessibility tree of the platform view as a subtree of the flutter
 * accessibility tree.
 *
 * This class relies on hidden system APIs to extract the accessibility information and does not work starting
 * Android P; If the reflection accessors are not available we fail silently by embedding a null node, the app
 * continues working but the accessibility information for the platform view will not be embedded.
 *
 * We use the term `flutterId` for virtual accessibility node IDs in the FlutterView tree, and the term `originId`
 * for the virtual accessibility node IDs in the platform view's tree. Internally this class maintains a bidirectional
 * mapping between `flutterId`s and the corresponding platform view and `originId`.
 */
class AccessibilityViewEmbedder {
    private static final String TAG = "AccessibilityBridge";

    private final ReflectionAccessors reflectionAccessors;

    // The view to which the platform view is embedded, this is typically FlutterView.
    private final View rootAccessibilityView;

    // Maps a flutterId to the corresponding platform view and originId.
    private final SparseArray<ViewAndId> flutterIdToOrigin;

    // Maps a platform view and originId to a corresponding flutterID.
    private final Map<ViewAndId, Integer> originToFlutterId;

    // Maps the flutterId of an accessibility node to the screen bounds of
    // the root semantic node for the embedded view.
    // This is used to translate the coordinates of the accessibility node subtree to the main display's coordinate
    // system.
    private final SparseArray<Rect> flutterIdToDisplayBounds;

    private int nextFlutterId;

    AccessibilityViewEmbedder(View rootAccessibiiltyView, int firstVirtualNodeId) {
        reflectionAccessors = new ReflectionAccessors();
        flutterIdToOrigin = new SparseArray<>();
        this.rootAccessibilityView = rootAccessibiiltyView;
        nextFlutterId = firstVirtualNodeId;
        flutterIdToDisplayBounds = new SparseArray<>();
        originToFlutterId = new HashMap<>();
    }

    /**
     * Returns the root accessibility node for an embedded platform view.
     *
     * @param flutterId the virtual accessibility ID for the node in flutter accessibility tree
     * @param displayBounds the display bounds for the node in screen coordinates
     */
    public AccessibilityNodeInfo getRootNode(View embeddedView, int flutterId, Rect displayBounds) {
        AccessibilityNodeInfo originNode = embeddedView.createAccessibilityNodeInfo();
        Long originPackedId = reflectionAccessors.getSourceNodeId(originNode);
        if (originPackedId == null) {
            return null;
        }
        int originId = ReflectionAccessors.getVirtualNodeId(originPackedId);
        flutterIdToOrigin.put(flutterId, new ViewAndId(embeddedView, originId));
        flutterIdToDisplayBounds.put(flutterId, displayBounds);
        originToFlutterId.put(new ViewAndId(embeddedView, originId), flutterId);
        return convertToFlutterNode(originNode, flutterId, embeddedView);
    }

    /**
     * Returns the accessibility node info for the node identified with `flutterId`.
     */
    public AccessibilityNodeInfo createAccessibilityNodeInfo(int flutterId) {
        ViewAndId origin = flutterIdToOrigin.get(flutterId);
        if (origin == null) {
            return null;
        }
        AccessibilityNodeProvider provider = origin.view.getAccessibilityNodeProvider();
        if (provider == null) {
            // The provider is null for views that don't have a virtual accessibility tree.
            // We currently only support embedding virtual hierarchies in the Flutter tree.
            // TODO(amirh): support embedding non virtual hierarchies.
            // https://github.com/flutter/flutter/issues/29717
            return null;
        }
        AccessibilityNodeInfo originNode =
                origin.view.getAccessibilityNodeProvider().createAccessibilityNodeInfo(origin.id);
        return convertToFlutterNode(originNode, flutterId, origin.view);
    }

    private AccessibilityNodeInfo convertToFlutterNode(AccessibilityNodeInfo originNode, int flutterId, View embeddedView) {
        AccessibilityNodeInfo result = AccessibilityNodeInfo.obtain(rootAccessibilityView, flutterId);
        result.setPackageName(rootAccessibilityView.getContext().getPackageName());
        result.setSource(rootAccessibilityView, flutterId);
        result.setClassName(originNode.getClassName());

        Rect displayBounds = flutterIdToDisplayBounds.get(flutterId);

        copyAccessibilityFields(originNode, result);
        translateBounds(originNode, displayBounds, result);
        addChildren(originNode, embeddedView, displayBounds, result);
        setParent(originNode, embeddedView, result);

        return result;
    }

    private void setParent(AccessibilityNodeInfo originNode, View embeddedView, AccessibilityNodeInfo result) {
        Long parentOriginPackedId = reflectionAccessors.getParentNodeId(originNode);
        if (parentOriginPackedId == null) {
            return;
        }
        int parentOriginId = ReflectionAccessors.getVirtualNodeId(parentOriginPackedId);
        int parentFlutterId = originToFlutterId.get(new ViewAndId(embeddedView, parentOriginId));
        result.setParent(rootAccessibilityView, parentFlutterId);
    }


    private void addChildren(AccessibilityNodeInfo originNode, View embeddedView, Rect displayBounds, AccessibilityNodeInfo resultNode) {
        for (int i = 0; i < originNode.getChildCount(); i++) {
            Long originPackedId = reflectionAccessors.getChildId(originNode, i);
            if (originPackedId == null) {
                continue;
            }
            int originId = ReflectionAccessors.getVirtualNodeId(originPackedId);
            ViewAndId origin = new ViewAndId(embeddedView, originId);
            int childFlutterId;
            if (originToFlutterId.containsKey(origin)) {
                childFlutterId = originToFlutterId.get(origin);
            } else {
                childFlutterId = nextFlutterId++;
                originToFlutterId.put(origin, childFlutterId);
                flutterIdToOrigin.put(childFlutterId, origin);
                flutterIdToDisplayBounds.put(childFlutterId, displayBounds);
            }
            resultNode.addChild(rootAccessibilityView, childFlutterId);
        }
    }

    private void translateBounds(AccessibilityNodeInfo originNode, Rect displayBounds, AccessibilityNodeInfo resultNode) {
        Rect boundsInParent = new Rect();
        originNode.getBoundsInParent(boundsInParent);
        resultNode.setBoundsInParent(boundsInParent);

        Rect boundsInScreen = new Rect();
        originNode.getBoundsInScreen(boundsInScreen);
        boundsInScreen.offset(displayBounds.left, displayBounds.top);
        resultNode.setBoundsInScreen(boundsInScreen);
    }

    private void copyAccessibilityFields(AccessibilityNodeInfo input, AccessibilityNodeInfo output) {
        output.setAccessibilityFocused(input.isAccessibilityFocused());
        output.setCheckable(input.isCheckable());
        output.setChecked(input.isChecked());
        output.setContentDescription(input.getContentDescription());
        output.setEnabled(input.isEnabled());
        output.setClickable(input.isClickable());
        output.setFocusable(input.isFocusable());
        output.setFocused(input.isFocused());
        output.setLongClickable(input.isLongClickable());
        output.setMovementGranularities(input.getMovementGranularities());
        output.setPassword(input.isPassword());
        output.setScrollable(input.isScrollable());
        output.setSelected(input.isSelected());
        output.setText(input.getText());
        output.setVisibleToUser(input.isVisibleToUser());

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
            output.setEditable(input.isEditable());
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            output.setCanOpenPopup(input.canOpenPopup());
            output.setCollectionInfo(input.getCollectionInfo());
            output.setCollectionItemInfo(input.getCollectionItemInfo());
            output.setContentInvalid(input.isContentInvalid());
            output.setDismissable(input.isDismissable());
            output.setInputType(input.getInputType());
            output.setLiveRegion(input.getLiveRegion());
            output.setMultiLine(input.isMultiLine());
            output.setRangeInfo(input.getRangeInfo());
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            output.setError(input.getError());
            output.setMaxTextLength(input.getMaxTextLength());
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            output.setContextClickable(input.isContextClickable());
            // TODO(amirh): copy traversal before and after.
            // https://github.com/flutter/flutter/issues/29718
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            output.setDrawingOrder(input.getDrawingOrder());
            output.setImportantForAccessibility(input.isImportantForAccessibility());
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            output.setAvailableExtraData(input.getAvailableExtraData());
            output.setHintText(input.getHintText());
            output.setShowingHintText(input.isShowingHintText());
        }
    }

    private static class ViewAndId {
        final View view;
        final int id;

        private ViewAndId(View view, int id) {
            this.view = view;
            this.id = id;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            ViewAndId viewAndId = (ViewAndId) o;
            return id == viewAndId.id &&
                    view.equals(viewAndId.view);
        }

        @Override
        public int hashCode() {
            final int prime = 31;
            int result = 1;
            result = prime * result + view.hashCode();
            result = prime * result + id;
            return result;
        }
    }

    private static class ReflectionAccessors {
        private final Method getSourceNodeId;
        private final Method getParentNodeId;
        private final Method getChildId;

        private ReflectionAccessors() {
            Method getSourceNodeId = null;
            Method getParentNodeId = null;
            Method getChildId = null;
            try {
                getSourceNodeId = AccessibilityNodeInfo.class.getMethod("getSourceNodeId");
            } catch (NoSuchMethodException e) {
                Log.w(TAG, "can't invoke getSourceNodeId with reflection");
            }
            try {
                getParentNodeId = AccessibilityNodeInfo.class.getMethod("getParentNodeId");
            } catch (NoSuchMethodException e) {
                Log.w(TAG, "can't invoke getParentNodeId with reflection");
            }
            try {
                getChildId = AccessibilityNodeInfo.class.getMethod("getChildId", int.class);
            } catch (NoSuchMethodException e) {
                Log.w(TAG, "can't invoke getChildId with reflection");
            }
            this.getSourceNodeId = getSourceNodeId;
            this.getParentNodeId = getParentNodeId;
            this.getChildId = getChildId;
        }

        /** Returns virtual node ID given packed node ID used internally in accessibility API. */
        private static int getVirtualNodeId(long nodeId) {
            return (int) (nodeId >> 32);
        }

        private Long getSourceNodeId(AccessibilityNodeInfo node) {
            if (getSourceNodeId == null) {
                return null;
            }
            try {
                return (Long) getSourceNodeId.invoke(node);
            } catch (IllegalAccessException e) {
                Log.w(TAG, e);
            } catch (InvocationTargetException e) {
                Log.w(TAG, e);
            }
            return null;
        }

        private Long getChildId(AccessibilityNodeInfo node, int child) {
            if (getChildId == null) {
                return null;
            }
            try {
                return (Long) getChildId.invoke(node, child);
            } catch (IllegalAccessException e) {
                Log.w(TAG, e);
            } catch (InvocationTargetException e) {
                Log.w(TAG, e);
            }
            return null;
        }

        private Long getParentNodeId(AccessibilityNodeInfo node) {
            if (getParentNodeId == null) {
                return null;
            }
            try {
                return (long) getParentNodeId.invoke(node);
            } catch (IllegalAccessException e) {
                Log.w(TAG, e);
            } catch (InvocationTargetException e) {
                Log.w(TAG, e);
            }
            return null;
        }
    }
}
