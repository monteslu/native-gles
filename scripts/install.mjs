
if (!process.env.NATIVE_GLES_FROM_SOURCE) {
	try {
		await import('./download-release.mjs')
		process.exit(0)
	}
	catch (_) {
		console.log("failed to download release, building from source")
	}
}
else {
	console.log("skip download and build from source")
}

// Download ANGLE for macOS/Windows (no-op on Linux)
try {
	await import('./download-angle.mjs')
}
catch (error) {
	if (process.platform !== 'linux') {
		console.error("failed to download ANGLE:", error.message)
		console.error("ANGLE is required on macOS/Windows — cannot build from source without it")
		process.exit(1)
	}
}

await import('./build.mjs')
