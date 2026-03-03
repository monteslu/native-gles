const Fs = require('fs')
const Path = require('path')

const dir = {}
dir.root = Path.join(__dirname, '../..')
dir.build = Path.join(dir.root, 'build')
dir.dist = Path.join(dir.root, 'dist')
dir.publish = Path.join(dir.root, 'publish')
dir.angle = Path.join(dir.root, 'angle')

const pkgPath = Path.join(dir.root, 'package.json')
const pkg = JSON.parse(Fs.readFileSync(pkgPath).toString())
const version = pkg.version
const isPrerelease = version.includes('-')

let owner = 'nicholasgasior'
let repo = 'native-gles'
try {
	const [ , _owner, _repo ] = pkg.repository.url.match(/([^/:]+)\/([^/]+).git$/u)
	owner = _owner
	repo = _repo
} catch (_) {}

const { platform, arch } = process
const targetArch = process.env.CROSS_COMPILE_ARCH || arch
const assetName = `gles.node-v${version}-${platform}-${targetArch}.tar.gz`

const angle = pkg.config.angle
angle.assetName = `ANGLE-v${angle.version}-${platform}-${targetArch}.tar.gz`

module.exports = {
	dir,
	version,
	isPrerelease,
	owner,
	repo,
	platform,
	arch,
	targetArch,
	assetName,
	angle,
}
