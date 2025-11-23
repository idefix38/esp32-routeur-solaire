import { defineConfig } from 'vite'
import preact from '@preact/preset-vite'
import * as fs from 'fs'
import * as path from 'path'
import * as zlib from 'zlib'

// https://vite.dev/config/
export default defineConfig({
  plugins: [
    preact(),
    // Custom plugin to gzip all built assets and remove originals so only .gz files remain in output
    {
      name: 'gzip-and-clean-output',
      apply: 'build',
      closeBundle: async () => {
        const outDir = path.resolve(__dirname, 'data');
        if (!fs.existsSync(outDir)) {
          console.warn('[gzip-plugin] outDir not found:', outDir);
          return;
        }

        const walk = (dir: string) => {
          for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
            const full = path.join(dir, entry.name);
            if (entry.isDirectory()) {
              walk(full);
            } else if (entry.isFile()) {
              if (full.endsWith('.gz')) continue;
              try {
                const content = fs.readFileSync(full);
                const gz = zlib.gzipSync(content);
                fs.writeFileSync(full + '.gz', gz);
                fs.unlinkSync(full); // remove original
                console.log('[gzip-plugin] gzipped and removed:', full);
              } catch (e) {
                console.error('[gzip-plugin] failed to gzip', full, e);
              }
            }
          }
        }

        walk(outDir);
      }
    }
  ],
  root: './web',
  build: {
    outDir: '../data',
  }
})
