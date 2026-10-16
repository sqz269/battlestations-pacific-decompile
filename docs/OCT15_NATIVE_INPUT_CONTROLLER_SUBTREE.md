# Controller tree subtree destruction and clear

Whole `006A62D0` has61bytes/25instructions, including RET4. Whole `006A6650`
has43bytes/16instructions. All104bytes/41starts and four transfers match fresh
PE/live evidence. The earlier24-instruction handcount was corrected before
Source authorization. Descriptive C++ names remain hypotheses; existing library
names are retained. Pair dependency is published at Main85938d17b.

Subtree uses fixed nil21 and links0/4/8. It forwards the same opaque tree into
right recursion. Only after right recursion returns does it capture CURRENT
left0, then destroy the actual pair at ORIGINALnode+C. Only successful pair
completion permits original-node free and iteration into the captured left.
There is no count/head update, node guard, cycle check, local EH or rollback.
Failure retains completed right-subtree progress and the child's own key unwind
semantics; it prevents current-node free and further left traversal.

Clear captures initial head/root and calls the actual subtree. On success it
reloads headA and sets root=self; captures headB BEFORE count0 and left=self;
then reloads headC AFTER left publication and sets right=self. It never frees
the head or writes opaque0. No null, identity or ownership guard is invented.

| Native transfer | Target |
| --- | --- |
| `006A62E7 CALL 006A62D0` | right subtree |
| `006A62F1 CALL 006A2BB0` | whole actual pair |
| `006A62F7 CALL 00BF65AC` | original node free |
| `006A665C CALL 006A62D0` | captured root |

All actual CMake code was reviewed:190bytes/86physicalinstructions across7
sections, four code REL32 and no local EH. Public subtree76B/34 retains original
nodeESI, leftEDI, RawCtxEBX and opaque treeEBP-4. Left capture+22 follows recursive
call+1D, precedes pair+2A and free+30. It uses RET8 rather than NativeRET4.
Public clear52B/20 reloads heads+17/+1D/+29 around root+1A, count0+20, left+27
and right+2D; SourceRET4 differs NativeplainRET. EBP is compilerframe throughout.
The five helpers contribute62B/32. Semantic void qualifies incidental NativeEAX;
the explicit borrowed RawCtx and canonical unusedEDX0 are new Source interfaces.

Default MSVC Win32 Release passed three existing checks in9.23seconds. No new
tests/probes or Native execution occurred. Fourteen primary checks include
40 published provider LF comparisons. Those existing suites exercise older
math/tool behavior, not these routines against Native game state. CurrentCpp,
borrowed RawCtx/provider ownership, current CRT, private frames, flags, faults,
heap, FH3/SEH/RTTI, binary ABI and game equivalence remain unproved. The pair
and its populated AV/typed projections remain unchanged. Controller parent,
range, member and configuration owner are outside this packet.
