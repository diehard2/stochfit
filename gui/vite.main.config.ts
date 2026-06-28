import { defineConfig } from 'vite';
import path from 'path';

export default defineConfig({
  build: {
    lib: {
      entry: path.join(__dirname, 'src/main/index.ts'),
      formats: ['cjs'],
      fileName: () => 'index.js',
    },
    outDir: path.join(__dirname, '.vite/build/main'),
    emptyOutDir: true,
    rollupOptions: {
      external: ['electron', 'koffi', 'path', 'fs', 'h5wasm'],
    },
  },
});
