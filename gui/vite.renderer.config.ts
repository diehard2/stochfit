import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import path from 'path';

export default defineConfig({
  root: path.join(__dirname, 'src/renderer'),
  plugins: [react()],
  css: {
    postcss: path.join(__dirname, 'postcss.config.js'),
  },
  build: {
    outDir: path.join(__dirname, '.vite/renderer/main_window'),
    emptyOutDir: true,
  },
});
