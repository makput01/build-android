#ifndef MY_PROJECT_NATIVE_LIB_H
#define MY_PROJECT_NATIVE_LIB_H

#include "Includes/log.h"
#include "Includes/utils.h"
#include "Includes/classesdex.h"

using namespace std;

inline JavaVM* JNI_VM = nullptr;
inline jclass com_id9909_ImGuiManager = nullptr;
inline jclass com_id9909_NativeKeyboard = nullptr;
inline vector<string> JNI_MAPPING = {};
inline bool ImGui_Inited = false;
inline int glWidth = 0;
inline int glHeight = 0;

void* hack_initchecker(void*);

#endif //MY_PROJECT_NATIVE_LIB_H