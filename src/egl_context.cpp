#include "egl_context.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
// EGL extension function types for device-based display
typedef EGLBoolean (*PFNEGLQUERYDEVICESEXTPROC)(EGLint, EGLDeviceEXT*, EGLint*);
typedef EGLDisplay (*PFNEGLGETPLATFORMDISPLAYEXTPROC)(EGLenum, void*, const EGLint*);

#ifndef EGL_PLATFORM_DEVICE_EXT
#define EGL_PLATFORM_DEVICE_EXT 0x313F
#endif

// Try to get an independent EGL display via EGL_EXT_device_enumeration.
// This avoids conflicts with SDL or other libraries using the default display.
static EGLDisplay getIndependentDisplay() {
    // Skip device enumeration when LIBGL_ALWAYS_SOFTWARE is set — the device
    // path selects a hardware device which conflicts with software rendering
    // and causes a segfault on headless CI runners.
    const char* swFlag = getenv("LIBGL_ALWAYS_SOFTWARE");
    if (swFlag && swFlag[0] == '1') {
        return EGL_NO_DISPLAY;
    }

    // Check if extensions are available
    const char* clientExts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (!clientExts) return EGL_NO_DISPLAY;

    bool hasDeviceEnum = strstr(clientExts, "EGL_EXT_device_enumeration") != nullptr;
    bool hasPlatformDevice = strstr(clientExts, "EGL_EXT_platform_device") != nullptr;
    bool hasPlatformBase = strstr(clientExts, "EGL_EXT_platform_base") != nullptr;

    if (!hasDeviceEnum || !hasPlatformDevice || !hasPlatformBase) {
        return EGL_NO_DISPLAY;
    }

    auto eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)
        eglGetProcAddress("eglQueryDevicesEXT");
    auto eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
        eglGetProcAddress("eglGetPlatformDisplayEXT");

    if (!eglQueryDevicesEXT || !eglGetPlatformDisplayEXT) {
        return EGL_NO_DISPLAY;
    }

    EGLDeviceEXT devices[8];
    EGLint numDevices = 0;
    if (!eglQueryDevicesEXT(8, devices, &numDevices) || numDevices == 0) {
        return EGL_NO_DISPLAY;
    }

    // Use the first available device
    EGLDisplay display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, devices[0], nullptr);
    return display;
}

// Mali fbdev native window struct
struct fbdev_window {
    unsigned short width;
    unsigned short height;
};

bool gles_context_create(GLESContext* ctx, int width, int height, bool windowSurface, void* nativeWindow) {
    ctx->valid = false;
    ctx->width = width;
    ctx->height = height;
    ctx->isWindowSurface = windowSurface;

    if (windowSurface) {
        // Window surface mode: use default display (fbdev on Mali)
        ctx->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    } else {
        // Try device-based display first (independent from SDL/X11/Wayland)
        ctx->display = getIndependentDisplay();
        if (ctx->display == EGL_NO_DISPLAY) {
            // Fallback to default display
            ctx->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        }
    }

    if (ctx->display == EGL_NO_DISPLAY) {
        fprintf(stderr, "native-gles: eglGetDisplay failed\n");
        return false;
    }

    EGLint major, minor;
    if (!eglInitialize(ctx->display, &major, &minor)) {
        fprintf(stderr, "native-gles: eglInitialize failed\n");
        return false;
    }

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        fprintf(stderr, "native-gles: eglBindAPI failed\n");
        eglTerminate(ctx->display);
        return false;
    }

    EGLint surfaceType = windowSurface ? EGL_WINDOW_BIT : EGL_PBUFFER_BIT;
    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, surfaceType,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };

    EGLint numConfigs;
    if (!eglChooseConfig(ctx->display, configAttribs, &ctx->config, 1, &numConfigs) || numConfigs == 0) {
        fprintf(stderr, "native-gles: eglChooseConfig failed\n");
        eglTerminate(ctx->display);
        return false;
    }

    if (windowSurface) {
        EGLNativeWindowType winHandle;
        static fbdev_window fbdevWin; // fbdev fallback
        if (nativeWindow) {
            // Use the native window handle from SDL (X11 Window, HWND, etc.)
            winHandle = (EGLNativeWindowType)nativeWindow;
        } else {
            // Fallback: fbdev window (Mali/Knulli)
            fbdevWin.width = (unsigned short)width;
            fbdevWin.height = (unsigned short)height;
            winHandle = (EGLNativeWindowType)&fbdevWin;
        }
        ctx->surface = eglCreateWindowSurface(ctx->display, ctx->config,
            winHandle, nullptr);
        if (ctx->surface == EGL_NO_SURFACE) {
            fprintf(stderr, "native-gles: eglCreateWindowSurface failed (0x%x)\n", eglGetError());
            eglTerminate(ctx->display);
            return false;
        }
    } else {
        EGLint pbufferAttribs[] = {
            EGL_WIDTH, width,
            EGL_HEIGHT, height,
            EGL_NONE
        };
        ctx->surface = eglCreatePbufferSurface(ctx->display, ctx->config, pbufferAttribs);
        if (ctx->surface == EGL_NO_SURFACE) {
            fprintf(stderr, "native-gles: eglCreatePbufferSurface failed\n");
            eglTerminate(ctx->display);
            return false;
        }
    }

    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };

    ctx->context = eglCreateContext(ctx->display, ctx->config, EGL_NO_CONTEXT, contextAttribs);
    if (ctx->context == EGL_NO_CONTEXT) {
        fprintf(stderr, "native-gles: eglCreateContext failed\n");
        eglDestroySurface(ctx->display, ctx->surface);
        eglTerminate(ctx->display);
        return false;
    }

    if (!eglMakeCurrent(ctx->display, ctx->surface, ctx->surface, ctx->context)) {
        fprintf(stderr, "native-gles: eglMakeCurrent failed\n");
        eglDestroyContext(ctx->display, ctx->context);
        eglDestroySurface(ctx->display, ctx->surface);
        eglTerminate(ctx->display);
        return false;
    }

    ctx->valid = true;
    return true;
}

void gles_context_destroy(GLESContext* ctx) {
    if (!ctx->valid) return;

    eglMakeCurrent(ctx->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(ctx->display, ctx->context);
    eglDestroySurface(ctx->display, ctx->surface);
    eglTerminate(ctx->display);

    ctx->valid = false;
}

bool gles_context_resize(GLESContext* ctx, int width, int height) {
    if (!ctx->valid) return false;

    // Destroy old surface
    eglMakeCurrent(ctx->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(ctx->display, ctx->surface);

    // Create new pbuffer with new size
    EGLint pbufferAttribs[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE
    };

    ctx->surface = eglCreatePbufferSurface(ctx->display, ctx->config, pbufferAttribs);
    if (ctx->surface == EGL_NO_SURFACE) {
        fprintf(stderr, "native-gles: resize eglCreatePbufferSurface failed\n");
        ctx->valid = false;
        return false;
    }

    if (!eglMakeCurrent(ctx->display, ctx->surface, ctx->surface, ctx->context)) {
        fprintf(stderr, "native-gles: resize eglMakeCurrent failed\n");
        ctx->valid = false;
        return false;
    }

    ctx->width = width;
    ctx->height = height;
    return true;
}

bool gles_context_make_current(GLESContext* ctx) {
    if (!ctx->valid) return false;
    return eglMakeCurrent(ctx->display, ctx->surface, ctx->surface, ctx->context) == EGL_TRUE;
}

bool gles_context_release_current(GLESContext* ctx) {
    if (!ctx->valid) return false;
    return eglMakeCurrent(ctx->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_TRUE;
}

bool gles_context_swap(GLESContext* ctx) {
    if (!ctx->valid) return false;
    return eglSwapBuffers(ctx->display, ctx->surface) == EGL_TRUE;
}
