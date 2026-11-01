# Normal raw Icon state range-copy wrapper proposal

AB58A0 is37B/14instructions, one AB4C50 call and no local EH. Original three
stacked DWORD addresses plus entryECX/tag/RET0Ch become this ordinary six-argument
Source interface. It explicitly borrows a genuine entry-ECX bit value, ordinary
tag DWORD, three incoming homes, four mutable callee homes and the existing
retained20B RangeFrame/context. No new Frame, allocator, owner, callback or
invented constant/preimage is introduced; this is not a Native register hook.

Native order matters even for the currently unused callee homes. Full tag HOME
gets entryECX. Read incoming destinationHOME2, then overwrite only tag LOWBYTE0
and read its full DWORD. Store that masked tag in calleeHOME3. Reread incoming
destinationHOME2 separately; store the FIRST destination capture in calleeHOME2.
Read incoming endHOME1, store ORIGINAL entryECX in calleeHOME1, read incoming
beginHOME0, then store the SECOND destination capture in calleeHOME0. Do not
collapse the two destination reads or send the low-byte-masked value as home1.
All32 input/tag bits and reached ordinary/home alias behavior remain material.

Directly invoke the frozen genuine AB4C50 NORMAL primitive with captured begin/
end and those SAME four homes/RangeFrame/NativeGuiIconStorageContext. The callee
may overwrite its home0 and live end home; the wrapper neither copies output
back to the outer destination home nor rereads outer inputs. Return the callee's
numeric destination result unchanged. Original POP-ECX/private stack restoration
is excluded ABI evidence, not an exposed Source register or ownership receipt.

The dependency is exact corrected AB4C50v2, not a forward declaration/stub or a
semantic stdvector range operation. Its implementation is unchanged from v1;
the v2 qualification permits declared operand-home writes separately from
private Source controls/spills. Its distinct actual records, individual no-write
birth, old raw AB3A70 pool/string/texture/FP domain and current end-home behavior
remain inherited. No stronger BF0/overlap/string/Original contract is inferred.
The13 Native catch rows/prefix destruction/rethrow remain held; a wrapper cannot
authorize those actions on arbitrary Source escape. Retain every partial/
completed/remaining graph, tag and both home sets/frame/providers on UNKNOWN.

Ordinary live operand objects may receive their genuine callback writes and
legal C++ aliases at the reached epochs. Private Source controls/spills are
disjoint from these operands and provider-write extents. Distinct active entries
need distinct retained backing. No catch, cleanup, retry/rollback/noexcept,
terminal, reset, count/depth cap, Native Origin or normal-disposition policy.

This is a real prerequisite of AB3310 currentA4/AB66C0 -> AB63A0 -> AB6070 ->
AB59E0 growth. Whole insertion, Native cleanup, Page Type6/current18/current78/
current80/custody and activation remain separate. Existing HIGH STL/vendor tags
and prior body credits remain unchanged; new Native/Original/vendor/helper/CG/
duplicate credit is zero. Ignored support only, no tracked Source/CMake/ledger/
Ghidra mutation, build, tests, probes or Game execution.
