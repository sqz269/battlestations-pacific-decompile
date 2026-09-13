# Actual renderer cache clear and third-state binding

The complete normal `B241C0` body now operates on the actual cache at
**renderer+34**, including all 26 current intrusive owner cells. It composes
the existing raw destruction implementations. The independent 130-byte raw
`B27B00` third-state binder is also complete. Names are descriptive hypotheses.

| Routine | Inclusive native span | Original ABI | Coverage |
| --- | --- | --- | --- |
| `B241C0` | `00B241C0..00B24457`, 664 bytes | ECX cache; no stack arguments; RET; no semantic result | complete normal body, with propagation from reached providers |
| `B27B00` | `00B27B00..00B27B81`, 130 bytes | ECX renderer; stack incoming owner; RET4; no semantic result | complete raw body; `no_ghidra_function` at worker capture |

These are new MSVC Win32 C++ interfaces. The binder adds its fixed context in
EDX and saves one extra stack cell; it is not the original caller ABI. Neither
interface establishes original SEH compatibility, device recreation or gameplay.

## Actual storage and release order

Every offset in this table is relative to cache `renderer+34`.

| Order | Accessed storage | Action |
| --- | --- | --- |
| 1 | owners `+00`, `+04`, `+08` | Release each captured owner, then clear that current cell on return |
| 2 | `+0C..+DD` | `memset(0, D2h)` render-valid bytes |
| 3 | `+1738`, `+173C` | Ordered DWORD zero writes |
| 4 | owners `+1780`, `+1784` | Independent current captures, release then clear |
| 5 | `+1788` | Write `FFFFFFFF` |
| 6 | four records `+1740 + i*10` | Release/clear owner first, then zero DWORDs `+4,+8,+C` |
| 7 | sixteen banks with owner `+4D0 + i*AC` | Zero eight DWORDs at owner-`A8` through owner-`8C`, then byte owner-`88`; only then capture/release/clear owner |
| 8 | twenty banks `+1198 + i*48` | Zero three DWORDs and one WORD (14 bytes per bank) |
| 9 | owner `+18D4` | Current capture, release then clear |
| 10 | 24 DWORD float-bit cells `+18D8..+1934` | Write zero in original order: `18D8,18E8,18DC,18E0,18E4`, then ascending `18EC..1934` |

The final four texture owners, gamma at `+1938`, planes, cached values and all
sparse gaps outside these writes survive. The old private all-null construction
fragment is not used. Null owner cells receive no owner-cell write.

For each nonnull captured identity, the real Win32 `InterlockedDecrement`
targets that allocation's same LONG at `+04`. Only a zero result reads the
current profile and slot0. The full reached `BD30E0` action rereads the owner's
current profile, reads slot4 and supplies deleting flag1. The captured identity
survives callbacks; the next cell is not captured until the previous terminal
returns. The current cell is cleared after return even if the callback changed
its contents. A throw prevents that clear and all later parent work. There is
no added cleanup, rollback, guard, terminal `noexcept` boundary or owner registry.

Assembly resolves the pseudocode's reused registers: EBP initially captures
the decrement import for the first five releases, becomes4 for the stream
loop, then16 for the texture loop, and reaches0 before `LEA EDX,[EBP+14]`.
Thus the final validity loop is20, not a value inferred from the decompiler's
reused local. EBX is zero for the first five clears, then captures stream and
texture identities. EDI remains the cache throughout; ESI is the current
owner or bank cursor. The real immutable system imports are the source domain;
runtime IAT rewriting is not part of it.

## Concrete current-profile composition

All table views contain original numeric tokens and borrow the existing
application's storage. They are not host function pointers or injected
destructor callbacks. Every listed profile has slot0 `BD30E0`; slot4 selects
the full existing raw provider below. Current selection is independent of the
cell position, as in the native type-erased release. Unsupported profiles are
outside this interface's input domain, with no fallback.

| Profile | Established usual cache cell | Current slot4 / complete raw provider |
| --- | --- | --- |
| `D61A2C` | `+00` render states | `B422F0`, `native_material_pass_states` |
| `D61A3C` | `+04`, conditional third-state application domain | `B42330`, same owner module |
| `D61A34` | `+08` sampler states | `B42310`, same owner module |
| `D62AF4` | `+1780` hardware layout | `B60770`, `native_hardware_layout_owner` |
| `D61DE0` | `+1784` logical index | `B4C1F0`, `native_logical_index_owner` |
| `D61D6C` | four stream cells | `B4BF10`, `native_logical_vertex_owner` |
| `D61948` | texture cells | `B3F590`, `native_texture_2d_owner` |
| `D61870` | texture cells | `B3F410`, `native_cube_texture_owner` |
| `D618B0` | texture cells | `B3F430`, `native_volume_texture_owner` |
| `D5E600` | `+18D4` frame targets | `B1FCF0`, `native_frame_target_owner` |

The borrowed layout/index/vertex/texture contexts reuse their current binding
and lifetime contracts. They must share the application's actual pools,
renderer publication, synchronization, strings and other service domains.
Only a reached profile/provider is accessed. This packet supplies no pool,
replacement cache, companion reference count or recreation implementation.

Logical vertex deletion calls the complete raw `B4BF10/B4B5D0/B62010` route,
including its existing canonical nested `actual_owners` domain at `+68` and
independent retained `+4C`. That existing terminal contract is nonthrowing;
lookup exceptions and the other raw lifetime/provider exceptions keep their
existing behavior. This packet neither inserts a new companion boundary nor
widens the nested provider contract. A genuine nonnull `+4C` identity and its
current terminal are application preconditions: its runtime producer/type
remain unproved. Full top-level logical, layout, index, 2D and volume final-zero
composition is source-reviewed/build-tested, not independently native-tested
by this packet's fixture.

## Third-state producer and binder boundary

Live references to `D61A3C` identify two positive producers:

* `B5F7F6`, within `B5F720`: write the profile after base/count initialization;
  zero the three row-header words and publish the owner at pass+`1C` at `B5F812`.
  Whole-function register inspection establishes EBX0 and EDI1 at the writes.
* `B456F7`, within `B455C0`: produce the same profile/header and publish at
  destination pass+`1C` at `B4570A`, then copy the source count and third rows.

This proves the actual 14h third-state owner and pass-field production. It does
**not** prove a caller transporting it into renderer+38. At capture, Ghidra
has no function or xrefs at `B27B00`, and an installed-image DWORD scan for
its address found no matches. No exhaustive reachability conclusion follows.

The independently decoded `B27B00` reads/writes renderer+38. Its actual incoming
owner has LONG+04, current row pointer+08 and signed count+0C. On an outer
identity hit it returns without context access. Otherwise it recaptures old,
publishes incoming, retains incoming and releases the captured old at final
zero. It keeps the original incoming identity for the row loop despite
callbacks. Each iteration reloads the row base, reads value/state/stage from
offsets8/4/0 and calls full `B24510(stage,state,value)` at `B27B64`.
`B245FE: RET Ch` consumes the three arguments; `B27B7F: RET4` consumes the
incoming owner. EDI is the signed row index and EBX the wrapped12-byte offset;
both start0 and increment independently. The current count is reread after
each call. Changed null input skips rows but still increments current `+1B98`.
Throws prevent the counter increment; there is no outer guard or cleanup.

The source name remains “third states.” Stage/state/value is established for
the binder's input rows; joining that interpretation to every `D61A3C` runtime
instance remains conditional on the missing caller/provenance link.

## Evidence and verification

The report records full native spans, ABI, source pins, every parent call site,
producer evidence and the remaining analysis limitation. Every Ghidra read
verified `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe` through
the existing bridge. The worker made no Ghidra writes, annotations or saves.
Fourteen specified live/installed-PE spans, **1,225 bytes**, match, including
both full reconstructed bodies, full `BD30E0`, full consumed `B24510`, all ten
profile pairs and the two bounded third-owner production/publication slices.
`verify_report_calls.py` checked15 numeric call rows: the only failure is
`B27B64 -> B24510`, because its containing raw body has no Ghidra function.
The eight numeric slot0 rows remain indirect/static-profile checks. Root must
define the full independently decoded `B27B00` body under its own lease/write
lock and rerun the verifier before accepting integration.

Strict owned-source compilation passed `/MD /W4 /WX /O2 /fp:strict`. After
`verify-seeds`, `scripts/build.ps1` passed both existing CTests. Shared CMake
was not edited: the standalone strict compile/probe compiles the new source;
the normal build tests the current existing library until root integration.

The ignored fixture is preserved at `C:/Users/sqz269/bsp-av-cache-clear/`.
`run.ps1 -Root <repo>` compiles `probe.cpp` plus the owned implementation and
links the current three libraries. After integration, `-CurrentLibrary` compiles
**only `probe.cpp`** and links `bsp_core`, `bsp_lua511`, `bsp_zlib121`; it does
not include reconstructed `.cpp` or `.obj` files. `capture.py`, exact original
byte/header inputs, evidence, source/header pins, map, binary and logs accompany
the fixture. Its fixed original pages can collide with the randomized process
stack; only that pre-execution exit77 is retried, at most eight times.

Seven original-parent/source comparisons passed **54,704 postimage bytes**:
all26 nonnull retained cells; nonempty render/third/sampler deletion; sixteen
actual cube-owner terminal paths and frame-target terminal/COM release; a cube
COM callback replacing the next owner with an alias and changing the current
cell; a throwing cube release stopping later writes; and third-state COM
callbacks changing row pointer/count or throwing before counter publication.
The fixture also checks identity/null bypass and unchanged loaded parent,
invoker and profile bytes. Intentionally displaced/failing fixture objects
survive until subprocess exit; the fixture introduces no production cleanup.

It executes unmodified `B241C0`, `B27B00` and `BD30E0` bytes. Explicit jump-only
bridges at material/cube/frame deleting entries and `B24510` call the current
complete production dependencies, while the decrement/increment IAT binds
the real Win32 imports and `memset` binds the real CRT. Thus it independently
tests parent scheduling across real source owners, not the original bytes of
every nested terminal. COM observation objects are fixture providers, not a
GPU/device or original-game runtime. Full renderer reset/recreation, original
caller ABI, exhaustive failure domains and gameplay remain separate work.
