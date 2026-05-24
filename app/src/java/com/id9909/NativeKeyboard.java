package com.id9909;

import android.app.Activity;
import android.content.Context;
import android.graphics.drawable.GradientDrawable;
import android.text.Editable;
import android.text.InputFilter;
import android.text.InputType;
import android.text.TextWatcher;
import android.text.method.ArrowKeyMovementMethod;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import com.JniMethodId;
import com.SkipRename;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicBoolean;

public class NativeKeyboard
{
    private static final AtomicBoolean isActive = new AtomicBoolean(false);
    private static final AtomicBoolean isDone = new AtomicBoolean(false);
    private static volatile String currentText = "";
    private static volatile String finalText = "";
    private static volatile boolean finishedOnce = false;
    private static EditText editText;
    private static LinearLayout inputPanel;

    @JniMethodId(1)
    public static void Create(final Activity activity, final String oldText, final int charLimit, final int inputMode) {
        if (isActive.get()) return;
        final CountDownLatch latch = new CountDownLatch(1);
        activity.runOnUiThread(() -> {
            try {
                isActive.set(true);
                isDone.set(false);
                finishedOnce = false;
                currentText = oldText == null ? "" : oldText;
                finalText = "";
                inputPanel = new LinearLayout(activity);
                inputPanel.setOrientation(LinearLayout.VERTICAL);
                inputPanel.setPadding(dp(activity, 12), dp(activity, 8), dp(activity, 12), dp(activity, 8));
                GradientDrawable bgDrawable = new GradientDrawable();
                bgDrawable.setColor(0xE0FFFFFF);
                bgDrawable.setCornerRadius(dp(activity, 12));
                bgDrawable.setStroke(dp(activity, 2), 0xFF000000);
                inputPanel.setBackground(bgDrawable);
                inputPanel.setClickable(true);
                LinearLayout row = new LinearLayout(activity);
                row.setOrientation(LinearLayout.HORIZONTAL);
                row.setGravity(Gravity.CENTER_VERTICAL);
                row.setClickable(true);
                editText = new EditText(activity);
                editText.setText(oldText);
                editText.setSingleLine(true);
                editText.setImeOptions(EditorInfo.IME_ACTION_NONE | EditorInfo.IME_FLAG_NO_EXTRACT_UI);
                int inputTypeFlags = InputType.TYPE_CLASS_TEXT | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
                if (inputMode == 1) {
                    inputTypeFlags = InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_SIGNED;
                } else if (inputMode == 2) {
                    inputTypeFlags = InputType.TYPE_CLASS_NUMBER | InputType.TYPE_NUMBER_FLAG_SIGNED | InputType.TYPE_NUMBER_FLAG_DECIMAL;
                }
                editText.setInputType(inputTypeFlags);
                editText.setFilters(new InputFilter[]{ new InputFilter.LengthFilter(charLimit) });
                editText.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16);
                editText.setTextColor(0xFF000000);
                editText.setPadding(dp(activity, 8), dp(activity, 6), dp(activity, 8), dp(activity, 6));
                GradientDrawable etBg = new GradientDrawable();
                etBg.setColor(0xFFF5F5F5);
                etBg.setCornerRadius(dp(activity, 8));
                etBg.setStroke(dp(activity, 1), 0xFF888888);
                editText.setBackground(etBg);
                editText.setTextIsSelectable(true);
                editText.setLongClickable(true);
                editText.setClickable(true);
                editText.setFocusable(true);
                editText.setFocusableInTouchMode(true);
                editText.setCursorVisible(true);
                editText.setShowSoftInputOnFocus(true);
                editText.setEnabled(true);
                editText.setMovementMethod(ArrowKeyMovementMethod.getInstance());
                currentText = editText.getText().toString();
                editText.addTextChangedListener(new TextWatcher() {
                    @Override public void beforeTextChanged(CharSequence s,int st,int c,int a){}
                    @Override public void afterTextChanged(Editable s){}
                    @Override public void onTextChanged(CharSequence s,int st,int b,int c){
                        currentText = s == null ? "" : s.toString();
                    }
                });
                Button ok = new Button(activity);
                ok.setText("OK");
                ok.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16);
                ok.setPadding(dp(activity, 24), dp(activity, 12), dp(activity, 24), dp(activity, 12));
                ok.setMinimumWidth(dp(activity, 60));
                ok.setMinimumHeight(dp(activity, 40));
                GradientDrawable buttonBg = new GradientDrawable();
                buttonBg.setColor(0xFF2196F3);
                buttonBg.setCornerRadius(dp(activity, 8));
                ok.setBackground(buttonBg);
                ok.setTextColor(0xFFFFFFFF);
                ok.setOnClickListener(v -> finish(activity, currentText));
                LinearLayout.LayoutParams etParams = new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f);
                etParams.setMargins(0, 0, dp(activity, 8), 0);
                LinearLayout.LayoutParams btnParams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
                row.addView(editText, etParams);
                row.addView(ok, btnParams);
                LinearLayout.LayoutParams rowParams = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
                inputPanel.addView(row, rowParams);
                ViewGroup decor = (ViewGroup) activity.getWindow().getDecorView().findViewById(android.R.id.content);
                FrameLayout.LayoutParams panelParams = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.WRAP_CONTENT);
                panelParams.gravity = Gravity.BOTTOM;
                panelParams.setMargins(dp(activity, 8), 0, dp(activity, 8), dp(activity, 8));
                decor.addView(inputPanel, panelParams);
                final View rootView = activity.getWindow().getDecorView().getRootView();
                rootView.getViewTreeObserver().addOnGlobalLayoutListener(() -> {
                    if (inputPanel != null && inputPanel.getParent() != null) {
                        inputPanel.requestLayout();
                        inputPanel.post(() -> {
                            if (inputPanel != null) inputPanel.bringToFront();
                        });
                    }
                });
                editText.requestFocus();
                editText.requestFocusFromTouch();
                editText.post(() -> {
                    try {
                        InputMethodManager imm = (InputMethodManager) activity.getSystemService(Context.INPUT_METHOD_SERVICE);
                        if (imm != null) imm.showSoftInput(editText, InputMethodManager.SHOW_IMPLICIT);
                    } catch (Throwable ignored) {}
                });

            } catch (Throwable t) {
                cleanup(activity);
            } finally {
                latch.countDown();
            }
        });
        try { latch.await(); } catch (InterruptedException ignored) {}
    }

    private static void finish(Activity activity, String text) {
        if (finishedOnce) return;
        finishedOnce = true;
        finalText = text == null ? "" : text;
        isDone.set(true);
        try {
            InputMethodManager imm = (InputMethodManager) activity.getSystemService(Context.INPUT_METHOD_SERVICE);
            if (imm != null && editText != null) {
                imm.hideSoftInputFromWindow(editText.getWindowToken(), 0);
            }
        } catch (Throwable ignored) {}
        if (editText != null) {
            editText.post(() -> cleanup(activity));
        } else {
            cleanup(activity);
        }
    }

    private static void cleanup(Activity activity) {
        try {
            if (inputPanel != null) {
                ViewGroup parent = (ViewGroup) inputPanel.getParent();
                if (parent != null) parent.removeView(inputPanel);
            }
        } catch (Throwable ignored) {}
        editText = null;
        inputPanel = null;
        isActive.set(false);
    }

    @JniMethodId(2) public static void Destroy() { isDone.set(true); }
    @JniMethodId(3) public static boolean isDone() { return isDone.get(); }
    @JniMethodId(4) public static String getText() { return isDone.get() ? finalText : currentText; }
    private static int dp(Activity a, int v) { return (int)(a.getResources().getDisplayMetrics().density * v + 0.5f); }
}