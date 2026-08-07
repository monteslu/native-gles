// macOS native-window resolution for eglCreateWindowSurface.
//
// ANGLE's macOS frontends (Metal and CGL) validate EGLNativeWindowType as a
// CALayer* — not an NSView* and not an NSWindow*. SDL's window.native.handle
// is an NSView* on macOS, so the raw handle must be resolved to the view's
// backing layer before it reaches EGL. Passing the view or window pointer
// straight through is why window surfaces "worked" on X11 (where the handle
// really is what EGL expects) but not here.

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

extern "C" void* gles_mac_layer_for_native_window(void* handle) {
    if (!handle) return nullptr;
    // AppKit is main-thread-only; wantsLayer mutation off-main is undefined.
    if (![NSThread isMainThread]) return nullptr;

    id obj = (id)handle;
    if ([obj isKindOfClass:[CALayer class]]) return handle;

    NSView* view = nil;
    if ([obj isKindOfClass:[NSView class]]) {
        view = (NSView*)obj;
    } else if ([obj isKindOfClass:[NSWindow class]]) {
        view = ((NSWindow*)obj).contentView;
    }
    if (!view) return nullptr;

    [view setWantsLayer:YES];
    return (void*)view.layer;
}

// Find the CAMetalLayer ANGLE's Metal backend hangs under the layer we gave
// it and set displaySyncEnabled. This is the actual vsync switch on macOS:
// this ANGLE build accepts eglSwapInterval(1) but never syncs, so swaps
// free-run at thousands per second and timer-paced presents land at random
// refresh phases (microstutter). Returns false while the CAMetalLayer does
// not exist yet — ANGLE creates it lazily, so callers retry after a swap.
static CAMetalLayer* findMetalLayer(CALayer* layer, int depth) {
    if ([layer isKindOfClass:[CAMetalLayer class]]) return (CAMetalLayer*)layer;
    if (depth <= 0) return nil;
    for (CALayer* sub in layer.sublayers) {
        CAMetalLayer* found = findMetalLayer(sub, depth - 1);
        if (found) return found;
    }
    return nil;
}

extern "C" bool gles_mac_set_display_sync(void* layerHandle, bool enabled) {
    if (!layerHandle) return false;
    if (![NSThread isMainThread]) return false;
    CALayer* root = (CALayer*)layerHandle;
    CAMetalLayer* metal = findMetalLayer(root, 3);
    if (!metal) return false;
    if (@available(macOS 10.13, *)) {
        metal.displaySyncEnabled = enabled ? YES : NO;
        return true;
    }
    return false;
}
