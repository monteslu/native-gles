# native-gles

OpenGL ES 3.0 bindings for Node.js via EGL. Uses native drivers on Linux/ARM and [ANGLE](https://chromium.googlesource.com/angle/angle) on macOS/Windows.

Renders to an offscreen EGL pbuffer — no window system required. Designed for headless GPU rendering, game engines, and [wasmcart](https://github.com/monteslu/wasmcart) GL carts.

## Install

```bash
npm install native-gles
```

### Linux

```bash
sudo apt install libegl-dev libgles-dev
```

### macOS / Windows

ANGLE binaries are downloaded automatically during `npm install`. Set `ANGLE_INC` and `ANGLE_LIB` environment variables to use a custom ANGLE build.

## Usage

```js
const gl = require('native-gles')

// Create an offscreen EGL pbuffer context
gl.createContext(800, 600)

// Standard GLES3 calls
gl.glClearColor(0.2, 0.3, 0.3, 1.0)
gl.glClear(0x4000) // GL_COLOR_BUFFER_BIT

// Read pixels back
const pixels = new Uint8Array(800 * 600 * 4)
gl.glReadPixels(0, 0, 800, 600, 0x1908, 0x1401, pixels) // GL_RGBA, GL_UNSIGNED_BYTE

gl.destroyContext()
```

## API

### Context Management

native-gles is **multi-context**: `createContext` returns an integer handle,
and any number of contexts can coexist in one process, each with its own
object namespace. Every context-management call takes an optional trailing
`id`; omitting it targets the *current* context (the last one created or made
current), which is exactly the old single-context behavior — existing callers
keep working unchanged. GL draw calls (`gl.gl*`) are global and act on
whichever context is current, which is EGL's own model: call `makeCurrent(id)`
before rendering whenever more than one context exists.

| Function | Description |
|----------|-------------|
| `createContext(width, height, opts?)` | Create an EGL context (pbuffer by default). Returns the context **handle** (int > 0), or `0` on failure — truthy/falsy like the old boolean. `opts`: `{ windowSurface, nativeWindow }`. |
| `destroyContext(id?)` | Destroy a context and free its resources. The shared `EGLDisplay` is refcounted, so destroying one context never tears down the others. |
| `resizeContext(width, height, id?)` | Resize the pbuffer surface. No-op (returns `true`) while a window surface is attached — window surfaces track their window. |
| `makeCurrent(id?)` | Make a context current. Required before rendering when the process holds more than one context. |
| `attachWindow(handleBuffer, id?)` | Bind a native window surface to the **existing** context — every texture, FBO and compiled program survives, because the context is never destroyed. Takes the pointer buffer from SDL's `window.native.handle` (X11 `Window`, `HWND`; on macOS an `NSView*`, resolved internally to its backing `CALayer`). Fails non-destructively: on error the pbuffer stays current. |
| `detachWindow(id?)` | Restore the retained pbuffer surface and destroy the window surface. The context and all GL objects are untouched. |
| `swapBuffers(id?)` | Swap the surface (present, when a window surface is attached). |
| `setSwapInterval(n, id?)` | Swap interval; `1` blocks presents on vsync. On macOS this drives `CAMetalLayer.displaySyncEnabled` (see below). |
| `getContextInfo(id?)` | Returns `{ id, valid, width, height, isWindowSurface, contextCount }`. |

### GL Functions (246 — full GLES 3.0 spec)

All functions use the `gl.glFunctionName(...)` convention. Every function in the OpenGL ES 3.0 specification is implemented.

**State** — `glEnable`, `glDisable`, `glGetError`, `glGetIntegerv`, `glGetBooleanv`, `glGetFloatv`, `glGetInteger64v`, `glGetIntegeri_v`, `glGetInteger64i_v`, `glGetString`, `glGetStringi`, `glFinish`, `glFlush`, `glHint`, `glPixelStorei`, `glIsEnabled`

**Viewport & Clear** — `glViewport`, `glScissor`, `glClear`, `glClearColor`, `glClearDepthf`, `glClearStencil`, `glClearBufferiv`, `glClearBufferuiv`, `glClearBufferfv`, `glClearBufferfi`

**Blending** — `glBlendFunc`, `glBlendFuncSeparate`, `glBlendEquation`, `glBlendEquationSeparate`, `glBlendColor`, `glColorMask`

**Depth & Stencil** — `glDepthFunc`, `glDepthMask`, `glDepthRangef`, `glStencilFunc`, `glStencilFuncSeparate`, `glStencilOp`, `glStencilOpSeparate`, `glStencilMask`, `glStencilMaskSeparate`

**Face Culling** — `glCullFace`, `glFrontFace`, `glPolygonOffset`, `glLineWidth`, `glSampleCoverage`

**Buffers** — `glGenBuffers`, `glDeleteBuffers`, `glBindBuffer`, `glBufferData`, `glBufferSubData`, `glBindBufferRange`, `glBindBufferBase`, `glGetBufferParameteriv`, `glGetBufferParameteri64v`, `glIsBuffer`, `glMapBufferRange`\*, `glUnmapBuffer`, `glFlushMappedBufferRange`\*, `glGetBufferPointerv`\*

**Textures** — `glGenTextures`, `glDeleteTextures`, `glBindTexture`, `glActiveTexture`, `glTexImage2D`, `glTexSubImage2D`, `glTexImage3D`, `glTexSubImage3D`, `glTexStorage2D`, `glTexStorage3D`, `glTexParameteri`, `glTexParameterf`, `glTexParameteriv`, `glTexParameterfv`, `glGenerateMipmap`, `glCompressedTexImage2D`, `glCompressedTexSubImage2D`, `glCompressedTexImage3D`, `glCompressedTexSubImage3D`, `glCopyTexImage2D`, `glCopyTexSubImage2D`, `glCopyTexSubImage3D`, `glGetTexParameteriv`, `glGetTexParameterfv`, `glIsTexture`

**Samplers** — `glGenSamplers`, `glDeleteSamplers`, `glBindSampler`, `glSamplerParameteri`, `glSamplerParameterf`, `glSamplerParameteriv`, `glSamplerParameterfv`, `glGetSamplerParameteriv`, `glGetSamplerParameterfv`, `glIsSampler`

**Shaders** — `glCreateShader`, `glDeleteShader`, `glShaderSource`, `glCompileShader`, `glGetShaderiv`, `glGetShaderInfoLog`, `glGetShaderSource`, `glGetShaderPrecisionFormat`, `glIsShader`, `glReleaseShaderCompiler`, `glShaderBinary`\*

**Programs** — `glCreateProgram`, `glDeleteProgram`, `glAttachShader`, `glDetachShader`, `glLinkProgram`, `glUseProgram`, `glGetProgramiv`, `glGetProgramInfoLog`, `glValidateProgram`, `glBindAttribLocation`, `glGetAttribLocation`, `glGetUniformLocation`, `glGetActiveUniform`, `glGetActiveAttrib`, `glGetAttachedShaders`, `glGetFragDataLocation`, `glProgramParameteri`, `glGetProgramBinary`\*, `glProgramBinary`\*, `glIsProgram`

**Uniforms** — `glUniform{1,2,3,4}{i,f,ui}`, `glUniform{1,2,3,4}{iv,fv,uiv}`, `glUniformMatrix{2,3,4}fv`, `glUniformMatrix{2x3,3x2,2x4,4x2,3x4,4x3}fv`, `glGetUniformiv`, `glGetUniformfv`, `glGetUniformuiv`

**Uniform Buffer Objects** — `glGetUniformBlockIndex`, `glGetActiveUniformBlockiv`, `glGetActiveUniformBlockName`, `glUniformBlockBinding`, `glGetUniformIndices`, `glGetActiveUniformsiv`

**Vertex Attributes** — `glEnableVertexAttribArray`, `glDisableVertexAttribArray`, `glVertexAttribPointer`, `glVertexAttribIPointer`, `glVertexAttrib{1,2,3,4}f`, `glVertexAttrib{1,2,3,4}fv`, `glVertexAttribI4i`, `glVertexAttribI4ui`, `glVertexAttribI4iv`, `glVertexAttribI4uiv`, `glVertexAttribDivisor`, `glGetVertexAttribiv`, `glGetVertexAttribfv`, `glGetVertexAttribIiv`, `glGetVertexAttribIuiv`, `glGetVertexAttribPointerv`

**VAOs** — `glGenVertexArrays`, `glDeleteVertexArrays`, `glBindVertexArray`, `glIsVertexArray`

**Drawing** — `glDrawArrays`, `glDrawElements`, `glDrawRangeElements`, `glDrawArraysInstanced`, `glDrawElementsInstanced`, `glDrawBuffers`

**Framebuffers** — `glGenFramebuffers`, `glDeleteFramebuffers`, `glBindFramebuffer`, `glCheckFramebufferStatus`, `glFramebufferTexture2D`, `glFramebufferTextureLayer`, `glFramebufferRenderbuffer`, `glBlitFramebuffer`, `glInvalidateFramebuffer`, `glInvalidateSubFramebuffer`, `glReadBuffer`, `glGetFramebufferAttachmentParameteriv`, `glIsFramebuffer`

**Renderbuffers** — `glGenRenderbuffers`, `glDeleteRenderbuffers`, `glBindRenderbuffer`, `glRenderbufferStorage`, `glRenderbufferStorageMultisample`, `glGetRenderbufferParameteriv`, `glGetInternalformativ`, `glIsRenderbuffer`

**Readback** — `glReadPixels` (with automatic float-to-ubyte conversion for float FBOs)

**Queries** — `glGenQueries`, `glDeleteQueries`, `glBeginQuery`, `glEndQuery`, `glGetQueryiv`, `glGetQueryObjectiv`, `glGetQueryObjectuiv`, `glIsQuery`

**Sync** — `glFenceSync`, `glClientWaitSync`, `glWaitSync`, `glDeleteSync`, `glGetSynciv`, `glIsSync`

**Transform Feedback** — `glBeginTransformFeedback`, `glEndTransformFeedback`, `glTransformFeedbackVaryings`, `glGetTransformFeedbackVarying`, `glGenTransformFeedbacks`, `glDeleteTransformFeedbacks`, `glBindTransformFeedback`, `glPauseTransformFeedback`, `glResumeTransformFeedback`, `glIsTransformFeedback`

\* **WASM-incompatible functions** — `glMapBufferRange` returns `null`, `glFlushMappedBufferRange`/`glGetBufferPointerv` are no-ops (host pointers can't cross the WASM boundary; use `glBufferSubData` instead). `glShaderBinary`/`glGetProgramBinary`/`glProgramBinary` are no-ops (driver-specific binaries aren't portable).

## EGL Context

Headless contexts use `EGL_EXT_device_enumeration` when available for a device-based display independent of X11/Wayland/SDL. Falls back to the default EGL display otherwise. This prevents conflicts with other libraries that create their own EGL contexts.

Configuration: GLES 3.0, 8-bit RGBA, 24-bit depth, 8-bit stencil. Configs are chosen with `EGL_WINDOW_BIT | EGL_PBUFFER_BIT` so `attachWindow` can bind a window surface to a live context (single-bit fallback where the display exposes no dual config — there, `attachWindow` refuses non-destructively).

### macOS

- The display is requested from **ANGLE's Metal backend** (`EGL_ANGLE_platform_angle`). The default display resolves to ANGLE's deprecated CGL backend, whose swap layer free-runs — `eglSwapInterval` is accepted but ignored, making vsync impossible. Falls back to the default display on builds without Metal.
- SDL's native handle is an `NSView*`; it is resolved to the view's backing `CALayer` (what ANGLE actually validates), with `NSWindow*` and `CALayer*` handles also accepted.
- `setSwapInterval(1)` applies `CAMetalLayer.displaySyncEnabled` on the layer ANGLE creates — lazily, since that layer only exists after ANGLE's first present.
- The layer's `contentsScale` is re-synced to the window's `backingScaleFactor` on every swap, so dragging a window between displays with different scales (Retina laptop ↔ external monitor) keeps the drawable at the right resolution.

## Tests

```bash
npm test
```

Runs the suite:
- `test_context.js` — context create/destroy/resize lifecycle
- `test_triangle.js` — renders a red triangle, reads back pixels, verifies color
- `test_shader.js` — shader compilation and program linking
- `test_window_attach.js` — attach/detach API refuses bad input non-destructively (headless-safe)
- `test_multi_context.js` — two contexts: isolated object namespaces and pixels; destroying one leaves the other rendering

`test/manual_window_attach.js` (not in `npm test`: it opens a window) proves
GL objects survive `attachWindow` against a real SDL window — run it with
`NODE_PATH` pointing at a node_modules that has `@kmamal/sdl`.

## Building from Source

```bash
npm run build
```

Requires `node-gyp`, `node-addon-api`, and platform GL libraries (see Install above).

## License

MIT
