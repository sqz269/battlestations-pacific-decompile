# Startup VFS search registrations

The shader and texture search directories are hardcoded in `00738360`, called
by application initialization at `0073d894`. They are not inferred from an
installed directory walk or from a Lua configuration. This establishes actual
native candidates for `guifontbilinear.shfx`, `dummy.shfx`, and texture names
such as `fonts/white.tga`; provider selection and candidate availability still
determine which file wins.

`make_asset_search_registrations_00738360_fragment()` returns the established
texture and shaderfx groups as a typed snapshot for `VfsCandidateRegistrations`.
It omits unrelated groups, direct extension roots, mount/provider state and
aliases. It is sufficient to exercise these candidate lists with an explicitly
supplied loose-directory provider; it does not reconstruct startup mounts or
archive-versus-loose priority.

## Storage, writer ABI, and ordering

Manager constructor `00be1dc0` initializes the `+54` tree (sentinel at `+58`,
count at `+5C`) and `+60` linked list (sentinel at `+64`, count at `+68`). These
are separate structures. The latter is a list of named extension groups.

- `00be25e0`: ECX manager, stack group-name string and extension string,
  `RET8`. It pushes selector 0 and forwards to `00be2310`.
- `00be2600`: ECX manager, stack group-name string and directory string,
  `RET8`. It copies/lowercases the directory, converts backslashes to slashes,
  appends a trailing slash if absent, then calls `00be2310` with selector 1.
  The native last-character access does not safely handle empty directory
  strings; all directories projected here are nonempty literals.
- `00be2310`: ECX manager, three stack arguments (group, value, selector),
  `RETC`. Group lookup `00bdc230` lowercases a copy of the group name and scans
  existing groups for equal string length and `_stricmp` equality. The value
  is lowercased; selector 1 also normalizes/trails its slash. `00bdb2e0`
  selects the group's extension list at node `+10` for selector 0 or prefix
  list at `+1C` for selector 1.
- New groups are inserted **before the first node**. At `00be2455..00be2486`,
  the insertion receives `head->next` and its previous link and reconnects
  them around the new node. Thus the outer group order reverses first
  registration order. Existing groups are not moved.
- Values are appended **before the sentinel**, preserving call order. At
  `00be24d3..00be24f8`, insertion receives the sentinel and its previous link.
  Existing equal-length, case-insensitive matching values are skipped. The
  typed snapshot contains no duplicate literals, so it needs no runtime
  registration allocator or intrusive iterator substitute.
- `00be1480`: ECX manager, stack extension and directory, `RET8`, registers
  a separate `+54` extension-prefix tree record. It checks the pair through
  `00bdc3f0`, lowercases the copied strings, normalizes/trails the directory
  slash, then inserts through `00be0b90`. Its full tree tie-order/comparator
  is not implemented by this defaults fragment.

The complete 20-byte extension wrapper is
`8b4424088b5424046a005052e81ffdffffc20800`.

## Exact texture and shaderfx defaults

The `textures` group registers extension `dds` at `00738401`, then `tga`
at `007384d2`. These strings are literal data at `00cff084` and `00cff074`.
Its prefix list, after native normalization, is:

```text
textures/
models/textures/
models/textures/noseart/
models/gui/map/units/
models/gui/map/icons/
interface/textures/mainmenu/
interface/textures/
particles/textures/
particles/textures/anims/fragments/
effects/flares/textures/
fonts/
weather/
terrain/
effects/
effects/coast/
effects/watertracer/
effects/foam/
effects/caustics/
effects/lightning/
effects/traceline/
effects/foliage/
effects/postprocess/
interface/textures/terkep/
```

The later `shaderfx` group has extension `shfx` (literal `00cfee90`) and:

```text
shaderfx/
shaderfx/plane/
shaderfx/ship/
shaderfx/common/
shaderfx/particles/
shaderfx/terrain/
shaderfx/ocean/
shaderfx/lights/
shaderfx/postprocess/
shaderfx/gui/
```

Within the two-group snapshot, shaderfx precedes textures because group
insertion prepends. Both lists are straight-line unconditional registrations
inside `00738360`. They contain neither recursive directory expansion nor a
per-basename special rule. No `+54` entry for `shfx`, `dds` or `tga` is added
by this startup function. Other startup records include direct roots for
`lua -> interface/`, `mmod -> models/`, `dat -> fonts/` and `textures/`,
`shbin -> shaderfx/bin/`, `pso/vso -> shaders/`, plus unrelated groups such as
`fshaders` with extension `mshd`. Those records are not included in the typed
fragment; the reported sequence for repeated `dat` is registration order,
not a claim about the unported tree's equal-key iteration order.

The independently audited resolver `00bddc80` first attempts direct provider
resolution and later uses these extension/prefix lists in several ordered
passes. Therefore `dds,tga` registration order is not permission to replace
every direct TGA hit with DDS. See the candidate resolver audit/implementation
for full-name versus basename and alternate-extension pass precedence.
`effects/white.dds`, `shaderfx/gui/guifontbilinear.shfx`, and
`shaderfx/lights/dummy.shfx` are consequently native-generated candidates,
subject to provider existence and earlier hits. This improves on fixture
per-filename mappings without proving the original mount state.

## Initial mounts and directory enumeration boundary

The initial manager is constructed only if global `0109ceec` is null.
Startup registers provider factories, reads `GetCurrentDirectoryA`, appends
`\\`, and calls `00be1890` as follows (raw arguments in call order):

| Call | System path | Virtual path | Argument 3 | Argument 4 | Argument 5 |
|---|---|---|---:|---:|---:|
| `0073d6f9` | current directory plus `\\` | `.` | 0 | 1 | -1 |
| `0073d792` | current directory plus `\\` | `persistent_data` | 99 | 1 | -1 |
| `0073d829` | `filestore` | `.` | 300 | 0 | -1 |

Assembly establishes that `00be1890` is ECX manager with **five** stack
arguments and `RET14h`, despite its incomplete decompiler signature. It calls
manager virtual `+18` with system and virtual paths to select a provider;
argument 5 is stored in provider `+10` and is printed as `deviceid`. It
forwards provider, virtual path, arguments 3 and 4 to `00be1740`. Interpretation
and ordering of those latter values must be tied to mount insertion/traversal;
this audit does not assert a priority direction or silently convert virtual
`.` to an empty root.

Before `00738360`, startup also invokes `0073cb10` twice. That routine queries
the root for extension `mpkg`, detects a case-insensitive leading `patch`,
and uses `1000 + atol(suffix)` versus 1000 as its third mount argument.
Archive enumeration/provider semantics and why this caller repeats the scan
remain unported. These details prevent claiming that a single host loose root
is equivalent to the complete native mount list.

`00886280` constructs an output listing, normalizes a copied directory name,
then calls manager query `00bdd990` with the directory, supplied filter/flags
and output. It is an enumeration consumer, not the source of the established
texture/shader group registrations. The shader directory list should not be
replaced by arbitrary recursive file discovery on its account.

## Verification and limits

Every live batch verified project `bsp`, `/battlestationspacific.exe`, x86 LE,
base `00400000`. These saved-image byte spans matched installed PE bytes:

| Start | Bytes | SHA-256 |
|---|---:|---|
| `00738360` full registration body | 14197 | `69fee27dbfafde84f1de4c8debe61839d9b25b82d6cc3b6771828d039174e0da` |
| `00be2310` full group/value writer | 684 | `2767433449ca084f49ee036e150d1ec2a93e068e8f336c34693281f38657ada7` |
| `00be25e0` full extension wrapper | 20 | `5940556a3f4dc9af0142fcb77a6898ce03d010bf378c53b208468e7fb56d5c0c` |
| `00be2600` full directory wrapper | 242 | `2be45ed54c251183a01b1dcdfa73d1d93352568f3c22b96c6553ab5eafff3b61` |
| `00be1480` full extension-prefix registration | 286 | `4cd530b60ce4628b8ddf212c352b7f4bef06aa9189fb91aa4ec3f0a07aacc815` |
| `0073d68d` mount/call evidence window | 524 | `7afb65368afc6a5c929b4c7ebd0462ac90958816113fb993d0228f8083da508f` |
| `00cfec00` startup literal window | 1544 | `b4d164b08a96128f3c327a26ff88c856189314395cc6fb050588ce68814b1db1` |

Ignored exports are under `exports/bsp/vfs_registration/`. The separate
`persistent_data` string at `00cff208` was read live beyond that literal hash
window. C++ defaults were checked against registration order and assembly;
no tests, builds, Ghidra mutations or shared metadata edits were performed by
this bounded task. Native mounts, archive priority, runtime registration
changes and the omitted extension groups remain explicit dependencies.
