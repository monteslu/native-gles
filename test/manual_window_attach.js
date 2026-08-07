// End-to-end window attach/detach against a REAL SDL window.
//
// Not part of `npm test` — it opens a window, so it needs a display and
// @kmamal/sdl on the resolve path:
//
//   NODE_PATH=/path/to/node_modules node test/manual_window_attach.js
//
// The point of the whole feature is the assertion in the middle: a texture
// uploaded into the ORIGINAL pbuffer context must still be drawable after
// attachWindow(), because the context was never destroyed.

let sdl
try {
  sdl = require('@kmamal/sdl')
} catch {
  console.log('SKIP: @kmamal/sdl not resolvable (set NODE_PATH)')
  process.exit(0)
}
const gl = require('../index.js')

let failed = false
function check(name, cond) {
  console.log(`${cond ? 'ok  ' : 'FAIL'} ${name}`)
  if (!cond) failed = true
}

const W = 640, H = 480

check('createContext (pbuffer)', gl.createContext(W, H))

// A 2x2 checker texture created in the pbuffer context, before any window exists.
const tex = new Uint32Array(1)
gl.glGenTextures(1, tex)
gl.glBindTexture(0x0DE1, tex[0])
gl.glTexImage2D(0x0DE1, 0, 0x1908, 2, 2, 0, 0x1908, 0x1401, new Uint8Array([
  255, 0, 0, 255,   0, 255, 0, 255,
  0, 0, 255, 255,   255, 255, 0, 255,
]))
gl.glTexParameteri(0x0DE1, 0x2801 /* MIN_FILTER */, 0x2600 /* NEAREST */)
gl.glTexParameteri(0x0DE1, 0x2800 /* MAG_FILTER */, 0x2600)

// Minimal textured-quad program, also compiled pre-attach.
function shader(type, src) {
  const s = gl.glCreateShader(type)
  gl.glShaderSource(s, src)
  gl.glCompileShader(s)
  return s
}
const prog = gl.glCreateProgram()
gl.glAttachShader(prog, shader(0x8B31 /* VERTEX */, `#version 300 es
  layout(location=0) in vec2 p; out vec2 uv;
  void main() { uv = p * 0.5 + 0.5; gl_Position = vec4(p, 0.0, 1.0); }`))
gl.glAttachShader(prog, shader(0x8B30 /* FRAGMENT */, `#version 300 es
  precision mediump float; uniform sampler2D t; in vec2 uv; out vec4 c;
  void main() { c = texture(t, uv); }`))
gl.glLinkProgram(prog)

const vao = new Uint32Array(1); gl.glGenVertexArrays(1, vao)
const vbo = new Uint32Array(1); gl.glGenBuffers(1, vbo)
gl.glBindVertexArray(vao[0])
gl.glBindBuffer(0x8892 /* ARRAY_BUFFER */, vbo[0])
gl.glBufferData(0x8892, new Float32Array([-1, -1, 3, -1, -1, 3]), 0x88E4 /* STATIC_DRAW */)
gl.glEnableVertexAttribArray(0)
gl.glVertexAttribPointer(0, 2, 0x1406 /* FLOAT */, false, 0, 0)

function drawAndProbe(where) {
  gl.glViewport(0, 0, W, H)
  gl.glClearColor(0, 0, 0, 1)
  gl.glClear(0x4000)
  gl.glUseProgram(prog)
  gl.glBindTexture(0x0DE1, tex[0])
  gl.glDrawArrays(0x0004 /* TRIANGLES */, 0, 3)
  const px = new Uint8Array(4)
  gl.glReadPixels(4, 4, 1, 1, 0x1908, 0x1401, px) // bottom-left = red texel
  const hit = px[0] > 200 && px[1] < 60 && px[2] < 60
  check(`textured draw in ${where} (got ${px[0]},${px[1]},${px[2]})`, hit)
  return hit
}

drawAndProbe('pbuffer, pre-attach')

const win = sdl.video.createWindow({ title: 'native-gles attach test', width: W, height: H, resizable: false })

const attached = gl.attachWindow(win.native.handle)
check('attachWindow(real window)', attached)

if (attached) {
  check('reports window surface', gl.getContextInfo().isWindowSurface === true)
  check('texture survived attach', gl.glIsTexture(tex[0]) === 1)
  check('program survived attach', gl.glIsProgram(prog) === 1)
  gl.setSwapInterval(0)
  // Draw the same textured quad INTO THE WINDOW for ~1s of frames.
  let swaps = 0
  for (let i = 0; i < 60; i++) {
    if (!drawAndProbeQuiet()) break
    if (gl.swapBuffers()) swaps++
  }
  check(`swapBuffers ran (${swaps}/60)`, swaps === 60)
  drawAndProbe('window, post-attach')

  check('detachWindow()', gl.detachWindow() === true)
  check('back to pbuffer', gl.getContextInfo().isWindowSurface === false)
  drawAndProbe('pbuffer, post-detach')
}

function drawAndProbeQuiet() {
  gl.glViewport(0, 0, W, H)
  gl.glClear(0x4000)
  gl.glUseProgram(prog)
  gl.glBindTexture(0x0DE1, tex[0])
  gl.glDrawArrays(0x0004, 0, 3)
  return gl.glGetError() === 0
}

win.destroy()
gl.destroyContext()
console.log(failed ? 'RESULT: FAIL' : 'RESULT: PASS')
process.exit(failed ? 1 : 0)
