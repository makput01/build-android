package com.id9909;

import android.app.Activity;
import android.graphics.PixelFormat;
import android.provider.Settings;
import android.view.View;
import android.view.ViewGroup;
import android.content.Context;
import android.opengl.GLSurfaceView;
import com.JniMethodId;
import java.util.concurrent.CountDownLatch;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class ImGuiManager
{
    private static volatile boolean inited = false;
    private static ImGuiRender renderView = null;

    @JniMethodId(1) public static native void bridge_init();
    @JniMethodId(2) public static native void bridge_render();
    @JniMethodId(3) public static native void bridge_resize(int w, int h);
    @JniMethodId(4) public static native boolean bridge_click(int action, float x, float y);

    @JniMethodId(5)
    public static void Init(Activity activity) {
        if (inited) return;
        final CountDownLatch latch = new CountDownLatch(1);
        activity.runOnUiThread(() -> {
            try {
                if (activity.isFinishing() || activity.isDestroyed()) {
                    latch.countDown();
                    return;
                }
                renderView = new ImGuiRender(activity);
                ViewGroup decorView = (ViewGroup) activity.getWindow().getDecorView();
                ViewGroup.LayoutParams params = new ViewGroup.LayoutParams(
                        ViewGroup.LayoutParams.MATCH_PARENT,
                        ViewGroup.LayoutParams.MATCH_PARENT
                );
                decorView.addView(renderView, params);
                renderView.post(() -> {
                    renderView.setVisibility(View.VISIBLE);
                    renderView.bringToFront();
                    renderView.requestLayout();
                    renderView.onResume();
                    inited = true;
                    latch.countDown();
                });
            } catch (Exception e) {
                latch.countDown();
            }
        });
    }

    private static class ImGuiRender extends GLSurfaceView implements GLSurfaceView.Renderer
    {
        private volatile boolean surfaceCreated = false;

        public ImGuiRender(Context context) {
            super(context);
            setEGLContextClientVersion(3);
            setEGLConfigChooser(8, 8, 8, 8, 16, 0);
            setRenderer(this);
            setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
            setZOrderOnTop(true);
            getHolder().setFormat(PixelFormat.TRANSLUCENT);
            setClickable(false);
            setFocusable(false);
            setFocusableInTouchMode(false);
            setOnTouchListener((v, e) -> {
                int action = e.getActionMasked();
                float x = e.getX();
                float y = e.getY();
                v.performClick();
                return bridge_click(action, x, y);
            });
        }

        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            if (!surfaceCreated) {
                bridge_init();
                surfaceCreated = true;
            }
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            if (surfaceCreated) bridge_render();
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            if (surfaceCreated) bridge_resize(width, height);
        }
    }
}