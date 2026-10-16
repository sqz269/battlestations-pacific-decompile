# Actual input int/string subtree destruction

Whole `006A0AA0..006A0AF1` is 82 bytes and 34 current instruction starts.
The existing STL `_Tree::_Erase(_Nodeptr)` identity is retained; the int-key
instantiation name remains a hypothesis. Actual 1Ch nodes have links at 0/4/8,
the mapped pooled-string length/data at 10h/14h, and nil BYTE19h. Existing real
typed/AV cleanup and the different-value-offset scene subtree remain unchanged.

Test current nil19, recurse CURRENT right8, then capture CURRENT string data14
BEFORE CURRENT left0. Only nonnull captured data reads CURRENT length10, adds
one with DWORD wrap, calls the actual419CC0 getter, and returns that captured
block through actualBD1510. The getter still runs before large/free or disabled
small-return branches. Free original current node, then test captured left's
nil and continue there. No data/length reread through41DD20 is substituted.
There is no header clearing, tree count/reset, null/cycle guard, rollback,
callback, storage provider, local EH or noexcept promise. Getter failure escapes
after completed right-subtree effects, before current-node free/left traversal.

Actual Native transfers:

006a0ab7 CALL 006a0aa0
006a0acf CALL 00419cc0
006a0ad6 CALL 00bd1510
006a0adc CALL 00bf65ac

Native ECX is opaque tree bits, only forwarded to recursion; EDX is unused;
stacked node, RET4; no semantic result beyond EAX residue. The new physical
fastcall Source retains those register slots and adds borrowed RawCtx after
node, yielding Source RET8. Recursive unused EDX is canonical0, with Native
ambient-register/private-frame identity explicitly excluded. The context borrows
the real pool publication01090AA8, small-return gate01090AA4 and manager
publication01090AA0. It owns no storage. Provider files match accepted published
Main `d607aaa2510cbb6943d425bf278e291c46d5c9b1`.

Fresh PE/live bytes and all34 current starts agree. Bounded caller proof shows
the actual range producer loads CURRENT head/root, pushes root, and forwards
the tree in ECX. Its body and the pair/controller/range Source remain outside
this packet. Correct library provenance and the frozen readiness are preserved.

Default `./scripts/build.ps1` MSVC Win32 Release passed all three existing suites
in13.91s; no tests were added. The differential suite covers seeded math, not
this subtree. Full actual CMake COFF review covers116B/48 public instructions,
153B/67 total instructions including three private helpers, and all four REL32
relocations. Its own EBP frame and RET8 are verified. The optimizer defers the
captured-length increment until after the getter while retaining the earlier
length read; no storage read/call order changes. No independent ad hoc probe
was compiled and no probe byte-equivalence claim is made.

Current C++ propagation, current CRT allocation/free and Windows pool services
require valid reached storage and compatible allocations. Source/build/static
proof does not establish original binary ABI, EAX/flags/register residue,
private-stack aliasing, FH3/SEH/RTTI/fault delivery, application adoption or
game validation. The report records the fresh byte/start, provider, build,
compiled-code and transfer evidence. Ghidra mutation and publishing are left
to the primary integrator.
