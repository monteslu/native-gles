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
