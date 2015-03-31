package test.zeptoteam.mk.asteroids;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Build;

public class MainView extends GLSurfaceView {

	public MainView(Context context) {
		super(context);
		setEGLContextClientVersion(2);
		setEGLConfigChooser(new MultiSampleConfigChooser(16));
		setRenderer(new Renderer());
	}

    //Включаем сглаживание
	private static class MultiSampleConfigChooser implements GLSurfaceView.EGLConfigChooser {
		
		private int mSamples;

		public MultiSampleConfigChooser(int samples) {
			mSamples = samples;
		}
		
		//Выбираем конфигурацию с ближайшим к samples количеством образцов
		@Override
		public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
            int[] configSpec = {
            		EGL10.EGL_SAMPLE_BUFFERS, 1,
            		EGL10.EGL_SAMPLES, 2,
            		EGL10.EGL_NONE
            };
            int[] configNum = new int[1];
            
            if (!egl.eglChooseConfig(display, configSpec, null, 0, configNum)) {
                configNum[0] = 0;
            }    

            if ((configNum[0] <= 0) || (mSamples < 2)) {
                //Если не нашли конфигурацию со сглаживанием, то ищем без
            	EGLConfig[] configs = new EGLConfig[1];
            	if (!egl.eglChooseConfig(display, new int[] {EGL10.EGL_NONE}, configs, 1, configNum)) {
                    throw new IllegalArgumentException("eglChooseConfig without MS failed");
                } 
            	if (configNum[0] <= 0) {
            		throw new IllegalArgumentException("No config chosen");
            	}
            	return configs[0];
            } else {
            	EGLConfig[] configs = new EGLConfig[configNum[0]];
                if (!egl.eglChooseConfig(display, configSpec, configs, configs.length, configNum)) {
                    throw new IllegalArgumentException("No config chosen");
                }
                int[] samplesNum = new int[1];
                int prevSamplesNum = -1;
                int configIndex = 0;
                for(int i = 0; i < configs.length; ++i) {
                	egl.eglGetConfigAttrib(display, configs[i], EGL10.EGL_SAMPLES, samplesNum);
                	if((samplesNum[0] <= mSamples) && (prevSamplesNum < samplesNum[0])) {
                		configIndex = i;
                	}
                	prevSamplesNum = samplesNum[0];
                }
                return configs[configIndex];
            }            
		}
		
	}

	private static class Renderer implements GLSurfaceView.Renderer {

		@Override
		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            NativeWrapper.GLCreated();
		}

		@Override
		public void onSurfaceChanged(GL10 gl, int width, int height) {
			NativeWrapper.GLChanged(width, height);
		}

		@Override
		public void onDrawFrame(GL10 gl) {
            NativeWrapper.Update();
		}

    }
}
