# Actual physical provider ownership

Addresses: 00bf4d30 00bf4df0 00bf4c70 00bf4dd0

The actual provider constructor, factory, destructor and deleting wrapper now use the
recovered application pool, raw string headers, pending record vector and complete
physical-index tree cleanup. They preserve the earlier typed projection as a separate
record. Descriptive names are hypotheses, not recovered symbols.

| Entry | Complete logical bytes | Original ABI | Source behavior |
| --- | ---: | --- | --- |
| BF4D30 | 152 | ECX owner; system header and DWORD flags; RET8; EAX owner | Actual base, pending/cache headers, low flag byte,20h sentinel |
| BF4DF0 | 187 | Incoming factory ECX unused; system/virtual headers; RET8; EAX slot/null | Validate recorded final backslash, compare persistent_data, acquire/construct same3Ch pool slot |
| BF4C70 | 190 | ECX owner; RET0 | Complete index erase, current sentinel free, cache return, pending resize/free, base destruction |
| BF4DD0 | 32 | ECX owner; flags; RET4; EAX original slot | Always destruct; flag bit0 returns the slot |

## Storage and ordering

BF4D30 calls the existing actual BB5590 base constructor and writes profile D69168.
It zeroes pending14/18/1C and cached string20/24, copies only the low flag byte28,
and preserves padding29..2B, allocator state2C and pool metadata38. BDABA0 allocates
the actual20h sentinel, published at30; nil1D becomes1, each self-link reloads the
current head, and count34 becomes0. The apparent diagnostic4254B0 is exactly RET.

BF4DF0 checks nonzero unsigned recorded root length and only its final byte5C. It
does not normalize the root or test disk existence. Actual425850 compares the virtual
header with CFF208 persistent_data without case sensitivity; the constructor receives
1 or0. The context borrows the existing BF34D0/BF2FC0/BF3200 pool and application
AllocatorListDomain. It introduces no private pool or publication.

BF4C70 invokes complete BE0C30 with captured first/head and the actual tree owner2C.
It then frees the current head and clears head/count. Cache data is captured before
state1; its current length plus1 is captured before release. Pending storage is read
after actual BF3ED0 resize0, then freed, and current base state is passed to BB5380.
The pending destructor releases the two names per record and vector backing. It
does not cancel I/O, close handles or free OVERLAPPED allocations; external shutdown
invariants remain necessary.

## Cleanup evidence and boundaries

The complete live/installed spans include both three-state unwind maps E02A10 and
E02A4C, their six funclets, and factory map E02A88 with two same-pool return funclets.
State2 chains cache owner20 -> pending owner14 -> base; state1 starts at pending,
state0 at base. Neither map includes index cleanup. Factory constructor failure
returns its captured slot through BF3200. Normal deleting destruction returns it
through BF2FC0 only for bit0.

Production strings bind ActualNativeStringPoolStorage. The shared release interface
is noexcept, so native throwing lazy-getter identity and FH3/SEH dispatch are not
reproduced by these explicit C++ interfaces. Simultaneous cleanup failure remains
unvalidated. These are not drop-in binary replacements.

## Verification

All24 captured spans,1307 bytes including data and external evidence, match the live
Ghidra program and installed PE. The four owned bodies total561 unchanged bytes.
Strict MSVC Win32 compilation and both existing CTests passed. An independently
reviewed private fixture compared14 normal native/source cases and580 normalized
observations, including populated index/pending/cache ownership, current-field
mutation across release boundaries, low flags, root rejection and slot reuse.
Two source factory-failure cases and a canonical string-pool shutdown composition
also passed. All1850 frozen inputs were unchanged across that execution.

The original normal bodies run shifted in a private process; external callees are
redirected to separately reviewed source and explicit shared services. Original
FH3 dispatch and the game were not run. The report pins the complete inputs, actual
object/library/executable, independent review, and observed limits. No permanent
test cases were added.

## Ghidra metadata qualification

The locked repair cleared BF4CBA CALL_RETURN and decoded the111-byte tail through
BF4D2E exclusive. The stored BF4C70 body still ends at BF4CBE. Its five later calls
remain separately labelled raw-tail evidence; the live call-row checker does not
claim those sites belong to the stored function. No global no-return flag or
unsupported body extension was applied. See reports/native_physical_provider_flow_repair.json.

## Follow-up packets

Compose this owner with actual BF4BA0 stream opening and real local fixture I/O.
Manager base BDA6F0/BDA790, request-list ownership and complete BE1DC0/BE1F60 remain
separate prerequisites for manager/startup ownership. This packet makes no claim
that the game rebuild or gameplay validation is complete.
