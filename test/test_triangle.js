const gl = require('../index.js')

console.log('--- test_triangle ---')

const W = 64, H = 64

if (!gl.createContext(W, H)) {
  console.error('FAIL: could not create context')
  process.exit(1)
}

// GL constants
const GL_VERTEX_SHADER = 0x8B31
const GL_FRAGMENT_SHADER = 0x8B30
const GL_COMPILE_STATUS = 0x8B81
const GL_LINK_STATUS = 0x8B82
const GL_COLOR_BUFFER_BIT = 0x4000
const GL_DEPTH_BUFFER_BIT = 0x100
const GL_TRIANGLES = 0x0004
const GL_FLOAT = 0x1406
const GL_ARRAY_BUFFER = 0x8892
const GL_STATIC_DRAW = 0x88E4
const GL_RGBA = 0x1908
const GL_UNSIGNED_BYTE = 0x1401

// Shaders — use ES 300 to test the shader rewrite on macOS too
const vsSource = `#version 300 es
in vec2 aPos;
void main() {
  gl_Position = vec4(aPos, 0.0, 1.0);
}
`

const fsSource = `#version 300 es
precision mediump float;
out vec4 fragColor;
void main() {
  fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
`

function compileShader(type, source) {
  const shader = gl.glCreateShader(type)
  gl.glShaderSource(shader, source)
  gl.glCompileShader(shader)
  const status = gl.glGetShaderiv(shader, GL_COMPILE_STATUS)
  if (!status) {
    console.error('Shader compile error:', gl.glGetShaderInfoLog(shader))
    process.exit(1)
  }
  return shader
}

const vs = compileShader(GL_VERTEX_SHADER, vsSource)
const fs = compileShader(GL_FRAGMENT_SHADER, fsSource)

const program = gl.glCreateProgram()
gl.glAttachShader(program, vs)
gl.glAttachShader(program, fs)
gl.glLinkProgram(program)
const linkStatus = gl.glGetProgramiv(program, GL_LINK_STATUS)
if (!linkStatus) {
  console.error('Program link error:', gl.glGetProgramInfoLog(program))
  process.exit(1)
}

gl.glUseProgram(program)

// Full-screen triangle (covers entire viewport)
const vertices = new Float32Array([
  -1, -1,
   3, -1,
  -1,  3,
])

// VAO (required for ES3 / GL core)
const vaoArr = new Uint32Array(1)
gl.glGenVertexArrays(1, vaoArr)
gl.glBindVertexArray(vaoArr[0])

// VBO
const bufArr = new Uint32Array(1)
gl.glGenBuffers(1, bufArr)
gl.glBindBuffer(GL_ARRAY_BUFFER, bufArr[0])
gl.glBufferData(GL_ARRAY_BUFFER, new Uint8Array(vertices.buffer), GL_STATIC_DRAW)

const aPos = gl.glGetAttribLocation(program, 'aPos')
gl.glEnableVertexAttribArray(aPos)
gl.glVertexAttribPointer(aPos, 2, GL_FLOAT, false, 0, 0)

// Draw
gl.glViewport(0, 0, W, H)
gl.glClearColor(0, 0, 0, 1)
gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
gl.glDrawArrays(GL_TRIANGLES, 0, 3)
gl.glFinish()

// Readback
const pixels = new Uint8Array(W * H * 4)
gl.glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, pixels)

// Check center pixel is red
const cx = Math.floor(W / 2)
const cy = Math.floor(H / 2)
const idx = (cy * W + cx) * 4
const r = pixels[idx], g = pixels[idx + 1], b = pixels[idx + 2], a = pixels[idx + 3]
console.log(`center pixel: rgba(${r}, ${g}, ${b}, ${a})`)

if (r < 200 || g > 50 || b > 50) {
  console.error(`FAIL: expected red, got rgba(${r}, ${g}, ${b}, ${a})`)
  process.exit(1)
}

// Cleanup
gl.glDeleteBuffers(1, bufArr)
gl.glDeleteVertexArrays(1, vaoArr)
gl.glDeleteProgram(program)
gl.glDeleteShader(vs)
gl.glDeleteShader(fs)

const err = gl.glGetError()
if (err !== 0) {
  console.error('FAIL: GL error:', err)
  process.exit(1)
}

gl.destroyContext()
console.log('PASS: test_triangle')
