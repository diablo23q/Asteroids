package test.zeptoteam.mk.asteroids;

import android.app.Application;
import android.content.Context;

//http://stackoverflow.com/a/5114361/2502024
public class App extends Application {

    private static Context context;

    public void onCreate(){
        super.onCreate();
        App.context = getApplicationContext();
    }

    public static Context getContext() {
        return App.context;
    }
}