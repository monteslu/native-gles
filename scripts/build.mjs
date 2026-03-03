import Fs from 'node:fs'
import Path from 'node:path'
import { execSync } from 'node:child_process'
import C from './util/common.js'

await Promise.all([
	C.dir.build,
	C.dir.dist,
	C.dir.publish,
].map(async (dir) => {
	await Fs.promises.rm(dir, { recursive: true }).catch(() => {})
}))

console.log("build in", C.dir.build)

let archFlag = ''
if (process.env.CROSS_COMPILE_ARCH) {
	archFlag = `--arch ${process.env.CROSS_COMPILE_ARCH}`
}

let parallelFlag = '-j max'
if (process.env.NO_PARALLEL) {
	parallelFlag = ''
}

// Set up ANGLE paths for macOS/Windows
const buildEnv = { ...process.env }
const needsAngle = C.platform !== 'linux'

if (needsAngle) {
	const angleInc = Path.join(C.dir.angle, 'include')
	const angleLib = Path.join(C.dir.angle, 'lib')
	if (!Fs.existsSync(angleInc) || !Fs.existsSync(angleLib)) {
		throw new Error(`ANGLE not found at ${C.dir.angle} — run 'npm run download-angle' first`)
	}
	buildEnv.ANGLE_INC = angleInc
	buildEnv.ANGLE_LIB = angleLib
	console.log("ANGLE_INC:", angleInc)
	console.log("ANGLE_LIB:", angleLib)
}

process.chdir(C.dir.root)
execSync(`npx -y node-gyp rebuild ${archFlag} ${parallelFlag} --verbose`, {
	stdio: 'inherit',
	env: buildEnv,
})

console.log("install to", C.dir.dist)
await Fs.promises.rm(C.dir.dist, { recursive: true }).catch(() => {})
await Fs.promises.mkdir(C.dir.dist, { recursive: true })

await Fs.promises.cp(
	Path.join(C.dir.build, 'Release/gles.node'),
	Path.join(C.dir.dist, 'gles.node'),
)

// Bundle ANGLE libraries alongside gles.node on macOS/Windows
if (needsAngle) {
	const angleLib = Path.join(C.dir.angle, 'lib')
	const libs = await Fs.promises.readdir(angleLib)
	await Promise.all(libs.map(async (name) => {
		// Copy .dylib, .dll, .so files
		if (name.endsWith('.dylib') || name.endsWith('.dll') || name.endsWith('.so')) {
			console.log(`  bundle: ${name}`)
			await Fs.promises.cp(
				Path.join(angleLib, name),
				Path.join(C.dir.dist, name),
				{ verbatimSymlinks: true },
			)
		}
	}))
}

// Strip binaries on linux
if (C.platform === 'linux') {
	execSync(`strip -s ${Path.join(C.dir.dist, 'gles.node')}`)
}
