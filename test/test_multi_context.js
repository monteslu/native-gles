const gl = require('../index.js')

console.log('--- test_multi_context ---')

let failed = false
function check(name, cond) {
  console.log(`${cond ? 'ok  ' : 'FAIL'} ${name}`)
  if (!cond) failed = true
}

// Two consumers in one process — the exact shape that leaked one session's
// game into another session's window when everything shared a single
// context. Each must get its own context, own object namespace, own pixels.
const a = gl.createContext(64, 64)
check('context A created', a > 0)
const texA = new Uint32Array(1)
gl.glGenTextures(1, texA)
gl.glBindTexture(0x0DE1, texA[0])
gl.glTexImage2D(0x0DE1, 0, 0x1908, 2, 2, 0, 0x1908, 0x1401, new Uint8Array(16).fill(0x11))

const b = gl.createContext(64, 64)
check('context B created with a different id', b > 0 && b !== a)
check('B is current after create', gl.getContextInfo().id === b)
check('registry sees both', gl.getContextInfo().contextCount === 2)

// A's texture name must mean nothing in B. (Fresh contexts allocate from
// their own name space; with a shared context this reads as a live texture.)
check('A texture is not a texture in B', gl.glIsTexture(texA[0]) === 0)

// Render RED in A, GREEN in B, then read both back — each must keep its own.
const px = new Uint8Array(4)
check('makeCurrent(A)', gl.makeCurrent(a))
gl.glClearColor(1, 0, 0, 1)
gl.glClear(0x4000)
gl.glReadPixels(0, 0, 1, 1, 0x1908, 0x1401, px)
check(`A is red (got ${px[0]},${px[1]},${px[2]})`, px[0] === 255 && px[1] === 0)

check('makeCurrent(B)', gl.makeCurrent(b))
gl.glClearColor(0, 1, 0, 1)
gl.glClear(0x4000)
gl.glReadPixels(0, 0, 1, 1, 0x1908, 0x1401, px)
check(`B is green (got ${px[0]},${px[1]},${px[2]})`, px[0] === 0 && px[1] === 255)

// Switch back: A's pixels and A's texture must have survived B's work.
check('makeCurrent(A) again', gl.makeCurrent(a))
gl.glReadPixels(0, 0, 1, 1, 0x1908, 0x1401, px)
check(`A still red (got ${px[0]},${px[1]},${px[2]})`, px[0] === 255 && px[1] === 0)
check('A texture still alive in A', gl.glIsTexture(texA[0]) === 1)

// Destroying B must not disturb A (shared EGLDisplay is refcounted, not
// terminated out from under the survivor).
gl.destroyContext(b)
check('B gone from registry', gl.getContextInfo(b).valid === false)
check('makeCurrent(A) after destroying B', gl.makeCurrent(a))
gl.glClearColor(0, 0, 1, 1)
gl.glClear(0x4000)
gl.glReadPixels(0, 0, 1, 1, 0x1908, 0x1401, px)
check(`A still renders (got ${px[0]},${px[1]},${px[2]})`, px[2] === 255 && gl.glGetError() === 0)

gl.destroyContext(a)
check('all contexts released', gl.getContextInfo().contextCount === 0)

process.exit(failed ? 1 : 0)
