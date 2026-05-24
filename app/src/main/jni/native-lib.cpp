#include <jni.h>
#include <android/log.h>
#include <GLES3/gl3.h>
#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>

#define LOG_TAG "ImGuiNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" {

JNIEXPORT void JNICALL
Java_com_example_imgui_1app_ImGuiRenderer_nativeSurfaceCreated(JNIEnv *env, jobject thiz) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplAndroid_Init(nullptr)) {
        LOGI("Gagal inisialisasi ImGui Android");
        return;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 300 es")) {
        LOGI("Gagal inisialisasi ImGui OpenGL3");
        return;
    }
    LOGI("ImGui berhasil diinisialisasi");
}

JNIEXPORT void JNICALL
Java_com_example_imgui_1app_ImGuiRenderer_nativeSurfaceChanged(JNIEnv *env, jobject thiz, jint width, jint height) {
    glViewport(0, 0, width, height);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
}

JNIEXPORT void JNICALL
Java_com_example_imgui_1app_ImGuiRenderer_nativeDrawFrame(JNIEnv *env, jobject thiz) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();

    // Contoh antarmuka ImGui
    ImGui::Begin("Halo Android!");
    ImGui::Text("Ini contoh ImGui di Android dengan Java + C++");
    static float nilai = 0.5f;
    ImGui::SliderFloat("Nilai", &nilai, 0.0f, 1.0f);
    if (ImGui::Button("Tekan Saya")) {
        LOGI("Tombol ditekan!");
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // extern "C"

