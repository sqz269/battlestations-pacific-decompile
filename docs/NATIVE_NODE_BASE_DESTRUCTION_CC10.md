# Actual node base destruction

Addresses: `00B6F440`, `00B6D940`, `00B6DBE0`, `00B6F3E0`; requested-null projection of `00B6E680`; compiler boundaries `00CC19E0`, `00CC19E8`, `00CC19F3`, `00CC1A01`.

`native_node_base_destruction.hpp/.cpp` implements the actual node base destructor and its reached null-parent path. It uses the same actual 174h node prefix, 24h outer scene, 3Ch resource, raw string pool and canonical actual+4 owners as the published raw providers. It does not install a derived camera/light terminal or route actual resources into legacy `SceneResource` companions. Public source interfaces differ from the original x86 ABI.

| Routine | Native extent, exclusive end | Original ABI | Coverage |
| --- | --- | --- | --- |
| B6F440 | B6F440–B6F569, 297 B | ECX actual node, RET | Complete body in the required genuine-provider domain |
| B6D940 | B6D940–B6D980, 64 B | ECX parent, captured stack child, RET4 | Complete |
| B6DBE0 | B6DBE0–B6DBF9, 25 B | ECX node; current3C tail JMP or RET | Complete with required exact dispatch |
| B6F3E0 | B6F3E0–B6F3F7, 23 B | ECX actual 0Ch array, RET | Complete in valid owned-array domain |
| B6E680 | B6E680–B6E74C, 204 B evidenced | ECX node, requested parent, RET4 | **Partial projection: requested null only**, 145 B reached; excludes [B6E6B8,B6E6C0) and [B6E6C5,B6E6F8), 59 B |

The excluded B6E680 regions include old-parent-null work reachable only for a nonnull request and the new-parent branch. No full reparent compatibility is claimed. Existing typed reconstruction records remain intact.

## Actual storage and required providers

Every payload argument is an actual native identity. Hierarchy fields are parent30, child34/count38, next3C/previous40, notification ownerA0 and outer sceneA4; retained owner130 and resource170 are separate actual owners. Point-light storage164 is the original 0Ch `{begin,count,capacity}` descriptor. Name54/58 is the original length/data header. No duplicate ownership arrays, projected roots or logical resource fields are introduced.

The context borrows scene attachment, tree retirement, raw string storage and world-dispatch bindings. Before native work, a pure metadata admission check requires scene/tree contexts to reference the **same canonical registry and same current CE2220 function-pointer cell**. A mismatch throws before any native read/store or acquired-start flag. This check is outside the admitted native behavior domain.

Current profile resolution is pure borrowed-table lookup. Reached virtual40 and virtual3C must invoke the exact captured target on the actual receiver. The genuine B6DBE0 and B6DBC0 providers are usable only when that target and profile are admitted; there is no fallback after mutation. Root propagation uses published B6D890 and its exact current50 dispatch with caller-prepared recursive frames. Recursive unregister uses published B6D850 and persistent tree frames. Raw B72220, B8F4C0, B6EE10 and B6DA30 remain external source reuse.

Retained130 zero first resolves the canonical owner and checks its count address is the captured actual+4. The required current0 provider must additionally check observed actual zero, the current exact target and the retained owner's concrete family/extent, and perform genuine retirement. A 3Ch resource is not a node: node+170/hierarchy checks must not be applied to it. The fixture admits the genuine resource companion and destructor. No payload read follows its terminal. Existing legacy camera/light terminals remain forbidden on raw3Ch-attached nodes.

Arrays require valid nonnegative count/capacity, accessible live entries and backing owned by the same `NativePointLightLinksRuntime`. B6EC70(0) decrements count to zero; the caller then reads current begin and genuinely frees its allocation. Pointer/capacity and stale entries remain unchanged. This packet neither adopts foreign backing nor destroys referenced lights. Raw name cleanup uses actual current pool publications and return gate, never a semantic host pool.

## Ordering and caller frames

B6F440 stamps D62C88, captures A0 and enters state2. It unregisters the captured notification owner, clears A0 and requests null parent. B6D940 captures the previous sibling before clearing child30, rewires current sibling fields and conditionally reloads the parent's first child before decrementing count38. B6E680 returns immediately for an already-null parent; otherwise it unlinks, reloads notification A0, captures rootA4 before clearing parent/root/notification, propagates the captured root and recursively unregisters. The flags5C path captures child presence before publishing masked flags; virtual40 is resolved after those operations.

Back in B6F440, root and parent govern the root-unlink block. Child34 is captured before rootA4 is cleared, and each successor3C is freshly read after propagation. Retained130 is captured once; its actual+4 decrement uses current CE2220. After any zero terminal, retained130 is cleared. The routine then captures **current scene170 before a second retained130 clear**, writes recursion1 then the captured expected-scene argument, and calls real B6EE10. All callback mutations and completed prefixes remain visible.

Array state is consumed before resize/free. Name data58 is captured before state0 is consumed; current length54+1, with DWORD wrap, is captured before the pool getter. The captured data and size reach BD1510. Neither name header nor freed-array descriptor is zeroed. Finally state becomes -1, D5C104 is stamped and real BD30F0 stamps CEB130. The destructor does not return the node's physical slot or change its own reference count.

Caller frames are address-stable, initialized native argument cells plus separate persistent nested frames. `attachment_node_argument` is written before B8F4C0; the parent-null frame has a distinct argument and recursive-unregister frame. The scene-remove frame receives recursion then expected-scene in native push order. Unwritten frame bytes retain their preimage. Fresh disjoint `Acquired` objects retain nested acquisitions, active call, EH state and completed cleanup flags through failure. Original saved registers, return addresses, private-frame offsets and stack alias gaps are excluded from this source interface; no private native stack ABI is claimed.

## Compiler boundaries and failures

The 43 existing compiler bytes are independently pinned:

| Boundary | Extent | Action |
| --- | --- | --- |
| CC19E0 | 8 B, last CC19E3 JMP length5 | Saved [EBP-10] -> AA6E10 base cleanup |
| CC19E8 | 11 B, last CC19EE JMP length5 | Saved node+54 -> 41DD20 name cleanup |
| CC19F3 | 14 B, last CC19FC JMP length5 | Saved node+164 -> B6F3E0 array cleanup |
| CC1A01 | 10 B, last CC1A06 JMP length5 | EAX=DFA900 -> BF6B43 native FH3 |

DFA900 has maxstate3, unwind map DFA8E8: state0 -> -1/CC19E0, state1 -> 0/CC19E8, state2 -> 1/CC19F3; no try blocks, EHflags1. These are evidence boundaries, not translated native handler entry points. All four functions already exist in Ghidra; no missing-function or listing-repair claim is needed.

The source projects ordinary C++ exceptions through the same remaining cleanup schedule. It consumes each state before calling its action; a second cleanup exception terminates. It never replays scene, retained-owner or hierarchy work, removes extra backlinks, repairs stale fields, restores argument cells or invents rollback. Native FH3/SEH faults, native CRT exception identity and asynchronous observation are excluded. The focused comparison exercises normal execution only; native exception unwinding is not tested.

## Evidence and validation

The report records all 30 original call/tail sites, including explicitly excluded nonnull-parent calls and six indirect targets. Mechanical verification checks all 24 direct rows against live Ghidra. Every normal byte (613 B, including the full 204 B parent body) and compiler byte (43 B) matches the original PE. Complete source-body coverage is 409 B plus the explicit 145 B null-parent projection. There are no x87 operations or hidden floating register arguments in this packet.

The ignored focused fixture copies five native normal bodies plus the existing 25 B bounds leaf, rebases direct calls/current import addresses to genuine providers, and compares them with the source. It uses real B6F5A0 node and B83C50 resource construction, canonical resource/ambient references, borrowed initialized real type-predicate state, actual raw string-pool storage and owned array allocations. A real resource free callback replaces current node.scene170 while retiring retained130; the comparison verifies the later scene capture, remaining unmatched child credit, root/parent/notification unlink, all node DWORDs after pointer normalization, actual resource counts, current40/3C, name return and stale freed-array fields. An additional call within the same sequence checks complete B6F3E0 with actual backing. All resources, ambient references and pool allocations receive genuine cleanup. This is source-provider composition, not native-byte proof of every dependency or game validation.

The strict MSVC Win32 build and all three existing CTests pass. Probe builds use `/MD /O2 /fp:strict /link /MANIFEST:EMBED`; no new tracked tests were added. The report pins source/dependency/compiled hashes and preserves all 14 earlier archives. Worker Ghidra use is read-only. Required dispatch, raw invalid-input dependency limits, native exception transport, full derived terminals and nonnull reparenting remain explicit boundaries.
