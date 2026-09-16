# Native renderer member-array cleanup

These parent member cleanups operate on actual 0Ch headers: current data +00,
signed count +04, signed capacity +08. Cells are DWORD pointers. They do not
inspect pointee layouts, destroy records, or release engine references.

| Cleanup body | Complete range | Resize body | Reserve leaf | Parent header |
| --- | --- | --- | --- | --- |
| B29B60 | B29B60..B29B76, 23 bytes | B25CF0..B25D3F, 80 bytes | B22DD0..B22E2E, 95 bytes | +1AC4 |
| B29B80 | B29B80..B29B96, 23 bytes | B25D40..B25D8F, 80 bytes | B22E30..B22E8E, 95 bytes | +1AD0 |
| B28090 | B28090..B280A6, 23 bytes | B22E90..B22EDF, 80 bytes | B22650..B226AE, 95 bytes | +1ADC |

The complete reserve bodies are byte-identical to existing actual B22D10 after
masking only their two CALL rel32 operands; both allocator/free targets are the
same. Each complete resize is likewise identical to B25940 after its one CALL
operand is masked, and each cleanup is identical to B29B20 after its two CALL
operands are masked. Twelve complete spans, 792 bytes, were freshly compared
between live Ghidra memory and the installed PE before making those comparisons.
The evidence records each original range, bytes, hash, and grouped identity.

The new native-address entries therefore share the existing substantive raw
pointer-array chain from commit `556afdb28f03182d0ba3425e2a7652454adf69cf`.
Private dependency merge `5cde3e0207cb0fbca690e827244b2225191f6e59` preserves that
commit's original ancestry; the packet did not cherry-pick or modify its source.
No resize/reserve/storage algorithm or projected container was copied here.
Six owned entries (three resizes and three cleanup bodies, 309 bytes) remain
distinct from the reused reserve functions and their existing ledger records.

Existing shader reserve APIs operate on raw headers but require retained
diagnostic operation objects: their failed destructors terminate until explicitly
retired. The third-state cache reserve view adds host extent/overflow checks and
an unpublished-allocation cleanup path. Those APIs remain unchanged. Sharing the
instruction-equivalent raw chain preserves these native entry points' storage
and failure behavior without adding either source API's extra obligations.

Reserve clamps signed capacity requests to one, grows on signed comparison,
allocates the wrapping request*4 size, and copies current count cells while
reloading current base/count. Its per-destination null check remains. It frees
current old backing before publishing replacement data and capacity; count is
unchanged. Allocation failure propagates before later resize/cleanup work, and
retains earlier allocation-handler effects without added rollback.

Resize reserves if signed requested count exceeds current capacity. It captures
current count after reserve, then zero-initializes newly exposed cells using
current base plus the captured index on each iteration. It skips a null computed
destination and increments the index without publishing count per cell. Next it
repeatedly decrements current count while greater than the request, then writes
the requested count. Removed cells retain their bits. Negative initial indices
are preserved, subject to valid reachable storage and wrapping-address preconditions.

Cleanup calls resize0 and only then loads current data for free. Pointer and
capacity retain their stale postimage; the header is not freed or reset. If a
negative capacity causes reserve to publish a new allocation, cleanup frees that
current allocation. A reserve exception prevents its final free. No member-level
catch, rollback, pointee release, or source diagnostic retirement is introduced.

Read-only parent actions CBDF9C/CBDFaa/CBDFB8 load the captured owner from EBP-14h,
add +1AC4/+1AD0/+1ADC and tail-jump to B29B60/B29B80/B28090 respectively. Parent
FH3 state assignment and unwind composition remain integrator-owned.

The cleanup listings stop at their free calls B29B6D/B29B8D/B2809D. Returning-call
overrides must recover the five-byte tails through B29B76/B29B96/B280A6. The
three resize and three reserve listings are complete. The worker made no Ghidra
mutation; definitions, overrides, names, preserved old values, save and refreshed
exports remain integrator-owned after lease release.

Validation is recorded in the report: eight verified seeds before configuration,
strict Win32 build with both CTests, and one focused original/source fixture.
The fixture executes all nine original bodies in their three chains, rebinding
only CALL operands between relocated bodies and the actual shared allocator/free.
Twenty-one resize/cleanup comparisons cover growth, shrink, valid negative-index
initialization, null destination skip, negative-capacity replacement, empty/null
storage and stale-header postimages. The source side uses the built library.
No original EXE address is called. Failure ordering is static/source evidence;
allocation failure is not injected, and native CRT/private-frame/SEH identity
and game behavior remain unvalidated.

Immutable evidence includes source, library, fixture executable/object/reference,
tool executables, and logs. Actual loaded x86 runtime DLLs are recorded separately,
with canonical paths resolved through handles opened in the fixture process and
machine identity read from loaded images. Subsequent disk/archive hashes are not
mapped-image hashes. Observed tool executables do not cover every dependency.
