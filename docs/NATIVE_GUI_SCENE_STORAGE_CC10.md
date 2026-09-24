# Actual outer GUI scene storage lifecycle

This packet reconstructs the complete normal bodies over the actual24h outer scene. It keeps actual3Ch lighting and actual root identities in native fields. The older `NativeGuiSceneOwner` path remains a separate typed provider. No source here converts actual storage into that owner or a host `SceneResource`.

| Native entry | Exact range (end exclusive) | Bytes | Coverage / original ABI |
|---|---|---:|---|
| B724E0 | B724E0–B72576 | 150 | Complete normal construction; ECX destination, stacked raw name pointer, EAX self, RET4 |
| B72430 | B72430–B724D6 | 166 | Complete normal destruction; ECX actual owner, RET |
| B72580 | B72580–B7259E | 30 | Complete scalar wrapper; ECX owner, stacked flags, EAX original identity, RET4 |

All346 body bytes match both live Ghidra and the installed PE. The last instructions are B72573 RET4 (3B), B724D5 RET (1B), and B7259B RET4 (3B). Six compiler boundaries total58B; they are evidence for source cleanup scheduling, not native EH implementations.

The layout is profile00 / atomic count04 / weak08 / root0C / name length10 and data14 / scalar18 / actual resource1C / word20. Construction first establishes a private actual-only C++ storage lifetime. It restores count preimage bits using the live atomic's relaxed store and restores the other20 bytes before calling925490. This handles MSVC's non-trivially-copyable atomic without an owning string, root-list object, or logical resource. No observation or callback occurs during this preparatory lifetime step. Caller backing must be aligned, writable and exclusively available.

After the weak constructor, B724E0 stamps D62D48, clears root0C and both name words, then captures the **current incoming name argument**. It arms name cleanup and clears1C/20. A non-self source passes its then-current length to raw41DD40; after resize, the captured source header is reread for length, then current destination length, current source data, and current destination data are read in that order. The final liveD7A24C load/store is MOVSS, preserving raw bits. Constructor failure runs the armed raw name and weak cleanup; it never frees the24h destination.

B72430 stamps D62D48, captures current1C, calls the currentCE2220 decrement on that actual+4, and reaches its same-count canonical current0 only at zero. The1C zero store follows the callback. A null initial resource skips that store. The root loop separately rereads current0C for the condition and captured call argument, then calls genuineB6DFA0; the root can be retired, so no root payload is read after return. Nonempty roots require the exact same actual-owner registry and a genuine `NativeNodeTreeRetirementContext`. Caller-prepared persistent acquisitions retain each reached call; capacity exhaustion is an explicit host diagnostic with prior effects preserved.

After roots, name data14 is captured, name cleanup is consumed, then current length10+1 wraps as a DWORD and is captured across the real419CC0 getter and BD1510 return. Normal and unwind name destruction do not clear the header; its pointer can name returned storage. Weak cleanup is consumed before925540. The scalar wrapper reads the **current flags low byte only after destruction**, then calls the genuine shared CRT free when bit0 is set. No owner access follows free.

`NativeGuiSceneReference` is separate postconstruction host metadata. Admission borrows the same actual+4 in the existing actual-owner registry; it performs no native store or retain. Duplicate or failed admission leaves the owner unchanged. Canonical release requires observed actual count0 and current0=BD30E0, then BD30E0 captures the profile for a fresh current4=B72580/flags1 dispatch. Separately selected explicit scalar entry permits nonzero counts and flags0. Both flags0 payload destruction and flags1 free retire/unbind metadata exactly once. Unbind and the retired companion destructor use only host metadata and numeric identity, never dead native storage.

The companion/context/frames/acquisitions must remain address-stable and external synchronization must prevent reuse until retirement finishes. Every admitted owner must be destroyed through its companion; calling the raw scalar body directly would strand its registry binding. Source exceptions retire the metadata only after source cleanup. They do **not** prove payload destruction/free completed: raw backing, persistent acquisitions and remaining credits need explicit disposition. Canonical release remains noexcept and terminates on an escaping source exception after that retirement.

Native cleanup evidence:

| Entry | Size | Final instruction | Contract |
|---|---:|---|---|
| CC1B40 | 8 | CC1B43 JMP925540,5B | Destructor weak cleanup |
| CC1B48 | 11 | CC1B4E JMP41DD20,5B | Destructor name+10 cleanup |
| CC1B53 | 10 | CC1B58 JMPBF6B43,5B | Raw handler, descriptorDFAAD4; no Ghidra function |
| CC1B60 | 8 | CC1B63 JMP925540,5B | Constructor weak cleanup |
| CC1B68 | 11 | CC1B6E JMP41DD20,5B | Constructor name+10 cleanup |
| CC1B73 | 10 | CC1B78 JMPBF6B43,5B | Raw handler, descriptorDFAB08; no Ghidra function |

DFAAD4 uses maxState2/mapDFAAC4: {-1,CC1B40},{0,CC1B48}. DFAB08 uses maxState2/mapDFAAF8: {-1,CC1B60},{0,CC1B68}. Both have19930522 magic. Constructor state0 follows925490 and precedes name initialization; state1 follows the source/self comparison. Destructor starts state1, consumes name before its getter/return, then consumes weak before925540. The C++ try/catch schedules use these boundaries; native private stack, FH3/SEH transport and faults are outside the contract. No Ghidra mutation was made; the two undefined10B raw handlers need primary metadata handling if desired.

The strict MSVC Win32 build and both configured existing CTests pass. One ignored focused executable compares two normal original/source compositions: named data and current-one/name-length alias, then self-name with raw sNaN MOVSS bits and a reached imported decrement callback changing the incoming scalar flags. It compares normalized full24h constructor bytes, uses real weak handles/string pool/raw3Ch resources and canonical ambient retirement, and drains a genuine physical point light through B6DFA0/B6F310 in the scene-null/cleared-root terminal domain. It also checks source canonical zero/flags1, explicit positive-count flags0, unchanged admission/duplicate refusal, and a source-only prepared-capacity failure with retained root followed by explicit disposition. No source production callback was added for the probe.

The **346B extent is byte evidence**, not346 unchanged execution bytes. Copied original direct calls, current import operand and live-one operand are relocated to explicit genuine shared providers. The eight bytes B72471–B72478 (`MOV EDX,[EDI]; MOV EAX,[EDX]; MOV ECX,EDI; CALL EAX`) are replaced by `MOV ECX,EDI; CALL bridge; NOP`, because D63168 is occupied by the reconstructed executable. That mapping is left untouched. The bridge passes the captured EDI identity, resolves its existing canonical companion, checks the same actual+4 and observed zero, then invokes genuine source canonical release, which validates current resource profile/current0 and fresh current4. Those original virtual-load/call instructions are **not independently compared**; assembly/byte evidence and the declared provider contract cover them. The remaining instruction stream is retained with the noted call/operand relocations. Nested weak/string/resource/tree machine code and native exception transport are not compared.

Exact probe flags are retained in the report and `local/output/cc10_gui_scene_storage_probe.cmd`: `/MD /EHsc /std:c++20 /O2 /Gy /W4 /WX /fp:strict`, no `/DNDEBUG`, compile-time `#ifdef NDEBUG` rejection, and `/link /OPT:REF /MANIFEST:EMBED`. This is source composition evidence, not a native binary replacement, complete camera/light terminal graph, application binding, AC59A0 admission, or gameplay test.
