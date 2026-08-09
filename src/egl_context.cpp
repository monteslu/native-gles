#include "egl_context.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

// eglGetDisplay / eglGetPlatformDisplayEXT return the SAME EGLDisplay for the
// same arguments, and eglTerminate destroys every context and surface on it —
// so with more than one GLESContext alive, terminating on one destroy would
// kill the others. Refcount instead; terminate when the last user goes away.
static std::map<EGLDisplay, int> g_displayRefs;
static void retainDisplay(EGLDisplay d) { g_displayRefs[d]++; }
static void releaseDisplay(EGLDisplay d) {
    auto it = g_displayRefs.find(d);
    if (it == g_displayRefs.end()) { eglTerminate(d); return; }
    if (--it->second <= 0) { g_displayRefs.erase(it); eglTerminate(d); }
}
// EGL extension function types for device-based display
typedef EGLBoolean (*PFNEGLQUERYDEVICESEXTPROC)(EGLint, EGLDeviceEXT*, EGLint*);
typedef EGLDisplay (*PFNEGLGETPLATFORMDISPLAYEXTPROC)(EGLenum, void*, const EGLint*);

#ifndef EGL_PLATFORM_DEVICE_EXT
#define EGL_PLATFORM_DEVICE_EXT 0x313F
#endif
#ifndef EGL_PLATFORM_X11_KHR
#define EGL_PLATFORM_X11_KHR 0x31D5
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

    // Skip device enumeration when a desktop is reachable. A device-platform
    // display has no native window concept, so attach_window on a context
    // created against it ALWAYS fails (EGL_BAD_NATIVE_WINDOW) — which
    // silently killed GL-direct window present for every consumer on a
    // desktop (measured: 29ms software blits and a half-speed game). With
    // DISPLAY set we bind the X11/default display instead, exactly what
    // 0.5.x did; headless CI has no DISPLAY and keeps the device path.
    const char* x11Disp = getenv("DISPLAY");
    if (x11Disp && x11Disp[0]) {
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

// The display to use when a desktop session is reachable. On an XWayland
// desktop BOTH WAYLAND_DISPLAY and DISPLAY are set, and Mesa's
// EGL_DEFAULT_DISPLAY prefers the Wayland platform — whose
// eglCreateWindowSurface then rejects an X11 XID, which is exactly what SDL
// window handles are. Bind the X11 platform explicitly (Mesa resolves the
// connection from $DISPLAY when native_display is null); fall back to the
// default display where the platform extension is missing (fbdev/Mali has
// no DISPLAY at all and never reaches this).
static EGLDisplay getDesktopDisplay() {
    const char* x11Disp = getenv("DISPLAY");
    if (!x11Disp || !x11Disp[0]) return EGL_NO_DISPLAY;
    const char* clientExts = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    bool hasX11 = clientExts &&
        (strstr(clientExts, "EGL_KHR_platform_x11") ||
         strstr(clientExts, "EGL_EXT_platform_x11"));
    auto eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
        eglGetProcAddress("eglGetPlatformDisplayEXT");
    if (hasX11 && eglGetPlatformDisplayEXT) {
        EGLDisplay d = eglGetPlatformDisplayEXT(EGL_PLATFORM_X11_KHR, nullptr, nullptr);
        if (d != EGL_NO_DISPLAY) return d;
    }
    return EGL_NO_DISPLAY;
}

#ifdef __APPLE__
#ifndef EGL_PLATFORM_ANGLE_ANGLE
#define EGL_PLATFORM_ANGLE_ANGLE 0x3202
#define EGL_PLATFORM_ANGLE_TYPE_ANGLE 0x3203
#endif
#ifndef EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE
#define EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE 0x3489
#endif
// Prefer ANGLE's Metal backend on macOS. The default display resolves to the
// CGL (OpenGL) backend, whose ANGLESwapCGLLayer free-runs: eglSwapInterval is
// a no-op there, so a window present can never sync to the display — measured
// ~20,000 swaps/sec with interval 1. The Metal backend presents through a
// CAMetalLayer, where displaySyncEnabled gives real vsync (and CGL itself is
// deprecated). EGL_NO_DISPLAY when this build lacks the Metal backend.
static EGLDisplay getMetalDisplay() {
    auto eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)
        eglGetProcAddress("eglGetPlatformDisplayEXT");
    if (!eglGetPlatformDisplayEXT) return EGL_NO_DISPLAY;
    const EGLint attribs[] = {
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE,
        EGL_NONE
    };
    return eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE,
        (void*)EGL_DEFAULT_DISPLAY, attribs);
}
#endif

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
    ctx->pbufferSurface = EGL_NO_SURFACE;
    ctx->macLayer = nullptr;
    ctx->macView = nullptr;
    ctx->macDesiredSync = -1;   // unset: leave whatever the driver does alone
    ctx->macSyncApplied = true;

    bool displayInitialized = false;
#ifdef __APPLE__
    ctx->display = getMetalDisplay();
    if (ctx->display != EGL_NO_DISPLAY) {
        EGLint mtlMajor, mtlMinor;
        if (eglInitialize(ctx->display, &mtlMajor, &mtlMinor)) {
            displayInitialized = true;
        } else {
            ctx->display = EGL_NO_DISPLAY;   // build lacks Metal; use default
        }
    }
#endif
    if (!displayInitialized) {
        if (windowSurface) {
            // Window surface mode: explicit X11 platform on desktops,
            // default display elsewhere (fbdev on Mali).
            ctx->display = getDesktopDisplay();
            if (ctx->display == EGL_NO_DISPLAY) {
                ctx->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            }
        } else {
            // Try device-based display first (independent from SDL/X11/Wayland;
            // skipped on desktops so attach_window stays possible)
            ctx->display = getIndependentDisplay();
            if (ctx->display == EGL_NO_DISPLAY) {
                ctx->display = getDesktopDisplay();
            }
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
    }
    retainDisplay(ctx->display);

    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        fprintf(stderr, "native-gles: eglBindAPI failed\n");
        releaseDisplay(ctx->display);
        return false;
    }

    // Prefer a config usable with BOTH pbuffer and window surfaces: EGL only
    // lets a context bind surfaces from a compatible config, so a dual-bit
    // config is what makes attach_window possible later WITHOUT destroying
    // the context (and with it every texture, FBO and compiled program).
    // Headless displays (the device-platform path) may expose no
    // window-capable config — fall back to the single required bit; the
    // context still works, and attach_window fails non-destructively.
    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
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
        configAttribs[1] = windowSurface ? EGL_WINDOW_BIT : EGL_PBUFFER_BIT;
        if (!eglChooseConfig(ctx->display, configAttribs, &ctx->config, 1, &numConfigs) || numConfigs == 0) {
            fprintf(stderr, "native-gles: eglChooseConfig failed\n");
            releaseDisplay(ctx->display);
            return false;
        }
    }

    if (windowSurface) {
        EGLNativeWindowType winHandle;
        static fbdev_window fbdevWin; // fbdev fallback
#ifdef __APPLE__
        if (nativeWindow) {
            ctx->macView = nativeWindow;
            nativeWindow = gles_mac_layer_for_native_window(nativeWindow);
            if (!nativeWindow) {
                fprintf(stderr, "native-gles: macOS native window is not an NSView/NSWindow/CALayer\n");
                releaseDisplay(ctx->display);
                return false;
            }
            ctx->macLayer = nativeWindow;
        }
#endif
        if (nativeWindow) {
            // Use the native window handle from SDL (X11 Window, HWND;
            // on macOS resolved above to the view's backing CALayer)
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
            releaseDisplay(ctx->display);
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
            releaseDisplay(ctx->display);
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
        releaseDisplay(ctx->display);
        return false;
    }

    if (!eglMakeCurrent(ctx->display, ctx->surface, ctx->surface, ctx->context)) {
        fprintf(stderr, "native-gles: eglMakeCurrent failed\n");
        eglDestroyContext(ctx->display, ctx->context);
        eglDestroySurface(ctx->display, ctx->surface);
        releaseDisplay(ctx->display);
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
    if (ctx->pbufferSurface != EGL_NO_SURFACE) {
        eglDestroySurface(ctx->display, ctx->pbufferSurface);
        ctx->pbufferSurface = EGL_NO_SURFACE;
    }
    ctx->macLayer = nullptr;
    ctx->macView = nullptr;
    ctx->macDesiredSync = -1;
    ctx->macSyncApplied = true;
    releaseDisplay(ctx->display);

    ctx->valid = false;
}

bool gles_context_resize(GLESContext* ctx, int width, int height) {
    if (!ctx->valid) return false;

    // A window surface tracks its window's size on its own; recreating it as
    // a pbuffer here would silently detach the window. Just record the size.
    if (ctx->isWindowSurface) {
        ctx->width = width;
        ctx->height = height;
        return true;
    }

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
    if (eglMakeCurrent(ctx->display, ctx->surface, ctx->surface, ctx->context) != EGL_TRUE) {
        fprintf(stderr, "native-gles: makeCurrent failed (0x%x)\n", eglGetError());
        return false;
    }
    return true;
}

bool gles_context_release_current(GLESContext* ctx) {
    if (!ctx->valid) return false;
    return eglMakeCurrent(ctx->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_TRUE;
}

bool gles_context_swap(GLESContext* ctx) {
    if (!ctx->valid) return false;
    bool ok = eglSwapBuffers(ctx->display, ctx->surface) == EGL_TRUE;
#ifdef __APPLE__
    if (ok && ctx->isWindowSurface && ctx->macLayer) {
        /* Cross-monitor drags change the backing scale under us. */
        gles_mac_sync_backing_scale(ctx->macView, ctx->macLayer);
        /* ANGLE's Metal backend creates its CAMetalLayer on the first
         * present, so a swap interval set before then had nothing to apply
         * to. Retry until the layer exists; one boolean check after that. */
        if (!ctx->macSyncApplied) {
            ctx->macSyncApplied = gles_mac_set_display_sync(ctx->macLayer, ctx->macDesiredSync >= 1);
        }
    }
#endif
    return ok;
}

bool gles_context_set_swap_interval(GLESContext* ctx, int interval) {
    if (!ctx->valid) return false;
    bool ok = eglSwapInterval(ctx->display, interval) == EGL_TRUE;
#ifdef __APPLE__
    /* This ANGLE build accepts eglSwapInterval but Metal never syncs; the
     * real switch is CAMetalLayer.displaySyncEnabled. Apply now if ANGLE
     * already made its layer, else let swap() retry once it exists. */
    if (ctx->isWindowSurface && ctx->macLayer) {
        ctx->macDesiredSync = interval;
        ctx->macSyncApplied = gles_mac_set_display_sync(ctx->macLayer, interval >= 1);
    }
#endif
    return ok;
}

bool gles_context_attach_window(GLESContext* ctx, void* nativeWindow) {
    if (!ctx->valid || !nativeWindow) return false;
    if (ctx->isWindowSurface) return false; // already attached

#ifdef __APPLE__
    void* macOriginalHandle = nativeWindow;
    nativeWindow = gles_mac_layer_for_native_window(nativeWindow);
    if (!nativeWindow) {
        fprintf(stderr, "native-gles: attachWindow: macOS native window is not an NSView/NSWindow/CALayer\n");
        return false;
    }
#endif

    // Same display, same config, same context — only the surface changes.
    // If the config lacks EGL_WINDOW_BIT (single-bit fallback at create),
    // this fails with EGL_BAD_MATCH and the pbuffer stays current.
    EGLSurface winSurface = eglCreateWindowSurface(ctx->display, ctx->config,
        (EGLNativeWindowType)nativeWindow, nullptr);
    if (winSurface == EGL_NO_SURFACE) {
        fprintf(stderr, "native-gles: attachWindow eglCreateWindowSurface failed (0x%x)\n", eglGetError());
        return false;
    }

    if (!eglMakeCurrent(ctx->display, winSurface, winSurface, ctx->context)) {
        fprintf(stderr, "native-gles: attachWindow eglMakeCurrent failed (0x%x)\n", eglGetError());
        eglDestroySurface(ctx->display, winSurface);
        eglMakeCurrent(ctx->display, ctx->surface, ctx->surface, ctx->context);
        return false;
    }

    ctx->pbufferSurface = ctx->surface;
    ctx->surface = winSurface;
    ctx->isWindowSurface = true;
#ifdef __APPLE__
    ctx->macLayer = nativeWindow;
    ctx->macView = macOriginalHandle;
    ctx->macSyncApplied = ctx->macDesiredSync < 0;  // re-apply a pending interval to the new layer
#endif
    return true;
}

bool gles_context_detach_window(GLESContext* ctx) {
    if (!ctx->valid || !ctx->isWindowSurface) return false;
    if (ctx->pbufferSurface == EGL_NO_SURFACE) return false; // created AS a window; nothing to restore

    if (!eglMakeCurrent(ctx->display, ctx->pbufferSurface, ctx->pbufferSurface, ctx->context)) {
        fprintf(stderr, "native-gles: detachWindow eglMakeCurrent failed (0x%x)\n", eglGetError());
        return false;
    }

    eglDestroySurface(ctx->display, ctx->surface);
    ctx->surface = ctx->pbufferSurface;
    ctx->pbufferSurface = EGL_NO_SURFACE;
    ctx->macLayer = nullptr;
    ctx->macView = nullptr;
    ctx->macDesiredSync = -1;   // unset: leave whatever the driver does alone
    ctx->macSyncApplied = true;
    ctx->isWindowSurface = false;
    return true;
}
