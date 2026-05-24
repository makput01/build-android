-dontwarn **
-ignorewarnings
-renamesourcefileattribute ''
-allowaccessmodification
-repackageclasses ''
-optimizations *
-optimizationpasses 7
-overloadaggressively
-keepattributes RuntimeVisibleAnnotations,Signature,InnerClasses,EnclosingMethod

-keep class a { *; }
-keep,includedescriptorclasses class com.mycompany.application.** { *; }

-keepclassmembers class ** {
    @com.SkipRename <methods>;
}

-keepclassmembers,allowobfuscation class * {
    @com.JniMethodId <methods>;
}

#-assumenosideeffects class android.util.Log { public static *** d(...); public static *** v(...); public static *** i(...); public static *** w(...); public static *** e(...); }