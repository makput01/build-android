#ifndef MY_UTILS_H
#define MY_UTILS_H

#include <random>
#include <fcntl.h>
#include <unistd.h>
#include <jni.h>
#include <string>
#include <pthread.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <thread>
#include <chrono>
#include <linux/input.h>
#include <map>
#include <fstream>
#include <array>
#include <vector>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <dlfcn.h>
#include <sys/system_properties.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include "Includes/KittyMemory/KittyInclude.hpp"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_android.h"
#include "ImGui/backends/imgui_impl_opengl3.h"
#include "Includes/Roboto-Regular.h"

#define o2m(offset, name) name = reinterpret_cast<decltype(name)>(offset)

enum JNIMAPPING : int {
    JNIMAPPING_IMGUIMANAGER = 0,
    JNIMAPPING_NATIVEKEYBOARD = 1,
    JNIMAPPING_UTILS = 2,
    JNIMAPPING_IMGUIMANAGER_BRIDGE_INIT = 3,
    JNIMAPPING_IMGUIMANAGER_BRIDGE_RENDER = 4,
    JNIMAPPING_IMGUIMANAGER_BRIDGE_RESIZE = 5,
    JNIMAPPING_IMGUIMANAGER_BRIDGE_CLICK = 6,
    JNIMAPPING_IMGUIMANAGER_INIT = 7,
    JNIMAPPING_NATIVEKEYBOARD_CREATE = 8,
    JNIMAPPING_NATIVEKEYBOARD_DESTROY = 9,
    JNIMAPPING_NATIVEKEYBOARD_ISDONE = 10,
    JNIMAPPING_NATIVEKEYBOARD_GETTEXT = 11,
    JNIMAPPING_ = 256,
};

extern JavaVM* JNI_VM;
extern jclass com_id9909_ImGuiManager;
extern jclass com_id9909_NativeKeyboard;
extern std::vector<std::string> JNI_MAPPING;

inline static std::string js2std(JNIEnv* env, jstring jstr) {
    if (!env || !jstr) return {};
    const char* cstr = env->GetStringUTFChars(jstr, nullptr);
    if (!cstr) return {};
    std::string str(cstr);
    env->ReleaseStringUTFChars(jstr, cstr);
    return str;
}

inline static std::vector<std::string> jsa2vstd(JNIEnv* env, jobjectArray array) {
    std::vector<std::string> result;
    if (!array) return result;
    jsize len = env->GetArrayLength(array);
    result.reserve(len);
    for (jsize i = 0; i < len; ++i) {
        jstring jstr = (jstring)env->GetObjectArrayElement(array, i);
        result.push_back(js2std(env, jstr));
        env->DeleteLocalRef(jstr);
    }
    return result;
}

inline static void JavaVM_GetEnv(JNIEnv** env) {
    if (JNI_VM->GetEnv((void**)env, JNI_VERSION_1_6) != JNI_OK) {
        JNI_VM->AttachCurrentThread(env, nullptr);
    }
}

inline static jobject getCurrentContext(JNIEnv* env) {
    jclass at = env->FindClass("android/app/ActivityThread");
    jmethodID curApp = env->GetStaticMethodID(at, "currentApplication", "()Landroid/app/Application;");
    return env->CallStaticObjectMethod(at, curApp);
}

inline static jobject getCurrentActivity(JNIEnv* env) {
    jclass atCls  = env->FindClass("android/app/ActivityThread");
    jobject atObj = env->CallStaticObjectMethod(atCls,env->GetStaticMethodID(atCls, "currentActivityThread", "()Landroid/app/ActivityThread;"));
    jfieldID fidActs = env->GetFieldID(atCls, "mActivities", "Landroid/util/ArrayMap;");
    if (!fidActs) fidActs = env->GetFieldID(atCls, "mActivities", "Ljava/util/HashMap;");
    if (!fidActs) fidActs = env->GetFieldID(atCls, "mActivities", "Ljava/util/Map;");
    if (!fidActs) return nullptr;
    jobject actsMap = env->GetObjectField(atObj, fidActs);
    if (!actsMap) return nullptr;
    jclass mapCls = env->FindClass("java/util/Map");
    jobject values = env->CallObjectMethod(actsMap, env->GetMethodID(mapCls, "values","()Ljava/util/Collection;"));
    jclass collCls = env->FindClass("java/util/Collection");
    jobject it = env->CallObjectMethod(values, env->GetMethodID(collCls, "iterator", "()Ljava/util/Iterator;"));
    jclass itCls = env->FindClass("java/util/Iterator");
    jmethodID midHas = env->GetMethodID(itCls, "hasNext", "()Z");
    jmethodID midNext = env->GetMethodID(itCls, "next", "()Ljava/lang/Object;");
    jfieldID fidPaused = nullptr;
    jfieldID fidActivity = nullptr;
    while (env->CallBooleanMethod(it, midHas)) {
        jobject rec = env->CallObjectMethod(it, midNext);
        if (!rec) continue;
        if (!fidPaused) {
            jclass recCls = env->GetObjectClass(rec);
            fidPaused = env->GetFieldID(recCls, "paused", "Z");
            fidActivity = env->GetFieldID(recCls, "activity", "Landroid/app/Activity;");
            if (!fidPaused || !fidActivity) return nullptr;
        }
        if (!env->GetBooleanField(rec, fidPaused)) {
            jobject activity = env->GetObjectField(rec, fidActivity);
            return activity;
        }
    }
    return nullptr;
}

inline static void NativeKeyboardServiceString(std::string* textPointer, bool hasOldText, int charLimit) {
    JavaVM* jvm = JNI_VM;
    JNIEnv* env = nullptr;
    JavaVM_GetEnv(&env);
    jobject activity = getCurrentActivity(env);
    if (!activity) {
        jvm->DetachCurrentThread();
        return;
    }
    jclass nkClass = com_id9909_NativeKeyboard;
    if (!nkClass) {
        env->DeleteLocalRef(activity);
        jvm->DetachCurrentThread();
        return;
    }
    jstring oldJ = nullptr;
    if (hasOldText && textPointer != nullptr) {
        oldJ = env->NewStringUTF(textPointer->c_str());
    } else {
        oldJ = env->NewStringUTF("");
    }
    jmethodID createMethod = env->GetStaticMethodID(nkClass, JNI_MAPPING[JNIMAPPING_NATIVEKEYBOARD_CREATE].c_str(), "(Landroid/app/Activity;Ljava/lang/String;II)V");
    if (!createMethod) {
        env->DeleteLocalRef(oldJ);
        env->DeleteLocalRef(activity);
        jvm->DetachCurrentThread();
        return;
    }
    env->CallStaticVoidMethod(nkClass, createMethod, activity, oldJ, (jint)charLimit, 0);
    env->DeleteLocalRef(oldJ);
    jmethodID isDoneMethod = env->GetStaticMethodID(nkClass, JNI_MAPPING[JNIMAPPING_NATIVEKEYBOARD_ISDONE].c_str(), "()Z");
    jmethodID getTextMethod = env->GetStaticMethodID(nkClass, JNI_MAPPING[JNIMAPPING_NATIVEKEYBOARD_GETTEXT].c_str(), "()Ljava/lang/String;");
    if (!isDoneMethod || !getTextMethod) {
        env->DeleteLocalRef(activity);
        jvm->DetachCurrentThread();
        return;
    }
    bool done = false;
    while (!done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        jboolean jdone = env->CallStaticBooleanMethod(nkClass, isDoneMethod);
        done = (jdone == JNI_TRUE);
        jstring jcur = (jstring)env->CallStaticObjectMethod(nkClass, getTextMethod);
        if (jcur) {
            const char* utf = env->GetStringUTFChars(jcur, nullptr);
            if (utf) {
                *textPointer = utf;
                env->ReleaseStringUTFChars(jcur, utf);
            }
            env->DeleteLocalRef(jcur);
        }
    }
    jstring jfinal = (jstring)env->CallStaticObjectMethod(nkClass, getTextMethod);
    if (jfinal) {
        const char* utf = env->GetStringUTFChars(jfinal, nullptr);
        if (utf) {
            *textPointer = utf;
            env->ReleaseStringUTFChars(jfinal, utf);
        }
        env->DeleteLocalRef(jfinal);
    }
    env->DeleteLocalRef(activity);
    jvm->DetachCurrentThread();
}

inline static void NativeKeyboardServiceInt(int* intPointer, bool hasOldValue) {
    JavaVM* jvm = JNI_VM;
    JNIEnv* env = nullptr;
    JavaVM_GetEnv(&env);
    jobject activity = getCurrentActivity(env);
    if (!activity) {
        jvm->DetachCurrentThread();
        return;
    }
    jclass nkClass = com_id9909_NativeKeyboard;
    if (!nkClass) {
        env->DeleteLocalRef(activity);
        jvm->DetachCurrentThread();
        return;
    }
    jstring oldJ = nullptr;
    if (hasOldValue && intPointer != nullptr) {
        oldJ = env->NewStringUTF(std::to_string(*intPointer).c_str());
    } else {
        oldJ = env->NewStringUTF("");
    }
    jmethodID createMethod = env->GetStaticMethodID(nkClass, JNI_MAPPING[JNIMAPPING_NATIVEKEYBOARD_CREATE].c_str(), "(Landroid/app/Activity;Ljava/lang/String;II)V");
    if (!createMethod) {
        env->DeleteLocalRef(oldJ);
        env->DeleteLocalRef(activity);
        jvm->DetachCurrentThread();
        return;
    }
    env->CallStaticVoidMethod(nkClass, createMethod, activity, oldJ, (jint)12, (jint)1);
    env->DeleteLocalRef(oldJ);
    jmethodID isDoneMethod = env->GetStaticMethodID(nkClass, JNI_MAPPING[JNIMAPPING_NATIVEKEYBOARD_ISDONE].c_str(), "()Z");
    jmethodID getTextMethod = env->GetStaticMethodID(nkClass, JNI_MAPPING[JNIMAPPING_NATIVEKEYBOARD_GETTEXT].c_str(), "()Ljava/lang/String;");
    if (!isDoneMethod || !getTextMethod) {
        env->DeleteLocalRef(activity);
        jvm->DetachCurrentThread();
        return;
    }
    bool done = false;
    std::string tempStr;
    while (!done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        jboolean jdone = env->CallStaticBooleanMethod(nkClass, isDoneMethod);
        done = (jdone == JNI_TRUE);
        jstring jcur = (jstring)env->CallStaticObjectMethod(nkClass, getTextMethod);
        if (jcur) {
            const char* utf = env->GetStringUTFChars(jcur, nullptr);
            if (utf) {
                tempStr = utf;
                env->ReleaseStringUTFChars(jcur, utf);
            }
            env->DeleteLocalRef(jcur);
        }
    }
    jstring jfinal = (jstring)env->CallStaticObjectMethod(nkClass, getTextMethod);
    if (jfinal) {
        const char* utf = env->GetStringUTFChars(jfinal, nullptr);
        if (utf) {
            tempStr = utf;
            env->ReleaseStringUTFChars(jfinal, utf);
        }
        env->DeleteLocalRef(jfinal);
    }
    if (intPointer != nullptr && !tempStr.empty()) {
        *intPointer = std::stoi(tempStr);
    }
    env->DeleteLocalRef(activity);
    jvm->DetachCurrentThread();
}

inline static void NativeKeyboardServiceFloat(float* floatPointer, bool hasOldValue) {
    JavaVM* jvm = JNI_VM;
    JNIEnv* env = nullptr;
    JavaVM_GetEnv(&env);
    jobject activity = getCurrentActivity(env);
    if (!activity) {
        jvm->DetachCurrentThread();
        return;
    }
    jclass nkClass = com_id9909_NativeKeyboard;
    if (!nkClass) {
        env->DeleteLocalRef(activity);
        jvm->DetachCurrentThread();
        return;
    }
    jstring oldJ = nullptr;
    if (hasOldValue && floatPointer != nullptr) {
        oldJ = env->NewStringUTF(std::to_string(*floatPointer).c_str());
    } else {
        oldJ = env->NewStringUTF("");
    }
    jmethodID createMethod = env->GetStaticMethodID(nkClass, JNI_MAPPING[JNIMAPPING_NATIVEKEYBOARD_CREATE].c_str(), "(Landroid/app/Activity;Ljava/lang/String;II)V");
    if (!createMethod) {
        env->DeleteLocalRef(oldJ);
        env->DeleteLocalRef(activity);
        jvm->DetachCurrentThread();
        return;
    }
    env->CallStaticVoidMethod(nkClass, createMethod, activity, oldJ, 20, 2);
    env->DeleteLocalRef(oldJ);
    jmethodID isDoneMethod = env->GetStaticMethodID(nkClass, JNI_MAPPING[JNIMAPPING_NATIVEKEYBOARD_ISDONE].c_str(), "()Z");
    jmethodID getTextMethod = env->GetStaticMethodID(nkClass, JNI_MAPPING[JNIMAPPING_NATIVEKEYBOARD_GETTEXT].c_str(), "()Ljava/lang/String;");
    if (!isDoneMethod || !getTextMethod) {
        env->DeleteLocalRef(activity);
        jvm->DetachCurrentThread();
        return;
    }
    bool done = false;
    std::string tempStr;
    while (!done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        jboolean jdone = env->CallStaticBooleanMethod(nkClass, isDoneMethod);
        done = (jdone == JNI_TRUE);
        jstring jcur = (jstring)env->CallStaticObjectMethod(nkClass, getTextMethod);
        if (jcur) {
            const char* utf = env->GetStringUTFChars(jcur, nullptr);
            if (utf) {
                tempStr = utf;
                env->ReleaseStringUTFChars(jcur, utf);
            }
            env->DeleteLocalRef(jcur);
        }
    }
    jstring jfinal = (jstring)env->CallStaticObjectMethod(nkClass, getTextMethod);
    if (jfinal) {
        const char* utf = env->GetStringUTFChars(jfinal, nullptr);
        if (utf) {
            tempStr = utf;
            env->ReleaseStringUTFChars(jfinal, utf);
        }
        env->DeleteLocalRef(jfinal);
    }
    if (floatPointer != nullptr && !tempStr.empty()) {
        *floatPointer = std::stof(tempStr);
    }
    env->DeleteLocalRef(activity);
    jvm->DetachCurrentThread();
}

inline static void ImGui_InputTextAndroid(const char* label, std::string* textPointer) {
    char* inputText = const_cast<char*>((*textPointer).c_str());
    ImGui::InputText(label, inputText, ImGuiInputTextFlags_ReadOnly);
    if (ImGui::IsItemActivated()) {
        std::thread(NativeKeyboardServiceString, textPointer, true, 11000).detach();
    }
}

inline static void ImGui_InputTextIntAndroid(const char* label, int* intPointer) {
    char buffer[64];
    snprintf(buffer, 64, "%d", *intPointer);
    ImGui::InputText(label, buffer, ImGuiInputTextFlags_ReadOnly);
    if (ImGui::IsItemActivated()) {
        std::thread(NativeKeyboardServiceInt, intPointer, true).detach();
    }
}

inline static void ImGui_InputTextFloatAndroid(const char* label, float* floatPointer) {
    char buffer[64];
    snprintf(buffer, 64, "%f", *floatPointer);
    ImGui::InputText(label, buffer, ImGuiInputTextFlags_ReadOnly);
    if (ImGui::IsItemActivated()) {
        std::thread(NativeKeyboardServiceFloat, floatPointer, true).detach();
    }
}

inline static bool ImGui_WantMouse(float x, float y) {
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (!ctx) return false;
    for (ImGuiWindow* window : ctx->Windows) {
        if (!window->Active || window->Hidden || window->IsFallbackWindow) continue;
        if (window->Flags & ImGuiWindowFlags_NoInputs) continue;
        ImVec2 min = window->Pos;
        ImVec2 max = { window->Pos.x + window->Size.x, window->Pos.y + window->Size.y };
        if (x >= min.x && x < max.x && y >= min.y && y < max.y) {
            return true;
        }
    }
    return false;
}

inline static jobject LoadDEX(JNIEnv* env, const uint8_t* data, size_t size, bool encrypted = false) {
    jclass byteBufferClass = env->FindClass("java/nio/ByteBuffer");
    jmethodID allocateDirectMethod = env->GetStaticMethodID(byteBufferClass, "allocateDirect", "(I)Ljava/nio/ByteBuffer;");
    jobject byteBuffer = env->CallStaticObjectMethod(byteBufferClass, allocateDirectMethod, (jint)size);
    uint8_t* bufferAddress = (uint8_t*)env->GetDirectBufferAddress(byteBuffer);
    memcpy(bufferAddress, data, size);
    if (encrypted) {
        for (size_t i = 0; i < size; i++) {
            bufferAddress[i] ^= ((int)i * 10101011011);
        }
    }
    jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
    jmethodID getSystemClassLoaderMethod = env->GetStaticMethodID(classLoaderClass, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
    jmethodID getParentMethod = env->GetMethodID(classLoaderClass, "getParent", "()Ljava/lang/ClassLoader;");
    jobject systemClassLoader = env->CallStaticObjectMethod(classLoaderClass, getSystemClassLoaderMethod);
    jobject bootClassLoader = env->CallObjectMethod(systemClassLoader, getParentMethod);
    jclass inMemoryDexClassLoaderClass = env->FindClass("dalvik/system/InMemoryDexClassLoader");
    jmethodID constructorMethod = env->GetMethodID(inMemoryDexClassLoaderClass, "<init>", "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
    jobject dexClassLoader = env->NewObject(inMemoryDexClassLoaderClass, constructorMethod, byteBuffer, bootClassLoader);
    jobject globalClassLoader = env->NewGlobalRef(dexClassLoader);
    env->DeleteLocalRef(byteBuffer);
    env->DeleteLocalRef(byteBufferClass);
    env->DeleteLocalRef(classLoaderClass);
    env->DeleteLocalRef(systemClassLoader);
    env->DeleteLocalRef(bootClassLoader);
    env->DeleteLocalRef(inMemoryDexClassLoaderClass);
    env->DeleteLocalRef(dexClassLoader);
    return globalClassLoader;
}

inline static jclass ClassLoader_GetClass(JNIEnv* env, jobject classLoader, const std::string& name) {
    jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
    jmethodID loadClassMethod = env->GetMethodID(classLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    jstring className = env->NewStringUTF(name.c_str());
    jclass loadedClass = (jclass)env->CallObjectMethod(classLoader, loadClassMethod, className);
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(className);
        env->DeleteLocalRef(classLoaderClass);
        return nullptr;
    }
    if (!loadedClass) {
        env->DeleteLocalRef(className);
        env->DeleteLocalRef(classLoaderClass);
        return nullptr;
    }
    jclass result = (jclass)env->NewGlobalRef(loadedClass);
    env->DeleteLocalRef(loadedClass);
    env->DeleteLocalRef(className);
    env->DeleteLocalRef(classLoaderClass);
    return result;
}

#endif