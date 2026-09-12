# Native physical-directory index tree lifetime

Nine complete raw-storage bodies are reconstructed in `src/native_physical_index_tree.cpp`.
The range operation includes partial erase, successor transplantation and both red-black
fixup directions. No typed container substitutes for the actual node links. The source
interfaces take explicit existing string-storage and returning-CRT dependencies; original
FH3/RTTI transport and binary replacement compatibility remain unclaimed.

| Entry | End exclusive | Bytes | Original ABI | Coverage |
| --- | --- | ---: | --- | --- |
| 00489D50 | 00489DC6 | 118 | ECX pair, RET0 | complete storage behavior; throwing getter boundary below |
| 00BD9380 | 00BD939C | 28 | ECX node, EAX maximum, RET0 | complete |
| 00BD93A0 | 00BD93BB | 27 | ECX node, EAX minimum, RET0 | complete |
| 00BD93E0 | 00BD9432 | 82 | ECX tree, stack node, RET4 | complete right rotation |
| 00BDA090 | 00BDA0DE | 78 | ECX tree, stack node, RET4 | complete left rotation |
| 00BDF7A0 | 00BDF7DD | 61 | ECX tree, stack root, RET4 | complete right recursion and left-chain loop |
| 00BDFD80 | 00BE0033 | 691 | ECX tree, output/owner/node, RET0C, EAX output | complete including successor transplant and tail |
| 00BE0C30 | 00BE0CF9 | 201 | ECX tree, output/first-owner/first-node/last-owner/last-node, RET14, EAX output | complete full and partial ranges |
| 00BE1700 | 00BE1734 | 52 | ECX tree, RET0 | complete including head/count clearing |

Names are descriptive hypotheses. `STL_xlen_throw_00bdfd80` is an incorrect saved
classification: the conditional sentinel throw is only its prefix. The stored 637-byte
body ends immediately after `CALL _free` at BDFFF8. Its reachable 54-byte tail reads
current count, conditionally decrements it, publishes the advanced iterator and returns.
The report records the old names; workers did not mutate Ghidra.

## Raw layout and order

Tree+4 is the current head, +8 the unsigned count; tree+0 is untouched. Nodes use
left/parent/right at +0/+4/+8, a pair of eight-byte pooled strings at +C/+14,
color at +1C (black=1), nil at +1D. The constructor producers BDAFA0/BDB450
publish the BDABA0 allocation at tree+4, mark nil, self-link the head, then clear
count. BDABA0 independently writes three links, black=1 and nil=0 in a 20h
allocation. BDCC60 establishes the two length/data string headers. The existing
physical index reader and iterator agree with these offsets.

489D50 reads second-string data before its length, releases it, then reads the
current first-string data and length. It never clears either header. BDF7A0 recurses
right, reads the current left only after recursion returns, then destroys the pair,
frees the original node and iterates the captured left.

BDFD80 captures the original node before advancing its by-value checked iterator.
The advanced successor supplies the transplant node for two nonnil children.
BDFE8F-BDFEE5 was removed from pseudocode as unreachable; assembly establishes the
whole successor transplant, replacement parent choice and color swap. A nil
replacement does not get a parent write. Both balancing directions preserve the
native current sibling/head reloads and color order. Pair cleanup/free precedes
the current unsigned count read. Only nonzero counts decrement. Output owner is
written before output node; no destination-tree/input-owner equality check is added.

BE0C30 captures minimum before validating the first owner, and captures head before
validating the last owner when the first node equals minimum. A returning invalid
handler does not restart these comparisons. The full branch destroys current root,
then reloads head separately for root/minimum/maximum reset; count clears between
the root and minimum writes. The partial branch advances first before erasing the
captured old owner/node and then reloads the advanced first iterator. BE1700 frees
the current head after full erasure, then zeros head and count.

## Calls and exception schedule

All 29 direct call sites carry their stored containing-function proof and callee ABI
in `reports/native_physical_index_tree.json`. Existing BD9860 is the actual raw
iterator, not a map iterator. Min/max and both rotations are implemented here.
`singleton_lifetime_free` is the established malloc-paired CRT service.

The original pool getter 419CC0 takes no arguments (RET0, EAX pool). The three
already-pushed words belong to BD1510 (ECX pool, block/size/unused, RET0C).
Production binds `NativeStringStorage&` to `ActualNativeStringPoolStorage` so every
nonnull release repeats the actual publication/getter and return behavior.

Pair handler C62988 loads FuncInfo D8A4A8; max-state=1, map D8A4A0 maps state0 to
C62980 (ECX captured first-string header, tail 41DD20). State0 is armed before
second-string release and disarmed before first-string release. A throwing second
getter originally cleans first string. Existing `NativeStringStorage::release` is
noexcept: C++ failure from a lazily recreated production pool terminates instead.
This source boundary is explicit; native throwing-getter EH is not fixture-tested.

Erase handler CC66A8 loads E00C98; max-state=1, E00C90 maps state0 to CC66A0,
which destroys the completed temporary at EBP-50 using 4072D0. State0 is armed
only after 408720 returns from assigning the 27-byte counted message. 411700
constructs the logic-error owner, then the prefix publishes D6926C and calls
BF6885 with D863A8 throw metadata. Source preserves that arming point and uses
the existing owning `NativeHardwareLayoutInvalidIterator` (native 28h storage,
441760 copy and 4412B0 destruction), with host exception RTTI/ABI. No synthetic
CRT global, private native handler or borrowed exception object is introduced.
The normal erase path has state=-1, so failure during pair cleanup originally
neither frees the node nor decrements count.

## Evidence and validation

Each CLI batch verifies `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
x86 language and image base 00400000. Installed PE remains unchanged at
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
19 fresh full-span live/disk comparisons cover 1,616 bytes, including all nine
owned bodies (1,338 bytes), the existing iterator/allocation producer and eight
handler/map spans. Hashes and exact boundaries are in the report.

Stored Ghidra flow gaps remain: BDF7CC-BDF7D7, BDFFFD-BE0033, BE1724-BE1734
(end exclusive). Their initial instructions are respectively ADD ESP,4; MOV
EAX,[EBP+8]; ADD ESP,4. BDFD80's stored function ends at BDFFFC; the recovered
tail has no containing function. The source does not extend that body or change
_free's annotation. BD938D-BD9390 and BE0CAF-BE0CB0 are padding, not flow gaps.

Strict MSVC Win32 /W4 /WX /fp:strict build passed with process-scoped
MSBUILDDISABLENODEREUSE=1. After the eight seed byte checks, both existing CTests
passed. The focused fixture passed eight original/source scenarios with 1,469
normalized DWORD observations, plus source sentinel exception ownership. All
1,770 exact source/header/object/library/probe/native-span inputs were frozen
before its first execution and rehashed afterward: zero changes. Both compile/execution revisions succeeded; the second removes only a trailing
blank source line reported by git diff --check. Both frozen inputs and logs are
retained. Current manifest SHA256
`02128aba0af7783adc0d2ecb89f47c8e5fa575ab67d50ab3374acd8ceb85757a`.
The report records literal source/object/library/probe hashes and ignored paths.
The ignored probe copies the ten guarded original normal-path spans into a private
process and redirects only pool getter/return, free and returning invalid-parameter
entries. It compares raw balanced deletion/successor, both range modes, head reset,
current first-string/count reloads and result ownership. It also checks source
sentinel exception ownership without running the original FH3 throw path. These
checks do not claim native exception ABI compatibility or game execution.
