# Actual preset-key subtree

Complete native entry006A0770 spans82bytes/34current instruction starts. It
destroys right subtrees before each current key/node, then continues left.
The2Ch layout uses nilBYTE29; mapped payload bytes are untouched. Existing
typed/AV projections remain separate.

After right recursion, capture key pointer first and left pointer second.
Only a nonnull captured key reads current length+1 with DWORD wrapping, then
resolves the actual raw pool and returns that captured key. Getter/pool effects
cannot change captured left. Free current node before reading left's nil byte.
No key clear, head/count reset, mapped destructor, null-node guard, rollback,
local catch, callback or storage adapter is introduced.

Actual Native transfers:

006a0787 CALL 006a0770
006a079f CALL 00419cc0
006a07a6 CALL 00bd1510
006a07ac CALL 00bf65ac

The new Source fastcall retains tree/unusedEDX/node and adds borrowed RawCtx:
SourceRET8 differs from NativeRET4. Context borrows actual publication/shutdown
cells, owning no storage. Unused recursive EDX is explicitly canonical zero.
Getter failures escape before current-node free; actual pool-return/free retain
their current noexcept boundaries. Prior node/pool effects are not rolled back.
Original CRT/privateframes/FH3/SEH/RTTI/fault/register/binaryABI and gameplay
identity remain unproved. Fresh PE/live82bytes, all34current starts, four
transfers and seven current provider fingerprints are recorded in the report.
Independent source review and optimized Win32 object review passed: the Source
core is220B/68instructions with ten focused emitted-code checks. Full CMake
registration/build and Main integration remain pending. No new tests are added.
