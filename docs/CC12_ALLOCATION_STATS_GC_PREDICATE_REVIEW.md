# Allocation statistics derived-profile GC predicate review

`D685F4 + 0Ch` points to `00BE2740`, whose complete reachable body is
`32 C0 C3`: `XOR AL,AL; RET`. This exact derived-profile predicate always returns
false. It reads no receiver fields, globals, allocator state or Lua state.
The current `GameScriptOrdersHost` false result therefore already matches this
predicate's boolean result. No predicate or command activation is justified.

This packet changes only this document and
[`reports/cc12_allocation_stats_gc_predicate_review.json`](../reports/cc12_allocation_stats_gc_predicate_review.json).
It adds no C++ API, command binding, test or fixture. Names remain descriptive
hypotheses. Evidence is scoped to this profile and the retained caller contract;
it does not establish complete raw dispatch, startup, ABI or gameplay parity.

## Identity, finite bytes and saved-analysis boundary

The assigned accepted Source baseline is
`4c0e429c3493d48af726c528a7e9854ef9cafad8`. Capture HEAD was the preceding worker
review `2929fe387ab428d488a38baa847185a1f8246a33`; its entire `src/` and `include/`
Git tree equals the assigned baseline. The preceding review was pending main
integration at capture and is frozen separately at its own commit. After its
acceptance, the worker fast-forwarded to `c44c65ee91bceff4390a544a6d3f0a4148ed4183`;
the original assigned/capture epochs remain distinct, and the frozen Source
tree and working-file verification still match after the fast-forward.

The worker first leased only `D685F4..D68603` and the two deliverables/local
evidence directory. The finite profile read identified `BE2740`; the worker then
claimed that exact entry before any body/prototype query. Typed project identity
is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 32-bit, image base
`00400000`. Typed instruction metadata retained modification number **49**
before and after, with no analysis or repair performed.

| Original range | PE file offset | Retained interpretation |
|---|---:|---|
| `00D685F4..00D68603` | `009685F4`, `.rdata` | 16 bytes; four little-endian profile entries |
| `00BE2740..00BE2742` | `007E2740`, `.text` | 3-byte reachable body; two instructions |
| `00BE2743..00BE274F` | `007E2743`, `.text` | 13 separate `CC` padding bytes before known entry `BE2750` |

The profile bytes are
`30 29 BE 00 00 27 BE 00 20 27 BE 00 40 27 BE 00`, giving slots
`+0=BE2930`, `+4=BE2700`, `+8=BE2720`, `+C=BE2740`. The read ends at `D68603`:
it neither reads the next DWORD freshly nor proves the table's full extent.
It agrees with the first 16 bytes of the separately retained older 20-byte
profile record; that record did not admit its neighboring word as slot `+10`.

Complete inert PE retention is 12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Replay parses its section headers and checks the exact mappings and finite
payloads. Retaining the PE does not extend this packet's interpretation scope.

Current typed metadata reports **no function at or containing `BE2740`**.
Prototype/decompile/function-disassembly requests consequently fail or report no
function; their exact outputs are retained. The decompiler frontend's cached
index additionally proposes enclosing candidate `BE2720`. This is a historical
index hint, not current typed containment. Earlier no-body/interior/candidate
wording remains evidence of its earlier epoch and is not relabeled or repaired.
No conclusion here requires the entry to be a named Ghidra function.

Zero-context typed instruction reads show `BE2740: XOR AL,AL` and
`BE2742: RET`. Typed flow gives lengths 2 and 1, ordinary fallthrough to `BE2742`,
a terminal return there, and no flow override. The original finite bytes and
offline bounded disassembly agree. Thus the three-byte reachable body is closed
without claiming a saved Ghidra function body or prototype exists.

## Predicate and caller contract

The retained `ENTITY_THINK_DISPATCH` evidence supplies the caller; this packet
makes no fresh caller-body query. At `00929568`, the caller passes current
`[0109CEFC]` in ECX, supplies no stack arguments, and calls its vtable `+0Ch`.
`0092956A` tests **AL**. The recovered leaf ignores ECX, clears AL and returns
with plain `RET`. It leaves EAX's upper 24 bits unchanged; a full-EAX zero return
must not be inferred. There is no x87/SSE arithmetic, memory operand, nested
call, allocation, global publication, logging or Lua operation in the leaf.

| Layer | Reads/effects established here |
|---|---|
| Exact `BE2740` leaf | No receiver/global reads; AL becomes zero; return |
| Retained Native caller | Current `0109CEFC` publication, receiver vptr at `+0`, profile slot `+C`; AL result |
| Existing constructors/lifecycle | Raw publication, manager registration/unregistration, locking, profile writes and lifetime effects, retained from prior evidence |
| Mission Lua owner | Separate owner/binding/readiness contract, retained from the preceding Source review |

The leaf does **not** read receiver `+4` (`40000000` after construction) or `+8`
(initially zero). Those fields are not GC thresholds. Calling another metric
slot, consulting a semantic `AllocationStatsState`, or adding a memory-pressure
condition would invent predicate behavior. An ordinary Source constant-false
leaf is behaviorally ready at the boolean boundary; the existing override
already provides that result. No new API or original-register-ABI claim follows.

Retained caller ordering refills the expired countdown before invoking the gate.
Only a nonzero AL reaches `collectgarbage()` through `006B8AD0`; pending-list
splice/clear follows. For this derived profile, that explicit command is skipped.
This says nothing about Lua's other garbage-collection mechanisms or other
callers. It also does not prove that a null/stale/transitional publication can
safely be bypassed: caller-side object/vptr observations precede this pure leaf.

## Current Source ownership and separate boundaries

Complete Source searches and pinned excerpts establish these current facts:

- `GameNativeStringProcess` retains the actual volatile `0109CEFC` authority;
  `GameSingletonHost` borrows it and the same canonical manager publication in
  `NativeAllocationStatsConstructorContext`.
- Phase 0 allocates a genuine 12-byte receiver, conditionally calls the raw
  `BE2900` reconstruction, and hands normal success to existing singleton
  lifecycle management. This is accepted Source, not a live run in this packet.
- The base constructor publishes/registers the actual receiver under profile
  `D685E0`. After base success, the derived constructor writes `D685F4`, raw
  `40000000` at `+4`, and zero at `+8`. The pure predicate does none of this.
- Destruction stamps the base profile, unregisters the current publication and
  clears the cell; derived deletion frees the captured receiver when flags bit
  zero is set. The retained host installs the actual stats deletion context.
  Partial construction/destruction, failure and reentrancy remain distinct from
  a completed derived-profile observation.
- `GameScriptOrdersHost::gc_gate_predicate_0109cefc_vtable0c` simply returns false.
  Its comment that the process has no such object is stale relative to accepted
  raw startup. The comment is untouched in this two-file review. The parallel
  `GameStepSubsystemsHost` adapter also returns false but logs an unimplemented
  diagnostic; that Source logging is not an effect of the original pure leaf.
- Neither adapter binds a raw stats receiver at the predicate call. Matching
  the boolean result for `D685F4` does not certify original publication/vptr
  observation, fault behavior, other profile handling or object lifetime.
- The preceding owner review retains the real mission Lua owner, the sole
  ScriptOrders construction site, and borrower destruction before owner
  replacement. Cached `machine_state_` is a trampoline argument, not proof of
  owner identity. This packet creates no VM, owner, command route or readiness
  fallback and changes no scheduling.

No fresh `D685E0 +C` or other-profile target was read. The constant-false result
must remain qualified by the observed completed `D685F4` profile. The earlier
review's unfinished-gate wording is now resolved at this derived leaf only;
separate command-owner and raw dispatch questions are not activated by it.

## Replay and closeout

`local/cc12_allocation_stats_gc_predicate_review` contains the frozen evidence,
capture scripts, portable `replay.py`, separate `verify_git.py`, and their JSON
results. It retains all 4,206 Source files in both Git and working archives,
38 additional complete Source/Git inputs, and two whole preceding-review files.
The reviewed 19-root include closure has 592 project files, 1,164 include edges
and four configured Lua API headers. External system/SDK dependencies are
listed as exclusions; no compiler/SDK preimage or compilation is claimed.

Artifact-only replay validates all hashes, Git blob IDs from retained data,
12 complete-corpus searches, 35 exact excerpts, 44 Source anchors, include closure,
14 command receipts, typed project/flow identity, finite bytes and PE placement.
Its audit hook rejects outside reads, writes, subprocesses, dynamic-library loads
and network access. A physically copied evidence directory produces the identical
result. The separate live-Git mode validates 4,246 blobs and 4,244 working files.

These are static evidence/integrity checks. No Original code, Lua interpreter,
game, compiler, build, test or new fixture was executed. Ghidra analysis, names,
prototypes, listing, flow and bodies were preserved. **No C++ GO or activation.**
