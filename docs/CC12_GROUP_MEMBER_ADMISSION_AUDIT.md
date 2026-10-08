# Unit-group member admission: complete caller audit

`0070EF30` (`BSP_UnitGroup_AddMember`, a descriptive hypothesis) is completely
read, but is **not ready as a whole native Source provider**. It publishes a
member identity and an observer relation; it does not produce the member's
callable type profile, class pointer, linked-unit pointer, or speed tuning.
No Source, native fixture, or gameplay execution was added or performed.

Evidence is pinned to worker commit `d02cf80822c487c2593696d6e1e0cf1b221c56a5`.
The companion [report](../reports/cc12_group_member_admission_audit.json) retains
the complete bytes, installed-PE hashes, five direct call rows, Source hashes,
and verification limits. Ghidra was the existing `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`; all queries were read-only.

| Routine | Coverage | Evidence |
| --- | --- | --- |
| `0070EF30..0070EFC0` inclusive | Complete | 145 bytes, 47 instructions, no listing gaps, two `RET 4` exits |
| Other native bodies | Not expanded in this packet | Existing ledger/Source contracts only |

The entire caller matched installed PE bytes at file offset `30EF30`, SHA-256
`7b49a52951ba9afae679364e494cd3bf2435fb2eb05b0cabc86dfe03521f10c6`.
The PE is SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Capstone independently decoded all 145 bytes and all five `E8` destinations.

## Entry, publication and control flow

Native entry has `ECX = actual group`, `[entry ESP+4] = actual entity`.
`PUSH ESI; PUSH EDI` saves both; `MOV EDI,[ESP+C]` captures the entity and
`MOV ESI,ECX` captures the group. Both exits restore them and execute `RET 4`
at `0070EFA3` or `0070EFBE`. EBX/EBP are not directly changed. EAX has no
established result contract: it is exposed from the last callee, not a member
index or a synthetic duplicate marker. ECX, EDX, XMM registers and flags must
retain the native callee/volatile behavior; no full preservation is promised.

1. `0070EF38` unconditionally writes `DWORD[entity+284] = group`, before any
   validation or duplicate test. No previous group is detached or compared.
2. `0070EF3E` captures signed `group+4F8` in EDX. For positive count, the loop
   compares exact pointer DWORDs at `group+18+i*34` against EDI. It has no class
   test, dereference of a record's entity, null filter, or capacity guard.
3. A duplicate branches to `0070EFA6`. The query at `0070EFAA` receives
   `ECX=entity, EDX=group`; only AL is tested. Nonzero AL returns through
   `0070EFA3`; zero AL invokes registration at `0070EFB7`, then returns through
   `0070EFBE`. The duplicate path does not initialize record speed, produce
   slots, increment count, or refresh the ceiling.
4. The new-member path reads actual constant `00CF4888` **at `0070EF5E`, after
   the duplicate scan**. MOVSS copies bits, with no numeric conversion. The
   installed cell is `4479C000` (999.0f); this is borrowed native data, not a
   substitute default. `0070EF69` stores EDI into the record indexed by the
   initially captured EDX count. `0070EF6D` reloads count for the speed write at
   `0070EF76`, to `group+48+count*34` (record+30).
5. `0070EF7C` reloads count again, pushes it, and calls `0070ED30` at
   `0070EF85` with ECX=group. This is the join index before increment. The
   existing callee ABI contract consumes the DWORD with `RET 4`; this caller
   has no `ADD ESP` for that argument. After its return, `0070EF8A` performs
   `ADD DWORD[group+4F8],1`, using the then-current cell, modulo 32 bits.
6. `0070EF95` registers the pair with ECX=entity and EDX=group. Then `0070EF9C`
   calls `0070DA00` with ECX=group. The append path does not query for an
   existing observer pair before registering.

The caller contains no x87 instruction. Its MOVSS initialization must not be
misreported as an x87 speed reduction. The called slot producer and ceiling
refresh retain their own FP-state requirements; this audit establishes no
new composite ambient-x87 admission. There is no caller-local lock, exception
handler, rollback, null fallback or bounds check. Earlier publication/stores
remain if a later dependency fails; no recovery behavior is inferred.

An ordinary bounded storage domain is the same writable 508h group, a writable
entity+284 cell, valid member records, count 0..23 when appending, or a valid
existing-member index within count up to 24. These are requirements for a
future admitted provider, not native guards. Negative/oversized counts retain
their literal native address arithmetic outside that domain. Lifecycle,
aliasing/concurrency, profile callability and group callback ownership remain
separate requirements, not consequences of a successful raw store.

## Direct dependencies and ownership

| Call site | Native target | Exact receiver/argument relation | Current contract boundary |
| --- | --- | --- | --- |
| `0070EF85` | `0070ED30` | ECX=group; stacked current join index | Semantic slot producer exists; actual whole producer unbound |
| `0070EF95` | `00694A60` | ECX=actual entity; EDX=actual group | Existing concrete observer registration; borrowed actual endpoint/lifetime services required |
| `0070EF9C` | `0070DA00` | ECX=actual group; no stacked input | Previous whole-body audit remains unready |
| `0070EFAA` | `00694AF0` | ECX=actual entity; EDX=actual group; consume AL | Named locked query; no complete current Source wrapper found; native body not expanded |
| `0070EFB7` | `00694A60` | Same actual endpoint identities | Registration occurs only after the duplicate query returns AL=0 |

All five sites are inside the live `0070EF30..0070EFC0` body, independently of
the call graph. Existing observer Source names its first endpoint and callback
owner in this order. It preserves actual arrays, edge identity and reference
counts under its existing lock/lifetime contract. That does not create the
entity/group instances, validate their profile stamps, or bind callback virtual
dispatch. The constructor's raw `CFD6F8` group stamp remains uncallable data.

There are **zero direct kind-query call sites** in this caller. The prior audit
of `0070DA00` identifies its own indirect call at `0070DA33`, on each actual
member through `[profile+5C]`, stacked kind 6. That site belongs to `0070DA00`,
not this body. Its true arm reads `unit+538 -> class+500`; its false arm reads
`unit+3D0 -> linked unit+538 -> class+188`. No instruction in this caller writes
any of those fields or constructs/admits the profile. Indirect callee effects
were not newly expanded or assumed.

## Current Source, including the completed reducer

`ship_ai_formation_add_member` (`src/ship_ai_formation.cpp:337`) is existing
semantic sequencing over separate member/count inputs and virtual host calls.
It returns index/-1, fixes the 999 sentinel in Source, and assigns `count=index+1`
after the host slot call. The native body instead borrows the constant and adds
to the freshly reached count cell. This Source is useful semantic work, not
proof of the actual caller's ABI, observer/class admission or mutation ordering
under changing/aliased state. No edit to that existing API is proposed here.

The following complete providers exist and must not be described as missing:

- `0070DAB0`: actual 508h constructor, with explicit uncallable profile and
  unbound allocation/publication/lifetime. It preserves group+504.
- `0070D140`: complete raw member-record speed reducer in
  `native_unit_group_member_speed_reducer.cpp`, integrated in CMake. It reduces
  supplied record+30 values and returns ST0; it does not write group+504 or
  dereference member classes. It is not called by `0070EF30` or `0070DA00`.
- Existing direct group views and raw ship/plane/root-kind leaves keep their
  literal memory admissions. Direct callable leaves do not bind a live
  `[profile+5C]` dispatch on an admitted group member.
- `00694A60`: concrete actual-storage observer registration already exists;
  its complete lifetime dependencies remain explicit in its own contract.

## Follow-up decision

`0070ED30` is the next bounded slot-producer **audit candidate**, not an
independently ready implementation. Its existing
`ship_ai_formation_produce_member_slots` (`src/ship_ai_formation.cpp:241`) handles
the null-member exit, relative-position/clamp sequence, wake column and table
columns through `ShipAiFormationSlotHost`. The original body was not expanded
in this packet. Existing qualified providers are already available for
`004142E0` (raw point transform), `00414DB0` (actual pose refresh), `0042B260`
(normalization) and `00B63D50` (typed inverse with documented alias limits).
Their presence is not a composite object/lifetime binding.

Two concrete missing native contracts prevent calling that follow-up ready:

- `00424C40`: the actual 76Ch gameplay-settings singleton and its publication,
  constructor/loading and lifetime. `GameplayTuningSettings` explicitly is an
  own-instance projection; its loader/default helpers are not this singleton.
- `00811180`: complete native wake decomposition. The current header explicitly
  leaves the x87 sign expression untranscribed, chooses a round-trip convention,
  and documents an additional winning-segment-line versus interpolated-point
  divergence. Source also has a host-only `trail.written==0` diagnostic return.
  Neither the sign nor the actual ring/owner contract is proved by that Source.

Separately, actual kind-profile publication and unit/class tuning ownership
remain necessary before `0070DA00` or standalone `0070D1B0` can be made ready.
`00694AF0`, detach `0070E4C0`, creator `0070DB20`, and sole observed caller
`0077F940` (call site `0077FA88`) remain named and unexpanded here. No smallest
independently ready new body is established by this audit.

Ghidra's generic `undefined(void)` prototype omits the demonstrated receiver
and stack input. No listing repair is required: every instruction, both exits
and all call continuations decode. The optional flow-property query was
unavailable because script execution is disabled; no no-return flag values
were inferred and no script permissions or annotations were changed.

Validation is complete-body static/export/PE and current-Source inspection,
plus the call-row verifier. Existing build/fixture claims are not new test
results. Native execution, original object ABI/lifetime, world and gameplay
remain unvalidated by this packet.
