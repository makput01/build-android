import com.JniMethodId;
import com.id9909.*;
import java.lang.reflect.Method;

public class a {
    public static String[] get() {
        Class imGuiManager = ImGuiManager.class;
        Class nativeKeyboard = NativeKeyboard.class;
        String[] result = new String[] {
                imGuiManager.getName(),   // 0
                nativeKeyboard.getName(), // 1
                "",                       // 2
                b(imGuiManager, 1),       // 3 - bridge_init
                b(imGuiManager, 2),       // 4 - bridge_render
                b(imGuiManager, 3),       // 5 - bridge_resize
                b(imGuiManager, 4),       // 6 - bridge_click
                b(imGuiManager, 5),       // 7 - Init
                b(nativeKeyboard, 1),     // 8 - Create
                b(nativeKeyboard, 2),     // 9 - Destroy
                b(nativeKeyboard, 3),     // 10 - isDone
                b(nativeKeyboard, 4),     // 11 - getText
        };
        return result;
    }

    private static String b(Class cls, int id) {
        for (Method m : cls.getDeclaredMethods()) {
            try {
                JniMethodId annotation = m.getAnnotation(JniMethodId.class);
                if (annotation != null && annotation.value() == id) {
                    return m.getName();
                }
            } catch (Exception e) { continue; }
        }
        return null;
    }
}