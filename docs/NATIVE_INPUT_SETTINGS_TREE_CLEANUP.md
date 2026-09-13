# Input settings tree cleanup

Addresses: 006A7540, 0055B690, 006A7AA0, 006A6A20, 0069FE70, 006A1AA0.
Supporting payload, subtree and iterator routines are listed in the report.

The settings parser now destroys actual device-tree storage directly. Settings
lifetime and constructor unwind use four concrete tree-range operations; the
required `NativeInputSettingsDestructionCalls` interface is removed. Five table
lookup/insertion calls and the separate keyboard-library providers remain.
This supplies populated member cleanup without shadow containers or empty
fallback implementations.

## Storage and destruction

Checked tree headers retain their opaque word at 0, head at 4 and count at 8.
Nodes have left, parent and right pointers at 0/4/8. Color and nil flags vary by
instantiation; the source selects the recovered layout for each payload.

| Storage | Nil byte | Owned payload |
| --- | --- | --- |
| Device node | 99h | Pooled key at Ch, device record at 14h |
| Controller-name node | 21h | Pooled key at Ch, integer/string tree at 14h |
| Default-action integer node | 15h | Scalar key and value |
| Preset node | 29h | Pooled key at Ch; scalar 20-byte descriptor |
| Device code/descriptor vector node | 25h | Pooled key; checked vector at 14h |
| Sensitivity node | 29h | Pooled key; checked vector at 18h |
| Packed-bit node | 29h | Pooled key; bit count at 14h, word vector at 18h |
| Scalar string-key node | 19h | Pooled key at Ch |
| Nested integer-tree node | 1Dh | Integer/scalar tree at 10h |
| Integer-only node | 11h | Scalar key |
| Integer/string node | 19h | Pooled mapped string at 10h |

Subtree cleanup recurses right, captures the current left child, destroys the
payload and frees the current node, then continues through the captured left
child. Nil nodes are retained. Whole-tree reset writes root, count, leftmost and
rightmost in native order, reloading the current head. An owning member then
frees its current head and clears head/count while preserving its opaque word.

0055B690 destroys the device record's members at offsets 78h, 6Ch, 5Ch, 50h,
40h, 30h, 24h, 18h, Ch and 0. This includes nested integer trees, packed-bit
storage, string-vector pairs, order strings, scalar maps and descriptor/code
vectors. The nested tree inside the member at 50h uses nil byte 15h; the separate
integer-only member at 78h uses 11h. Pooled keys are released after their mapped
payloads. 006A2BB0 frees the controller-name pair's integer/string tree before
releasing its pooled key. String and vector cleanup uses the existing concrete
source primitives and allocation domain.

## Range erasure

All four exported range contracts include partial erasure. They validate the
first owner against the receiver, recognize the full begin/end range, otherwise
advance the first iterator before erasing a separate copy of its old value.
The single-node operation advances that copy, handles zero/one/two child cases,
transplants the successor when required, swaps colors, updates parent/root and
extrema links, and performs the native red-black deletion fixup. Payload cleanup
and node release precede the conditional decrement of the current count.
The returned iterator stores owner before node.

These are new C++ service APIs over actual native storage. Original subtree ABI
is ECX tree plus one stack node (RET4); payload destructors take ECX self (RET).
Range ABI is ECX tree, stack output and two owner/node iterators, EAX output,
RET14h. Supporting single-node erase uses output and one iterator, RET0Ch.
Names describe established behavior and remain hypotheses, not recovered symbols.

Valid owned trees and forward ranges are required. Release callbacks must not
mutate topology or source iterator storage. The existing source string, CRT and
exception domains retain their boundaries. Native FH3/private-stack aliases,
original exception ABI, malformed-tree and hardware-fault behavior are unvalidated.

## Evidence and validation

The saved Ghidra bodies of 0055B690 and 006A2BB0 previously stopped at their first
returning free. They now cover 534 and 143 bytes. Four single-node erase bodies
are also complete: 006A71B0 840 bytes, 006A6010 691, 0069F690 679, 006A04A0 716.
The 0055AF60 nested-tree destructor also had a node-free instruction outside its
saved ownership; its full 102-byte body is now owned. All decoded instructions
in these seven repaired bodies were checked against their actual function owner.
Only proven call-site flow overrides were cleared; callee no-return flags and
script capabilities were unchanged. Supported recreation preserved prior
evidence and labels, recorded old signatures and restored observed register/stack
ABIs. The receipts retain the initial incomplete repair attempt and later success.

The retained isolated native fixture matches 122,545 settings-lifetime values,
1,448 vector-storage values and 335 checked-string values. One added scenario
matches 309,711 values while erasing roots, interior nodes, minima, tails, empty
ranges and full ranges across five populated settings trees. It compares all
remaining topology, parent/extrema links, counts, colors and nested payloads,
then performs complete destruction. Pooled allocation/release counts agree,
with no remaining pooled strings. Constructor-failure cleanup and the actual
raw singleton-manager drain pass using source tree cleanup.

An initial source fixture failure exposed the incorrect use of nil byte 11h for
the nested integer/scalar tree at device member 50h. The 00554F10 body establishes
15h; correcting that layout makes the unchanged comparison pass. Strict MSVC
Win32 compilation and both existing CTests pass. Exact source, fixture, native
byte, call ownership and artifact hashes are recorded in
`reports/native_input_settings_tree_cleanup.json`. Fixtures operate in separate
manifested processes and do not interact with the running game. Production
lookup/insertion providers, application wiring and gameplay validation remain.

All 33 checked byte envelopes match the live program and installed executable.
All 211 direct CALL rows pass the live instruction, callee and ownership audit;
none are excluded for missing ownership.
