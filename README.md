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

| Function | Description |
|----------|-------------|
| `createContext(width, height)` | Create EGL pbuffer context. Returns `true` on success. |
| `destroyContext()` | Destroy context and free resources. |
| `resizeContext(width, height)` | Resize the pbuffer surface. No-op (returns `true`) while a window surface is attached — window surfaces track their window. |
| `makeCurrent()` | Make this context current (useful after SDL or other EGL contexts). |
| `attachWindow(handleBuffer)` | Bind a native window surface to the **existing** context — every texture, FBO and compiled program survives, because the context is never destroyed. Takes the pointer buffer from SDL's `window.native.handle` (X11 `Window`, `HWND`; on macOS an `NSView*`, resolved internally to its backing `CALayer`). Fails non-destructively: on error the pbuffer stays current. |
| `detachWindow()` | Restore the retained pbuffer surface and destroy the window surface. The context and all GL objects are untouched. |
| `swapBuffers()` | Swap the current surface (present, when a window surface is attached). |
| `setSwapInterval(n)` | Swap interval; `0` = never block on vsync. |
| `getContextInfo()` | Returns `{ valid, width, height, isWindowSurface }`. |

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

The context uses `EGL_EXT_device_enumeration` when available for a device-based display independent of X11/Wayland/SDL. Falls back to the default EGL display otherwise. This prevents conflicts with other libraries that create their own EGL contexts.

Configuration: GLES 3.0, 8-bit RGBA, 24-bit depth, 8-bit stencil, pbuffer surface.

## Tests

```bash
npm test
```

Runs three tests:
- `test_context.js` — context create/destroy/resize lifecycle
- `test_triangle.js` — renders a red triangle, reads back pixels, verifies color
- `test_shader.js` — shader compilation and program linking

## Building from Source

```bash
npm run build
```

Requires `node-gyp`, `node-addon-api`, and platform GL libraries (see Install above).

## License

MIT
