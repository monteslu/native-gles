import Fs from 'node:fs'
import C from './util/common.js'

await Promise.all([
	C.dir.build,
	C.dir.dist,
	C.dir.publish,
	C.dir.angle,
].map(async (dir) => {
	await Fs.promises.rm(dir, { recursive: true }).catch(() => {})
	console.log("removed", dir)
}))
