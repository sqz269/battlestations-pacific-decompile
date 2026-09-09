# Mounted resource streams and font loading

Mounted physical and FileStore providers now supply retained memory streams to
font and shader loading. The current Win32 build and full D3D9 probe pass.
After explicit cache priming, the font load records **three FileStore opens,
zero physical opens and three cache entries**, followed by retained atlas
recreation and a successful installed-source glyph A draw: 74 nonblack pixels,
zero outside the expected bounds, target/depth/state restored.

The output is in [mounted_stream_font_probe.txt](../reports/mounted_stream_font_probe.txt).
Native byte identities and ABI evidence are recorded in
[mounted_stream_audit.json](../reports/mounted_stream_audit.json), with assembly
exports under `exports/bsp/mounted_streams/`. This validates the typed
mounted-stream integration, not the original game's preload policy, archive
contents, renderer cache identity or gameplay behavior.

## Open ordering and aliases

`open_resource_memory_00bdf310_fragment` projects read-only flags 2 from native
manager `00bdf310` and its `00bda690` provider callback. The native open ABI is
ECX manager, name and flags on the stack, EAX stream, RET 8. The host accepts
an explicit `VfsMountContext`, validates strings and operation callbacks, and
performs these steps:

1. Normalize a copy of the requested name through `00bee690`.
2. Scan the supplied alias array, corresponding to manager `+94/+98`.
   The first equal-length, case-insensitive `_stricmp` match replaces the name.
   Replacement happens once; aliases do not chain and replacement text is
   **not normalized a second time**.
3. Reset the context error field to -1 and visit matching mounts in supplied
   order. Empty prefixes match all names; nonempty prefixes require a following
   slash and pass the remaining suffix to the provider.
4. Stop at the first provider that opens an underlying stream.

The existence and direct logical-resolution wrappers remain separate and do
not apply aliases. Candidate search is also separate: this open routine does
not prepend search directories or try alternate extensions. Diagnostic reads
first call the recovered availability/candidate resolver, then open its result.

`VfsMemoryOpen::provider_opened` distinguishes a provider miss from a successful
open followed by host buffering failure. Once true, traversal stops even when
`stream` is null or conversion reports an error. Falling through at that point
would incorrectly replace the selected resource with a lower-priority file.
Before any provider opens, a later matching provider may still be tried.
Native logging, stream counters/tracking and manager error callbacks are not
implemented; host failure text and domain checks are explicit substitutes.

## Real provider adapters and lifetime

`bind_physical_directory_fragment` captures a shared `PhysicalDirectory` and
binds its recovered existence and logical-resolution operations. Its read-only
open constructs a physical path, uses `PhysicalFile` flags-2 opening, then
converts through `memory_stream_from_physical_00bef750_fragment`. The physical
handle closes after conversion. It records a successful underlying open
independently of subsequent conversion status.

`bind_file_store_fragment` captures a shared `FileStore`, uses its existing
membership/logical-resolution routines, and opens with flags 2 through
`open_00be5fa0`. Each hit returns a fresh zero-cursor memory wrapper sharing
the stored backing. FileStore insertion retains a wrapper; open clones do not
consume that stored cursor and can outlive the store.

Native manager open returns an arbitrary provider stream; conversion normally
occurs in its consumer. This host interface buffers physical sources early
and returns only memory streams. Shared C++ provider ownership also replaces
native intrusive/pool/manager lifetime. These differences are intentional
boundaries, not native stream ABI compatibility. A short physical read may
return a stream with an uninitialized tail; consumers reject incomplete data
rather than filling it or silently choosing another mount.

## FileStore population ABI

Fresh assembly confirms `00be7ab0` takes **ECX store, name and flags as two
stack arguments, RET 8**. The former single-path/RET-4 interpretation is
incorrect. At entry `00be7ab0`, EDX loads `[ESP+8]`; `00be7ac9/00be7aca`
push flags/name to manager virtual `+4`, followed by memory conversion
`00bef750`, insertion `00be7760`, and balanced temporary releases. The return
at `00be7b17` is `c2 08 00`. The complete 106-byte body retains SHA-256
`7b91fe3afa47a5a50baa1944ac0ce230a16cc5baba7ed3d322efdf6a31384e9b`.

`cache_resource_00be7ab0_fragment` fixes flags to 2. It opens before duplicate
insertion is checked, requires a complete returned stream, creates a reset
wrapper sharing that backing, and calls `add_file_00be7760` with the original
supplied name. Thus it does not adopt an alias target as the cache key merely
because opening used that target. Existing-key insertion remains first-wins.
Invalid input/read failures return host errors; no native null-dereference or
error-callback behavior is claimed. Search fallback is the caller's job.

## Connected diagnostic path

`AssetStreamProbe` constructs the two recovered physical provider policies
using a supplied installation root and creates a FileStore. It registers
root `.` with priority 0, `persistent_data` with 99, and FileStore `.` with
300. Recovered canonicalization and signed ordering produce the traversal
view. Texture/shader groups come from the hardcoded `00738360` defaults;
there are no per-basename file mappings or recursive host searches.

The diagnostic explicitly resolves and primes the three font assets before
loading. The subsequent observed names are:

| Requested | Resolved |
|---|---|
| `Fonts/arial18.tga` | `fonts/arial18.tga` |
| `Fonts/white.tga` | `effects/white.dds` |
| `Fonts/arial18.dat` | `fonts/arial18.dat` |
| `guifontbilinear.shfx` | `shaderfx/gui/guifontbilinear.shfx` |
| `dummy.shfx` | `shaderfx/lights/dummy.shfx` |

The three-open counters cover GFX/alpha/DAT loading **after priming**, not all
physical I/O in the run. Explicitly choosing these priming requests tests the
cache connection; it does not recover native startup preload selection or
prove every shader came from FileStore.

`FontStreamResolver` now supplies those streams directly. Image loading keeps
a fresh reset wrapper sharing backing for D3D9 recreation; DAT decoding uses
the supplied cursor without resetting it. The physical-name font entrypoint
remains a compatibility adapter. Both entrypoints retain GFX-alpha-DAT order,
nonnull/completeness checks and transactional FontResources publication.
Native DAT reads its opened stream directly; early physical DAT buffering is
a host difference. The installed bilinear shader is generated and compiled
from its real Lua source, then drawn through the existing geometry, material,
buffer and constant paths.

MPKG enumeration/compression/source slices, native preload choice, full manager
teardown, renderer cache lifecycle and original-game visual parity remain
unresolved. No new test target or suite was added for this integration.
