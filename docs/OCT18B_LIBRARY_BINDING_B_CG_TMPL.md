# October 18 B: compiler-generated and non-sort template binding audit

This is a bounded audit of **10 sampled families**, not a census of every `CG_`,
backtick, `cg_*`, `rtti_vslot`, `TMPL_`, `tmpl_*`, or trivial-body entry. Sort,
partial-sort and stable-sort work belongs to the sibling sort audit. A raw getter
and message serializer are adjacent controls, not additional counted families.

The useful boundary is ownership and representation. Ordinary construction,
destruction and compiler-generated exception cleanup can serve wholly fresh native
C++ objects. A generated-looking function operating on original game storage,
vtable words, embedded headers or an original EBP frame still needs its established
body or a specifically qualified adapter. A name or discovery tag does not make a
function suitable for a stock binding.

## Evidence and limits

- Baseline: branch `agent/oct18b-libB-cg`, initial tip `3d581c328`; snapshot
  `2026-10-16T10:58:44Z`, 76,774 total functions. A live `bsp.py ghidra count`
  reported the same total. Queries use `bsp.py`'s verifying client on every live
  invocation: project `bsp`, `C:/Users/sqz269/bsp.gpr`, program
  `/battlestationspacific.exe`, x86 little-endian 32-bit, image base `00400000`.
- `bsp.py callers` supplies snapshot caller functions. `tools/callsite_census.py`
  independently scans the installed image's `.text` bytes for E8/E9 rel32 targets
  and `.text/.rdata/.data` for absolute target dwords. Its hard-coded image path
  matches `config/target.json`:
  `I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
- In the table below, `C/J/A` means candidate CALL sites / JMP sites / absolute
  dwords. These are **sites**, not unique callers, source coverage, reachability,
  executed calls, or a complete indirect-call census. Vtable/exception-table
  dwords prove references, not the identity of a runtime dispatching caller.
- The census assigns owners by nearest preceding indexed function start, without
  checking its end. Two exceptions were checked: `00504E02 -> 00504960` is an
  unrecognized `00504E00` thunk (`PUSH 0; CALL; RET`), not the printed
  `BSP_LoadingQueue_SubmitWork`. `00BAEF53/00BAEF83 -> 00453ED0` lie beyond the
  printed `00BAEEB0` owner's live body end `00BAEF0E`. Ownership remains unresolved;
  these sites are not counted as additional named game callers.
- Source files and existing ledger claims were inspected; this audit adds no C++
  implementation, host wiring, tests, build result, native execution, or game
  validation. Existing reconstruction statuses below are prior evidence, not new
  validation. No Ghidra names, tags, comments, flows or exports were changed.

## Sampled families

Effort is relative follow-up cost: **low** is an existing leaf/adapter with a small
contract; **medium** includes object ownership or callback/layout qualification;
**high** includes original exception frames or a substantial semantic subsystem.

The four-class totals apply only to these ten family rows: **bind-safe 2,
layout-bound 6, behaviour-sensitive 1, unreached 1**. `bind-safe` is conditional on
the stated ABI/callback contract, not a tested whole-game substitution.
`layout-bound` requires original storage/frame semantics; the solver and GUI pool
are explicitly excluded from stock library substitution. `behaviour-sensitive`
marks an EH contract requiring qualification beyond the existing normal-loop
adapter; it is not a claim that a particular stock CRT replacement has failed.
`unreached` records zero scanned references, not proof of dead code.

| Family and representative addresses | Class | Concrete callers or reference evidence | Recommendation | Effort |
| --- | --- | --- | --- | --- |
| 1. Scalar deleting destructors: `00411F00`, `00B192D0` | **layout-bound**: compiler instance tied to object ownership | Snapshot callers 0 for both; image `0/0/1` each, vtable dwords `00CE37FC` and `00D5E538` | Keep original-storage bodies; use ordinary native destructors only for wholly native owners | Medium |
| 2. Vector/member cleanup: `004042C0`, `00B18D50` | **layout-bound**: object-specific cleanup, including a misleading vector-delete tag | `004042A4` in `Dyn_SAPBroadPhaseManager_vslot7` calls `004042C0` (`1/0/0`); `00CBC57E/00CBC5BE` unwind thunks jump to `00B18D50` (`0/2/0`) | Preserve member count, reverse order, allocator and field layout; no generic vector destructor substitution | Medium/high |
| 3. Adjustor thunk: `004134D0` | **unreached**: original subobject displacement, no scanned references | Snapshot callers 0; image `0/0/0`; assembly `ADD ECX,8; JMP 00C37450` | Keep the +8 adjustment if an original interface can reach it; compiler-generated adjustment is sufficient only for fresh native classes | Low for an explicit adapter; medium for ownership |
| 4. Non-EH constructor/copy iterators: `00403560`, `00453ED0` | **bind-safe**: stock compiler-runtime loop under the stated callback ABI | Game examples: `0078C44D` packet statistics ctor, `0081EF7D` unit vehicle ctor, `0086514D` gunnery tick -> `00403560`; `004558C2` action copy and `00BAF3BB/00BAFC60` coast copies -> `00453ED0` | Bind qualified runtime behavior, or use native construction for native elements; retain original callback ABI when operating on raw elements | Low for native objects; medium for raw callbacks |
| 5. EH array iterators: `00BF7C6E`, `00BF7CD1`, `00BF9A96` | **behaviour-sensitive**: stock compiler runtime with an unwind contract beyond normal loops | Game constructors/destructors and copies listed below; image `288/0/0`, `126/0/0`, `4/0/0` respectively | Bind the CRT contract rather than credit its body as game reconstruction; qualify callback ABI and partial-construction unwind separately | Medium/high for raw EH |
| 6. Per-function EH cleanup thunk: `00C5FA4C` | **layout-bound**: compiler instance tied to an original frame | Snapshot callers 0, image `0/0/1` at `.rdata` `00D866C0`; thunk loads `[EBP-10h]`, adds `34h`, jumps to `004404F0` | Let the compiler create fresh native EH glue; never reuse this frame-specific thunk as a general cleanup function | High for original-frame compatibility |
| 7. RTTI-discovered solver virtual method: `00403720` | **layout-bound**: concrete physics body; `rtti_vslot` is discovery provenance | Snapshot callers 0; image `0/0/1`, vtable slot 0 at `00D7A088`; no direct game caller proven by this audit | Retain the existing mode-0 solver/task implementation; excluded from stock library substitution | High if extending beyond current normal-return evidence |
| 8. Alleged static destructor: `00AAC260` | **layout-bound**: game GUI pool allocator; stale `cg_static_dtor_stub` classification | Snapshot factory caller `00AA4440`; image CALL `00AA445E` (`1/0/0`) | Keep the established pool allocator binding/body and name; excluded from stock library substitution | Low/medium |
| 9. Intrusive reference ownership: `004404F0`, `00504960` | **layout-bound**: raw-layout template semantics, not `shared_ptr`/`std::vector` ABI | Unit message handler CALL `0082232D` -> release; tactical preview CALL `0050A9F6` -> resize; additional sites and cleanup thunks below | Preserve refcount +4, vslot 0 destruction, slot nulling and three-word vector layout; reuse existing resize body | Medium |
| 10. Trivial virtual no-op: `00404540` | **bind-safe**: true input-independent no-op with an explicit ABI | Snapshot callers 0, image `0/0/1` at vtable `00D7A19C` | Native no-op is sufficient once its three stack words/RET 0Ch are honored | Low |

### 1. Scalar deleting destructors

`00411F00` is not interchangeable with `delete(void*)`. It writes vtable
`00CE37FC`, reads the owner at guard+4, decrements owner lock depth, leaves the
critical section, and frees the guard only when flags bit 0 is set. Original ABI:
ECX receiver, flags on stack, RET 4, returns the guard. The existing semantic body
is `scoped_lock_scalar_delete_00411f00` in
[`src/segs_0_9_11_cc13.cpp`](../src/segs_0_9_11_cc13.cpp).

`00B192D0` calls the material-effect-base destructor before conditional freeing.
The current actual-storage implementation,
`delete_native_material_effect_base_00b192d0`, is in
[`src/native_material_effect_owner.cpp`](../src/native_material_effect_owner.cpp).
Its source owner also checks the bound profile/vtable before disposal. Replacing
the mechanism with native RAII is sensible when the replacement owns a fresh
native object, but does not supply the game object's ownership contract.

### 2. Vector/member cleanup

`004042C0` has cleanup loops for buffers at original offsets +194h, +F8h/+FCh,
+5Ch/+60h, +28h/+2Ch; it then destroys three 0Ch-byte members from object+4 using
`00BF7C6E` and installs the base vtable. No exact `src/` body was found by its
address/name; its indexed artifact is the exported pseudocode. The separately
reconstructed `004043F0` derived SAP lifetime is not evidence that this entire
base cleanup is supplied.

`00B18D50` provides a useful counterexample to trusting the tag: its established
ledger explicitly identifies a full 16-byte member-cleanup helper, not a vector
deleting destructor. It destroys eleven eight-byte native string headers in
reverse, independently of the owner's +94h count. The body
`destroy_native_material_effect_names_00b18d50` already exists in
[`src/native_material_effect_owner.cpp`](../src/native_material_effect_owner.cpp),
including use by compiler-generated cleanup for the newly compiled source owner.

### 3. Adjustor thunk

The pseudocode drops the essential `ECX += 8`; the saved assembly contains exactly
`ADD ECX,8; JMP 00C37450`. No direct source mapping for `004134D0` was found.
`avoid_zone_dyn_hull_destroy_00c37450` in
[`src/avoid_zone_dyn_hull.cpp`](../src/avoid_zone_dyn_hull.cpp) is an existing
qualified handle-based destructor interface, not automatically a raw substitute
for this thunk. Zero scanned references are a negative result, not proof of
permanent unreachability.

### 4. Non-EH array helpers

Snapshot caller functions: 13 for `00403560` and 3 for `00453ED0`; the image has 13
and 5 candidate CALL sites respectively. The two additional copy sites are the
unresolved `00BAEF53/00BAEF83` described above.

Live assembly verifies the real callback interfaces: constructor iterator
`00403560` takes `(first,stride,count,ctor)`, calls `ctor(ECX=element)`, steps by
stride, and RET 10h. Copy iterator `00453ED0` takes
`(destination,source,stride,count,copy_ctor)`, calls with ECX=destination and one
source stack word, advances both pointers, and RET 14h. Ghidra's prototype for
the latter is still `undefined(void)`, so the listing is the relevant ABI evidence.

No general source bodies for these CRT helpers were found. Existing call sites
remain explicit services, for example the packet-statistics constructor in
[`src/segs_13_15_22_44_cc13.cpp`](../src/segs_13_15_22_44_cc13.cpp), and equivalent
element copies in [`src/actions_seg5_cc13.cpp`](../src/actions_seg5_cc13.cpp) and
[`src/water_weather_cc13.cpp`](../src/water_weather_cc13.cpp). These source
adaptations do not establish a universal callback ABI replacement.

### 5. EH array runtime

Snapshot callers are 250/84/4 unique functions for destructor/constructor/copy
iterators; byte-scan site counts are 288/126/4. Counts differ because functions can
contain several calls and snapshot call edges can omit unrecognized bodies.
Concrete snapshot game callers include:

- `004CB2F0` game-participant destruction and `004C9820` scene-player-slot
  destruction -> `00BF7C6E`.
- `00424A10` gameplay tuning, `004CB030` world construction, and `004D6BA0`
  participant-record construction -> `00BF7CD1`; image site `00424BDE` confirms
  the gameplay-settings call.
- `005C81F0` mission-record copy, `007042B0` message-record copy (site `007043AA`),
  and `008E79D0` powerup-class copy (site `008E7A42`) -> `00BF9A96`.

Live listings for `00BF7C6E` and `00BF7CD1` show EH prologue/epilogue and helper
calls beyond the normal element loop. The destructor walks backward, with ECX as
the element receiver and RET 10h; constructor uses ECX receivers and RET 14h.
The copied-element iterator's live prototype is the six-argument stdcall contract
`(dst,src,stride,count,copy_ctor,dtor)`.

Existing [`src/native_game_array_elements.cpp`](../src/native_game_array_elements.cpp)
and [`src/native_game_array_lifetime.cpp`](../src/native_game_array_lifetime.cpp)
provide selected actual-storage normal-flow operations. Their headers explicitly
exclude original CRT/FH3 reverse cleanup and binary calling conventions, reject
unsupported callback/stride pairs, and retain failure progress to prevent replay.
They are not complete generic CRT iterator implementations. No general source
body for `00BF9A96` was found. Fresh native objects should use their compiler's
array/RAII cleanup; raw callback bindings still need exact callback and exception
ownership qualification.

### 6. Frame-specific unwind glue

Live assembly at `00C5FA4C` is:

```asm
MOV ECX,[EBP-10h]
ADD ECX,34h
JMP 004404F0
```

The absolute dword `00D866C0` references this thunk. No direct source body or
owning original EH-state record was mapped in this bounded audit. A normal C++
function with one explicit object argument would not reproduce the EBP contract;
the compiler can generate suitable cleanup for a new source function instead.

### 7-8. Discovery tags are not semantics

`00403720` is the `Dyn::Scene::LCPSolverTask` method discovered from RTTI/vtable
data. It reads task world+8, first+0Ch, last+10h and dt+14h, and performs prestep,
normal/friction solves, velocity writeback and contact impulse storage. Existing
[`src/native_dyn_solver_mode0.cpp`](../src/native_dyn_solver_mode0.cpp) supplies the
actual-record task and callable table; the semantic alternative is in
[`src/dyn_lcp_impulse_math.cpp`](../src/dyn_lcp_impulse_math.cpp). Existing evidence
is explicitly qualified to normal return and does not establish original EH,
full-world ownership, concurrency or gameplay behavior. None of that work can be
replaced by a generic RTTI runtime merely because its tag is `rtti_vslot`.

`00AAC260` is already named `BSP_GuiButton_AllocatePoolBlock`: it selects the Button
pool and tail-calls `00AABF80`. Its concrete game factory caller is `00AA4440`,
confirmed by image CALL `00AA445E`. The current implementation/traits are in
[`src/native_gui_widget_pools_cc13.cpp`](../src/native_gui_widget_pools_cc13.cpp)
and its header. The stale `cg_static_dtor_stub` tag describes a previous pattern
guess. No tags were rewritten in this audit.

### 9. Intrusive references and raw vectors

`004404F0` takes the pointer slot in ECX, reads its pointee, interlocked-decrements
pointee+4, dispatches vtable slot 0 at zero, then clears the slot. Snapshot callers
include the actual unit-message handler `00821E80`, confirmed by CALL `0082232D`,
plus eleven `Unwind@` thunks. The census agrees at one CALL and eleven JMP sites.
There is no mapped exact `src/` body for `004404F0`; generic refcount helpers in
other packets are not an established adapter for this address.

`00504960` additionally depends on a raw three-word `[data,size,capacity]` vector,
zero-fills growing slots, releases shrinking slots, and reserves through
`005017A0`. Snapshot callers are destructor `00505730`, preview rotor cleanup
`0050A950`, and destructor `0050AF20`; the image contains their CALL sites
`00505735`, `0050A9F6`, `0050B0B6`, plus the unrecognized `00504E00` thunk's CALL
`00504E02`. Its existing body is `ref_ptr_array_resize_00504960` in
[`src/frontend_seg12_17_cc13.cpp`](../src/frontend_seg12_17_cc13.cpp).

Native `shared_ptr` and `std::vector` can serve new native ownership, but do not
supply this interlocked +4 counter, virtual deletion ABI, zeroed slots, allocation
family or original three-word layout. Unreferenced `00419F00` RefPtr reset and
`0042BF10` AddRef were also checked as negative controls: both snapshot callers
and image references were zero. They add no called-family coverage here.

### 10. Trivial no-op and adjacent getter control

`00404540`, `Dyn_TerrainShape_vslot2`, is exactly `RET 0Ch`: no receiver read,
storage write or semantic result. A source no-op is a safe semantic binding once
the ECX receiver, three stack words and callee cleanup are preserved at a raw
vtable boundary. Its reference is TerrainShape slot 2 at `00D7A19C`; a runtime
caller was not identified. Existing body: `terrain_noop_00404540` in
[`src/oct18_rec.cpp`](../src/oct18_rec.cpp).

The adjacent control `00B17320` is not another counted family or classification
row. It returns the DWORD at material+104h, with ECX receiver and
plain RET. Its actual material-draw caller `00B43410` has image CALL `00B43442`.
The existing raw getter is `get_native_material_word_104_00b17320` in
[`src/native_material_texture_queries.cpp`](../src/native_material_texture_queries.cpp),
used by [`src/native_material_pass_execution.cpp`](../src/native_material_pass_execution.cpp).
The body is tiny, but its fixed offset remains part of original material storage;
it is not replaced by a generic C++ getter on a differently laid-out class.

## Adjacent serialization boundary, not an extra sampled tag family

The indexed `TMPL_`/`tmpl_*` sample did not expose a named serializer. To avoid
inferring that generic serialization is stock, `00813DC0`
`BSP_UnitStateMessage_Serialize` was checked as a separate control. It is **not**
tagged `cg_*`, `rtti_vslot` or `tmpl_*` and contributes no in-scope family count.

It is message vtable `00D02D94` slot +4, image absolute reference `00D02D98`, with
zero snapshot direct callers or rel32 sites. The body serializes selected raw
message offsets through game bit-cursor functions, with field-specific bit widths
and ranges; it omits Y components +2Ch/+38h. The existing
`unit_state_message_serialize_00813dc0` body in
[`src/cc13_sweepC.cpp`](../src/cc13_sweepC.cpp) preserves that protocol. A stock
serializer is not an equivalent binding. The virtual dispatching game caller is
not proved in this audit; do not count the vtable dword as a named caller.

## Reproduction and delivery

Representative queries (all read-only apart from the named evidence appends):

```powershell
python tools/bsp.py callers 00403560 --limit 20
python tools/bsp.py callers 004404f0 --limit 12
python tools/bsp.py callers 00504960 --limit 8
python tools/bsp.py ghidra disasm 00403560 --lines 35
python tools/bsp.py ghidra disasm 00453ed0 --lines 38
python tools/bsp.py ghidra disasm 00c5fa4c --lines 14
python tools/callsite_census.py 00411f00 004042c0 004134d0 00403560 00bf7c6e 00bf7cd1 00403720 00419f00 0042bf10 00aac260
python tools/callsite_census.py 004404f0 00504960 00c5fa4c 00453ed0 00bf9a96 00b17320 00813dc0
python tools/callsite_census.py 00b192d0 00b18d50 00404540
```

Only existing-name evidence was appended for `00404540` (true no-op contract),
`00411F00` (layout-bound generated destructor), and `00403720` (RTTI discovery
does not imply stock runtime). Earlier ledger evidence/names are preserved. The
primary integrator owns any subsequent Ghidra application and taxonomy decisions.
