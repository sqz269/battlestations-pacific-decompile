# Native mesh texture field and unchecked assignment

Addresses: `00B189F0`, `00B93D30`; compiler support `00CC2E10`, `00CC2E18`, `00CC2E20`.

Two complete ordinary bodies (374 bytes) are reconstructed in
`src/native_mesh_texture_field.cpp` and registered in the tracked Win32 build.
The reader composes the existing actual texture cache, string pool, structured
reader and canonical texture owner interfaces. It does not complete the enclosing
material/subset or aggregate mesh parser.

Evidence: `reports/native_mesh_texture_field_cj.json`, its flow/annotation/integration
reports, and frozen local artifacts under `local/native_mesh_texture_field_cj/registered/`.
Names remain descriptive hypotheses. Existing Ghidra names and earlier comments
were preserved; the two ABI views and address ledger now point to these complete
ordinary source entries. Earlier checked GUI convenience interfaces remain available.

## Native behavior

| Address | Bytes | Original ABI | Behavior |
|---|---:|---|---|
| B189F0 | 80 | ECX material; stacked unsigned slot/texture; RET8 | Unchecked texture assignment with native high-water and reference ordering |
| B93D30 | 294 | ECX unused; stacked material/parent handle; RET8 | Counted name, native texture cache lookup, slot assignment and texture child fields |

`B189F0` sign-extends the material's SHORT at `+34`, then compares that result as
unsigned against the requested slot. It writes the low 16 bits of `slot+1` before
testing pointer identity. The slot address is `material+10+4*slot`, with 32-bit
wrapping arithmetic and no bounds check. A changed slot is published and the new
texture retained before releasing the captured old texture through its current
canonical zero-reference terminal. A negative SHORT therefore does not imply an
empty array. The source requires readable/writable native storage at the wrapped
address; it does not make invalid native addresses safe.

`B93D30` reads a local eight-byte name header and ignores `BEA010`'s returned
header. It captures the current renderer's `+64` entry before arming name cleanup,
then calls actual `B319B0(name,0)`. Numeric renderer profile `D5F0A8` selects that
existing source implementation; no native table is replaced with host callbacks.
It reads the slot DWORD and calls the unchecked assignment above.

Each completed child is named from its current `+14` field. A case-insensitive
`TextureAddress` match consumes three control DWORDs without storing them. Other
children are skipped/detached. Normal child release is disarmed before `BE9ED0`
is called, so a throwing release is not retried. After the loop, the temporary
texture is unconditionally released, then the captured local name buffer is
returned using its current length and current raw string-pool publication.

The two-state unwind map at `DFC5C0` and FuncInfo at `DFC5D0` establish:

| State | Next | Action | Owned object |
|---:|---:|---|---|
| 0 | -1 | CC2E10 -> 41DD20 | Completed local name at EBP-14 |
| 1 | 0 | CC2E18 -> BE9ED0 | Completed child handle at EBP+8 |

Neither action releases the texture temporary or rolls back material publication.
The acquired frame exposes those outstanding effects on failure. Its embedded
cache frame is per invocation; failed cache/provider continuations must remain
alive until their own obligations are resolved. Exceptions during unwind cleanup
terminate, matching the source interface's established exception boundary.

The read context and cache must borrow the same actual raw string-pool publication,
return gate and lifetime. Cache and material assignment must use the same canonical
texture owner domain. The fixture's texture notification and surface contexts also
alias the same renderer publication cell.

## Ghidra and byte evidence

The audit compared all 400 code bytes and the unwind, renderer-slot and child-name
data against the unchanged installed executable and saved Ghidra program. All 128
instruction addresses have the expected function owner; all 17 direct transfer
rows pass `verify_report_calls.py`. Six indirect transfers are recorded separately.
The missing ten-byte `CC2E20` handler was disassembled and created under the write
lock, without clearing bytes, changing no-return flags or repairing ordinary bodies.
The existing `Unwind@00cc2e10` and `Unwind@00cc2e18` names were retained. Five comments
were appended with previous values recorded, the project saved, and exports refreshed.
The restarted Ghidra instance retained the repaired handler.

Original executable SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Validation

The default tracked MSVC Win32 build passed both existing CTests. There is no cached
`CMAKE_PROJECT_INCLUDE_BEFORE` override. This was an incremental rebuild, not a new
empty build directory. `cmake/startup.cmake` was held only during the completed edit
and released before building; it is unleased at sealing. Future shared-registry
appends follow `docs/COORDINATION.md` without a whole-file lease.

A focused `/MD /O2 /fp:strict /W4 /WX` probe with an embedded manifest ran in a
controlled child. The copied original 80-byte setter used two relocated imports
for the real Windows Interlocked functions. Four source/original pairs checked
identity with count update, changed assignment with a zero-reference old owner,
negative SHORT count and a `40000000` slot index wrapping to slot zero. Both sides
used the same terminal observation boundary to verify publication and new retain
before the old terminal. These checks do not execute the original parent parser.

The source parent ran with an actual NVIDIA RTX 5090 D3D9 HAL texture, existing raw
string pool, retained-memory reader, real alias record and actual cache/owner/pool
implementations. The normal case consumed 12 reads, including mixed-case
`TextureAddress` and an unknown child, left material slot zero at texture refcount
two, exhausted the parent budget and retained the root at reader depth one. An
injected failure on the second address DWORD released the completed child and
name while preserving material publication and the temporary texture obligation
(refcount three). Only subsequent explicit fixture cleanup retired those effects.

Final canonical release ran `B3F590/B3F2E0`, actual `B32250/B31DC0` cache removal,
and texture pool return. One companion was bound and retired, cache count/accounting
and allocator list were empty, and D3D9 device/API final COM counts were zero.
An initial fixture assertion expecting root depth zero was corrected to depth one;
the log is retained. Fixture header/macro/qualification corrections did not change
production code. The final fixture was compiled and run after those corrections.

Only hot cache admission is tested. The material prefix, renderer/cache preimage
and platform observation callback are fixture boundaries. Cold D3DX loading,
resolver failures, fresh-acquire ownership, retry/device loss, surface creation,
complete renderer startup and XLive are not covered. No original FH3/SEH, binary
ABI compatibility or gameplay validation is claimed.

## Follow-up packets

- Recover `B451D0` and the selected instance-generator/binding constructors before
  accepting `B85610` section finalization. It is an actual effect/section/index-owner
  composition, not a no-op callback.
- Compose numeric-profile `535320` material admission with the effect cache and
  its cold `B18D60` texture acquisition. The current constructors still require
  callable renderer slots and cannot be supplied fabricated tables.
- Audit `B941D0` assembly and exception tail, then implement it with the above
  completed dependencies and canonical material registration.
- Continue `B944E0`, `B94710` and registered aggregate parser wrappers after the
  subset contract is ready. Keep these boundaries distinct from this texture field.

Publication is the `agent/orch4-20260910` branch only. Main was not integrated;
the later main deltas have not been reviewed by this worktree.
