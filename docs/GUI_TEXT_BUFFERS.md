# Native Text draw sections and glyph buffers

Parent integration follow-up: this source and both shared owner modules now
compile in the normal Win32 target; both existing tests pass. The exact call
verifier passes 43 numeric rows (40 direct, three resolved renderer calls),
with 18 additional symbolic indirect rows explicitly outside that check.
The existing actual-owner detach probe passes against the rebuilt library.
No buffer creation execution or complete renderer/Text factory is claimed.

Addresses reconstructed: `00AB8400`, `00AB8530`. Names are descriptive hypotheses.
Ghidra project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe` was
verified by the read-only wrappers before each live query. No saved analysis,
game installation, renderer table or resource was modified.

| Routine | Original ABI and boundary | Coverage |
| --- | --- | --- |
| `create_gui_text_glyph_buffers_00ab8400` | Two stack DWORDs: capacity, actual mesh; ECX unused. Body `00AB8400..00AB852D`, final `RET 8` at `00AB852B`, length 3 | Complete supported normal path; external actual renderer factories and successful native allocations required |
| `ensure_gui_text_draw_sections_00ab8530` | ECX Text; no stack arguments. Body `00AB8530..00AB87CB`, final `RET` at `00AB87CB`, length 1 | Complete supported normal path; existing canonical Text/model/resource domains required |

These are semantic Win32 C++ entry points, not original ABI replacements. Allocation
failure, SEH unwinding, invalid resource identities and concurrent destruction are
outside the supported native domain. Companion allocation/registration failures use
the existing owner cleanup pattern; this is not a recovered native failure path.

`00AB8400` constructs the native 16-byte `simplecolor.mvfm` string, obtains the
declaration from the current renderer's virtual `38`, and releases the temporary
string before reloading the renderer for virtual `5C`. That call receives
`(4 * capacity, 1, declaration)`. The actual mesh stream-zero setter retains its
new stream and releases its old one. The helper releases the vertex creator,
then the declaration creator, before reloading the renderer for virtual `60`
with `(6 * capacity, 1, 65h)`. The actual mesh index setter precedes the index
creator release. Products retain native DWORD wrap; there is no reuse or clamp.

The original renderer table at `00D5F0A8` resolves slots `38/5C/60` to
`00B317E0/00B287C0/00B288B0`. Their bodies establish lowercase declaration-cache
lookup and CPU vertex/index allocation plus renderer registry publication.
Numeric original table entries are evidence, not callable host pointers. The
provided current renderer slot must reference actual callable native-ABI methods
that return registered native owners. Semantic stream wrappers and default
callbacks do not meet this contract. These factory/renderer implementations are
not newly reconstructed here.

`00AB8530` creates a shadow only when the live Text `+188` association is null.
It uses the existing `GuiWidgetOwnerRuntime` model pool, model record map and
`NativeModelReference` registration. It publishes the same actual model to the
live shadow slot before releasing the `Shadow` name, then reloads that slot and
clears actual node `+138` bits 0 and 1. A new canonical mesh is assigned with
`(0, mesh, sentinel, sentinel)`, reading `00D7A260` once at the native point.
The section is appended before creating its `guidefault.mshd` material. The
material assignment precedes creator releases in section, material, mesh order.
An existing shadow skips this entire creation/repair branch.

The current primary node is then read. A null primary returns immediately.
An existing primary geometry receives a default section only if its actual
`+58` count is zero; absent primary geometry is not created. Attachment still
runs when primary geometry is absent. The current `shadowed` byte selects the
current primary parent or null. After actual reparenting, the byte is read again;
when clear, the live shadow slot is reloaded for null-root propagation.

`GuiNativeGeometryOwners` now places materials beside its existing mesh and
section companions, using the same supplied registration/retained-owner domain.
It calls the existing `00535320` material factory and `NativeMaterialReference`,
retains no duplicate material state, and erases its companion only on the actual
terminal callback. Section destruction releases its retained material; material
destruction releases the effect through the supplied real material access.
The registration domain, material access/profile and pools must outlive these
objects. The shadow slot owns the model creator reference; its real Text lifetime
must release that same native model, not delete a separate wrapper.

The owner extension also supplies a default-empty derived secondary-release
hook after child `+20` releases and before primary unlink. This consumes the
separately owned `00AA8320` lifetime contract: child call `00AA8352`, live type
query `00AA8372`, conditional shadow call `00AA837B`, primary call `00AA8387`.
Text's implementation must supply its real descriptor predicate and `00AB73B0`;
this packet does not enable Text construction through the ordinary type factory.

Producer evidence uses existing types: native model construction establishes
the `184h` payload in the `188h` pool slot and retained mesh `+180`; mesh
construction establishes its `BCh` payload in the `C0h` slot, sections `+54/+58`,
index `+60`, streams `+64` and high-water count `+7C`; section construction
establishes its `60h` payload in the `64h` slot, material `+20` and real `+04`
atomic reference count. Text's constructor writes the existing `+188` and
`+15C` associations; the adapter takes the same `GuiTextWidget` and live shadow
reference used by content/style/lifetime, with no new Text state.

Every direct and indirect call is enumerated in `reports/gui_text_buffers.json`.
Caller assembly establishes the one buffer call at `00ABA9C3`: EAX is the copied
UTF16 length, EBX the main geometry selected at `00ABA96D`. The ensure calls at
`00ABA8EC`, `00AB98D8`, `00AB9D93` and `00ABB601` pass their actual Text in ECX,
with no stack arguments. `00ABA8EC` precedes copying/comparison, including the
equal/empty path. Full-function register filters establish EBX as zero for the
stream index, then the `00CE2220` import; EDI holds capacity across calls, and
EBP/ESI hold declaration/stream results. `memcpy` uses three arguments and
`ADD ESP, 0Ch` at `00AB844E`; renderer `38` uses `RET 4`, `5C/60` use `RET 0Ch`,
mesh stream/index setters use `RET 8/4`, model geometry uses `RET 10h`.

Validation: all three changed translation units compile under MSVC Win32 with
`/std:c++17 /W4 /WX /O2 /fp:strict`. The live exact-call verifier is recorded in
the report. No new tests or native differential fixtures were added. The current
ordinary factory explicitly rejects Text, so this helper is not on a reachable
`bsp_game.exe` path in this checkout; no runtime or visual result is claimed.
The integrator runs the standard combined Win32 build. Actual Text construction,
remaining virtual behavior, font/shader resource loading, glyph filling/upload,
draw submission and renderer bindings remain prerequisites for a rendered label.
