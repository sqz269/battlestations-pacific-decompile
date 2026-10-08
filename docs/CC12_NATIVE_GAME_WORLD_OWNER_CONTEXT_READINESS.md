# Actual mission World owner and game caller readiness

The mission World and hierarchy World are the same **4BC-byte allocation**. The complete native caller proves the identity; `009037F0` allocates only two separate 0C-byte headers. Actual production Source construction and destruction remain **UNREADY**. This packet adds no Source, registered ready packet, build, test, probe, or execution.

This read-only packet owns `004DE610`, `004CB030`, `004C3080`, and `009037F0`. All names are descriptive hypotheses. Fresh exports came from `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Every byte and instruction start was compared with the installed original PE, without program mutation or flow repair.

| Complete body | Bytes | Instructions | Ordinary ABI |
| --- | ---: | ---: | --- |
| `004DE610..004DFAFC` | 5357 | 1370 | ECX actual Game; no stack args; plain RET; publication through fields/globals |
| `004CB030..004CB0AD` | 126 | 39 | ECX actual World; returns this in EAX; plain RET |
| `004C3080..004C3099` | 26 | 11 | Incoming ECX unused; no stack args; EAX new allocation; plain RET |
| `009037F0..00903848` | 89 | 34 | ECX same World; first stack BYTE consumed, second argument unused; RET8 |

The total is **5598 bytes / 1454 instructions**. Whole bodies and all call/branch sites are retained in `whole_native_analysis.json`. Closing those bodies does not close their callees' semantics.

## Proven construction and publication

`004DE651` pushes 4BC, `004DE656` calls `00BF55BE`, and EDI captures the result. `004DE664` calls `memset(EDI,0,4BC)` before the later null test. `004DE67D` passes that EDI to `004CB030`, whose `004CB09F` returns this in EAX. The caller sets ECX=EAX at `004DE68D`, stores that same EAX at `Game+19CC` at `004DE696`, then calls `009037F0` at `004DE69C` with arguments 1,1. There is no pointer adjustment, second World allocation, or separate 4A8-byte base object.

| Actual World field | Established write or ownership boundary |
| --- | --- |
| `+00` | Native vptr `00CE7784`; its physical first DWORD is `004CB0B0`. This is evidence, not a usable Source vtable. |
| `+04`, `+08` | `009037F0` allocates two independent 0C-byte intrusive headers and publishes these pointers. Each allocation is cleared in +4,+0,+8 order. |
| `+0C`, `+10`, `+14` | Constructor writes DWORD zero. |
| `+18..+4A3` | 0x61 = **97** category headers, stride 12. `00BF7CD1` receives constructor `004B7EC0` and destructor `004C2D30`. |
| `+4A4` | `00903802` writes the first argument's low BYTE. |
| `+4A8` | `009037FC` writes DWORD zero. Prior destructor context calls it a reference-counted pointer; this packet does not establish its full lifetime or retype it as a flag. |
| `+4AC` | Constructor writes ready BYTE 1 after the sentinel allocation returns. |
| `+4B0` | Untouched by these two constructor bodies; initial zero follows the real caller's memset. |
| `+4B4`, `+4B8` | Constructor publishes the actual 6C-byte sentinel result, then DWORD count zero. |

Category headers use the prior established `count,head,tail` convention; the two intrusive headers use `first,last,count`. Their shared 12-byte size does not make them interchangeable.

## Complete sentinel and allocation contract

`004C3080` ignores the supplied `World+4B0` value. It calls `00BF681B(0x6C)`, writes `[EAX]=EAX` if EAX is nonzero, computes ECX=EAX+4 in 32-bit arithmetic, then writes `[ECX]=EAX` if ECX is nonzero. Bytes +08..+6B remain uninitialized allocation contents. There is no payload constructor, World store, count initialization, or deallocation in this helper. Normal return leaves EAX as the allocation and ECX as allocation+4, with the final TEST flags; it leaves nonvolatile registers untouched.

The actual current allocator boundary is `singleton_lifetime_allocate`: `malloc`, `_callnewh` retry, then `std::bad_alloc`. Its matching free is `singleton_lifetime_free`. Current whole Source and historical complete physical objects are retained. Prior Native allocator105/free5 evidence is also retained and matches the installed PE. This is the established **host CRT service boundary**, not identical Native CRT classes, handler globals, or FH3 machinery.

The literal null branches are not graceful failure paths. A zero sentinel result still reaches a write to address 4. The caller memsets the World result before testing it, and later calls `009037F0` unconditionally. Older prose saying allocation failure simply stores null and continues must not be used as a failure contract. The qualified allocator returns storage or throws.

`009037F0` writes +4A8/+4A4 before allocation and publishes the first header at +4 before requesting the second. A second-allocation exception does not roll back the first inside this body. `004CB030` reaches its sentinel allocation only after the vptr, three DWORD stores, and 97 category initializations; it sets the ready byte afterwards. Its handler `00C65651`, state0 before the vector constructor iterator, and state1 before sentinel allocation remain real exception-cleanup prerequisites.

## Whole caller and real provider boundary

The complete `004DE610` contains **210 CALL sites: 184 direct, 26 indirect, and 83 distinct direct targets**. The retained schedule covers the scene root24, Operator458, child34 and reference release; renderer and scene settings; record/default resource and placement branches; ocean40; fog94; skyB8; eight channel30 objects; six tail managers; and final input-context enable. All direct branches target retained instruction starts. The decompiler's `004DF846` unreachable warning was not used to omit the discarded diagnostic-string block.

Actual read-only scalar/string bytes and the `InterlockedIncrement` / `InterlockedDecrement` import tuples are retained. Writable-global bytes from the PE are initial on-disk storage only, not live renderer, resource-manager, game, or record owners. `00C671A0` and its cleanup graph, all unresolved virtual receivers, and the unqualified callee bodies remain outside the completed scope.

Existing Source is concrete in some places but does not supply the missing mission World:

- `construct_world_object_004cb030` returns a `WorldObjectLayout` projection with `list_head=1` and `slot_count=97`; it allocates neither actual headers nor the sentinel or 4BC owner.
- `run_world_construct` forwards to abstract `WorldConstructHost` methods and returns uint32 handles. A bounded production Source search found no implementation of `create_world` / `world_post_construct`.
- `GameWorldHost::build_entity_chains_009037f0` fills index vectors and counters. It has no two real 0C headers or 4BC World field writes. The mission host builds this projection after units and explicitly leaves mission teardown unimplemented.
- `GameNativeGameRuntime` already wraps actual `NativeGameStorage`, operation frames, and canonical Dyn services. Its physics access is **Game+18**, distinct from the mission/hierarchy **Game+19CC**. This finding does not erase the existing game/Dyn owners.
- The concrete `initialize_world_fog_004df6a3` fragment preserves the fog ownership/x87 schedule but requires actual receiver, camera, record, and temporary inputs; it does not produce the whole caller context.
- Raw header13, destructor tail5, clear70, and the two raw unlink83 helpers exist. Their presence does not supply World allocation or lifecycle. The current header13 ledger still has its standalone admission reopened; the destructor tail's final admission depends on the named clear70 regate.

## Destruction and first unclosed owner

The physical `00CE7784` first pointer establishes the constructor's destructor target `004CB0B0`. Prior retained context pairs mission teardown `004D2BB0` releasing Game+19CC through slot0(1) with that deleting destructor and `00904C40`. Older context also assigns the +4A8 reference and +4B0 matrix-list cleanup to the latter. These teardown bodies were **not newly qualified** in this four-address packet. Current bounded lookup/search does not find a complete mission World lifetime Source implementation.

Before admitting the real owner, qualify its actual Source table/deleting destructor, full nonempty `00904C40` effects, array and constructor ordinary unwind, and Game publication clearing/partial construction ownership. A zeroed World, empty-list fixture, native numeric vtable, or fake callback cannot satisfy these requirements. Entire Game construction, mission startup, and gameplay remain UNREADY.

## Bounded next Source proposal

Propose `cc12_native_world_matrix_sentinel_source`, owning only **004C3080 (26 bytes / 11 instructions)** and new `native_world_matrix_sentinel.hpp/.cpp` plus its doc/report. It is an independent real allocation producer, not a World owner. Its sole ordinary call can bind through a concrete CDECL size adapter to the current `singleton_lifetime_allocate` with object/native/host size 0x6C. No test-supplied allocator is needed.

Preserve the complete native schedule outside that allocator relocation, both literal branches, self-link write order, and untouched +08..+6B bytes. Permit ordinary allocation exceptions; do not mark the path noexcept. Transfer the real raw allocation to its caller in the matching allocate/free domain. The later World publication and destruction remain caller responsibilities and must not be inferred from the helper.

The existing allocator Source, complete 90B/37-instruction provider with all five relocations, matching free6, prior Native allocator/free bytes, and physical archive origins make this a concrete bounded proposal at the explicit host CRT boundary. Primary registration/review is still required. The future authorized Source task should run the normal build and required existing checks, then qualify the fresh whole helper/adapter/provider objects and exact physical archive members against frozen actual inputs. This packet runs none of those actions and awards **zero ready packets**.

The first following owner remains `004CB030` with its real vtable, callback admission, array/unwind, and destructor contracts. Only after that can `009037F0` ownership and the much larger `004DE610` production context be admitted.

## Retained artifacts and validation

`cc12_native_game_world_owner_context_readiness_evidence.zip` contains 140 manifest files / 141 ZIP entries; every payload hash and ZIP CRC passed. It pins **78 physical inputs**, the whole original PE, complete fresh exports/byte captures, exact references, current Source, and the complete historical worker `bsp_core.lib` with whole selected objects. Seven physical archive members match exactly; the game-target vector host object is correctly absent from the core archive.

Historical body sizes are header13/6, callback5/1, clear70/29, allocator90/37 and free6/1, unlinks83/29 each, projected World74/15, host caller2534/768, fog319/111, and vector-chain builder443/145. The audit does not establish that a new build consumed the captured Source, and it makes no runtime claim from these object files.

ZIP SHA-256: `c00cf2420f3e1374a1ff8938ee93fbd58351e0a156bdf8af7a93caef01826ea7`. Manifest SHA-256: `c0275231b5adab9355256056b3fe0bab8cba067cf11c6dd0679f57601acc03e6`. Machine-readable details and whole-body schedules are in the accompanying report and sealed evidence. No Source, CMake, ledger, or Ghidra-program edits; no build, tests, probes, or execution.

## Fresh whole-caller primary follow-up

Root independently checked all 5,598 Native bytes and 1,454 instruction starts
across the four complete bodies, including the entire 5,357-byte caller. The
actual EDI allocation flows through the constructor EAX result to `Game+19CC`
and immediately to ECX for `009037F0`. This is the same 4BC-byte physical World.
`+4A8` is a DWORD store offset, not a second World allocation size. The two
separate allocations in `009037F0` are 0C-byte intrusive headers.

Root rehashed all 140 retained artifacts, all 141 ZIP payloads/CRCs, 78 physical
pins, and 26 current whole Source files; seven unique historical core members
match their complete objects, with the game-target vector object correctly
absent. These historical artifacts are not current compiler-consumption proof.
An initial reader assertion assumed every transfer had a direct-target key;
indirect calls omit it. The failed method and corrected reader are retained;
no Source, build or target execution changed.

Primary receipt: `local/cc12_world_identity_primary_review/receipt.json`, SHA256
`ae61faaad498ad2b37b76b64c8710a150b9db4d2bc704a0d42542cecaa59bd49`.
The tracked earlier hierarchy audit already identified a 4BC-byte owner; this
follow-up establishes the complete caller identity and supersedes any separate
4A8-byte-base interpretation. Full caller callees, World vtable/lifetime,
array/unwind and production context remain unready. No function, build,
execution, startup or gameplay credit is added.
