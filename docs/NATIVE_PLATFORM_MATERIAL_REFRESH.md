# Concrete renderer material refresh

The B24DD0 material scan now has a concrete overload that invokes the existing
complete B19000 source provider. `GameNativeRendererApplication` supplies a
ready-only borrowed context over its own texture cache, canonical actual-owner
registry, current F8D434 Lua-owner publication and mapped D5F0A8 profile. The
caller supplies the current F8D438 mutable C-string and CE3A0C empty literal.
Borrowing creates no native owner and executes no refresh.

This closes the material child in the bounded activation stream. It does not
bind B24FB0's effect/texture virtual+08 reloads, schedule WM_ACTIVATE, admit the
full BED3B0 message handler, or replace the current lost-device hold policy.

## Caller and operation contract

The callback overload remains available. Both overloads use the same B24DD0
loop: capture the original renderer+1A9C cursor, read each row's current +28
material, then reload renderer+1AA0 count before +1A9C data after the child.
The cursor advances by 2Ch from its original allocation. No snapshot of the
initial count, rescan, row retention or publication substitution is introduced.
The captured receiver remains distinct from the cache's current F8D394 cell.

`NativeRendererMaterialTextureRefreshOperation` owns separate address-stable
host child frames. Each records the captured material and its full
`NativeMaterialTextureRefreshOperation`. Frames are published before entering
B19000 and retained on failure. They hold diagnostics and reached native
temporaries, not additional material/texture references. A completed or failed
operation cannot replay. Running/failed frames cannot be discarded until their
native state is reconciled. No new FH3 unwind or rollback is invented.

The source interface is not the original binary ABI. Original B24DD0 takes
ECX renderer, returns with RET0, and preserves ESI/EDI. Original B19000 takes
ECX C4h/derived material, returns with RET0, and preserves EBP/EBX/ESI/EDI.
The existing B19000 domain and exception limits remain documented in
`NATIVE_MATERIAL_TEXTURE_REFRESH_CC10.md`.

## Whole-body native/source fixture

An isolated forwarding-main probe links 137 current production application
objects and the current core/Lua/zlib libraries. It uses the ordinary canonical
data handoff, actual singleton/VFS/string/Lua/renderer startup and native drain.
Root's GUI/compiler fixtures and production startup objects are unchanged.
The private XLive stub is an explicit startup input, not online validation.

| Native body | Complete span | Verification |
|---|---|---|
| B24DD0 | B24DD0..B24E11, 65 bytes | Installed PE equals live saved-program bytes |
| B19000 | B19000..B191CE, 462 bytes | Installed PE equals live saved-program bytes |

The original caller executes the complete copied original child. Named direct
calls are relocated to the actual source Lua/string/cache dependencies; native
global operands borrow the same current application publication cells and
Interlocked imports perform the actual increments/decrements. The fixture
temporarily relocates verified D5F0A8+64 from B319B0 to its concrete cache
adapter, preserving the original indirect call instructions. It restores the
numeric word and read-only page protection before source B19000 executes.
No branch is skipped and no resource result is fabricated. Original FH3
handlers and zero-reference virtual terminals are not entered by this comparison.

Two actual C4h materials come from source B18D60 and the real fallback cache.
Their names are written through B18FB0. B30410 constructs two real 2Ch records
in the actual renderer's initially empty effect array; the fixture supplies
their +28 material identities. This establishes this material-scan domain,
not a complete effect-file producer or D61A00 reload admission.

The probe cold-loads installed `black.tga` and `white.tga` through actual
B319B0/B30B40/B2C2D0, VFS, D3DX and canonical registration. Their real COM
descriptors are 32x32 and 128x128 DXT1, with retained source owners. The same
application Lua state maps one isolated global key to `BLACK.TGA`.

Both original and source then refresh the same material preimages:

* Lua override and lowercase/cache lookup replace white by black.
* A missing override falls back to `white.tga` and replaces black by white.
* An empty selected name preserves an existing white slot and count.
* A second material's black identity consumes only the returned acquisition.
* A wrapper delegates every message/cursor action to the real platform event
  host, then expands the actual effect count from one to two on the first load.
  Both callers process the second row and make exactly three real cache calls.

All 392 material bytes, black/white actual +04 counts, row identities and Lua
stack height agree. The source retains two separate completed child frames and
rejects replay without altering completed state. Original temporary headers use
the same real pool. The native terminal arm is avoided during comparison by
holding the genuine creator references; no terminal success callback is used.

Cleanup runs actual record shrink, material fallback/texture/name destruction,
and final black/white creator release. Both canonical companions are absent
after their real terminal callbacks. The original platform-event provider and
Lua key are restored. The two-frame process exits zero; native renderer/Lua/
definitions/system/cache/queue publications drain to null, the worker joins,
and final D3D device/API Release results are both zero.

The fixture's nil fallback is explicit mutable zero storage. It does not prove
the production window graph's F8D438 binding. Cache misses occur in preload;
the compared refresh calls are real hot-cache hits. No device loss, reentry,
registry relocation, malformed counts, provider failure/FH3, complete resource
reload, visual equivalence or gameplay parity is established.

## Reproduction and retained evidence

`local/platform_material_refresh/prepare.py` checks the native bytes and
generates the forwarding main and exact link inputs. `build_probe.cmd` uses
MSVC Win32 `/O2 /MD /EHsc /W4 /WX /fp:strict`, with `/MANIFEST:EMBED`.
`run_probe.py` supplies isolated settings and disables the separate font/GUI
probes. The initial late numeric-trampoline reservation failure is preserved;
it occurred before original-body execution and is not positive evidence.

The normal `scripts/build.ps1` and all three existing checks passed. No new
repository test case or CMake entry was added. The report records exact runtime
arguments, hashes, operation observations and limits. The final checkpoint
preserves the linked objects/libraries, map, fixture recipe, native bytes,
selected source/include closure, build logs and report; the manifest verifies
copies against their source hashes.
