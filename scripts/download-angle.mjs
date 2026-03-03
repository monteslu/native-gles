import Fs from 'node:fs'
import Path from 'node:path'
import { once } from 'node:events'
import C from './util/common.js'
import { fetch } from './util/fetch.js'
import * as Tar from 'tar'

const { platform } = process

// ANGLE is only needed on macOS and Windows — Linux uses system EGL/GLES
if (platform === 'linux') {
	console.log('Linux detected — using system EGL/GLES, skipping ANGLE download')
	process.exit(0)
}

const angleVersion = C.angle.version
const assetName = `ANGLE-v${angleVersion}-${platform}-${C.targetArch}.tar.gz`
const url = `https://github.com/${C.angle.owner}/${C.angle.repo}/releases/download/v${angleVersion}/${assetName}`

console.log("fetch", url)
const response = await fetch(url)

const angleDir = C.dir.angle
console.log("unpack to", angleDir)
await Fs.promises.rm(angleDir, { recursive: true }).catch(() => {})
await Fs.promises.mkdir(angleDir, { recursive: true })
const tar = Tar.extract({ gzip: true, C: angleDir })
response.stream().pipe(tar)
await once(tar, 'finish')

// Verify we got what we need
const libDir = Path.join(angleDir, 'lib')
const incDir = Path.join(angleDir, 'include')
if (!Fs.existsSync(libDir) || !Fs.existsSync(incDir)) {
	throw new Error(`ANGLE download incomplete — missing lib/ or include/ in ${angleDir}`)
}

console.log('ANGLE downloaded successfully')
const libs = await Fs.promises.readdir(libDir)
console.log('Libraries:', libs.join(', '))
