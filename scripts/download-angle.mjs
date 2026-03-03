import Fs from 'node:fs'
import Path from 'node:path'
import { once } from 'node:events'
import { execSync } from 'node:child_process'
import C from './util/common.js'
import { fetch } from './util/fetch.js'
import * as Tar from 'tar'

const { platform } = process

// ANGLE is only needed on macOS and Windows — Linux uses system EGL/GLES
if (platform === 'linux') {
	console.log('Linux detected — using system EGL/GLES, skipping ANGLE download')
	process.exit(0)
}

const angleDir = C.dir.angle
await Fs.promises.rm(angleDir, { recursive: true }).catch(() => {})
await Fs.promises.mkdir(Path.join(angleDir, 'lib'), { recursive: true })
await Fs.promises.mkdir(Path.join(angleDir, 'include'), { recursive: true })

if (platform === 'darwin') {
	// macOS: kivy/angle-builder — tar.gz with dylibs + include/ at archive root
	const tag = C.angle.kivy_tag
	const archName = C.targetArch === 'arm64' ? 'arm64' : 'x64'
	const url = `https://github.com/kivy/angle-builder/releases/download/${tag}/angle-macos-${archName}.tar.gz`

	console.log('fetch', url)
	const response = await fetch(url)

	// Extract to temp dir (archive has files at root, not in lib/ subdir)
	const tmpDir = Path.join(angleDir, '_tmp')
	await Fs.promises.mkdir(tmpDir, { recursive: true })
	const tar = Tar.extract({ gzip: true, C: tmpDir })
	response.stream().pipe(tar)
	await once(tar, 'finish')

	// Move dylibs to lib/
	for (const f of await Fs.promises.readdir(tmpDir)) {
		if (f.endsWith('.dylib')) {
			console.log(`  dylib: ${f}`)
			await Fs.promises.rename(Path.join(tmpDir, f), Path.join(angleDir, 'lib', f))
		}
	}

	// Copy include/ tree
	const tmpInclude = Path.join(tmpDir, 'include')
	if (Fs.existsSync(tmpInclude)) {
		await Fs.promises.cp(tmpInclude, Path.join(angleDir, 'include'), { recursive: true })
	}

	await Fs.promises.rm(tmpDir, { recursive: true })

} else if (platform === 'win32') {
	// Windows: mmozeiko/build-angle — zip with angle-{arch}/ subdirectory
	const tag = C.angle.mmozeiko_tag
	const archName = C.targetArch === 'arm64' ? 'arm64' : 'x64'
	const assetName = `angle-${archName}-${tag}.zip`
	const url = `https://github.com/mmozeiko/build-angle/releases/download/${tag}/${assetName}`

	console.log('fetch', url)
	const response = await fetch(url)

	// Save zip to temp file, extract with PowerShell
	const tmpZip = Path.join(angleDir, '_angle.zip')
	const tmpDir = Path.join(angleDir, '_tmp')
	const buffer = await response.buffer()
	await Fs.promises.writeFile(tmpZip, buffer)

	execSync(`powershell -Command "Expand-Archive -Path '${tmpZip}' -DestinationPath '${tmpDir}'"`, {
		stdio: 'inherit',
	})
	await Fs.promises.rm(tmpZip)

	// Find extracted subdir (e.g. angle-x64/)
	const entries = await Fs.promises.readdir(tmpDir)
	const subDir = Path.join(tmpDir, entries.find(e => e.startsWith('angle-')) || entries[0])

	// Copy DLLs from bin/ to lib/
	const binDir = Path.join(subDir, 'bin')
	if (Fs.existsSync(binDir)) {
		for (const f of await Fs.promises.readdir(binDir)) {
			if (f.endsWith('.dll')) {
				console.log(`  dll: ${f}`)
				await Fs.promises.cp(Path.join(binDir, f), Path.join(angleDir, 'lib', f))
			}
		}
	}

	// Copy import libs from lib/, renaming .dll.lib → .lib (binding.gyp expects libEGL.lib)
	const srcLibDir = Path.join(subDir, 'lib')
	if (Fs.existsSync(srcLibDir)) {
		for (const f of await Fs.promises.readdir(srcLibDir)) {
			if (f.endsWith('.dll.lib')) {
				const newName = f.replace('.dll.lib', '.lib')
				console.log(`  import lib: ${f} -> ${newName}`)
				await Fs.promises.cp(Path.join(srcLibDir, f), Path.join(angleDir, 'lib', newName))
			}
		}
	}

	// Copy include/
	const srcIncDir = Path.join(subDir, 'include')
	if (Fs.existsSync(srcIncDir)) {
		await Fs.promises.cp(srcIncDir, Path.join(angleDir, 'include'), { recursive: true })
	}

	await Fs.promises.rm(tmpDir, { recursive: true })
}

// Verify we got what we need
const libDir = Path.join(angleDir, 'lib')
const incDir = Path.join(angleDir, 'include')
if (!Fs.existsSync(libDir) || !Fs.existsSync(incDir)) {
	throw new Error(`ANGLE download incomplete — missing lib/ or include/ in ${angleDir}`)
}

console.log('ANGLE downloaded successfully')
const libs = await Fs.promises.readdir(libDir)
console.log('Libraries:', libs.join(', '))
