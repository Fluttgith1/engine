package io.flutter.embedding.engine.mutatorsstack;

import static android.view.View.OnFocusChangeListener;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Path;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.FrameLayout;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import io.flutter.embedding.android.AndroidTouchProcessor;

/**
 * A view that applies the {@link io.flutter.embedding.engine.mutatorsstack.MutatorsStack} to its
 * children.
 */
public class FlutterMutatorView extends FrameLayout {
  private FlutterMutatorsStack mutatorsStack;
  private float screenDensity;
  private int left;
  private int top;
  private int prevLeft;
  private int prevTop;

  private final AndroidTouchProcessor androidTouchProcessor;

  /**
   * Initialize the FlutterMutatorView. Use this to set the screenDensity, which will be used to
   * correct the final transform matrix.
   */
  public FlutterMutatorView(
      @NonNull Context context,
      float screenDensity,
      @Nullable AndroidTouchProcessor androidTouchProcessor) {
    super(context, null);
    this.screenDensity = screenDensity;
    this.androidTouchProcessor = androidTouchProcessor;
  }

  /** Initialize the FlutterMutatorView. */
  public FlutterMutatorView(@NonNull Context context) {
    this(context, 1, /* androidTouchProcessor=*/ null);
  }

  /**
   * Determines if the current view or any descendant view has focus.
   *
   * @param root the root view.
   * @return true if the current view or any descendant view has focus.
   */
  @VisibleForTesting
  public static boolean childHasFocus(@Nullable View root) {
    if (root == null) {
      return false;
    }
    if (root.hasFocus()) {
      return true;
    }
    if (root instanceof ViewGroup) {
      final ViewGroup viewGroup = (ViewGroup) root;
      for (int idx = 0; idx < viewGroup.getChildCount(); idx++) {
        if (childHasFocus(viewGroup.getChildAt(idx))) {
          return true;
        }
      }
    }
    return false;
  }

  /**
   * Adds a focus change listener that notifies when the current view or any of its descendant views
   * have received focus.
   *
   * @param focusListener The focus listener.
   */
  public void addOnFocusChangeListener(@NonNull OnFocusChangeListener focusListener) {
    final View mutatorView = this;
    final ViewTreeObserver observer = this.getViewTreeObserver();
    if (observer.isAlive()) {
      observer.addOnGlobalFocusChangeListener(
          new ViewTreeObserver.OnGlobalFocusChangeListener() {
            @Override
            public void onGlobalFocusChanged(View oldFocus, View newFocus) {
              focusListener.onFocusChange(mutatorView, childHasFocus(mutatorView));
            }
          });
    }
  }

  /**
   * Pass the necessary parameters to the view so it can apply correct mutations to its children.
   */
  public void readyToDisplay(
      @NonNull FlutterMutatorsStack mutatorsStack, int left, int top, int width, int height) {
    this.mutatorsStack = mutatorsStack;
    this.left = left;
    this.top = top;
    FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(width, height);
    layoutParams.leftMargin = left;
    layoutParams.topMargin = top;
    setLayoutParams(layoutParams);
    setWillNotDraw(false);
  }

  @Override
  public void draw(Canvas canvas) {
    // Apply all clippings on the parent canvas.
    canvas.save();
    for (Path path : mutatorsStack.getFinalClippingPaths()) {
      // Reverse the current offset.
      //
      // The frame of this view includes the final offset of the bounding rect.
      // We need to apply all the mutators to the view, which includes the mutation that leads to
      // the final offset. We should reverse this final offset, both as a translate mutation and to
      // all the clipping paths
      Path pathCopy = new Path(path);
      pathCopy.offset(-left, -top);
      canvas.clipPath(pathCopy);
    }
    super.draw(canvas);
    canvas.restore();
  }

  @Override
  public void dispatchDraw(Canvas canvas) {
    // Apply all the transforms on the child canvas.
    canvas.save();

    canvas.concat(getPlatformViewMatrix());
    super.dispatchDraw(canvas);
    canvas.restore();
  }

  private Matrix getPlatformViewMatrix() {
    Matrix finalMatrix = new Matrix(mutatorsStack.getFinalMatrix());

    // Reverse scale based on screen scale.
    //
    // The Android frame is set based on the logical resolution instead of physical.
    // (https://developer.android.com/training/multiscreen/screendensities).
    // However, flow is based on the physical resolution. For example, 1000 pixels in flow equals
    // 500 points in Android. And until this point, we did all the calculation based on the flow
    // resolution. So we need to scale down to match Android's logical resolution.
    finalMatrix.preScale(1 / screenDensity, 1 / screenDensity);

    // Reverse the current offset.
    //
    // The frame of this view includes the final offset of the bounding rect.
    // We need to apply all the mutators to the view, which includes the mutation that leads to
    // the final offset. We should reverse this final offset, both as a translate mutation and to
    // all the clipping paths
    finalMatrix.postTranslate(-left, -top);

    return finalMatrix;
  }

  /** Intercept the events here and do not propagate them to the child platform views. */
  @Override
  public boolean onInterceptTouchEvent(MotionEvent event) {
    return true;
  }

  @Override
  @SuppressLint("ClickableViewAccessibility")
  public boolean onTouchEvent(MotionEvent event) {
    if (androidTouchProcessor == null) {
      return super.onTouchEvent(event);
    }

    final Matrix screenMatrix = new Matrix();

    switch (event.getAction()) {
      case MotionEvent.ACTION_DOWN:
        prevLeft = left;
        prevTop = top;
        screenMatrix.postTranslate(left, top);
        break;
      case MotionEvent.ACTION_MOVE:
        // While the view is dragged, use the left and top positions as
        // they were at the moment the touch event fired.
        screenMatrix.postTranslate(prevLeft, prevTop);
        prevLeft = left;
        prevTop = top;
        break;
      case MotionEvent.ACTION_UP:
      default:
        screenMatrix.postTranslate(left, top);
        break;
    }
    return androidTouchProcessor.onTouchEvent(event, screenMatrix);
  }
}
