#include <jni.h>

#include <Game.h>
#include <Wrapper.h>
#include <Controls.h>

extern "C" {
    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM * vm, void * reserved);
    JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_GLCreated(JNIEnv *, jclass);
	JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_GLChanged(JNIEnv *, jclass, jint, jint);
	JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_Update(JNIEnv *, jclass);
	JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_OnPointerDown
                                        (JNIEnv *env, jclass obj, jint id, jfloat x, jfloat y);
    JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_OnPointerUp
                                        (JNIEnv *env, jclass obj, jint id, jfloat x, jfloat y);
    JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_OnPointerMove
                                            (JNIEnv *env, jclass obj, jint id, jfloat x, jfloat y);
    JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_OnPause(JNIEnv *, jclass);
    JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_OnResume(JNIEnv *, jclass);
};

static Game& gGame = Game::Get(); //Конструктор Game вызывается тут

static JavaVM* gjvm = 0;
static jclass wrapper = 0;
static jmethodID submitHS;
static jmethodID readHS;
static jmethodID vibrate;

///С++ из Java///

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    gjvm = vm;
    if(gjvm->AttachCurrentThread( (JNIEnv **) &env, NULL) != JNI_OK) return -1;
    jclass local = env->FindClass("com/hardcore/mk/asteroids/NativeWrapper");
    if (!local) return -1;

    wrapper = reinterpret_cast<jclass>(env->NewGlobalRef(local));
    if(wrapper) {
        vibrate = env->GetStaticMethodID(wrapper, "Vibrate", "()V");
        readHS = env->GetStaticMethodID(wrapper, "ReadHighScore", "()I");
        submitHS = env->GetStaticMethodID(wrapper, "SubmitHighScore", "(I)V");
        return JNI_VERSION_1_6;
    } else {
        return -1;
    }
}

JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_GLCreated (JNIEnv *env, jclass obj) {
    gGame.OnGLInit();
}

JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_GLChanged (JNIEnv *env, jclass obj, jint width, jint height) {
    gGame.OnResolutionChange(width, height);
}

JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_Update (JNIEnv *env, jclass obj) {
    gGame.Update();
}

JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_OnPointerDown
                                    (JNIEnv *env, jclass obj, jint id, jfloat x, jfloat y) {
    Controls::onPointerDown(id, x, y);
}

JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_OnPointerUp
                                    (JNIEnv *env, jclass obj, jint id, jfloat x, jfloat y) {
    Controls::onPointerUp(id, x, y);
}

JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_OnPointerMove
                                    (JNIEnv *env, jclass obj, jint id, jfloat x, jfloat y) {
    Controls::onPointerMove(id, x, y);
}

JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_OnPause(JNIEnv *, jclass) {
    gGame.Pause();
}

JNIEXPORT void JNICALL Java_test_zeptoteam_mk_asteroids_NativeWrapper_OnResume(JNIEnv *, jclass) {

}


///Java из C++///

void JavaCall::Vibrate() {
    if(gjvm) {
        JNIEnv* env;
        if(gjvm->AttachCurrentThread( (JNIEnv **) &env, NULL) != JNI_OK) return;
        env->CallStaticVoidMethod(wrapper, vibrate);
    }
}

int JavaCall::ReadHighScore() {
    if(gjvm) {
        JNIEnv* env;
        if(gjvm->AttachCurrentThread( (JNIEnv **) &env, NULL) != JNI_OK) return 0;
        return env->CallStaticIntMethod(wrapper, readHS);
    }
    return 0;
}

void JavaCall::SubmitHighScore(int score) {
    if(gjvm) {
        JNIEnv* env;
        if(gjvm->AttachCurrentThread( (JNIEnv **) &env, NULL) != JNI_OK) return;
        env->CallStaticVoidMethod(wrapper, submitHS, score);
    }
}