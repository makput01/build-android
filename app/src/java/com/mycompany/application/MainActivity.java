package com.mycompany.application;

import android.app.Activity;
import android.animation.ArgbEvaluator;
import android.animation.ValueAnimator;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.hardware.input.InputManager;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class MainActivity extends Activity
{
    public static final boolean EnableRGB = true;
    public static final boolean EnableTouchs = true;
    private ValueAnimator colorAnim;

    static {
        System.loadLibrary("main");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        hideSystemUI();
        FrameLayout root = new FrameLayout(this);
        root.setLayoutParams(new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
        ));
        if (EnableRGB) {
            colorAnim = ValueAnimator.ofObject(
                    new ArgbEvaluator(),
                    Color.RED,
                    Color.GREEN,
                    Color.BLUE,
                    Color.RED
            );
            colorAnim.setDuration(24000);
            colorAnim.setRepeatCount(ValueAnimator.INFINITE);
            colorAnim.setRepeatMode(ValueAnimator.RESTART);
            colorAnim.addUpdateListener(animator ->
                    root.setBackgroundColor((int) animator.getAnimatedValue())
            );
            colorAnim.start();
        } else {
            root.setBackgroundColor(Color.BLACK);
        }
        if (EnableTouchs) {
            TouchTrailView touchView = new TouchTrailView(this);
            root.addView(touchView, new FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT
            ));
            setContentView(root);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (colorAnim != null) {
            colorAnim.cancel();
        }
    }

    public void hideSystemUI() {
        getWindow().getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
        );
    }

    public static class TouchTrailView extends View
    {
        private static class TouchPoint {
            float x;
            float y;
            boolean isDown;
            long releasedAt = -1L;
            int color;
        }

        private static final long FADE_DURATION_MS = 1000L;
        private static final float RADIUS = 30f;
        private final Paint fillPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint strokePaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Paint textPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        private final Map<Integer, TouchPoint> points = new HashMap<>();

        private final Runnable frameUpdater = new Runnable() {
            @Override
            public void run() {
                boolean hasVisiblePoints = cleanupExpiredPoints();
                invalidate();
                if (hasVisiblePoints) {
                    postOnAnimation(this);
                }
            }
        };

        public TouchTrailView(Context context) {
            super(context);
            setFocusable(true);
            setFocusableInTouchMode(true);
            fillPaint.setStyle(Paint.Style.FILL);
            strokePaint.setStyle(Paint.Style.STROKE);
            strokePaint.setStrokeWidth(4f);
            strokePaint.setColor(Color.WHITE);
            textPaint.setColor(Color.WHITE);
            textPaint.setTextSize(28f);
        }

        @Override
        public boolean onTouchEvent(MotionEvent event) {
            int action = event.getActionMasked();
            int actionIndex = event.getActionIndex();
            switch (action) {
                case MotionEvent.ACTION_DOWN:
                case MotionEvent.ACTION_POINTER_DOWN: {
                    int pointerId = event.getPointerId(actionIndex);
                    TouchPoint point = new TouchPoint();
                    point.x = event.getX(actionIndex);
                    point.y = event.getY(actionIndex);
                    point.isDown = true;
                    point.releasedAt = -1L;
                    point.color = colorForPointer(pointerId);
                    points.put(pointerId, point);
                    invalidate();
                    break;
                }
                case MotionEvent.ACTION_MOVE: {
                    for (int i = 0; i < event.getPointerCount(); i++) {
                        int pointerId = event.getPointerId(i);
                        TouchPoint point = points.get(pointerId);
                        if (point != null) {
                            point.x = event.getX(i);
                            point.y = event.getY(i);
                            point.isDown = true;
                            point.releasedAt = -1L;
                        }
                    }
                    invalidate();
                    break;
                }
                case MotionEvent.ACTION_UP:
                case MotionEvent.ACTION_POINTER_UP: {
                    int pointerId = event.getPointerId(actionIndex);
                    TouchPoint point = points.get(pointerId);
                    if (point != null) {
                        point.x = event.getX(actionIndex);
                        point.y = event.getY(actionIndex);
                        point.isDown = false;
                        point.releasedAt = SystemClock.uptimeMillis();
                    }
                    postOnAnimation(frameUpdater);
                    invalidate();
                    break;
                }
                case MotionEvent.ACTION_CANCEL: {
                    long now = SystemClock.uptimeMillis();
                    for (TouchPoint point : points.values()) {
                        point.isDown = false;
                        point.releasedAt = now;
                    }
                    postOnAnimation(frameUpdater);
                    invalidate();
                    break;
                }
            }
            return true;
        }

        @Override
        protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);
            long now = SystemClock.uptimeMillis();
            for (Map.Entry<Integer, TouchPoint> entry : points.entrySet()) {
                int pointerId = entry.getKey();
                TouchPoint point = entry.getValue();
                int alpha = 255;
                if (!point.isDown && point.releasedAt > 0) {
                    long elapsed = now - point.releasedAt;
                    float t = 1f - Math.min(1f, elapsed / (float) FADE_DURATION_MS);
                    alpha = (int) (255f * t);
                }
                if (alpha <= 0) {
                    continue;
                }
                fillPaint.setColor(point.color);
                fillPaint.setAlpha(alpha);
                strokePaint.setAlpha(alpha);
                textPaint.setAlpha(alpha);
                canvas.drawCircle(point.x, point.y, RADIUS, fillPaint);
                canvas.drawCircle(point.x, point.y, RADIUS, strokePaint);
                canvas.drawText("ID " + pointerId, point.x + 70f, point.y - 20f, textPaint);
                canvas.drawText("(" + (int) point.x + ", " + (int) point.y + ")", point.x + 70f, point.y + 20f, textPaint);
            }
        }

        private boolean cleanupExpiredPoints() {
            long now = SystemClock.uptimeMillis();
            boolean hasVisible = false;
            Iterator<Map.Entry<Integer, TouchPoint>> it = points.entrySet().iterator();
            while (it.hasNext()) {
                TouchPoint point = it.next().getValue();
                if (point.isDown) {
                    hasVisible = true;
                    continue;
                }
                if (point.releasedAt <= 0) {
                    it.remove();
                    continue;
                }
                long elapsed = now - point.releasedAt;
                if (elapsed >= FADE_DURATION_MS) {
                    it.remove();
                } else {
                    hasVisible = true;
                }
            }
            return hasVisible;
        }

        private int colorForPointer(int pointerId) {
            int[] colors = {
                    Color.RED,
                    Color.GREEN,
                    Color.BLUE,
                    Color.YELLOW,
                    Color.CYAN,
                    Color.MAGENTA,
                    Color.WHITE,
                    Color.LTGRAY
            };
            return colors[Math.abs(pointerId) % colors.length];
        }
    }
}