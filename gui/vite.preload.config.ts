import { defineConfig } from 'vite';
import path from 'path';

export default defineConfig({
  build: {
    lib: {
      entry: path.join(__dirname, 'src/preload/index.ts'),
      formats: ['cjs'],
      fileName: () => 'index.js',
    },
    outDir: path.join(__dirname, '.vite/build/preload'),
    emptyOutDir: true,
    rollupOptions: {
      external: ['electron', 'koffi'],
    },
  },
});
