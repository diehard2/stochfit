import type { ForgeConfig } from '@electron-forge/shared-types';
import { MakerSquirrel } from '@electron-forge/maker-squirrel';
import { MakerZIP } from '@electron-forge/maker-zip';
import { MakerDeb } from '@electron-forge/maker-deb';
import { MakerRpm } from '@electron-forge/maker-rpm';
import { VitePlugin } from '@electron-forge/plugin-vite';
// @ts-ignore — type-only import not needed at runtime

const libExt = process.platform === 'win32' ? '.dll' : '.so';
const libPrefix = process.platform === 'win32' ? '' : 'lib';

const config: ForgeConfig = {
  outDir: '../build/electron',
  packagerConfig: {
    asar: false,
    name: 'StochFit',
    executableName: 'stochfit',
    ignore: [],
    extraResource: [
      `../build/bin/${libPrefix}stochfit${libExt}`,
      `../build/bin/${libPrefix}levmardll${libExt}`,
    ],
  },
  rebuildConfig: {
    onlyModules: ['koffi'],
  },
  makers: [
    new MakerSquirrel({
      authors: 'StochFit Contributors',
    }),
    new MakerZIP({}, ['darwin']),
    new MakerRpm({}),
    new MakerDeb({}),
  ],
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
