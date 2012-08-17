/*  Copyright 2011 Guillaume Duhamel

    This file is part of Yabause.

    Yabause is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Yabause is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yabause; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

package org.yabause.android;

import java.lang.Runnable;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.View;

class YabauseView extends SurfaceView implements Callback, Runnable, View.OnKeyListener, View.OnTouchListener{
    private static String TAG = "YabauseView";
    private static final boolean DEBUG = false; 
    private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

    private int axisX = 0; 
    private int axisY = 0;

    public boolean[] pointers = new boolean[256];
    public int[] pointerX = new int[256];
    public int[] pointerY = new int[256];
    
    private YabauseRunnable _Runnable = null;
    Thread thread;
   
    private EGLContext mEglContext;
    private EGLDisplay mEglDisplay;
    private EGLSurface mEglSurface;
    private EGLConfig mEglConfig;
   
   
    public YabauseView(Context context, AttributeSet attrs) {
        super(context,attrs);
        init(false, 0, 0);
    }   
     
    public YabauseView(Context context) {
        super(context); 
        init(false, 0, 0);     
    } 
    
    public YabauseView(Context context, boolean translucent, int depth, int stencil) {
        super(context);
        init(translucent, depth, stencil);
    }    
          
    public void setYabauseRunnable( YabauseRunnable runnable )
    { 
       _Runnable = runnable;        
    }   
             
    private void init(boolean translucent, int depth, int stencil) {

       setFocusable( true );
       setFocusableInTouchMode( true );
       requestFocus();
       setOnKeyListener( this );
       setOnTouchListener( this );  
        
       getHolder().addCallback(this);
       getHolder().setType(SurfaceHolder.SURFACE_TYPE_GPU);
       initGLES();

    }
    
    private boolean initGLES(){

        EGL10 egl = (EGL10)EGLContext.getEGL();
        
        mEglDisplay = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
        if( mEglDisplay == EGL10.EGL_NO_DISPLAY ){
            Log.e(TAG, "Fail to get Display");
            return false;
        }
            
        int[] version = new int[2];
        if( !egl.eglInitialize(mEglDisplay, version) ){
            Log.e(TAG, "Fail to eglInitialize");
            return false;
        }
        
        int[] configSpec = {
             EGL10.EGL_NONE
         };
            EGLConfig[] configs = new EGLConfig[1];
        
        int[] numConfigs = new int[1];
        if( !egl.eglChooseConfig(mEglDisplay, configSpec, configs, 1, numConfigs) ){
            Log.e(TAG, "Fail to Choose Config");
            return false;
        }
        mEglConfig = configs[0];
        
        int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
                
        mEglContext = egl.eglCreateContext(mEglDisplay, mEglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
        if( mEglContext == EGL10.EGL_NO_CONTEXT ){
            Log.e(TAG, "Fail to Create OpenGL Context");
            return false;
        }
        return true;
    }
    
    
    private boolean createSurface(){
        EGL10 egl = (EGL10)EGLContext.getEGL();
        mEglSurface = egl.eglCreateWindowSurface(mEglDisplay, mEglConfig, getHolder(), null);
        if( mEglSurface == EGL10.EGL_NO_SURFACE ){
            return false;
        }
        return true;
    }   
    
    private void endGLES(){
        EGL10 egl = (EGL10)EGLContext.getEGL();
        if( mEglSurface != null){
            //レンダリングコンテキストとの結びつけは解除
            egl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
            
            egl.eglDestroySurface(mEglDisplay, mEglSurface);
            mEglSurface = null;
        }
        
        if( mEglContext != null ){
            egl.eglDestroyContext(mEglDisplay, mEglContext);
            mEglContext = null;
        }
        
        if( mEglDisplay != null){
            egl.eglTerminate(mEglDisplay);
            mEglDisplay = null;
        }
    }   
   
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
          
        EGL10 egl = (EGL10)EGLContext.getEGL();
          
        YabauseRunnable.lockGL();
        egl.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext);     
        _Runnable.setUp();                
        YabauseRunnable.initViewport(width, height); 
        YabauseRunnable.unlockGL();
        
    }
    
    @Override
    public void run() {
       
       try {
        Thread.sleep(1000);
      } catch (InterruptedException e) {
        e.printStackTrace();
      }

       
       while(thread!=null)
       {
         YabauseRunnable.lockGL();
         _Runnable.run();
         YabauseRunnable.unlockGL();
       }
       
    }
 
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if( !createSurface() ){
             Log.e(TAG, "Fail to Creat4e Surface");
             return ;
        }
        thread = new Thread(this);
        thread.start();        
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        thread = null;
    }    

    // Key events
    public boolean onKey( View  v, int keyCode, KeyEvent event )
    {
        if( Yabause.mSingleton == null )
            return false;

        int key = keyCode;
        float str = 0;
        if( keyCode > 255 && Globals.analog_100_64 )
        {
            key = (int) ( keyCode / 100 );
            if( event.getAction() == KeyEvent.ACTION_DOWN )
                str = ( (float) keyCode - ( (float) key * 100.0f ) );
        }
        else if( event.getAction() == KeyEvent.ACTION_DOWN )
            str = 64.0f;
        int scancode;
        if( key < 0 || key > 255 )
            scancode = 0;
        else
            scancode = key;
        for( int p = 0; p < 2; p++ )
        {
            if( Globals.analog_100_64 && ( scancode == Globals.ctrlr[p][Globals.ANALOGR] || scancode == Globals.ctrlr[p][Globals.ANALOGL] ||
                                           scancode == Globals.ctrlr[p][Globals.ANALOGD] || scancode == Globals.ctrlr[p][Globals.ANALOGU] ) )
            {
                if( scancode == Globals.ctrlr[p][Globals.ANALOGR] )
                    axisX = (int) (80.0f * (str / 64.0f));
                else if( scancode == Globals.ctrlr[p][Globals.ANALOGL] )
                    axisX = (int) (-80.0f * (str / 64.0f));
                else if( scancode == Globals.ctrlr[p][Globals.ANALOGD] )
                    axisY = (int) (-80.0f * (str / 64.0f));
                else if( scancode == Globals.ctrlr[p][Globals.ANALOGU] )
                    axisY = (int) (80.0f * (str / 64.0f));
                // TODO: implement analog
                return true;
            }
        }
        if( event.getAction() == KeyEvent.ACTION_DOWN )
        {
            if( key == KeyEvent.KEYCODE_MENU )
                return false;
            if( key == KeyEvent.KEYCODE_VOLUME_UP ||
                key == KeyEvent.KEYCODE_VOLUME_DOWN )
            {
                if( Globals.volumeKeysDisabled )
                {
                    Yabause.keyDown( key );
                    return true;
                }
                return false;
            }
            Yabause.keyDown( key );
            return true;
        }
        else if( event.getAction() == KeyEvent.ACTION_UP )
        {
            if( key == KeyEvent.KEYCODE_MENU )
                return false;

            if( key == KeyEvent.KEYCODE_VOLUME_UP ||
                key == KeyEvent.KEYCODE_VOLUME_DOWN )
            {
                if( Globals.volumeKeysDisabled )
                {
                    Yabause.keyUp( key );
                    return true;
                }
                return false;
            }
            Yabause.keyUp( key );
            return true;
        }
        return false;
    }

    public boolean onTouch( View v, MotionEvent event )
    {
        if( Yabause.mSingleton == null )
            return false;
        int action = event.getAction();
        int actionCode = action & MotionEvent.ACTION_MASK;
        float x = event.getX();
        float y = event.getY();
        float p = event.getPressure();

        int maxPid = 0;
        int pid, i;
        if( actionCode == MotionEvent.ACTION_POINTER_DOWN )
        {
            pid = event.getPointerId( action >> MotionEvent.ACTION_POINTER_ID_SHIFT );
            if( pid > maxPid )
                maxPid = pid;
            pointers[pid] = true;
        }
        else if( actionCode == MotionEvent.ACTION_POINTER_UP )
        {
            pid = event.getPointerId( action >> MotionEvent.ACTION_POINTER_ID_SHIFT );
            if( pid > maxPid )
                maxPid = pid;
            pointers[pid] = false;
        }
        else if( actionCode == MotionEvent.ACTION_DOWN )
        {
            for( i = 0; i < event.getPointerCount(); i++ )
            {
                pid = event.getPointerId(i);
                if( pid > maxPid )
                    maxPid = pid;
                pointers[pid] = true;
            }
        }
        else if( actionCode == MotionEvent.ACTION_UP ||
                 actionCode == MotionEvent.ACTION_CANCEL )
        {
            for( i = 0; i < 256; i++ )
            {
                pointers[i] = false;
                pointerX[i] = -1;
                pointerY[i] = -1;
            }
        }

        for( i = 0; i < event.getPointerCount(); i++ )
        {
            pid = event.getPointerId(i);
            if( pointers[pid] )
            {
                if( pid > maxPid )
                    maxPid = pid;
                pointerX[pid] = (int) event.getX(i);
                pointerY[pid] = (int) event.getY(i);
            }
        }
        Yabause.mGamePad.updatePointers( pointers, pointerX, pointerY, maxPid );
        return true;
    }
    
}
