# Ordered mount lookup and installed font integration

The existing font probe now uses reconstructed name normalization, ordered
candidate searches and physical directory checks. Startup-derived texture and
shader lists replace its per-filename mappings. With one supplied loose game
root, `Fonts/white.tga` resolves to `effects/white.dds`,
`guifontbilinear.shfx` to `shaderfx/gui/guifontbilinear.shfx`, and `dummy.shfx`
to `shaderfx/lights/dummy.shfx`. These are observed results of the C++ probe,
not observations of the original game's complete mounted filesystem.

## Native mount traversal and callbacks

`00bdd0a0` takes manager ECX, name and callback stack pointers, RET8. It resets
manager error DWORD+18 to FFFFFFFF, copies the name and traverses the tree
rooted at manager+40 in iterator order. An empty virtual prefix matches every
name. A nonempty prefix must be strictly shorter than the name, match its
initial substring by equal-length `_stricmp` through `00435c40`, and have a
following slash. The callback receives the suffix after that slash. An exact
name equal to a nonempty prefix does not match. No longest-prefix preference
is inferred; the first successful callback ends traversal.

`00bdd440` takes manager ECX and one name pointer, returns AL, RET4. It copies
and normalizes the input using `00bee690`, then traverses with callback table
`00d68398`. Callback+4, `00bd90d0`, takes mount and suffix stack pointers,
RET8. It invokes the provider at mount+8 through provider+10, passing suffix,
and stores AL in callback+4. Callback+8, `00bd90f0`, reads that byte.

`00bdd6e0` takes manager ECX and input/output string pointers, returns AL,
RET8. It normalizes an input copy, uses callback table `00d683f4`, and copies
the callback's name to output only on success. Input and output may alias;
failure leaves output unchanged. Its callback+4 is `00bdbc70`, ECX callback,
stack mount/suffix, RET8. Provider+24 receives suffix and a name output.
With an empty mount prefix, it writes directly to the callback's name. With
a nonempty prefix, the callback constructs `prefix + '/' + provider-name`.
It also copies provider+10 to callback+10 on success. The outer routine does
not expose that device identifier. Callback+8, `00bdb670`, reads success.

The direct callback constructs its private prefixed result even on provider
failure. The typed projection skips that unused temporary construction;
allocation timing/SEH on failed calls is outside its contract. It preserves
the externally copied result and success condition. Provider+24 returns a
logical name; using the separate physical-path provider+18 here would add an
incorrect system root to the virtual name.

Neither wrapper applies alias substitution `00bdca80`. That belongs to the
separately audited file-open route `00bdf310`. Existence, resolved logical
names and physical paths are distinct operations.

## C++ boundaries

`VfsMountContext` supplies the ordered mount sequence and exposes the reset
error field. `VfsMount` supplies real existence/resolution callbacks.
`vfs_mounts.cpp` implements these two wrappers and their traversal behavior;
native tree insertion, comparator/priority, allocation and callback ownership
remain dependencies. Callbacks must not mutate the supplied mount sequence.
Host validation rejects missing callbacks, embedded NULs and lengths above
INT32_MAX. Oversized resolved names fail without copying to caller output.
Allocation and provider exceptions propagate; no native SEH ABI is claimed.

`resolve_existing_resource_00bdf4c0_fragment` normalizes the caller's string
then invokes the ordered lower resolver. Failure can leave the normalized
name changed. Successful-search logging `00bdeb40` is excluded. See
[VFS_CANDIDATE_RESOLUTION.md](VFS_CANDIDATE_RESOLUTION.md) for pass ordering,
[VFS_SEARCH_REGISTRATION.md](VFS_SEARCH_REGISTRATION.md) for the hardcoded
startup lists, and [PHYSICAL_DIRECTORY.md](PHYSICAL_DIRECTORY.md) for the
provider's cached, non-indexed existence mode.

`LooseAssetProbe` is diagnostic setup: one empty-prefix mount with the supplied
installation root and the two recovered search groups. Font scripts/images,
shader descriptors and combiners now pass through this route. The physical
path is constructed only after logical resolution succeeds. No recursive
filesystem scan or per-basename substitution is used. Startup archives,
patch packages, native virtual `.` handling, other groups, runtime search
changes and mount priority remain unported. The old direct DAT inspection
still supplies an independent fixture check before the resource-owner load.

## Evidence and validation

Live project `bsp`, program `/battlestationspacific.exe` were verified before
each batch. All 21 windows in `reports/vfs_lookup_audit.json` matched installed
PE bytes, including registration literals, provider/callback tables and:

| Function | Full body bytes | SHA-256 |
|---|---:|---|
| `00bdd0a0` | 672 | `6688eee94e647c22020b87083766a158686a073c47ff9d89162e5440bd5d3397` |
| `00bdd440` | 223 | `6a6dc353db9a08f05ab7efeedfbce1e16b4d8a283789fbd03c218f4ac3b5a40d` |
| `00bdd6e0` | 360 | `9ca39982231bc39db57dd066eb6af7b78db7295161ac351e8264e14d3e6afcc3` |
| `00bdbc70` | 396 | `03f512991938e529b88dc099991ba4c091b835a7d5bd88d75d7c725087b63198` |
| `00bd90d0` | 29 | `927c31d2787703e88198e0d7456596852fd7c499f91810b153abe79f9371a747` |

MSVC Win32 build and both existing CTests passed. The existing D3D9 probe
passed with installed assets; `reports/vfs_font_draw_probe.txt` records
the resolved names and bilinear A glyph readback: 74 lit pixels, zero outside
the expected bounds, sampler mask1, state restored. Native glyph-byte and
camera comparisons and existing texture/reset/query checks also passed.
No new test target or standalone test cases were added. Nonempty mount
prefixes, provider collision priority and stale existence cache behavior are
assembly-backed, not newly runtime-tested. No original-game visual comparison,
native container/ABI compatibility or complete game rebuild is established.

Seventeen reviewed names/comments were applied in Ghidra, preserving prior
comments and recording old values in
`local/ghidra-annotations-20260909T175545Z.json`. Three previously undefined
callback/provider functions were created at verified starts; their prior
absence was recorded in `local/vfs_function_creation_before.json`. The project
was saved, the inventory and all 17 affected exports refreshed, and the names
and evidence comments read back successfully.
