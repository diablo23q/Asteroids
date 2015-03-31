package test.zeptoteam.mk.asteroids;

import android.app.Activity;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;

public class MainActivity extends Activity {

	private MainView mView;
	
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mView = new MainView(getApplicationContext());
        mView.setOnTouchListener(new View.OnTouchListener() {

            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (event != null) {
                    final int action = event.getActionMasked();
                    final int index = event.getActionIndex();
                    final int pointerId = event.getPointerId(index);

                    //Координаты касаний передаем сразу в нормализованном виде
                    final float normX = ((event.getX(index) / (float) v.getWidth()) * 2 - 1) * v.getWidth() / (float) v.getHeight();
                    final float normY = -((event.getY(index) / (float) v.getHeight()) * 2 - 1);

                    switch (action) {
                        case MotionEvent.ACTION_UP:
                        case MotionEvent.ACTION_POINTER_UP:
                        case MotionEvent.ACTION_CANCEL:
                            mView.queueEvent(new Runnable() {
                                @Override
                                public void run() {
                                    NativeWrapper.OnPointerUp(pointerId, normX, normY);
                                }
                            });
                            return true;
                        case MotionEvent.ACTION_DOWN:
                        case MotionEvent.ACTION_POINTER_DOWN:
                            mView.queueEvent(new Runnable() {
                                @Override
                                public void run() {
                                    NativeWrapper.OnPointerDown(pointerId, normX, normY);
                                }
                            });
                            return true;
                        case MotionEvent.ACTION_MOVE:
                            mView.queueEvent(new Runnable() {
                                @Override
                                public void run() {
                                    NativeWrapper.OnPointerMove(pointerId, normX, normY);
                                }
                            });
                            return true;
                        default:
                            return false;
                    }
                } else {
                    return false;
                }
            }

        });
        setContentView(mView);
    }
    
    @Override
    protected void onPause() {
    	super.onPause();
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                NativeWrapper.OnPause();
            }
        });
    	mView.onPause();
    }
    
    @Override
    protected void onResume() {
    	super.onResume();
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                NativeWrapper.OnResume();
            }
        });
    	mView.onResume();
    }
}
