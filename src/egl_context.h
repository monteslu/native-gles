#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>

struct GLESContext {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLConfig config;
    int width;
    int height;
    bool valid;
    bool isWindowSurface;
};

bool gles_context_create(GLESContext* ctx, int width, int height, bool windowSurface = false);
void gles_context_destroy(GLESContext* ctx);
bool gles_context_resize(GLESContext* ctx, int width, int height);
bool gles_context_make_current(GLESContext* ctx);
bool gles_context_release_current(GLESContext* ctx);
bool gles_context_swap(GLESContext* ctx);
