# Font resource ownership

`FontResources` projects the successful image/DAT load and the two font-level
image references. It composes mounted physical/FileStore streams, shared memory
backing, the retained 2D loader and scalar `FontData` decoder. It does not
reproduce native font layout, renderer cache identity, reload or complete text
rendering. Mounted opening and explicit FileStore population are connected.

## Native ownership evidence

Verified live project `bsp`, program `/battlestationspacific.exe`, x86 LE base
`00400000` before each analysis batch. Exports and raw continuations are under
ignored `exports/bsp/owner_textures/font/`. The following raw windows matched
the installed executable byte for byte; ends are exclusive.

| Window | Bytes | SHA-256 |
|---|---:|---|
| `00ad53a0..00ad558b` | 491 | `727bac8633e6596c37db8683e054acc5219561491a04a88685023c515feadfab` |
| `00ad51d0..00ad5315` | 325 | `1bfa9dfe42dae37d6c2e8be7c2668330d462dec533e478bc5395b74f77ed0cec` |
| `00ad4e98..00ad4f07` | 111 | `e8ca10153e8ed14dedebca624489be2a0e4225708d6cfcdc85fcedb1c609e378` |
| `00ad5079..00ad5154` | 219 | `04db14e97661453d6df3303f4404c0b2bdc2a02ada1a5abbd38f7c7dab6d39a7` |

First two windows include trailing INT3 padding; destructor returns at
`00ad5586`, reload returns `RET 8` at `00ad5310`. The last window ends beyond
the relevant resource stores and is not a function extent. Raw assembly was
necessary because `_free` has erroneous no-return analysis, truncating the
decompiler's cleanup loops and tail.

Constructor `00ad55c0` has ECX font, stack name/scale/alpha-scale/uppercase,
`RET 10h`. It initializes the tree, name/path strings and scalar fields. It
does not install a font vtable. Destruction is direct `00ad53a0`, ECX font,
plain RET, called by `00ac3013` and `00ac374a`; no guessed virtual destructor
is needed. The scalar constructor does not initialize the later embedded
resource fields `+64/+68`, so arbitrary partial native construction cannot be
treated as safely destructible using the normal completed-font destructor.

The initial loader `00ad4c30` has ECX font and five string arguments, `RET 14h`:
prefix, Data, GFX, AlphaTexture, extra path component. Its image results are
copied to ordinary glyph `+18/+1C` without increments. After DAT reads, it
releases the input stream at `00ad5079..00ad508d`. Then it initializes embedded
space record base `font+4C`: `00ad50cb` assigns alpha to `font+68`, and
`00ad50d1` assigns GFX to `font+64`, again without increments. These two fields
hold the original loader references. The fallback record copies all 32 bytes
from key0091 using `REP MOVSD` at `00ad5109`; carriage-return fields receive
the same image pointers at `00ad5142/+5148`. They are aliases.

Destructor ordering is established directly:

1. Traverse glyph nodes; free each nonnull payload and set node+10 to null
   (`00ad5407..00ad5417`), then continue traversal at `00ad541e..00ad542f`.
   No image decrement accompanies each payload free.
2. Release `font+64` once, using resource intrusive count+4 and virtual zero
   on final release; clear the field (`00ad5431..00ad5450`).
3. Release/clear `font+68` in the same way (`00ad5457..00ad5470`).
4. Destroy string buffers in order `+40,+38,+30,+28,+20,+0C`, then tree nodes
   via `00ad4b60`, then the sentinel via `_free` at `00ad5564`. Raw tail clears
   font+4/+8 before return.

Thus a font owns two image references, not two references per glyph. If both
loads return the same cached object, both returned references still require
their separate release. Material slot assignment independently retains its
resource (see `FONT_MATERIAL_BINDINGS.md`), allowing material references to
outlive the font.

Reload `00ad51d0` compares supplied prefix/extra strings against saved paths.
On change it frees existing payloads, clears tree nodes, releases/clears
`+64/+68`, then calls `00ad4c30` at `00ad5304` with saved Data/GFX/AlphaTexture
names. This is destructive before loading. No reload method was added to the
typed projection; its transactional initial-load factory must not be called a
native reload reconstruction.

## Paths and typed interface

`load_font_resources_from_streams_00ad4c30_fragment` receives `FontDescriptor`,
explicit prefix and extra component, stable mip setting, D3DX imports and a
`FontStreamResolver`. That callback returns a retained `MemoryStream` for the
exact logical name, with an error string on failure. A mounted physical or
FileStore opener can supply that stream without inventing a physical filename
for archive-backed data. This interface does not itself choose providers,
rewrite names, or reproduce renderer cache identity.

The existing `load_font_resources_00ad4c30_fragment` remains compatible: its
`FontPhysicalResolver` maps the name to a physical ANSI filename, which the
adapter opens read-only and converts to a memory stream before delegating to
the same loader. There is no implicit separator, normalization or installation
root in either interface. Caller-supplied fixture roots do not establish the
unresolved startup extra component's value.

Order is GFX, alpha, DAT. GFX and DAT both use `(prefix + extra) + name`:
the DAT concatenations are `00ad4eaa/00ad4ec1`, followed by VFS virtual+4 open
with flags2 at `00ad4ed5`. Nonempty AlphaTexture uses `prefix + name` with no
extra; empty AlphaTexture loads unprefixed `white.tga`. The registry's missing
field default is nonempty `white.tga` and therefore takes the prefixed route.
Both image calls use the recovered flags-zero initial image-loading route.

The host stores scalar glyphs once in `FontData`, GFX and alpha as two shared
retained texture owners. Glyphs select the same two owners without creating
per-glyph ownership. Consumers may copy those shared pointers to represent
independently retained material resources. The font destructor clears glyphs,
then GFX, then alpha. Host map nodes are destroyed with their payloads, earlier
than native tree nodes; native string buffers/layout, pool, reference counters
and renderer cache identity are not reproduced.

Both entrypoints share GFX-alpha-DAT ordering, image metadata policy and
transactional ownership. Each successful stream callback must supply nonnull,
fully initialized backing. Images use `clone_reset_00bef6d0` to retain that
backing in a fresh wrapper with cursor zero: this is the already recovered
memory-input case of native `00bef750`. The retained texture keeps this new
wrapper for recreation. The resolver's original cursor is neither moved nor
retained merely to create the image; no image byte copy is added by cloning.

Native DAT reads its opened VFS stream directly. The typed DAT decoder consumes
the callback's supplied cursor directly and does not reset or clone it. A
resolver that models a fresh file open should provide a fresh cursor at zero;
a nonzero cursor is not silently repaired. Successful decoding or a failed DAT
parse may advance that supplied cursor, even when final FontResources output
is not published. This is consistent with the decoder's existing contract.
The callback's stream reference is released after decoding; an independent
caller reference can keep the stream alive.

The physical compatibility adapter still buffers all three resources through
the physical-to-memory helper and closes read-only handles through ownership.
Buffering a physical DAT is a host adapter, not evidence of native DAT
conversion. The stream entrypoint does not claim to accept arbitrary native
stream object ABIs or implement a FileStore decompressor by itself.

## Failure and validation boundaries

The factory requires empty output and rejects embedded-NUL logical inputs,
empty/embedded-NUL resolved physical paths, failed callbacks or physical reads,
null/missing stream backing, uninitialized tails, invalid DAT and failed image
HRESULTs. It also rejects successful HRESULT with
null texture. A failed HRESULT accompanying a nonnull texture is cleaned up,
unlike native's wrapper-publication branch. On any reported failure output
stays empty, and temporary glyph/image/source ownership is cleaned up. Native
does not have this transactional failure behavior and can leave partial state.
Resolver and allocation exceptions propagate under the same RAII cleanup.

Source ordering is backed by the native load/destruction evidence above.
Draw batching and full font reload remain separate from this initial-load
owner; installed shader drawing is covered by `FONT_MATERIAL_DRAW.md`.

The stream entrypoint is now integrated with mounted physical and FileStore
opening. The Win32 build and full installed-font D3D9 probe pass; no additional
test target was required.

The earlier physical-adapter integration passed the Win32 build, both existing CTests and full
D3D9 probe. Actual font metadata drives this factory; its three resolver calls
match GFX/alpha/DAT order, decoded height/count match, atlas recreation works,
and the installed bilinear shader draws a bounded glyph. Follow-up integration
resolves the white resource through recovered startup search lists and ordered
candidate passes, using recovered physical mount/factory policies and a supplied
root. The current mounted-stream report additionally records three FileStore
opens for GFX/alpha/DAT, zero physical opens during that font load, three cache
entries, retained atlas recreation, and 74 bounded lit glyph pixels. Cache
entries are explicitly primed by the diagnostic; native preload selection,
archive loading and full renderer cache lifecycle remain unported. See
[MOUNTED_RESOURCE_STREAMS.md](MOUNTED_RESOURCE_STREAMS.md),
[mounted_stream_font_probe.txt](../reports/mounted_stream_font_probe.txt),
`PROVIDER_FACTORY_STARTUP.md`, `VFS_MOUNT_LOOKUP.md`, `FONT_MATERIAL_DRAW.md`
and `reports/vfs_font_draw_probe.txt`.
