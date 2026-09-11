# Native frame-target group owner discovery

Read-only discovery, 2026-09-11. The concrete owner retained by renderer
`00B24E70` is constructed by `00B1FBB0`, with profile `00D5E600` and a
`40h` allocation. The actual final-zero path is **BD30E0 -> B1FCF0(flag 1)
-> B1FC00 -> BF65AC**, including the destructor's vector and exception
cleanup. `00B1F6B0 -> 00B1F330` belongs to the render-command queue and is
unrelated. This discovery implements no source and makes no build, native
fixture, ABI-compatibility, or game-validation claim.

The accompanying `reports/native_frame_target_owner_discovery.json` pins
complete live/installed-PE spans, including bytes omitted from current
Ghidra bodies. Every live read used `python tools/bsp.py ghidra ...`, which
verifies the configured saved `bsp` project and `/battlestationspacific.exe`
before querying. No Ghidra mutation or new export was made.

## Concrete construction and profile

The live constructor xrefs include `00B14D89`, `00B4E4D8`, `00B4E8A8`,
`00BA6E62`, and `00BB3B4A`. At `00B4E4A9`, a creator pushes `40h`, calls
`00BF681B` at `00B4E4C1`, then calls `00B1FBB0` at `00B4E4D8` and publishes
the result into its own `+08` field at `00B4E4ED`. `00B4CB90` returns that
field; callers use it with frame-target getters/setters. Separately,
`00B14D92` publishes a freshly constructed group at render-resource `+1D4`,
then `00B14DB6` sets color zero and `00B14DD2` sets depth from the renderer's
existing default-wrapper accessors.

`00B1FBB0` is ECX storage, no stack arguments, EAX original storage, plain
RET. It first writes base profile `00CEB130`, writes `+04 = 1`, and writes
concrete profile `00D5E600`. Its field stores establish this actual Win32
layout, rather than the earlier `std::shared_ptr` model:

| Offset | Size | Established storage and initialization |
| --- | --- | --- |
| `00` | 4 | Concrete profile `00D5E600` |
| `04` | 4 | Intrusive count, initialized to one |
| `08..14` | 4 x 4 | Color surface-owner pointers, initialized null |
| `18` | 4 | Depth surface-owner pointer, initialized null |
| `1C` | 4 | Raw vector data pointer, initialized null |
| `20` | 4 | Signed vector count, initialized zero |
| `24` | 4 | Signed vector capacity, initialized zero |
| `28..34` | 4 x 4 | Separately owned color COM interface pointers, initialized null |
| `38` | 4 | Separately owned depth COM interface pointer, initialized null |
| `3C` | 1 | Exact sRGB-write byte, initialized zero |
| `3D..3F` | 3 | Allocation padding; constructor leaves it untouched |

Only TWO entries belong to this profile: `D5E600+0 = BD30E0`,
`D5E600+4 = B1FCF0`. The adjacent DWORD `D5E608 = B237A0` belongs to a
different one-slot profile: disk scan found the currently undefined
constructor `00B21400`, whose complete nine bytes store `D5E608` into ECX
storage and return it. `D5E60C` is another independent profile written by
`B23750`/`B26130`. Neither adjacent destructor is a group prerequisite.
Live xrefs of `D5E600` identify only the group constructor and destructor;
this establishes the concrete profile, not a claim about all possible
foreign or derived objects supplied by arbitrary callers.

## Full destruction and exception behavior

`00BD30E0` does not decrement: if ECX is nonnull, it calls the CURRENT
virtual `+4` with flag one. `00B1FCF0` is ECX owner, stack flags, RET4,
EAX original address even when freed. It calls `B1FC00`, tests only bit zero,
and calls `BF65AC` only after normal destruction if that bit is set.

`00B1FC00` is ECX owner, no stack arguments, plain RET. Its full behavior is:

1. Install concrete profile `D5E600`, register EH state one.
2. For colors zero through three, capture the current intrusive owner.
   Decrement its actual `+04` using InterlockedDecrement. At zero call its
   current virtual zero; clear the group field only after the terminal
   returns. Then reload the corresponding COM field, invoke its stdcall
   virtual `+08` Release with the interface pointer, and clear that field
   after Release returns. These operations interleave per color.
3. Perform the same intrusive release/clear for depth `+18`, then COM
   Release/clear for depth `+38`.
4. Set EH state zero, call `B1F9F0(this+1C, 0)`, free the vector data through
   `BF6989`, set state minus one, and call complete base destructor
   `BD30F0` (store `CEB130`). The data/capacity fields are not cleared by
   vector destruction. Count becomes zero.
5. Restore the prior exception chain and preserved registers, then RET.

The current Ghidra body ends at `B1FCC9` because of a false no-return free
call. The real continuation `B1FCCA..B1FCEE` includes the base destructor
call at `B1FCD7` and the SEH epilogue. The report pins the full 239 bytes.
The scalar destructor also contains the real post-free stack adjustment;
its return value must not be taken from pseudocode `extraout_EAX`.

The native handler is `00CBCCE3`, loading FuncInfo
`00DF5214`. The two-entry unwind map at `00DF5204` is:

| Current state | Next state | Cleanup |
| --- | --- | --- |
| 1 | 0 | `CBCCD8`: ECX saved owner + `1C`, jump `B1FB90` |
| 0 | -1 | `CBCCD0`: ECX saved owner, jump `BD30F0` |

Thus a throwing intrusive terminal or COM Release performs vector
destruction followed by base-profile restoration. It does not synthesize
additional releases of remaining surface fields. If the normal vector
resize throws after state zero was set, only base cleanup applies. A
deleting-destructor failure does not reach object free. A host translation
must preserve these distinctions; it must not use a generic RAII container
that releases every remaining surface during unwinding.

## Smallest complete implementation packets

The following are proposed ownership assignments, not claims or metadata
edits made by this discovery. The primary agent coordinates sharded ledger
edits, refreshed exports, annotations, and CMake integration.

| Packet | Exact functions to claim | Proposed exclusive implementation files | Readiness |
| --- | --- | --- | --- |
| `native-frame-target-vector` | `B1F970`, `B1F9F0`, `B1FB90` | `include/bsp/native_frame_target_vector.hpp`, `src/native_frame_target_vector.cpp` | Ready from full spans; implement complete helper contracts |
| `native-frame-target-owner` | `B1FBB0`, `B1FC00`, `B1FCF0` | `include/bsp/native_frame_target_owner.hpp`, `src/native_frame_target_owner.cpp` | Ready once the vector packet is integrated, using the existing concrete surface and CRT providers |
| Optional assignment extension of the owner packet | `B1FAB0`, `B1FB00`, `B1F700` | The same owner files; same worker ownership | Ready and useful for constructing nonnull groups; not required by the destructor itself |

The primary approved the independent vector worker's concrete API during
finalization: `NativeFrameTargetVectorRow { uint32_t words[4]; }` and
`NativeFrameTargetVectorStorage { Row* data_00; int32_t count_04;
int32_t capacity_08; }`. The functions are
`reserve_native_frame_target_vector_00b1f970(Storage&, int32_t)`,
`resize_native_frame_target_vector_00b1f9f0(Storage&, int32_t)`, and
`destroy_native_frame_target_vector_00b1fb90(Storage&)`. They require no
extra context and use the actual shared CRT service. This is an approved
integration contract, not an assertion that the worker implementation has
already passed review or merged.

For the helper packet, preserve the raw 12-byte header and 16-byte records;
record contents have no recovered semantic type. `B1F970` takes ECX header,
signed stack capacity, RET4. Clamp requested capacity to at least one,
return if current signed capacity suffices, allocate requested count shifted
left four with native 32-bit arithmetic, copy each existing record as four
forward DWORDs, free the current old data, publish new data and capacity,
and preserve count. The current pseudocode omits that final publication:
the missing `B1F9E1..B1F9EA` tail is essential. Do not replace overflow or
signed comparisons with new validation policy.

`B1F9F0` is ECX header, signed stack count, RET4. It calls the full reserve
helper when requested count exceeds capacity, initializes newly added rows
to four zero DWORDs, shrinks count through the native decrement loop, and
stores requested count. `B1FB90` is ECX header, RET; call resize zero and
free current data without clearing the pointer/capacity. Its real epilogue
`B1FBA2..B1FBA6` is absent from the stored body. Implementing only resize-zero
would leave the named general helper incomplete. `B1FA50` append is not
reached by the owner lifetime; it is a separate optional routine and does
not block these packets.

The optional setters are complete small bodies: `B1FAB0` takes stack
slot/pointer, RET8; `B1FB00` takes stack pointer, RET4. On identity they do
nothing; otherwise publish incoming first, increment incoming `+04`, then
decrement captured outgoing and dispatch its current virtual zero at zero.
Color indexing has no bounds check. `B1F700` takes one stack word, RET4,
stores its low byte into `+3C`, and does not normalize it to Boolean.

## Existing and missing contracts

The existing `NativeSurfaceOwnerStorage` and `NativeSurfaceOwnerContext`
from `native_surface_owner.hpp` provide the concrete `D619A0` surface path.
Its first two live profile entries are `BD30E0`, `B3F5B0`, and the existing
`delete_native_surface_00b3f5b0` composes actual renderer unregister, support
singleton, COM/name release, and canonical pool return. Group destruction
must use that complete provider with its actual context; a borrowed surface
pointer or `shared_ptr<D3D9SurfaceBinding>` alone does not close final-zero
ownership. Support for a different runtime surface profile is unproven and
must remain an explicit unsupported input until its terminal is established.

Use the existing `singleton_lifetime_allocate` / `singleton_lifetime_free`
CRT service boundary with native sizes, not a fabricated callback that
pretends an unknown allocator succeeded. `BF55BE` jumps to operator-new
body `BF681B`, which retries malloc via the new-handler and throws on
exhaustion; `BF6989` forwards to `BF65AC`, then the CRT free body. The
surface pool is unrelated to allocation of the `40h` group. `BD30F0` and
`BD30E0` are fully established tiny body contracts; their concrete expansion
here need not claim reconstruction of every polymorphic caller.

At this discovery snapshot, `B1F970`, `B1F9F0`, `B1FB90`, `B1FBB0`,
`B1FC00`, `B1FCF0`, and the optional setters have no integrated full source.
The getter leaves `B1F6D0`, `B1F6E0`, `B1F710` and cached-state setter
`B24460` are owned by the primary agent. The primary reported getter
closure in `4bc114a` after this worktree's pinned `1085a30` snapshot and
assigned the vector packet to a separate worker; this discovery did not
revalidate those later changes. Existing full bindings `B23D80`
and `B21690` can consume the group's actual wrapper fields. `B24E70`
remains a typed fragment at this snapshot and still needs its own complete
native retained-owner/guard binding after the owner packet closes.

Future validation should use the required strict Win32 build and existing
relevant checks. A focused original fixture can exercise nonnull surfaces,
final-zero deletion, nonempty vector free, flag-zero destruction, and one
throwing terminal if a concrete regression risk warrants it. This discovery
added no tests and executed no game or fixture.
