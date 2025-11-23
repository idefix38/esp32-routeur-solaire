// Script to remove non-.gz files from the `data` directory after build.
// It recursively walks `data/` and deletes any file that does NOT end with `.gz`.
// Use with caution: this will permanently remove built uncompressed assets.

const fs = require('fs');
const path = require('path');

const DATA_DIR = path.resolve(__dirname, '..', 'data');

function walk(dir) {
  const entries = fs.readdirSync(dir, { withFileTypes: true });
  for (const entry of entries) {
    const full = path.join(dir, entry.name);
    if (entry.isDirectory()) {
      walk(full);
    } else if (entry.isFile()) {
      if (!full.endsWith('.gz')) {
        try {
          fs.unlinkSync(full);
          console.log('Deleted:', full);
        } catch (e) {
          console.error('Failed to delete', full, e);
        }
      }
    }
  }
}

if (!fs.existsSync(DATA_DIR)) {
  console.error('Data directory not found:', DATA_DIR);
  process.exit(1);
}

console.log('Cleaning non-.gz files from', DATA_DIR);
walk(DATA_DIR);
console.log('Done.');
