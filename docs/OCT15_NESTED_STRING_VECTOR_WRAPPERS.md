# Actual nested string-vector placement and cleanup

This packet supplies separate complete actual-header Source compositions for
`00558640`, `005590C0` and `00559DC0`, with the compiler placement-delete
actions verified in full. They operate on actual16B owning headers whose
8-byte string backing is owned by +4/+8/+C. The accepted actual `00557BC0`
copy constructor and `00432440`/`004324A0` cleanup services provide the concrete
raw string pool, allocation, validation and current Source C++ exception
behavior. No host vector, allocator callback or replacement stub is added.
Descriptive Source names are hypotheses; existing names are retained.

Source: `src/native_nested_string_vector_wrappers.cpp`; public declarations:
`include/bsp/native_nested_string_vector_wrappers.hpp`. The saved analysis is
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. Complete
cached native listings and live bytes, prototypes, EH records, compiler
actions, callers and call instructions were checked through `bsp.py ghidra`.

| Entry | Native physical interface and Source composition |
|---|---|
| `00558640` | ECX slot, EDX source; no stack input, plain RET. Null skips the child. Otherwise PUSH EDX then call `00557BC0` at `0055866C` with ECX unchanged and the same source bits still in EDX. The new fastcall Source adds stacked context. |
| `005590C0` | Entry ECX/EDX are unconsumed; stack slot/source, RET8. Load slot into ECX, skip null, otherwise stack source and call `00557BC0` at `005590F2`; incoming EDX remains untouched. Source retains the two physical register words and adds context after the original stacked pair. |
| `00559DC0` | Entry ECX/EDX unconsumed; stack actual header, RET4. Adapt the stack header to accepted actual `00432440`; Source retains the physical register words and adds stacked context. No semantic return is supplied. |

Both placement wrappers preserve the slot and source addresses; there is no
source snapshot, caller-side zero initialization, self-copy guard, validation
or old-buffer cleanup. The constructor owns those established semantics.
A null slot skips all source/header/pool accesses, including the source
pointer read through a header. Exceptions from the child propagate through
the current Source C++ domain; neither wrapper invents failed-slot destruction
or a second owning-header cleanup.

Native EAX was inspected separately from inferred semantics. On a nonnull
ordinary return, the constructor leaves EAX=slot and neither wrapper changes
it. On a null path, EAX still contains entry FS:[0], loaded by the native
prologue. Live Ghidra xref queries returned no references to either wrapper,
so a semantic return contract is unestablished. The new Source APIs are void
and do not fabricate a nullptr return or promise either native register residue. Added context
also changes Source stack cleanup: native RET/RET8/RET4 are not Source ABI
compatibility claims. Original parent-frame identities and aliasing of
compiler-private spills are not modeled.

The first handler `00C6D941` loads FuncInfo `00D97EC8`; the second
`00C6DA01` loads `00D98004`. Both jump to `00BF6B43`. Each FuncInfo has
magic19930522, maxState1, no try/catch map and EHFlags1. Its only unwind
record has state-1 and action `00C6D930` / `00C6D9F0`, respectively.
State0 is armed before either null branch.

The first action reads parent EBP-14h and EBP-10h; the second reads EBP+4
and EBP-10h. Both push those captured placement words; their instructions are
`00C6D938 CALL 00401130` and `00C6D9F8 CALL 00401130`. Both discard8
bytes and return. Complete17-byte action bodies were checked live;
`00401130` begins C3 and is RET-only. Consequently these actions introduce
no observable storage cleanup in the supported Source domain. There is no
new callback or empty function pretending to reconstruct an unknown action.
The actual native FH3 action entry and its private-frame reads are recorded
as evidence, not exported as independently callable Source functions.

`00559DC0` was compared instruction by instruction against accepted
`00432440`. After normalizing its stacked header and the latter's ECX input,
both capture begin at +4; nonnull captures end at +8 and calls `00432050`
with owner/header duplicates; then both reload CURRENT begin for `_free`.
The already accepted actual range destructor does not consume those owner
or duplicate words. Both clear +4, +8 and +C in order, even when begin was
null, and preserve +0. `00432440` composes accepted actual `004324A0`, whose
Source body has this same capture/destroy/current-free/clear schedule using
the concrete raw pool and current CRT free. A destruction failure prevents
later destruction, free and clears; there is no local catch, retry or rollback.

The historical `native_string_vector_placement_copy` and
`native_string_vector_tidy_00559dc0` in `src/frontend_seg12_17_cc13.cpp`
remain untouched. They use `Fe3TemplateHost`; the old tidy also returns early
for null begin, leaving stale end/capacity fields. The new actual service
clears those fields as the listing does. Existing reconstruction records are
retained as qualified historical projections and are not silently promoted.

Four live incoming tidy calls were checked: `00559E6A`, `006A32A3`,
`006A33BA` and `006A440B`. Each pushes the current16B header and places an
owner word in ECX; the callee ignores that entry register. This is interface
evidence only; no completion claim is made for these callers. The first
caller is owned by the separate nested-range packet and is not edited here.

The Win32 Release build and all three existing suites pass; CTest took7.55
seconds. All12 exact CALL rows pass the live call-site verifier. Results are in
`reports/oct15_nested_string_vector_wrappers.json`. No new tests are added.
Current Source C++ propagation does not prove original FH3/SEH catchability,
private-frame/stack ABI, CRT/exception identity, cleanup-failure unwind
identity, original-game runtime or gameplay behavior. Existing native math
seeds do not exercise these new wrapper functions.
