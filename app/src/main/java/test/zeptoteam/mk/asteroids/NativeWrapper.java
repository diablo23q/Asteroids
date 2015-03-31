package test.zeptoteam.mk.asteroids;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Vibrator;

public class NativeWrapper {
	
	static {
        System.loadLibrary("Asteroids");
    }

    public static native void GLCreated();

	public static native void GLChanged(int width, int height);
	
	public static native void Update();

    public static native void OnPointerDown(int id, float x, float y);

    public static native void OnPointerUp(int id, float x, float y);

    public static native void OnPointerMove(int id, float x, float y);

    public static native void OnPause();

    public static native void OnResume();

    public static void Vibrate() {
        Handler mainHandler = new Handler(Looper.getMainLooper());
        Runnable r = new Runnable() {
            @Override
            public void run() {
                ((Vibrator) App.getContext().getSystemService(Context.VIBRATOR_SERVICE)).vibrate(50);
            }
        };
        mainHandler.post(r);
    }

    public static int ReadHighScore() {
        return App.getContext().getSharedPreferences("Asteroids", Context.MODE_PRIVATE).getInt("HighScore", 0);
    }

    public static void SubmitHighScore(int score) {
        App.getContext().getSharedPreferences("Asteroids", Context.MODE_PRIVATE).edit().putInt("HighScore", score).commit();
    }

}
