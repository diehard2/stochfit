import type { ForgeConfig } from '@electron-forge/shared-types';
import { MakerSquirrel } from '@electron-forge/maker-squirrel';
import { MakerZIP } from '@electron-forge/maker-zip';
import { VitePlugin } from '@electron-forge/plugin-vite';
import { existsSync } from 'fs';
import { resolve } from 'path';

const libExt    = process.platform === 'win32' ? '.dll' : process.platform === 'darwin' ? '.dylib' : '.so';
const libPrefix = process.platform === 'win32' ? '' : 'lib';

// Include a build artifact only when it actually exists.
function opt(...paths: string[]): string[] {
  return paths.filter(p => existsSync(resolve(__dirname, p)));
}

// MakerDMG uses appdmg which has macOS-only native binaries — load it only on
// macOS so that `npm install` succeeds on Windows/Linux (optionalDependency).
const makers: ForgeConfig['makers'] = [
  new MakerSquirrel({ authors: 'StochFit Contributors' }),
  new MakerZIP({}, ['linux']),
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

const config: ForgeConfig = {
  outDir: '../build/electron',
  packagerConfig: {
    asar: false,
    name: 'StochFit',
    executableName: 'stochfit',
    extraResource: [
      // Core native libraries
      `../build/Release/bin/${libPrefix}stochfit${libExt}`,
      `../build/Release/bin/${libPrefix}levmardll${libExt}`,

      // OpenMP runtime — not present on stock macOS or Windows.
      // On macOS: libomp.dylib (copied from Homebrew by CMake).
      // On Windows: vcomp140.dll (Visual C++ OpenMP runtime, copied from VS Redist by CMake).
      // On Linux: libgomp is a system package; not bundled.
      ...opt('../build/Release/bin/libomp.dylib'),   // macOS
      ...opt('../build/Release/bin/vcomp140.dll'),   // Windows

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
