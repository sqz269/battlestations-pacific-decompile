# Actual scalar-tree iterator entries

The new whole ordinary APIs are
`increment_native_input_scalar_tree_iterator_005531f0` (005531F0..00553252,
99 bytes/38 instructions) and
`erase_native_input_scalar_tree_iterator_00556dc0` (00556DC0..00557066,
679 bytes/231 instructions). They directly reuse accepted complete ordinary
services `increment_native_int_pointer_tree18_00869a20` and
`erase_native_int_pointer_tree18_iterator_0086e8a0`, without a callback,
provider, copied algorithm or bounded STL substitution.

Descriptive names are hypotheses. Retain `STL_TreeIter_Inc_005531f0`.
The old `STL_xlen_throw_00556dc0`/throw-site label is disproven by the complete
normal erase path and RET0Ch; the proposed library-shaped name is
`STL_IntScalarTree_EraseIterator_N18_00556DC0`. Its previous name and undefined
prototype are recorded before primary annotation. Worker Ghidra is read-only.

## Complete reuse proof and actual storage

The entire 99-byte increment matches accepted 00869A20 after ONLY two proved
E8/E9 displacement fields, both targeting actual BF6713. The entire 679-byte
erase matches accepted 0086E8A0 after ONLY thirteen proved E8 displacement fields
and its own original EH-handler immediate. Every other byte is equal, including
all branch encodings, field offsets, message/count/vtable/ThrowInfo constants,
normal frame offsets, output order and RET0Ch. Full raw spans, masks, targets and
unmasked bytes are retained in the report. Fresh owned bytes and all 269 current
instruction starts match disk/Capstone; the accepted original image and six
actual service-file hashes are reverified unchanged.

Transitive native helpers are complete exact matches: 00552FB0 maximum versus
008697B0 (28-byte envelope, 25 typed body bytes plus 3 alignment bytes),
00552FD0 minimum versus 008697D0 (27 bytes), 005530D0 left rotation versus
0086A2F0 (78 bytes), and 00553120 right rotation versus 00869810 (82 bytes).
The accepted ordinary service already uses the corresponding actual Source
helpers. No new native helper implementation or provider is needed.

Current complete producer 00554EC0 requests 18h, writes links0/4/8 and
color14h/nil15h; constructor 00555540 publishes head at header+4, sets nil1,
self-links and zeroes count+8. Header+0 is opaque. Nodes have links0/4/8,
unowned key/value words+Ch/+10h, color14h/nil15h and padding16h/17h.
Checked iterator storage is owner0/node4. These entries do not read/release
payloads. Existing private AV Kind::defaults helpers and provider-bearing
range APIs remain unchanged; they are not public exact ordinary entry APIs.

## Physical arguments, mutation and cleanup

Native increment consumes ECX pointing to the actual 8-byte iterator, incoming
EDX is unused, no stack inputs, plain RET, no semantic EAX result. Owner null
reports through the actual returning diagnostic, then CURRENT node is reread.
Nil current node restores ESI and tails that service. Right-subtree minimum or
parent climb updates iterator.node only. The new fastcall API reserves unused
EDX; Source diagnostic continuation is the accepted SDK policy.

Native erase consumes ECX tree, incoming EDX is unused, and THREE stack words:
output, input_owner, input_node. It returns output in EAX with RET0Ch. It does
not add owner==tree validation. Its actual by-value iterator argument slots are
incremented; the original node is separately captured. Zero/one/two-child
transplant, extrema updates, color exchange and both rebalance directions are
preserved by the accepted ordinary service. The original node is freed, CURRENT
unsigned count is reread/decremented only if nonzero, CURRENT argument owner/node
are captured, then output.owner and output.node are stored. No payload cleanup.

## Actual exception and EH scope

Nil input builds no-init SBO with cap15/len0/NUL, assigns exactly 27 counted
bytes `invalid map/set<T> iterator`, and ONLY THEN arms state0. Actual 00411700
constructs the 28h legacy payload; D6926C profile and D863A8 ThrowInfo are the
same unmasked constants in both original bodies. Existing owning
`NativeHardwareLayoutInvalidIterator` retains actual copy00441760 and
destroy004412B0, plus completed temporary004072D0 cleanup. There is no generic
`std::out_of_range` replacement or new throw metadata.

Both full native FuncInfo structures have magic19930522/maxState1/zero try
blocks/EHFlags1, matching all fields except the proved map pointer. State0 has
previous=-1 and an action LEA ECX,[EH EBP-50h] followed by actual004072D0.
Handlers C6D7F8/C95BB8 select their own corresponding FuncInfo then BF6B43.
Actions, maps, handlers, message and shared ThrowInfo bytes are captured and
live/disk-equal. Source uses the accepted C++ owning transport/cleanup, with
new host RTTI/catch/exception ABI; it does not install native FH3 or promise
hardware-fault cleanup. Native increment has no local EH; erase has one
temporary unwind state and no native catch scope. No extra action is leased,
reconstructed or annotated in this packet.

## Decompiler correction and limits

Both live prototypes were undefined(void)/zero parameters. Decompiler output
incorrectly removes the reachable 00556ECF..00556F23 two-child path: native
00556E36 points ECX at stack owner/node, 00556E3E increments node at ESP+70h,
00556E5D reloads it and 00556E66 conditionally enters that path. Typed metadata
shows no flow overrides, normal fallthrough and the correct branch target.
Missing argument-write modeling explains the false constant-node result
(inference). Complete body ranges and instruction starts are already correct;
no Source or listing repair is needed. Primary handles future locked prototype,
name and evidence annotation.

Actual reached storage must support the observed accesses and continuation
after returning validation. Current SDK/CRT/owning C++ services apply. Source
private argument frames and by-value slot addresses, volatile-register residue,
original FH3/RTTI/catch/heap identity, hardware faults/SEH, original binary ABI
and game execution remain unverified. No x87 arithmetic occurs. Whole 005572F0
range erasure is outside this packet and is not promoted. Existing differential
seeds do not execute these APIs; no new tests are added.

## Exact native transfers

Increment: `005531f8 CALL 00bf6713`, `00553207 JMP 00bf6713`.
Erase: `00556e04 CALL 00408720`, `00556e16 CALL 00411700`,
`00556e2d CALL 00bf6885`, `00556e3e CALL 005531f0`,
`00556ea3 CALL 00552fd0`, `00556ec5 CALL 00552fb0`,
`00556f63 CALL 005530d0`, `00556f92 CALL 00553120`,
`00556fac CALL 005530d0`, `00556fc3 CALL 00553120`,
`00557007 CALL 005530d0`, `0055701f CALL 00553120`, `0055702c CALL 00bf65ac`.
Native EH context: `00c6d7f3 JMP 004072d0`, `00c6d7fd JMP 00bf6b43`.

The exact MSVC Win32 Source build and three existing suites passed in 8.38
seconds. All 17 native transfer rows and 17 Markdown claims passed with no
unchecked claims or warnings. Emitted COFF contains a 5-byte increment tail
transfer and a 9-byte erase entry that restores its EBP prologue before tail
transfer; original argument slots remain in place. Actual service symbols and
raw unlinked bytes/provenance are recorded. This static binding inspection does
not establish native ABI or game execution. Primary frozen-pin review is pending.
