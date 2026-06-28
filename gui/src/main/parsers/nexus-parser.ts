import path from 'path';
import type { Group, Dataset, File as H5File } from 'h5wasm';
import type { ReflData, ReflDataMeta } from '../../renderer/lib/types';

// Lazy-init h5wasm once and cache the ready promise
let h5wasmReady: Promise<typeof import('h5wasm')> | null = null;

async function getH5Wasm(): Promise<typeof import('h5wasm')> {
  if (!h5wasmReady) {
    h5wasmReady = (async () => {
      const mod = await import('h5wasm');
      await mod.ready;
      return mod;
    })();
  }
  return h5wasmReady;
}

// Dataset name variants to try (in priority order)
const Q_NAMES = ['Q', 'q', 'Qz', 'qz'];
const I_NAMES = ['I', 'R', 'i', 'r', 'reflectivity', 'Reflectivity'];
const IDEV_NAMES = ['Idev', 'dI', 'sigma_I', 'sI', 'eR', 'dR', 'sigmaR', 'sigma_R', 'sR'];
const QDEV_NAMES = ['Qdev', 'dQ', 'sigma_Q', 'sQ', 'dQz', 'sigmaQ'];

function firstMatch(keys: string[], candidates: string[]): string | null {
  for (const c of candidates) {
    if (keys.includes(c)) return c;
  }
  return null;
}

function toNumberArray(val: unknown): number[] {
  if (val instanceof Float64Array || val instanceof Float32Array ||
      val instanceof Int32Array || val instanceof Int16Array ||
      val instanceof Uint32Array || val instanceof Uint16Array) {
    return Array.from(val);
  }
  if (Array.isArray(val)) return val.map(Number);
  throw new Error(`Unexpected dataset value type: ${typeof val}`);
}

function isGroup(entity: unknown): entity is Group {
  return entity != null && (entity as Group).type === 'Group';
}

function isDataset(entity: unknown): entity is Dataset {
  return entity != null && (entity as Dataset).type === 'Dataset';
}

function getNXClass(group: Group): string | null {
  const attr = group.attrs['NX_class'];
  if (!attr) return null;
  const v = attr.value;
  return typeof v === 'string' ? v : null;
}

function getCanSASClass(group: Group): string | null {
  const attr = group.attrs['canSAS_class'];
  if (!attr) return null;
  const v = attr.value;
  return typeof v === 'string' ? v : null;
}

/**
 * Walk the HDF5 file to find an NXdata (or SASdata) group inside an NXentry group.
 * Tries common direct paths first, then falls back to walking by NX_class attributes.
 */
function findDataGroup(f: H5File): Group | null {
  // Try common direct paths first for speed
  const directPaths = [
    'entry/data',
    'entry1/data',
    'entry1/data1',
    'entry/sasdata',
    'sasentry/sasdata',
    'sasentry01/sasdata01',
  ];

  for (const p of directPaths) {
    try {
      const entity = f.get(p);
      if (isGroup(entity)) return entity;
    } catch {
      // path does not exist
    }
  }

  // Walk root groups looking for NXentry → NXdata
  for (const rootKey of f.keys()) {
    const entry = f.get(rootKey);
    if (!isGroup(entry)) continue;

    const entryClass = getNXClass(entry);
    if (entryClass && entryClass !== 'NXentry' && entryClass !== 'NXsubentry') continue;

    for (const dataKey of entry.keys()) {
      const child = entry.get(dataKey);
      if (!isGroup(child)) continue;
      const nxClass = getNXClass(child);
      const canSASClass = getCanSASClass(child);
      if (nxClass === 'NXdata' || canSASClass === 'SASdata') {
        return child;
      }
    }
  }

  return null;
}

function attrString(group: Group, name: string): string | null {
  const a = group.attrs[name];
  if (!a) return null;
  const v = a.value;
  return typeof v === 'string' ? v.trim() || null : null;
}

function datasetString(group: Group, name: string): string | null {
  try {
    const ds = group.get(name);
    if (!isDataset(ds)) return null;
    const v = ds.value;
    return typeof v === 'string' ? v.trim() || null : null;
  } catch {
    return null;
  }
}

/**
 * Extract human-readable metadata from an HDF5/NeXus file.
 * Looks at NXentry attrs and walks children for NXinstrument/NXsample/NXmonochromator.
 */
function extractNexusMetadata(f: H5File): ReflDataMeta[] {
  const meta: ReflDataMeta[] = [];

  for (const rootKey of f.keys()) {
    const entry = f.get(rootKey);
    if (!isGroup(entry)) continue;
    const entryClass = getNXClass(entry);
    if (entryClass && entryClass !== 'NXentry' && entryClass !== 'NXsubentry') continue;

    // Entry-level title and date
    const title = attrString(entry, 'title') ?? datasetString(entry, 'title');
    if (title) meta.push({ label: 'Title', value: title });

    const startTime = attrString(entry, 'start_time') ?? datasetString(entry, 'start_time');
    if (startTime) meta.push({ label: 'Date', value: startTime.split('T')[0] });

    // Walk children for instrument and sample info
    for (const childKey of entry.keys()) {
      const child = entry.get(childKey);
      if (!isGroup(child)) continue;
      const nxClass = getNXClass(child);

      if (nxClass === 'NXinstrument') {
        const name = attrString(child, 'name') ?? datasetString(child, 'name');
        if (name) meta.push({ label: 'Instrument', value: name });

        // Look for a monochromator for wavelength
        for (const mk of child.keys()) {
          const mono = child.get(mk);
          if (!isGroup(mono)) continue;
          if (getNXClass(mono) !== 'NXmonochromator') continue;
          const wl = datasetString(mono, 'wavelength');
          if (wl) { meta.push({ label: 'Wavelength', value: `${wl} Å` }); break; }
        }
      }

      if (nxClass === 'NXsample') {
        const name = attrString(child, 'name') ?? datasetString(child, 'name');
        if (name) meta.push({ label: 'Sample', value: name });
      }
    }

    break; // first NXentry only
  }

  return meta;
}

/**
 * Parse a NeXus/HDF5 file following the NXcanSAS application definition.
 * Walks the HDF5 tree using NX_class attributes to find NXentry → NXdata groups.
 * See: https://manual.nexusformat.org/classes/applications/NXcanSAS.html
 */
export async function parseNexusFile(filePath: string): Promise<ReflData> {
  const h5 = await getH5Wasm();
  const f = new h5.File(filePath, 'r');

  try {
    const dataGroup = findDataGroup(f);
    if (!dataGroup) {
      throw new Error(
        'No NXcanSAS data group found. Expected an NXentry group containing an NXdata ' +
        'group with Q and I datasets.'
      );
    }

    const keys = dataGroup.keys();
    const qName = firstMatch(keys, Q_NAMES);
    const iName = firstMatch(keys, I_NAMES);

    if (!qName) throw new Error(`No Q dataset found. Tried: ${Q_NAMES.join(', ')}`);
    if (!iName) throw new Error(`No intensity/reflectivity dataset found. Tried: ${I_NAMES.join(', ')}`);

    const qEntity = dataGroup.get(qName);
    const iEntity = dataGroup.get(iName);
    if (!isDataset(qEntity)) throw new Error(`${qName} is not a dataset`);
    if (!isDataset(iEntity)) throw new Error(`${iName} is not a dataset`);

    const q = toNumberArray(qEntity.value);
    const refl = toNumberArray(iEntity.value);

    if (q.length !== refl.length) {
      throw new Error(`Q length (${q.length}) does not match R length (${refl.length})`);
    }

    const idevName = firstMatch(keys, IDEV_NAMES);
    const qdevName = firstMatch(keys, QDEV_NAMES);

    let reflError: number[];
    if (idevName) {
      const e = dataGroup.get(idevName);
      reflError = isDataset(e) ? toNumberArray(e.value) : new Array(q.length).fill(0);
    } else {
      reflError = new Array(q.length).fill(0);
    }

    let qError: number[];
    if (qdevName) {
      const e = dataGroup.get(qdevName);
      qError = isDataset(e) ? toNumberArray(e.value) : new Array(q.length).fill(0);
    } else {
      qError = new Array(q.length).fill(0);
    }

    const metadata = extractNexusMetadata(f);

    return {
      q,
      refl,
      reflError,
      qError,
      filePath,
      fileName: path.basename(filePath),
      ...(metadata.length > 0 ? { metadata } : {}),
    };
  } finally {
    f.close();
  }
}
