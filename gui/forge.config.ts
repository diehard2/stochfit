import type { ForgeConfig } from '@electron-forge/shared-types';
import { MakerSquirrel } from '@electron-forge/maker-squirrel';
import { VitePlugin } from '@electron-forge/plugin-vite';
import { existsSync, readdirSync } from 'fs';
import { resolve } from 'path';

const libExt    = process.platform === 'win32' ? '.dll' : process.platform === 'darwin' ? '.dylib' : '.so';
const libPrefix = process.platform === 'win32' ? '' : 'lib';

// Include a build artifact only when it actually exists.
function opt(...paths: string[]): string[] {
  return paths.filter(p => existsSync(resolve(__dirname, p)));
}

// Match build artifacts by name pattern rather than a hardcoded filename —
// the MSVC redist DLLs we need (vcomp140.dll, msvcp140_1.dll, vcruntime140.dll,
// ...) are suffixed with a toolset-specific version we don't want to bake in
// here, so pick up whatever CMake actually copied into the build output.
function optGlob(dir: string, pattern: RegExp): string[] {
  const abs = resolve(__dirname, dir);
  if (!existsSync(abs)) return [];
  return readdirSync(abs)
    .filter(f => pattern.test(f))
    .map(f => `${dir}/${f}`);
}

// MakerDMG uses appdmg which has macOS-only native binaries — load it only on
// macOS so that `npm install` succeeds on Windows/Linux (optionalDependency).
const makers: ForgeConfig['makers'] = [
  new MakerSquirrel({ authors: 'StochFit Contributors' }),
];

if (process.platform === 'darwin') {
  try {
    // eslint-disable-next-line @typescript-eslint/no-var-requires
    const { MakerDMG } = require('@electron-forge/maker-dmg');
    makers.push(new MakerDMG({}, ['darwin']));
  } catch {
    // maker-dmg not installed (non-macOS environment)
  }
}

if (process.platform === 'linux') {
  try {
    // eslint-disable-next-line @typescript-eslint/no-var-requires
    const { MakerDeb } = require('@electron-forge/maker-deb');
    // electron-installer-debian declares Depends: on libnss3/libnspr4/etc,
    // so apt resolves them instead of relying on the user already having
    // Chromium's runtime libs installed (see zip-based Linux packaging issue).
    //
    // bin/name/productName must match packagerConfig.executableName/name below —
    // electron-installer-debian otherwise defaults `bin` to the gui/package.json
    // "name" ("stochfit-gui"), which doesn't exist in the packaged app dir.
    makers.push(new MakerDeb({
      options: {
        bin: 'stochfit',
        name: 'stochfit',
        productName: 'StochFit',
      },
    }, ['linux']));
  } catch {
    // maker-deb / electron-installer-debian not installed (non-Linux environment)
  }
}

const config: ForgeConfig = {
  outDir: '../build/electron',
  packagerConfig: {
    asar: false,
    name: 'StochFit',
    executableName: 'stochfit',
    // The Vite plugin's default ignore keeps only ".vite/**", which drops
    // node_modules entirely. koffi and h5wasm are deliberately left external
    // (see vite.*.config.ts rollupOptions.external) since they ship native/wasm
    // binaries Rollup can't bundle, so node_modules + package.json must survive
    // packaging too — electron-packager's default pruning still strips
    // devDependencies from what gets copied.
    ignore: (file) => {
      if (!file) return false;
      // node_modules/.bin holds symlinks to devDependency binaries (e.g. webpack);
      // those packages get pruned as devDependencies but the .bin symlinks aren't
      // module directories, so they survive and dangle — electron-installer-debian
      // stat()s them and fails. Not needed at runtime, so drop the whole dir.
      if (file.startsWith('/node_modules/.bin')) return true;
      return !(file.startsWith('/.vite') || file.startsWith('/node_modules') || file === '/package.json');
    },
    extraResource: [
      // Core native libraries
      `../build/Release/bin/${libPrefix}stochfit${libExt}`,
      `../build/Release/bin/${libPrefix}levmardll${libExt}`,

      // OpenMP runtime — not present on stock macOS or Windows.
      // On macOS: libomp.dylib (copied from Homebrew by CMake).
      // On Windows: vcomp<ver>.dll (Visual C++ OpenMP runtime, copied from VS Redist by CMake).
      // On Linux: libgomp is a system package; not bundled.
      ...opt('../build/Release/bin/libomp.dylib'),                          // macOS
      ...optGlob('../build/Release/bin', /^vcomp\d+\.dll$/),                // Windows

      // MSVC CRT runtime (msvcp140.dll, vcruntime140.dll, vcruntime140_1.dll, ...) —
      // the x64-windows-static-md triplet links the CRT dynamically, so a clean
      // Windows machine without the VC++ Redistributable installed can't load
      // stochfit.dll/levmardll.dll without these. Copied from VS Redist by CMake.
      ...optGlob('../build/Release/bin', /^(msvcp|vcruntime)\d+(_\w+)?\.dll$/), // Windows

      '../resources/test1refl.txt',
    ],
  },
  rebuildConfig: {
    onlyModules: ['koffi'],
  },
  makers,
  plugins: [
    new VitePlugin({
      build: [
        { entry: 'src/main/index.ts', config: 'vite.main.config.ts', target: 'main' },
        { entry: 'src/preload/index.ts', config: 'vite.preload.config.ts', target: 'preload' },
      ],
      renderer: [
        { name: 'main_window', config: 'vite.renderer.config.ts' },
      ],
    }),
  ],
};

export default config;
