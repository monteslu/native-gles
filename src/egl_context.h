#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>

struct GLESContext {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLConfig config;
    // The original pbuffer surface, retained while a window surface is
    // attached so detach can restore it without touching the context.
    EGLSurface pbufferSurface;
    int width;
    int height;
    bool valid;
    bool isWindowSurface;
};

bool gles_context_create(GLESContext* ctx, int width, int height, bool windowSurface = false, void* nativeWindow = nullptr);
void gles_context_destroy(GLESContext* ctx);
bool gles_context_resize(GLESContext* ctx, int width, int height);
bool gles_context_make_current(GLESContext* ctx);
bool gles_context_release_current(GLESContext* ctx);
bool gles_context_swap(GLESContext* ctx);
bool gles_context_set_swap_interval(GLESContext* ctx, int interval);

// Bind a window surface to the EXISTING context (no context destruction, so
// every GL object — textures, FBOs, programs — survives). Fails
// non-destructively: on any error the pbuffer stays current and true GL
// state is untouched. Detach restores the retained pbuffer surface.
bool gles_context_attach_window(GLESContext* ctx, void* nativeWindow);
bool gles_context_detach_window(GLESContext* ctx);

#ifdef __APPLE__
// EGLNativeWindowType for ANGLE on macOS is a CALayer*, but SDL's native
// handle is an NSView* (NSWindow* from other sources). Resolves whichever
// object arrives to its backing CALayer. Returns null if the pointer is not
// one of those or the view cannot be layer-backed.
extern "C" void* gles_mac_layer_for_native_window(void* handle);
#endif
