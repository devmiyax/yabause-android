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

#include <stdint.h>
#include <jni.h>
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer
#include <android/bitmap.h>
#include <android/log.h>

#include "../../config.h"
#include "yabause.h"
#include "scsp.h"
#include "vidsoft.h"
#include "vidogl.h"
#include "peripheral.h"
#include "m68kcore.h"
#include "sh2core.h"
#include "sh2int.h"
#include "cdbase.h"
#include "cs2.h"
#include "debug.h"

#include <stdio.h>
#include <dlfcn.h>

static jclass yclass;
static jmethodID countFrame;

//#define _ANDROID_2_2_
#ifdef _ANDROID_2_2_
#include "miniegl.h"
#else
#include <EGL/egl.h>
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <pthread.h>

#include "sndopensl.h"

JavaVM * yvm;
static jobject yabause;
static jobject ybitmap;

static char biospath[256] = "/mnt/sdcard/jap.rom";
static char cdpath[256] = "\0";
static char buppath[256] = "\0";
static char mpegpath[256] = "\0";
static char cartpath[256] = "\0";

EGLDisplay g_Display = EGL_NO_DISPLAY;
EGLSurface g_Surface = EGL_NO_SURFACE;
EGLContext g_Context = EGL_NO_CONTEXT;
ANativeWindow *g_window = 0;
GLuint g_FrameBuffer  = 0;
GLuint g_VertexBuffer = 0;
GLuint programObject  = 0;
GLuint positionLoc    = 0;
GLuint texCoordLoc    = 0;
GLuint samplerLoc     = 0;

int g_buf_width = -1;
int g_buf_height = -1;
pthread_mutex_t g_mtxGlLock = PTHREAD_MUTEX_INITIALIZER;
float vertices [] = {
   -1.0f, 1.0f, 0, 0,
   1.0f, 1.0f, 0, 0,
   1.0f, -1.0f, 0, 0,
   -1.0f,-1.0f, 0, 0
};

enum RenderThreadMessage {
        MSG_NONE = 0,
        MSG_WINDOW_SET,
        MSG_RENDER_LOOP_EXIT
};

int g_msg = MSG_NONE;
pthread_t _threadId;

M68K_struct * M68KCoreList[] = {
&M68KDummy,
#ifdef HAVE_C68K
&M68KC68K,
#endif
#ifdef HAVE_Q68
&M68KQ68,
#endif
NULL
};

SH2Interface_struct *SH2CoreList[] = {
&SH2Interpreter,
&SH2DebugInterpreter,
#ifdef SH2_DYNAREC
&SH2Dynarec,
#endif
NULL
};

PerInterface_struct *PERCoreList[] = {
&PERDummy,
NULL
};

CDInterface *CDCoreList[] = {
&DummyCD,
&ISOCD,
NULL
};

SoundInterface_struct *SNDCoreList[] = {
&SNDDummy,
&SNDOpenSL,
NULL
};

VideoInterface_struct *VIDCoreList[] = {
&VIDDummy,
&VIDSoft,
&VIDOGL,
NULL
};


#define  LOG_TAG    "yabause"

/* Override printf for debug*/
int printf( const char * fmt, ... )
{
   va_list ap;
   va_start(ap, fmt);
   int result = __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, fmt, ap);
   va_end(ap);
   return result;
}

/* Override printf for debug*/
int xprintf( const char * fmt, ... )
{
   va_list ap;
   va_start(ap, fmt);
   int result = __android_log_vprint(ANDROID_LOG_INFO, LOG_TAG, fmt, ap);
   va_end(ap);
   return result;
}

void YuiErrorMsg(const char *string)
{
    jclass yclass;
    jmethodID errorMsg;
    jstring message;
    JNIEnv * env;
    if ((*yvm)->GetEnv(yvm, (void**) &env, JNI_VERSION_1_6) != JNI_OK)
        return;

    yclass = (*env)->GetObjectClass(env, yabause);
    errorMsg = (*env)->GetMethodID(env, yclass, "errorMsg", "(Ljava/lang/String;)V");
    message = (*env)->NewStringUTF(env, string);
    (*env)->CallVoidMethod(env, yabause, errorMsg, message);
}

void* threadStartCallback(void *myself);

void YuiSwapBuffers(void)
{
   if( g_Display == EGL_NO_DISPLAY )
   {
      return;
   }

   eglSwapBuffers(g_Display,g_Surface);
}

JNIEXPORT int JNICALL Java_org_yabause_android_YabauseRunnable_initViewport( JNIEnv* jenv, jobject obj, jobject surface, int width, int height)
{
   int swidth;
   int sheight;
   int error;
   char * buf;
   EGLContext Context;
   EGLConfig cfg;
   int configid;
   int attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
   int config_attr_list[] = {EGL_CONFIG_ID,0,EGL_NONE} ;
   EGLint num_config;

    if (surface != 0) {
        g_window = ANativeWindow_fromSurface(jenv, surface);
        printf("Got window %p", g_window);
    } else {
        printf("Releasing window");
        ANativeWindow_release(g_window);
    }

    g_msg = MSG_WINDOW_SET;

   return 0;
}

int Java_org_yabause_android_YabauseRunnable_lockGL()
{
   pthread_mutex_lock(&g_mtxGlLock);
}

int Java_org_yabause_android_YabauseRunnable_unlockGL()
{
   pthread_mutex_unlock(&g_mtxGlLock);
}


jint
Java_org_yabause_android_YabauseRunnable_init( JNIEnv* env, jobject obj, jobject yab, jobject bitmap )
{
    int res;

    yabause = (*env)->NewGlobalRef(env, yab);
    ybitmap = (*env)->NewGlobalRef(env, bitmap);

    jmethodID getCDImage, getBios;
    jstring filename;
    yclass = (*env)->GetObjectClass(env, yabause);
    getCDImage = (*env)->GetStaticMethodID(env, yclass, "getCDImage", "()Ljava/lang/Object;");
    getBios = (*env)->GetStaticMethodID(env, yclass, "getBios", "()Ljava/lang/Object;");
    countFrame = (*env)->GetStaticMethodID(env, yclass, "countFrame", "()V");

    filename = (jstring) (*env)->CallStaticObjectMethod( env, yclass, getCDImage );
    const char *nativeS = (*env)->GetStringUTFChars( env, filename, 0 );
    strcpy( cdpath, nativeS );

    (*env)->ReleaseStringUTFChars( env, filename, nativeS );

    filename = (jstring) (*env)->CallStaticObjectMethod( env, yclass, getBios );
    nativeS = (*env)->GetStringUTFChars( env, filename, 0 );
    strcpy( biospath, nativeS );
    (*env)->ReleaseStringUTFChars( env, filename, nativeS);


    pthread_create(&_threadId, 0, threadStartCallback, NULL );

    return res;
}

int initEgl( ANativeWindow* window )
{
    int res;
    yabauseinit_struct yinit;
    void * padbits;

     const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE,24,
        EGL_STENCIL_SIZE,8,
        EGL_NONE
    };
    EGLDisplay display;
    EGLConfig config;
    EGLint numConfigs;
    EGLint format;
    EGLSurface surface;
    EGLContext context;
    EGLint width;
    EGLint height;
    int tmp;
    GLfloat ratio;
    int attrib_list[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };


    printf("Initializing context");

    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        printf("eglGetDisplay() returned error %d", eglGetError());
        return -1;
    }
    if (!eglInitialize(display, 0, 0)) {
        printf("eglInitialize() returned error %d", eglGetError());
        return -1;
    }

    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
        printf("eglChooseConfig() returned error %d", eglGetError());
        destroy();
        return -1;
    }

    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
        printf("eglGetConfigAttrib() returned error %d", eglGetError());
        destroy();
        return -1;
    }

    printf("ANativeWindow_setBuffersGeometry");
    ANativeWindow_setBuffersGeometry(window, 0, 0, format);

    printf("eglCreateWindowSurface");
    if (!(surface = eglCreateWindowSurface(display, config, window, 0))) {
        printf("eglCreateWindowSurface() returned error %d", eglGetError());
        destroy();
        return -1;
    }

    printf("eglCreateContext");
    if (!(context = eglCreateContext(display, config, 0, attrib_list))) {
        printf("eglCreateContext() returned error %d", eglGetError());
        destroy();
        return -1;
    }

    printf("eglMakeCurrent");
    if (!eglMakeCurrent(display, surface, surface, context)) {
        printf("eglMakeCurrent() returned error %d", eglGetError());
        destroy();
        return -1;
    }
   glClearColor( 0.0f, 0.0f,0.0f,1.0f);
    if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
        !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
        printf("eglQuerySurface() returned error %d", eglGetError());
        destroy();
        return -1;
    }


    g_Display = display;
    g_Surface = surface;
    g_Context = context;

//    g_width = width;
//    g_height = height;

/*
   tmp = width / 320;
   width = 320 * tmp;
   tmp = height /224;
   height = 224 * tmp;
   width = 320 * tmp;
*/
   printf("%s",glGetString(GL_VENDOR));
   printf("%s",glGetString(GL_RENDERER));
   printf("%s",glGetString(GL_VERSION));
   printf("%s",glGetString(GL_EXTENSIONS));
   printf("%s",eglQueryString(g_Display,EGL_EXTENSIONS));


    yinit.m68kcoretype = M68KCORE_C68K;
    yinit.percoretype = PERCORE_DUMMY;
#ifdef SH2_DYNAREC
    yinit.sh2coretype = 2;
#else
    yinit.sh2coretype = SH2CORE_DEFAULT;
#endif
    //yinit.vidcoretype = VIDCORE_SOFT;
    yinit.vidcoretype = 1;
    yinit.sndcoretype = SNDCORE_OPENSL;
    //yinit.sndcoretype = SNDCORE_DUMMY;
    //yinit.cdcoretype = CDCORE_DEFAULT;
    yinit.cdcoretype = CDCORE_ISO;
    yinit.carttype = CART_NONE;
    yinit.regionid = 0;
    yinit.biospath = biospath;
    yinit.cdpath = cdpath;
    yinit.buppath = buppath;
    yinit.mpegpath = mpegpath;
    yinit.cartpath = cartpath;
    yinit.videoformattype = VIDEOFORMATTYPE_NTSC;
    yinit.frameskip = 1;

    res = YabauseInit(&yinit);

    PerPortReset();
    padbits = PerPadAdd(&PORTDATA1);

    PerSetKey(0, PERPAD_UP, padbits);
    PerSetKey(1, PERPAD_RIGHT, padbits);
    PerSetKey(2, PERPAD_DOWN, padbits);
    PerSetKey(3, PERPAD_LEFT, padbits);
    PerSetKey(4, PERPAD_RIGHT_TRIGGER, padbits);
    PerSetKey(5, PERPAD_LEFT_TRIGGER, padbits);
    PerSetKey(6, PERPAD_START, padbits);
    PerSetKey(7, PERPAD_A, padbits);
    PerSetKey(8, PERPAD_B, padbits);
    PerSetKey(9, PERPAD_C, padbits);
    PerSetKey(10, PERPAD_X, padbits);
    PerSetKey(11, PERPAD_Y, padbits);
    PerSetKey(12, PERPAD_Z, padbits);

    ScspSetFrameAccurate(1);

   VIDCore->Resize(width,height,0);
   glViewport(0,0,width,height);

   glClearColor( 0.0f, 0.0f,0.0f,1.0f);
   glClear( GL_COLOR_BUFFER_BIT );
   return 1;
}

destroy() {
    printf("Destroying context");

    eglMakeCurrent(g_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(g_Display, g_Context);
    eglDestroySurface(g_Display, g_Surface);
    eglTerminate(g_Display);

    g_Display = EGL_NO_DISPLAY;
    g_Surface = EGL_NO_SURFACE;
    g_Context = EGL_NO_CONTEXT;
    return;
}

void
Java_org_yabause_android_YabauseRunnable_deinit( JNIEnv* env )
{
    YabauseDeInit();
}

void
Java_org_yabause_android_YabauseRunnable_exec( JNIEnv* env )
{

}

void
Java_org_yabause_android_YabauseRunnable_press( JNIEnv* env, jobject obj, jint key )
{
    PerKeyDown(key);
}

void
Java_org_yabause_android_YabauseRunnable_release( JNIEnv* env, jobject obj, jint key )
{
    PerKeyUp(key);
}

void log_callback(char * message)
{
    __android_log_print(ANDROID_LOG_INFO, "yabause", "%s", message);
}

jint JNI_OnLoad(JavaVM * vm, void * reserved)
{
    JNIEnv * env;
    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_6) != JNI_OK)
        return -1;

    yvm = vm;

    LogStart();
    LogChangeOutput(DEBUG_CALLBACK, (char *) log_callback);

    return JNI_VERSION_1_6;
}

void renderLoop()
{
    int renderingEnabled = 1;

    printf("enter render loop!");

    while (renderingEnabled != 0) {

        pthread_mutex_lock(&g_mtxGlLock);

        // process incoming messages
        switch (g_msg) {

            case MSG_WINDOW_SET:
                initEgl( g_window );
                break;

            case MSG_RENDER_LOOP_EXIT:
                renderingEnabled = 0;
                destroy();
                break;

            default:
                break;
        }
        g_msg = MSG_NONE;

        if (g_Display) {
           YabauseExec();
        }
        pthread_mutex_unlock(&g_mtxGlLock);
    }
}

void* threadStartCallback(void *myself)
{
    renderLoop();
    pthread_exit(0);
    return NULL;
}
