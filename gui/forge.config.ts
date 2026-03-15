import type { ForgeConfig } from '@electron-forge/shared-types';
import { MakerSquirrel } from '@electron-forge/maker-squirrel';
import { MakerDeb } from '@electron-forge/maker-deb';
import { MakerRpm } from '@electron-forge/maker-rpm';
import { VitePlugin } from '@electron-forge/plugin-vite';

const libExt = process.platform === 'win32' ? '.dll' : '.so';
const libPrefix = process.platform === 'win32' ? '' : 'lib';

// MakerDMG uses appdmg which has macOS-only native binaries — load it only on
// macOS so that `npm install` succeeds on Windows/Linux (optionalDependency).
const makers: ForgeConfig['makers'] = [
  new MakerSquirrel({ authors: 'StochFit Contributors' }),
  new MakerDeb({}),
  new MakerRpm({}),
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
    ignore: [],
    extraResource: [
      `../build/Release/bin/${libPrefix}stochfit${libExt}`,
      `../build/Release/bin/${libPrefix}levmardll${libExt}`,
      ...(process.platform === 'darwin' ? ['../build/Release/bin/libomp.dylib'] : []),
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
