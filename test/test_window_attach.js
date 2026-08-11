const gl = require('../index.js')

console.log('--- test_window_attach (headless) ---')

let failed = false
function check(name, cond) {
  console.log(`${cond ? 'ok  ' : 'FAIL'} ${name}`)
  if (!cond) failed = true
}

// A pbuffer context created the normal way — the state every attach starts from.
check('createContext(320, 240)', gl.createContext(320, 240))
check('starts as pbuffer', gl.getContextInfo().isWindowSurface === false)

// Objects created BEFORE any attach attempt; they must survive everything below.
const tex = new Uint32Array(1)
gl.glGenTextures(1, tex)
gl.glBindTexture(0x0DE1 /* TEXTURE_2D */, tex[0])
gl.glTexImage2D(0x0DE1, 0, 0x1908 /* RGBA */, 4, 4, 0, 0x1908, 0x1401 /* UNSIGNED_BYTE */,
  new Uint8Array(4 * 4 * 4).fill(0xAB))
check('texture created', gl.glGetError() === 0)

// Attach with nothing / with a junk-sized buffer must refuse without touching
// the context. (A real window handle is exercised in manual_window_attach.js —
// it needs a display and an SDL window.)
check('attachWindow() refuses with no handle', gl.attachWindow() === false)
check('attachWindow(short buffer) refuses', gl.attachWindow(Buffer.alloc(2)) === false)
check('still a pbuffer after refused attach', gl.getContextInfo().isWindowSurface === false)
check('context still valid', gl.getContextInfo().valid === true)

// Nothing attached, so detach must refuse too.
check('detachWindow() refuses when not attached', gl.detachWindow() === false)

// The context must still actually WORK: clear and read back.
gl.glClearColor(0.0, 1.0, 0.0, 1.0)
gl.glClear(0x4000 /* COLOR_BUFFER_BIT */)
const px = new Uint8Array(4)
gl.glReadPixels(0, 0, 1, 1, 0x1908, 0x1401, px)
check('renders after refused attach', px[0] === 0 && px[1] === 255 && px[2] === 0)

// And the pre-existing texture must still be alive.
check('texture survives', gl.glIsTexture(tex[0]) === 1)

gl.destroyContext()
check('destroyed', gl.getContextInfo().valid === false)

process.exit(failed ? 1 : 0)
