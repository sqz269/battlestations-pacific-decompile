#!/usr/bin/env python
"""FMOD Ex SDK types for the Ghidra project (types and prototypes only; no renames).

The game ships FMOD Ex 4.18.04 (fmodex.dll, fmod_event.dll). Firelight's headers are public
but 4.18.04 itself is not archived; the closest SDK is 4.24.16, whose versioned structs have
the sizes the game writes (FMOD_ADVANCEDSETTINGS 44h, FMOD_CREATESOUNDEXINFO 6Ch) and whose
revision history records no member changes to them after 4.17. FMOD_RESULT did change (six
codes were inserted alphabetically), so its 4.18 numbering is recovered from the game's own
inlined FMOD_ErrorString() jump table instead of the SDK. See docs/FMOD_SDK_TYPES.md.

Subcommands
  extract --sdk DIR --exe PATH [--out config/fmod_ex_types.json]
      Parse fmod.h, fmod.hpp, fmod_errors.h, fmod_event.h and fmod_event.hpp plus the game's
      error-string table into one JSON file (only the methods the game imports are kept).
  verify [--types PATH] [--exe PATH]
      Re-derive the executable evidence and compare it with the JSON.
  apply [--types PATH] [--config PATH] [--dry-run]
      Under the Ghidra write lock: create missing FMOD class placeholders, replace the
      demangler's empty placeholder enums, create callback signatures, fill the placeholder
      structs and set a __stdcall prototype (with an explicit `this`) on every imported FMOD
      thunk. Previous values are written to local/fmod-types-<stamp>.json first.
"""
import argparse
import json
import re
import struct
import sys
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import urlencode
from urllib.request import Request, urlopen

sys.path.insert(0, str(Path(__file__).resolve().parent))
from coordination import check_writable, ghidra_lock  # noqa: E402
from ghidra_export import Client, ROOT  # noqa: E402

DEFAULT_TYPES = ROOT / 'config/fmod_ex_types.json'

# Structs the game passes to the library, with the sizes the game itself writes or the SDK gives.
STRUCTS = {'FMOD_VECTOR': 0x0c, 'FMOD_ADVANCEDSETTINGS': 0x44, 'FMOD_CREATESOUNDEXINFO': 0x6c,
           'FMOD_EVENT_INFO': 0x34}
ENUMS = ['FMOD_RESULT', 'FMOD_OUTPUTTYPE', 'FMOD_SPEAKERMODE', 'FMOD_SPEAKER', 'FMOD_SOUND_TYPE',
         'FMOD_SOUND_FORMAT', 'FMOD_OPENSTATE', 'FMOD_DSP_TYPE', 'FMOD_CHANNELINDEX',
         'FMOD_EVENT_PITCHUNITS', 'FMOD_SPEAKERMAPTYPE']
FLAGS = ['FMOD_MODE', 'FMOD_INITFLAGS', 'FMOD_TIMEUNIT', 'FMOD_CAPS', 'FMOD_EVENT_MODE',
         'FMOD_EVENT_STATE', 'FMOD_EVENT_INITFLAGS']
CALLBACKS = ['FMOD_FILE_OPENCALLBACK', 'FMOD_FILE_CLOSECALLBACK', 'FMOD_FILE_READCALLBACK',
             'FMOD_FILE_SEEKCALLBACK', 'FMOD_SOUND_PCMREADCALLBACK', 'FMOD_SOUND_PCMSETPOSCALLBACK',
             'FMOD_SOUND_NONBLOCKCALLBACK']
PRIMITIVES = {'char': 'char', 'int': 'int', 'unsigned int': 'uint', 'unsigned short': 'ushort',
              'short': 'short', 'unsigned char': 'uchar', 'float': 'float', 'double': 'double',
              'void': 'void', 'bool': 'bool', 'FMOD_BOOL': 'int', 'long': 'long',
              'unsigned long': 'ulong', 'FMOD_MEMORY_TYPE': 'uint', 'FMOD_DEBUGLEVEL': 'uint',
              'FMOD_REVERB_FLAGS': 'uint', 'FMOD_REVERB_CHANNELFLAGS': 'uint'}
C_OPAQUE = {'FMOD_SYSTEM': 'System', 'FMOD_SOUND': 'Sound', 'FMOD_CHANNEL': 'Channel',
            'FMOD_CHANNELGROUP': 'ChannelGroup', 'FMOD_SOUNDGROUP': 'SoundGroup', 'FMOD_DSP': 'DSP',
            'FMOD_DSPCONNECTION': 'DSPConnection', 'FMOD_GEOMETRY': 'Geometry',
            'FMOD_REVERB': 'Reverb', 'FMOD_EVENTSYSTEM': 'EventSystem', 'FMOD_EVENT': 'Event',
            'FMOD_EVENTPROJECT': 'EventProject', 'FMOD_EVENTGROUP': 'EventGroup',
            'FMOD_EVENTCATEGORY': 'EventCategory', 'FMOD_EVENTPARAMETER': 'EventParameter',
            'FMOD_EVENTREVERB': 'EventReverb', 'FMOD_MUSICSYSTEM': 'MusicSystem',
            'FMOD_MUSICPROMPT': 'MusicPrompt'}
CLASS_CATEGORY = '/Demangler/FMOD'
PLACEHOLDER_FIELDS = '[{"name":"opaque","type":"undefined1","offset":0}]'


def now_stamp():
    return datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')


# ----------------------------------------------------------------------------------------------
# Executable evidence
# ----------------------------------------------------------------------------------------------

def load_image(exe):
    import pefile
    pe = pefile.PE(str(exe))
    return pe, pe.OPTIONAL_HEADER.ImageBase, pe.get_memory_mapped_image()


def cstring(img, base, va, limit=512):
    off = va - base
    if off < 0 or off >= len(img):
        return None
    end = img.find(b'\0', off, off + limit)
    if end < 0:
        return None
    try:
        return img[off:end].decode('ascii')
    except UnicodeDecodeError:
        return None


def find_error_tables(img, base, minimum_cases=80):
    """Every inlined FMOD_ErrorString(): cmp eax,N ; ja ; jmp [eax*4+table] with `mov eax,str ; ret` cases."""
    tables = []
    for m in re.finditer(rb'\x83\xf8(.)[\x00-\xff]{0,12}?\xff\x24\x85(....)', img, re.S):
        count = m.group(1)[0] + 1
        if count < minimum_cases:
            continue
        table = struct.unpack('<I', m.group(2))[0]
        strings = []
        for i in range(count):
            off = table - base + 4 * i
            if off + 4 > len(img):
                break
            target = struct.unpack_from('<I', img, off)[0]
            code = img[target - base:target - base + 6] if 0 <= target - base < len(img) else b''
            if len(code) == 6 and code[0] == 0xb8 and code[5] == 0xc3:
                text = cstring(img, base, struct.unpack_from('<I', code, 1)[0])
                strings.append(text)
            else:
                strings.append(None)
        if all(s is not None for s in strings):
            tables.append({'jump': base + m.end() - 7, 'table': table, 'strings': strings})
    return tables


def fmod_imports(pe):
    """(iat_slot, dll, mangled) for every import from an FMOD DLL."""
    out = []
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        dll = entry.dll.decode()
        if 'fmod' not in dll.lower():
            continue
        for imp in entry.imports:
            if imp.name:
                out.append((imp.address, dll, imp.name.decode()))
    return out


def parse_mangled(name):
    m = re.match(r'\?(\w+)@(\w+)@FMOD@@', name)
    if m:
        return m.group(2), m.group(1)
    return None, re.sub(r'^_|@\d+$', '', name)


def thunk_map(pe, base, img):
    """thunk VA -> (dll, mangled, class or None, method) by locating `jmp [iat_slot]`."""
    out = {}
    for slot, dll, mangled in fmod_imports(pe):
        pattern = b'\xff\x25' + struct.pack('<I', slot)
        pos = img.find(pattern)
        if pos < 0:
            continue
        cls, method = parse_mangled(mangled)
        out[base + pos] = {'dll': dll, 'mangled': mangled, 'class': cls, 'method': method}
    return out


# ----------------------------------------------------------------------------------------------
# SDK header parsing
# ----------------------------------------------------------------------------------------------

def strip_comments(text):
    text = re.sub(r'/\*.*?\*/', '', text, flags=re.S)
    return re.sub(r'//[^\n]*', '', text)


def parse_enum_body(body):
    members, values = [], {}
    value = -1
    for raw in body.split(','):
        item = raw.strip()
        if not item:
            continue
        name, _, expr = [p.strip() for p in item.partition('=')]
        if not re.match(r'^\w+$', name):
            continue
        if expr:
            try:
                value = int(expr, 0)
            except ValueError:
                if expr in values:
                    value = values[expr]
                else:
                    continue
        else:
            value += 1
        values[name] = value
        members.append([name, value])
    return members


def split_params(text):
    text = text.strip()
    if text in ('', 'void'):
        return []
    out = []
    for piece in text.split(','):
        piece = re.sub(r'=.*$', '', piece).strip()
        m = re.match(r'^(?:const\s+)?([\w ]+?)\s*(\**)\s*(\w+)\s*(\[\s*\d*\s*\])?$', piece)
        if not m:
            raise ValueError('cannot parse parameter: %r' % piece)
        base, stars, name, array = m.groups()
        out.append([base.strip(), len(stars) + (1 if array else 0), name])
    return out


class TypeMapper:
    def __init__(self, enums, flags, structs, callbacks, classes):
        self.enums, self.flags, self.structs = set(enums), set(flags), set(structs)
        self.callbacks, self.classes = set(callbacks), set(classes)
        self.unmapped = set()

    def ghidra(self, base, stars):
        base = re.sub(r'\bconst\b', '', base).strip()
        if base in PRIMITIVES:
            name = PRIMITIVES[base]
        elif base in self.enums or base in self.flags or base in self.structs or base in self.callbacks \
                or base in self.classes:
            name = base
        elif base in C_OPAQUE:
            name = C_OPAQUE[base]
        else:
            self.unmapped.add(base)
            name = 'void'
        return name + ' *' * stars


def extract(args):
    sdk = Path(args.sdk)
    h_core = (sdk / 'api/inc/fmod.h').read_text(encoding='latin-1')
    h_event = (sdk / 'fmoddesignerapi/api/inc/fmod_event.h').read_text(encoding='latin-1')
    h_errors = (sdk / 'api/inc/fmod_errors.h').read_text(encoding='latin-1')
    hpp_core = (sdk / 'api/inc/fmod.hpp').read_text(encoding='latin-1')
    hpp_event = (sdk / 'fmoddesignerapi/api/inc/fmod_event.hpp').read_text(encoding='latin-1')

    version = {}
    for label, text in (('fmod.h', h_core), ('fmod_event.h', h_event)):
        m = re.search(r'#define\s+FMOD_(?:EVENT_)?VERSION\s+(0x[0-9A-Fa-f]+)', text)
        version[label] = m.group(1) if m else None

    enums, flags, structs, callbacks = {}, {}, {}, {}
    for text in (h_core, h_event):
        for gm in re.finditer(r'\[NAME\]\s*\n\s*(\w+)(.*?)\[DEFINE_END\]', text, re.S):
            defs = [[n, int(v, 0)] for n, v in
                    re.findall(r'^#define\s+(\w+)\s+(0x[0-9A-Fa-f]+|-?\d+)', gm.group(2), re.M)]
            if defs:
                flags[gm.group(1)] = defs
        clean = strip_comments(text)
        clean = re.sub(r'#ifdef __cplusplus.*?#endif', '', clean, flags=re.S)
        for em in re.finditer(r'typedef enum\s*\w*\s*\{(.*?)\}\s*(\w+)\s*;', clean, re.S):
            enums[em.group(2)] = parse_enum_body(em.group(1))
        for sm in re.finditer(r'typedef struct\s*\w*\s*\{(.*?)\}\s*(\w+)\s*;', clean, re.S):
            structs[sm.group(2)] = [decl for decl in
                                    (' '.join(d.split()) for d in sm.group(1).split(';')) if decl]
        for cm in re.finditer(r'typedef\s+(\w+)\s*\(F_CALLBACK\s*\*(\w+)\)\s*\(([^)]*)\)\s*;', clean):
            callbacks[cm.group(2)] = {'return': cm.group(1), 'params': split_params(cm.group(3))}

    classes, methods = [], {}
    for text in (hpp_core, hpp_event):
        clean = strip_comments(text)
        for km in re.finditer(r'class\s+(\w+)\s*\{(.*?)\n\s*\};', clean, re.S):
            cls = km.group(1)
            classes.append(cls)
            for mm in re.finditer(r'(?:virtual\s+)?(\w+)\s+F_API\s+(\w+)\s*\(([^)]*)\)', km.group(2)):
                methods.setdefault(cls, {})[mm.group(2)] = {
                    'return': mm.group(1), 'params': split_params(mm.group(3)),
                    'raw': ' '.join(mm.group(0).split())}
    c_functions = {}
    for text in (h_core, h_event):
        for fm in re.finditer(r'(\w+)\s+F_API\s+(FMOD_\w+)\s*\(([^)]*)\)\s*;', strip_comments(text)):
            c_functions[fm.group(2)] = {'return': fm.group(1), 'params': split_params(fm.group(3)),
                                        'raw': ' '.join(fm.group(0).split())}

    # Game evidence: the inlined FMOD_ErrorString() switch gives the 4.18.04 FMOD_RESULT numbering.
    pe, base, img = load_image(Path(args.exe))
    tables = find_error_tables(img, base)
    if not tables:
        sys.exit('no inlined FMOD_ErrorString jump table found in the executable')
    first = tables[0]['strings']
    if any(t['strings'] != first for t in tables):
        sys.exit('the inlined FMOD_ErrorString copies disagree')
    by_text = {}
    for name, text in re.findall(r'case (FMOD_\w+):\s*return "([^"]*)";', h_errors):
        by_text.setdefault(text.strip(), name)
    result_418, unmatched, prefix_matched = [], [], []
    for value, text in enumerate(first):
        key = text.strip()
        name = by_text.get(key)
        if name is None:
            # Firelight reworded a few messages between 4.18 and 4.24; accept a unique 40-char prefix.
            candidates = {n for t, n in by_text.items() if t[:40] == key[:40]}
            if len(candidates) == 1:
                name = candidates.pop()
                prefix_matched.append([value, name, text])
            else:
                unmatched.append([value, text])
        result_418.append([name or 'FMOD_RESULT_%d' % value, value])
    result_418.append(['FMOD_RESULT_FORCEINT', 65536])
    sdk_result = enums['FMOD_RESULT']
    enums['FMOD_RESULT'] = result_418

    imports = thunk_map(pe, base, img)
    mapper = TypeMapper(ENUMS, FLAGS, STRUCTS, CALLBACKS, classes)
    kept_methods, kept_c = {}, {}
    for va, info in sorted(imports.items()):
        if info['class']:
            decl = methods.get(info['class'], {}).get(info['method'])
            if decl is None:
                sys.exit('no SDK declaration for %s::%s' % (info['class'], info['method']))
            kept_methods.setdefault(info['class'], {})[info['method']] = {
                'return': decl['return'], 'raw': decl['raw'],
                'params': [[mapper.ghidra(b, s), n] for b, s, n in decl['params']]}
        else:
            decl = c_functions.get(info['method'])
            if decl is None:
                sys.exit('no SDK declaration for %s' % info['method'])
            kept_c[info['method']] = {'return': decl['return'], 'raw': decl['raw'],
                                      'params': [[mapper.ghidra(b, s), n] for b, s, n in decl['params']]}
    kept_structs = {}
    for name in STRUCTS:
        fields = []
        for decl in structs[name]:
            base_t, stars, fname = split_params(decl)[0]
            fields.append([fname, mapper.ghidra(base_t, stars)])
        kept_structs[name] = fields
    kept_callbacks = {}
    for name in CALLBACKS:
        cb = callbacks[name]
        kept_callbacks[name] = {'return': cb['return'],
                                'params': [[mapper.ghidra(b, s), n] for b, s, n in cb['params']]}
    kept_enums = {name: enums[name] for name in ENUMS if name in enums}
    kept_flags = {name: flags[name] for name in FLAGS if name in flags}

    out = {
        'sdk': {'product': 'FMOD Ex Programmers API', 'version': '4.24.16', 'fmod_version': version,
                'source': 'archive.org item fmodapi44452nacl.tar, file fmodapi42416win32-installer.exe'},
        'game': {'dll_version': '4.18.4', 'note': 'fmodex.dll and fmod_event.dll version resources; '
                 'the runtime reports 0x41804'},
        'evidence': {
            'result_table': {'jump_sites': ['%08x' % t['jump'] for t in tables],
                             'table': '%08x' % tables[0]['table'], 'cases': len(first),
                             'unmatched': unmatched, 'prefix_matched': prefix_matched,
                             'sdk_codes_absent_in_game': [n for n, v in sdk_result
                                                          if n not in {x[0] for x in result_418}]},
            'struct_sizes': {'FMOD_ADVANCEDSETTINGS': 'game zeroes a 44h block and stores cbsize=44h '
                             '(docs/SOUND_CONFIGURATION.md step 4)',
                             'FMOD_CREATESOUNDEXINFO': 'game memsets 6Ch and stores cbsize=6Ch before '
                             'System::createSound in FUN_00a823f0',
                             'FMOD_EVENT_INFO': 'SDK 4.24.16 size; revision history records no member '
                             'change after 4.13.00 (provisional for 4.18)'},
        },
        'enums': kept_enums, 'flags': kept_flags, 'structs': kept_structs,
        'callbacks': kept_callbacks, 'classes': sorted(set(classes)),
        'methods': kept_methods, 'c_functions': kept_c,
        'imports': {'%08x' % va: info for va, info in sorted(imports.items())},
        'unmapped_types': sorted(mapper.unmapped),
    }
    Path(args.out).write_text(json.dumps(out, indent=1) + '\n', encoding='utf-8')
    print('wrote', args.out)
    print('FMOD_RESULT: %d game codes, %d unmatched, absent SDK codes %s' % (
        len(first), len(unmatched), out['evidence']['result_table']['sdk_codes_absent_in_game']))
    print('imports %d (methods %d, C functions %d), unmapped types -> void: %s' % (
        len(imports), sum(len(v) for v in kept_methods.values()), len(kept_c), sorted(mapper.unmapped)))


# ----------------------------------------------------------------------------------------------
# verify
# ----------------------------------------------------------------------------------------------

def verify(args):
    types = json.loads(Path(args.types).read_text(encoding='utf-8'))
    pe, base, img = load_image(Path(args.exe))
    tables = find_error_tables(img, base)
    strings = tables[0]['strings'] if tables else []
    listed = [v for n, v in types['enums']['FMOD_RESULT'] if v < 65536]
    ok = len(strings) == len(listed) and all(t['strings'] == strings for t in tables)
    print('error tables: %d copies, %d cases, JSON codes %d -> %s' % (
        len(tables), len(strings), len(listed), 'match' if ok else 'MISMATCH'))
    imports = thunk_map(pe, base, img)
    missing = [k for k in types['imports'] if int(k, 16) not in imports]
    print('imports: %d in exe, %d in JSON, missing %s' % (len(imports), len(types['imports']), missing))
    return 0 if ok and not missing else 1


# ----------------------------------------------------------------------------------------------
# apply
# ----------------------------------------------------------------------------------------------

class Bridge:
    def __init__(self, config, record, dry_run):
        self.client = Client(config)
        self.config = config
        self.record = record
        self.dry_run = dry_run

    def get(self, endpoint, **params):
        result = self.client.get(endpoint, **params)
        return result.get('result', result) if isinstance(result, dict) else result

    def post(self, endpoint, **body):
        entry = {'endpoint': endpoint, 'body': body}
        self.record['actions'].append(entry)
        if self.dry_run:
            entry['result'] = 'dry-run'
            return 'dry-run'
        url = self.config['ghidra_url'].rstrip('/') + '/' + endpoint + '?' + urlencode(
            {'program': self.config['program']})
        request = Request(url, data=json.dumps(body).encode(), headers={'Content-Type': 'application/json'},
                          method='POST')
        with urlopen(request, timeout=120) as response:
            text = response.read().decode('utf-8')
        try:
            decoded = json.loads(text)
        except json.JSONDecodeError:
            decoded = text
        if isinstance(decoded, dict):
            if decoded.get('error') or decoded.get('success') is False or decoded.get('status') == 'error':
                raise RuntimeError('%s %s: %s' % (endpoint, body, decoded))
            decoded = decoded.get('result', decoded)
            if isinstance(decoded, dict) and (decoded.get('error') or decoded.get('status') == 'error'):
                raise RuntimeError('%s %s: %s' % (endpoint, body, decoded))
        if isinstance(decoded, str) and re.match(r'\s*(Error|Failed|Invalid|.*not found)', decoded, re.I):
            raise RuntimeError('%s %s: %s' % (endpoint, body, decoded))
        entry['result'] = decoded if isinstance(decoded, str) else json.dumps(decoded)
        return decoded

    def type_paths(self, name):
        text = self.get('search_data_types', pattern=name, limit=50)
        paths = []
        for line in str(text).splitlines():
            m = re.match(r'(.+?) \| Size: (-?\d+) \| Path: (.+)$', line.strip())
            if m and m.group(1) == name:
                paths.append((m.group(3), int(m.group(2))))
        return paths

    def struct_layout(self, name):
        text = str(self.get('get_struct_layout', struct_name=name))
        size = re.search(r'Size: (\d+)', text)
        fields = re.findall(r'^\s*(0x[0-9a-fA-F]+|\d+)\s*\|\s*(\d+)\s*\|\s*(.+?)\s*\|\s*(\S+)\s*$', text, re.M)
        return {'size': int(size.group(1)) if size else None, 'fields': fields, 'text': text}


def prototype_text(name, ret, params):
    inner = ', '.join('%s %s' % (t, n) for t, n in params)
    return '%s __stdcall %s(%s)' % (ret, name, inner)


def fill_struct(bridge, name, fields, expected, record, dry_run):
    """Append the SDK fields to a placeholder struct, restore the SDK names (the bridge rewrites
    them in Hungarian notation) and pad to the C alignment. Idempotent."""
    rows = bridge.struct_layout(name)['fields']
    for index, (fname, ftype) in enumerate(fields):
        if index < len(rows):
            continue
        bridge.post('add_struct_field', struct_name=name, field_name=fname, field_type=ftype)
    if dry_run:
        return
    rows = bridge.struct_layout(name)['fields']
    for index, (fname, ftype) in enumerate(fields):
        if index < len(rows) and rows[index][3] != fname:
            bridge.post('modify_struct_field', struct_name=name, field_name=rows[index][3], new_name=fname)
    size = bridge.struct_layout(name)['size']
    while size is not None and size < expected:
        gap = expected - size
        ptype = 'uint' if gap >= 4 else 'ushort' if gap >= 2 else 'byte'
        bridge.post('add_struct_field', struct_name=name, field_name='padding_%x' % size, field_type=ptype)
        size = bridge.struct_layout(name)['size']
    layout = bridge.struct_layout(name)
    if layout['size'] != expected:
        raise RuntimeError('%s size %s != expected %#x' % (name, layout['size'], expected))
    wrong = [(row[3], fname) for row, (fname, _) in zip(layout['fields'], fields) if row[3] != fname]
    if wrong:
        record['errors'].append('%s field names still differ from the SDK: %s' % (name, wrong))


def apply(args):
    config = json.loads(Path(args.config).read_text(encoding='utf-8'))
    types = json.loads(Path(args.types).read_text(encoding='utf-8'))
    stamp = now_stamp()
    record = {'stamp': stamp, 'types': str(args.types), 'dry_run': args.dry_run, 'before': {}, 'actions': [],
              'errors': []}
    bridge = Bridge(config, record, args.dry_run)

    info = str(bridge.get('get_current_program_info'))
    if config['program'] not in info:
        sys.exit('Ghidra program mismatch: expected %s, got %s' % (config['program'], info[:200]))
    thunks = sorted(types['imports'])
    check_writable(thunks)

    # Snapshot the state we are about to change.
    before = record['before']
    before['prototypes'] = {}
    missing = []
    for va in thunks:
        text = bridge.get('get_function_by_address', address=va)
        text = json.dumps(text) if not isinstance(text, str) else text
        before['prototypes'][va] = text
        name = types['imports'][va]['method']
        if 'No function found' in text:
            missing.append(va)  # a `jmp [iat]` stub Ghidra never turned into a thunk function
        elif name not in text:
            sys.exit('thunk %s is not named %s in Ghidra: %s' % (va, name, text[:160]))
    before['enums'] = {name: str(bridge.get('get_enum_values', enum_name=name))
                       for name in list(types['enums']) + list(types['flags']) if bridge.type_paths(name)}
    before['structs'] = {name: bridge.struct_layout(name)['text'] for name in types['structs']}
    before['classes'] = {cls: bridge.type_paths(cls) for cls in types['classes']}

    needed_classes = set()
    for cls, methods in types['methods'].items():
        needed_classes.add(cls)
        for decl in methods.values():
            for t, _ in decl['params']:
                needed_classes.add(t.replace('*', '').strip())
    for decl in types['c_functions'].values():
        for t, _ in decl['params']:
            needed_classes.add(t.replace('*', '').strip())
    for fields in types['structs'].values():
        for _, t in fields:
            needed_classes.add(t.replace('*', '').strip())
    needed_classes &= set(types['classes'])

    plan = {'classes': [], 'enums': [], 'structs': [], 'prototypes': 0, 'created_thunks': []}
    skipped = set()
    out = ROOT / 'local' / ('fmod-types-%s%s.json' % (stamp, '-dry' if args.dry_run else ''))
    out.parent.mkdir(exist_ok=True)
    try:
        apply_locked(args, bridge, types, thunks, missing, needed_classes, plan, skipped, record)
    finally:
        record['plan'] = plan
        out.write_text(json.dumps(record, indent=1), encoding='utf-8')
    print('%s: classes %s, enums %d, structs %s, prototypes %d, errors %d -> %s' % (
        'planned' if args.dry_run else 'applied', plan['classes'], len(plan['enums']), plan['structs'],
        plan['prototypes'], len(record['errors']), out))
    for err in record['errors']:
        print('  note:', err)


def apply_locked(args, bridge, types, thunks, missing, needed_classes, plan, skipped, record):
    with ghidra_lock(ttl_seconds=3600, wait_seconds=180, purpose='fmod_sdk_types apply'):
        # 0. Define the import stubs Ghidra missed so their callers decompile as calls at all.
        for va in missing:
            bridge.post('create_function', address=va)
            if not args.dry_run:
                text = str(bridge.get('get_function_by_address', address=va))
                if 'No function found' in text:
                    record['errors'].append('could not create a function at %s; prototype skipped' % va)
                    skipped.add(va)
                    continue
                record['errors'].append('created thunk function at %s: %s' % (va, text.splitlines()[0][:120]))
            plan['created_thunks'].append(va)

        # 1. Opaque class placeholders that the demangler never created (no `this` in its signatures).
        for cls in sorted(needed_classes):
            if not bridge.type_paths(cls):
                bridge.post('create_struct', name=cls, fields=PLACEHOLDER_FIELDS)
                bridge.post('move_data_type_to_category', type_name=cls, category_path=CLASS_CATEGORY)
                plan['classes'].append(cls)

        # 2. Enums: the demangler's placeholders are empty and cannot be filled in place, so they are
        #    deleted (references become undefined4 until step 5 restores every prototype).
        for group in ('enums', 'flags'):
            for name, members in types[group].items():
                if bridge.type_paths(name) and re.search(r'^\S+\s+\|\s+-?\d+', str(bridge.get(
                        'get_enum_values', enum_name=name)), re.M):
                    continue  # already a populated enum (an earlier run), not a placeholder
                for _ in range(3):
                    if not bridge.type_paths(name):
                        break
                    bridge.post('delete_data_type', type_name=name)
                if not args.dry_run and bridge.type_paths(name):
                    raise RuntimeError('could not remove placeholder %s' % name)
                bridge.post('create_enum', name=name, values=json.dumps({n: v for n, v in members}), size=4)
                plan['enums'].append(name)

        # 3. Callback signatures as function definitions plus pointer typedefs under the SDK names.
        for name, cb in types['callbacks'].items():
            fn_name = name + '_FN'
            have_fn = bool(bridge.type_paths(fn_name))
            if not have_fn:
                params = ', '.join('%s %s' % (t, n) for t, n in cb['params'])
                try:
                    bridge.post('create_function_signature', name=fn_name, return_type=cb['return'],
                                parameters=params)
                    have_fn = True
                except RuntimeError as exc:
                    record['errors'].append(str(exc))
            if not bridge.type_paths(name):
                try:
                    bridge.post('create_typedef', name=name, base_type=(fn_name + ' *') if have_fn else 'void *')
                except RuntimeError as exc:
                    record['errors'].append(str(exc))
                    bridge.post('create_typedef', name=name, base_type='void *')

        # 4. Fill the placeholder structs in place so every existing pointer reference follows.
        for name, fields in types['structs'].items():
            fill_struct(bridge, name, fields, STRUCTS[name], record, args.dry_run)
            plan['structs'].append(name)

        # 5. Prototypes: FMOD's Win32 C++ exports are __stdcall members, so `this` is the first stack slot.
        this_name = args.this_name
        for va in thunks:
            if va in skipped:
                continue
            imp = types['imports'][va]
            if imp['class']:
                decl = types['methods'][imp['class']][imp['method']]
                params = [[imp['class'] + ' *', this_name]] + decl['params']
            else:
                decl = types['c_functions'][imp['method']]
                params = decl['params']
            text = prototype_text(imp['method'], decl['return'], params)
            try:
                bridge.post('set_function_prototype', function_address=va, prototype=text,
                            calling_convention='__stdcall')
            except RuntimeError as exc:
                if imp['class'] and this_name == 'this':
                    this_name = 'self'
                    record['errors'].append('retrying with `self`: %s' % exc)
                    params[0][1] = this_name
                    bridge.post('set_function_prototype', function_address=va,
                                prototype=prototype_text(imp['method'], decl['return'], params),
                                calling_convention='__stdcall')
                else:
                    raise
            plan['prototypes'] += 1
        if not args.dry_run:
            bridge.post('save_program')


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest='command', required=True)
    p = sub.add_parser('extract')
    p.add_argument('--sdk', required=True)
    p.add_argument('--exe', required=True)
    p.add_argument('--out', default=str(DEFAULT_TYPES))
    p.set_defaults(func=extract)
    p = sub.add_parser('verify')
    p.add_argument('--types', default=str(DEFAULT_TYPES))
    p.add_argument('--exe', default=None)
    p.add_argument('--config', default=str(ROOT / 'config/target.json'))
    p.set_defaults(func=verify)
    p = sub.add_parser('apply')
    p.add_argument('--types', default=str(DEFAULT_TYPES))
    p.add_argument('--config', default=str(ROOT / 'config/target.json'))
    p.add_argument('--dry-run', action='store_true')
    p.add_argument('--this-name', default='this')
    p.set_defaults(func=apply)
    args = parser.parse_args()
    if args.command == 'verify' and not args.exe:
        args.exe = json.loads(Path(args.config).read_text(encoding='utf-8'))['binary']
    rc = args.func(args)
    sys.exit(rc or 0)


if __name__ == '__main__':
    main()
