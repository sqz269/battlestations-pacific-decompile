# Actual input int/string tree leaves

This packet reconstructs five complete fixed-nil19 entries: successor0069AE50
99B/38starts, right rotation0069A780 82B/30, left rotation0069C300 78B/30,
maximum0069AAC0 25definedB/10, and minimum0069AAE0 27B/10. Total311definedB
and118current starts. Maximum has a28B envelope; its unreachable3B at
0069AACD..CF remain undefined and unpromoted. Existing STL identities and
int/string instantiation hypotheses, old typed/AV domains and other fixed
nil15/99 entries are preserved. No field-offset or callback bridge is used.

Actual raw links are0/4/8 and nil BYTE19h; tree head is+4, iterator owner/node
are0/4. Successor checks CURRENT owner, calls the accepted diagnostic ifzero,
then reloads CURRENT node after a returning handler. Nil node diagnoses and
returns without advancing. Otherwise it selects the right subtree's minimum
or climbs parents, rereading CURRENT iterator.node before each right-child
comparison and publishing each matched parent before reading its parent.
Only iterator.node is written. There is no receiver-owner equality check.

Actual Native transfers:

0069ae58 CALL 00bf6713
0069ae67 JMP 00bf6713

The other four entries are callee-free. Each rotation captures its pivot,
stores the first pivot child into the node, then REREADS pivot child before
the nil19 test and conditional child-parent store. It publishes pivot.parent,
captures CURRENT head once, tests root, then reloads node.parent for the
direction-specific first side test. Final pivot-child and node-parent stores
remain ordered. These are separate right/left bodies, with no direction model.
Maximum/minimum repeatedly read CURRENT right/left and that child's nil19;
they return the actual selected node without an initial-node guard or writes.

Source preserves the physical fastcall slots: successor ECXiterator/unusedEDX
with no stacked inputs; rotations ECXtree/unusedEDX/stacknode andRET4; min/max
ECXsubtree/plainRET/EAXnode. Source void qualifies incidental Native EAX for
successor/rotations. The successor's nil diagnostic is expressed as call and
return; emitted-code evidence records its actual transfer/frame shape rather
than assuming the Native tail encoding.

All raw DWORD/BYTE accesses are volatile. The only external operation is the
accepted actual invoke_native_invalid_parameter_00bf6713 service, pinned to
published Main4d758e463a636d5be69254bf4abdbf0c1b8746e0. It invokes the
current SDK handler and may return or throw. It does not recreate Original
encoded-handler selection, Watson/debug delivery, fault addresses or private
frames. No callback, provider, allocation, new guard, rollback, local EH or
noexcept bridge is introduced. Every reached access requires valid backing.

Fresh PE/live311definedB/118current starts and16B/9diagnostic-provider starts
agree; padding and the complete typed bodies were checked independently.
The frozen readiness's six full-envelope comparisons remain preserved:
corresponding accepted tree18 leaves differ only in nil19 versus nil15 and
resolved diagnostic rel32 fields. This establishes the new fixed-entry need,
not permission to delegate to those different-offset services.

Source/build/compiled-code evidence is qualified separately from original
binary ABI, incidental EAX/EDX/flags, private-stack aliases, CRT/FH3/SEH/RTTI
and hardware-fault identity, application adoption and game execution. No
716B eraser, range, pair or cleanup-owner Source is included. The report records
the final default Win32 suites, actual CMake object and transfer checks.

Default `./scripts/build.ps1` MSVC Win32 Release passed all three existing
suites in12.70s; no tests were added. Existing native differential seeds cover
math, not these leaves. The actual CMake object was fully reviewed:394B/168
instructions, including332B/136 across the five public functions, all2code
REL32 transfers and3compiler `.debug$F` relocations. Eighteen compiled-code
checks passed. It emits the successor's restored-ESI diagnostic tail and each
rotation's threeRET4 arms. An independent object probe has identical successor,
min/max bytes; rotations and private helpers differ in compiler EBP frames.
Their actual CMake read/store order was reviewed separately. No executable
probe or Native binary/game comparison is claimed.
