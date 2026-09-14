# Native renderer capability DWORD-array cleanup

The two entries operate on an actual 0Ch header: data +00, signed count +04,
signed capacity +08. Elements are four-byte values. Neither entry dereferences
values as pointers, releases an engine reference, or performs record cleanup.

| Entry | Complete inclusive range | Native dependency | Source provider |
| --- | --- | --- | --- |
| B260B0 resize | B260B0..B260FF, 80 bytes | B236B0 reserve | existing actual B25940 chain |
| B29E40 cleanup | B29E40..B29E56, 23 bytes | B260B0 resize0; BF6989 free | existing actual B29B20 chain |

Original resize ABI is ECX=header, one signed stack word, RET4. The new fastcall
interface includes an explicit ignored EDX parameter. Cleanup consumes ECX and
returns with plain RET. These are new source interfaces: incidental register
results, original CRT/private frames and native SEH exception identity are not
claimed to be reproduced.

The existing B236B0 provider is `reserve_native_capability_dwords_00b236b0` in
`native_renderer_capability_array_reserves.cpp`. Its complete 95-byte native
body is identical to B22D10 after masking only two CALL rel32 displacements;
the allocator BF55BE and free BF6989 targets are identical. Complete B260B0
matches B25940 after its one reserve CALL operand is masked, and B29E40 matches
B29B20 after its resize/free CALL operands are masked. All other bytes were
compared, including signed branches, zero initialization and returning tails.
The report preserves exact operand masks and every decoded native target.

Six full spans, 396 bytes, were freshly verified live against the installed PE:
103 owned bytes and 293 read-only dependency bytes. The two distinct new entries
share the already proven substantive raw chain from commit
`556afdb28f03182d0ba3425e2a7652454adf69cf`. Private merge
`95d33975d39572aa2cec7978002073734ea08921` preserves that original ancestry.
Its disjoint CMake/ledger append conflicts were resolved by retaining both sets
of existing entries. No previous implementation or reserve ledger row was changed.
No algorithm, projected array, or retained diagnostic operation was copied here.

Reserve clamps requested capacity to at least one, compares signed current
capacity, and allocates the wrapping requested*4 byte size. Copying reloads
current count and source data, and preserves the native destination-null check.
It frees current old data before publishing replacement data and capacity;
count is not changed. The existing shared allocator retains its malloc,
new-handler, retry and throw behavior.

Resize calls reserve when signed requested count exceeds current capacity.
It captures current count after reserve as the initialization index. Each
iteration reloads current data, computes wrapping base+index*4, skips a null
destination, otherwise writes zero, then increments the captured index.
Count is not published per initialized cell. It next decrements current count
while greater than the request and finally publishes the requested count.
Removed cells retain their bits. Valid reachable storage and wrapping addresses
remain caller preconditions, including when an initial count is negative.

Cleanup calls resize0 before loading current data for free. The header's stale
pointer and capacity remain unchanged; it does not free the header. Negative
capacity can first cause reserve to publish fresh backing which cleanup then
frees. An allocation exception bypasses later count publication and cleanup's
final free, without added catch, rollback or diagnostic retirement.

The read-only parent action CBE052 is `MOV ECX,[EBP-10h]; ADD ECX,44h; JMP B29E40`.
The integrator's parent mapping is renderer+1B5C, implying a captured base at
renderer+1B18. The action's captured+44 form is directly verified here; the full
parent state/frame interpretation remains integrator-owned.

The B29E40 listing stops at free CALL B29E4D (saved end B29E51), omitting the
five-byte stack cleanup/ESI restore/RET tail through B29E56. The resize and reserve
listings are complete. The worker made no Ghidra mutation. The integrator owns
the returning override, names, preserved old values, save and refreshed exports.

Validation is recorded in the report: eight native seeds before configure,
strict Win32 build with both CTests, and one focused original/source fixture.
The fixture runs all three complete original bodies with only their five CALL
operands rebound to relocated children and the actual shared allocator/free.
Seven comparisons cover growth, shrink, valid negative-index initialization,
wrapping null-destination skip, ordinary and negative-capacity cleanup, and
empty/null storage. Source symbols come from the built library; no original
EXE address is invoked. Allocation failure is static/source evidence without
fault injection; native exception identity and game behavior remain unvalidated.

The immutable archive contains source, built libraries, probe executable/object,
reference bytes, tools and logs. Actual loaded x86 runtime DLL evidence is stored
separately. Paths are resolved through handles opened in the running fixture;
loaded PE machine identity is read from memory, while later disk/archive hashes
are not mapped-image hashes. Observed tools do not cover every toolchain dependency.

## Integrated validation at aa836cdd

Two distinct actual entry adapters share the fully checked raw reserve/resize/free implementation. Seven original/source comparisons passed; allocation-failure behavior remains static. The combined strict Win32 build, eight seed checks and both CTests passed.
The six final-library fixtures, 503 direct/tail audit rows, 31 saved/read-back
annotations, and 59 live/PE spans are preserved in `local/checkpoints/aa836cdd/native-renderer-parent-dependencies/validation.json`
(SHA256 `e6cbd24bb0166528f733e3075862ab1edb655ea9624b623e7cf08f6df90fdbc3`). Original CRT/FH3/SEH/private-frame identity,
full renderer lifecycle/adoption and gameplay remain unvalidated.
