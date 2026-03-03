const gl = require('../index.js')

console.log('--- test_shader ---')

const GL_VERTEX_SHADER = 0x8B31
const GL_FRAGMENT_SHADER = 0x8B30
const GL_COMPILE_STATUS = 0x8B81

if (!gl.createContext(16, 16)) {
  console.error('FAIL: could not create context')
  process.exit(1)
}

// Test 1: Valid shader compiles
const validSource = `#version 300 es
precision mediump float;
out vec4 fragColor;
void main() {
  fragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
`

const validShader = gl.glCreateShader(GL_FRAGMENT_SHADER)
gl.glShaderSource(validShader, validSource)
gl.glCompileShader(validShader)
const validStatus = gl.glGetShaderiv(validShader, GL_COMPILE_STATUS)
console.log('valid shader compile status:', validStatus)
if (!validStatus) {
  console.error('FAIL: valid shader did not compile:', gl.glGetShaderInfoLog(validShader))
  process.exit(1)
}

// Test 2: Invalid shader fails with info log
const invalidSource = `#version 300 es
precision mediump float;
out vec4 fragColor;
void main() {
  fragColor = undeclared_variable;
}
`

const invalidShader = gl.glCreateShader(GL_FRAGMENT_SHADER)
gl.glShaderSource(invalidShader, invalidSource)
gl.glCompileShader(invalidShader)
const invalidStatus = gl.glGetShaderiv(invalidShader, GL_COMPILE_STATUS)
console.log('invalid shader compile status:', invalidStatus)
if (invalidStatus) {
  console.error('FAIL: invalid shader should not compile')
  process.exit(1)
}

const infoLog = gl.glGetShaderInfoLog(invalidShader)
console.log('info log:', infoLog.trim())
if (!infoLog || infoLog.length === 0) {
  console.error('FAIL: expected non-empty info log')
  process.exit(1)
}

// Test 3: Vertex shader compiles
const vsSource = `#version 300 es
in vec3 position;
uniform mat4 mvp;
void main() {
  gl_Position = mvp * vec4(position, 1.0);
}
`

const vs = gl.glCreateShader(GL_VERTEX_SHADER)
gl.glShaderSource(vs, vsSource)
gl.glCompileShader(vs)
const vsStatus = gl.glGetShaderiv(vs, GL_COMPILE_STATUS)
console.log('vertex shader compile status:', vsStatus)
if (!vsStatus) {
  console.error('FAIL: vertex shader did not compile:', gl.glGetShaderInfoLog(vs))
  process.exit(1)
}

// Cleanup
gl.glDeleteShader(validShader)
gl.glDeleteShader(invalidShader)
gl.glDeleteShader(vs)
gl.destroyContext()

console.log('PASS: test_shader')
