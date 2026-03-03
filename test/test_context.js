const gl = require('../index.js')

console.log('--- test_context ---')

// Create context
const ok = gl.createContext(320, 240)
console.log('createContext(320, 240):', ok)
if (!ok) {
  console.error('FAIL: could not create context')
  process.exit(1)
}

// Check context info
const info = gl.getContextInfo()
console.log('contextInfo:', info)
if (!info.valid || info.width !== 320 || info.height !== 240) {
  console.error('FAIL: bad context info')
  process.exit(1)
}

// Query GL strings
const vendor = gl.glGetString(0x1F00) // GL_VENDOR
const renderer = gl.glGetString(0x1F01) // GL_RENDERER
const version = gl.glGetString(0x1F02) // GL_VERSION
const glslVersion = gl.glGetString(0x8B8C) // GL_SHADING_LANGUAGE_VERSION
console.log('GL_VENDOR:', vendor)
console.log('GL_RENDERER:', renderer)
console.log('GL_VERSION:', version)
console.log('GL_SHADING_LANGUAGE_VERSION:', glslVersion)

// Check no errors
const err = gl.glGetError()
if (err !== 0) {
  console.error('FAIL: GL error:', err)
  process.exit(1)
}

// Resize
const resizeOk = gl.resizeContext(640, 480)
console.log('resizeContext(640, 480):', resizeOk)
if (!resizeOk) {
  console.error('FAIL: could not resize')
  process.exit(1)
}

const info2 = gl.getContextInfo()
if (info2.width !== 640 || info2.height !== 480) {
  console.error('FAIL: bad size after resize')
  process.exit(1)
}

// Destroy
gl.destroyContext()
const info3 = gl.getContextInfo()
if (info3.valid) {
  console.error('FAIL: context still valid after destroy')
  process.exit(1)
}

console.log('PASS: test_context')
