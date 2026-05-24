#include "native-lib.h"

void ImGuiManager_Init() {
    if (ImGui_Inited) return;
    LOGI("ImGuiManager_Init");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImFontConfig robotFont;
    robotFont.FontDataOwnedByAtlas = false;
    io.Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(Roboto_Regular), sizeof(Roboto_Regular), 30.f, &robotFont, io.Fonts->GetGlyphRangesCyrillic());
    io.ConfigWindowsMoveFromTitleBarOnly = false;
    ImGui_Inited = true;
}

string testinput = "пусто";
int testinputint = 9909;
float testinputfloat = 69.6969;

void ImGuiManager_Render() {
    static bool imguiDebug = false;
    if (!ImGui_Inited) return;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    if (imguiDebug) ImGui::ShowDemoWindow(&imguiDebug);
    ImGui::SetNextWindowSize(ImVec2(1000, 500), ImGuiCond_Once);
    ImGui::Begin("ImGui | by id9909");
    ImGui::Text("privet xd");
    ImGui::Text("input text: %s", testinput.c_str());
    ImGui::Text("input int: %d", testinputint);
    ImGui::Text("input float: %f", testinputfloat);
    ImGui::Checkbox("ImGui Debug", &imguiDebug);
    ImGui_InputTextAndroid("string", &testinput);
    ImGui_InputTextIntAndroid("int", &testinputint);
    ImGui_InputTextFloatAndroid("float", &testinputfloat);
    ImGui::End();
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiManager_Resize(int w, int h) {
    LOGI("ImGuiManager_Resize %d %d", w, h);
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2((float)w, (float)h);
    glWidth = w;
    glHeight = h;
}

bool ImGuiManager_Click(int action, float x, float y) {
    ImGuiIO& io = ImGui::GetIO();
    switch (action) {
        case 0: // ACTION_DOWN
            if (io.MouseDown[0]) io.AddMouseButtonEvent(0, false);
            io.AddMousePosEvent(x, y);
            io.AddMouseButtonEvent(0, true);
            break;
        case 1: // ACTION_UP
            io.AddMousePosEvent(x, y);
            io.AddMouseButtonEvent(0, false);
            break;
        case 2: // ACTION_MOVE
            io.AddMousePosEvent(x, y);
            break;
        case 3: // ACTION_CANCEL
            if (io.MouseDown[0]) io.AddMouseButtonEvent(0, false);
            break;
        default: break;
    }
    return ImGui_WantMouse(x, y);
}

void com_id9909_ImGuiManager_init(JNIEnv* env, jclass) {
    if (!ImGui_Inited) ImGuiManager_Init();
}

void com_id9909_ImGuiManager_render(JNIEnv* env, jclass) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (ImGui_Inited) ImGuiManager_Render();
}

void com_id9909_ImGuiManager_resize(JNIEnv* env, jclass, jint w, jint h) {
    if (ImGui_Inited) ImGuiManager_Resize(w, h);
}

jboolean com_id9909_ImGuiManager_click(JNIEnv* env, jclass, jint action, jfloat x, jfloat y) {
    if (ImGui_Inited) return ImGuiManager_Click(action, x, y) ? JNI_TRUE : JNI_FALSE;
    else return JNI_FALSE;
}

void com_id9909_LuaRunnable_nativeRun(JNIEnv* env, jclass, jint ref) {
    //LuaApi::Runnable_nativeRun(ref);
}

void com_id9909_LuaProxy_nativeInvoke(JNIEnv*, jclass, jint ref) {
    //LuaApi::LuaProxy_nativeInvoke(ref);
}

void com_id9909_LuaProxy_nativeDestroy(JNIEnv*, jclass, jint ref) {
    //LuaApi::LuaProxy_nativeDestroy(ref);
}

void hack_art_init(JNIEnv** result_env = nullptr, ElfScanner* result_libart = nullptr) {
    LOGD("hack_art_init");
    ElfScanner libart;
    jint (*jgetCreatedJavaVMs)(JavaVM**, jsize, jsize*);
    while (!(libart = ElfScanner::findElf("libart.so", EScanElfType::Any, EScanElfFilter::System)).isValid()) usleep(1000);
    LOGI("libart.so found: 0x%lx - 0x%lx (0x%lx)", libart.base(), libart.end(), libart.end() - libart.base());
    uintptr_t ptr_jgetCreatedJavaVMs = libart.findSymbol("JNI_GetCreatedJavaVMs");
    LOGD("Finded JNI_GetCreatedJavaVMs at 0x%lx", (uintptr_t)ptr_jgetCreatedJavaVMs);
    o2m(ptr_jgetCreatedJavaVMs, jgetCreatedJavaVMs);
    JavaVM* vms[1];
    jsize vmCount = 0;
    if (jgetCreatedJavaVMs(vms, 1, &vmCount) != JNI_OK || vmCount == 0) {
        LOGE("Failed to get JVM");
        return;
    }
    JavaVM* vm = vms[0];
    JNIEnv* env = nullptr;
    LOGD("Finded %d JVMs, vm=0x%lx", vmCount, (uintptr_t)vm);
    JNI_VM = vm;
    JavaVM_GetEnv(&env);
    if (!env) { LOGE("Failed to get JNIEnv"); return; }
    if (result_env) *result_env = env;
    if (result_libart) *result_libart = libart;
    LOGD("hack_art_init done");
}

void* imgui_initthread(void* p0) {
    LOGD("imgui_initthread");
    JNIEnv* env = nullptr;
    jobject com_id9909 = nullptr;
    hack_art_init(&env);
    if (!JNI_VM || !env) return NULL;
    LOGD("JavaVM && JNIEnv not null");
    *(bool*) p0 = true;
    jobject context = nullptr;
    for (int i = 0; i < 10001 && !context; i++) {
        usleep(10000);
        context = getCurrentActivity(env);
    }
    if (!context) return NULL;
    LOGD("Context not null");
    com_id9909 = LoadDEX(env, com_id9909_dex, com_id9909_dex_len, true);
    if (!com_id9909) return NULL;
    LOGD("Dex loaded");
    jclass mappingManager = ClassLoader_GetClass(env, com_id9909, "a");
    if (!mappingManager) return NULL;
    jmethodID getMapping = env->GetStaticMethodID(mappingManager, "get", "()[Ljava/lang/String;");
    if (!getMapping) return NULL;
    jobjectArray arr = (jobjectArray) env->CallStaticObjectMethod(mappingManager, getMapping);
    if (!arr) return NULL;
    JNI_MAPPING = jsa2vstd(env, arr);
    com_id9909_ImGuiManager = ClassLoader_GetClass(env, com_id9909, JNI_MAPPING[JNIMAPPING_IMGUIMANAGER]);
    if (!com_id9909_ImGuiManager) return NULL;
    com_id9909_NativeKeyboard = ClassLoader_GetClass(env, com_id9909, JNI_MAPPING[JNIMAPPING_NATIVEKEYBOARD]);
    if (!com_id9909_NativeKeyboard) return NULL;
    env->DeleteGlobalRef(mappingManager);
    env->DeleteLocalRef(arr);
    LOGD("Classes loaded");
    const JNINativeMethod bridgeMethods[] = {
            {JNI_MAPPING[JNIMAPPING_IMGUIMANAGER_BRIDGE_INIT].c_str(),   "()V",    (void*)com_id9909_ImGuiManager_init},
            {JNI_MAPPING[JNIMAPPING_IMGUIMANAGER_BRIDGE_RENDER].c_str(), "()V",    (void*)com_id9909_ImGuiManager_render},
            {JNI_MAPPING[JNIMAPPING_IMGUIMANAGER_BRIDGE_RESIZE].c_str(), "(II)V",  (void*)com_id9909_ImGuiManager_resize},
            {JNI_MAPPING[JNIMAPPING_IMGUIMANAGER_BRIDGE_CLICK].c_str(),  "(IFF)Z", (void*)com_id9909_ImGuiManager_click},
    };
    if (env->RegisterNatives(com_id9909_ImGuiManager, bridgeMethods, 4) != JNI_OK) return NULL;
    LOGD("Natives registered");
    jmethodID initMethod = env->GetStaticMethodID(com_id9909_ImGuiManager, JNI_MAPPING[JNIMAPPING_IMGUIMANAGER_INIT].c_str(), "(Landroid/app/Activity;)V");
    if (!initMethod) return NULL;
    env->CallStaticVoidMethod(com_id9909_ImGuiManager, initMethod, context);
    LOGD("ImGuiManager Init done");
    env->DeleteLocalRef(context);
    LOGD("imgui_initthread done");
    return NULL;
}

__attribute__((constructor))
void lib_constructor() {
    LOGD("lib_constructor");
    bool done = false;
    if (pthread_t currentThread = pthread_self()) pthread_create(&currentThread, NULL, imgui_initthread, (void*)&done);
    while (!done) { usleep(10000); }
    usleep(1);
    LOGD("lib_constructor done");
}