# Actual outer string-vector cleanup

This packet reconstructs complete `0055A3A0..0055A3E7` range destruction
and `0055AFF0..0055B02C` outer tidy over actual16B inner owning headers.
Source is `src/native_outer_string_vector_cleanup.cpp`; the contract is
`include/bsp/native_outer_string_vector_cleanup.hpp`. Accepted actual
`00432440`/`004324A0` per-header cleanup, the concrete raw string pool and
the current CRT free service are composed directly. No host callback, fake
allocator, replacement stub or general STL container is introduced.
Descriptive Source names are hypotheses; existing names are retained.

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Complete native listings, live72/61-byte
envelopes, prototypes, direct callees and xrefs were checked through
`bsp.py ghidra`. Neither reconstructed entry creates a local EH frame or
catch, and Source adds none.

| Entry | Original physical interface |
|---|---|
| `0055A3A0` | ECX first, EDX last; two unconsumed stacked DWORDs, RET8; no semantic result. Source retains those words and adds stacked raw-pool context. |
| `0055AFF0` | ECX outer actual16B header, incoming EDX unconsumed; no stack input, plain RET; no semantic result. The new fastcall Source passes added context in EDX. |

The range captures first and last once and walks ascending actual16B headers
by DWORD addition/equality. Empty range skips all field reads, pool operations
and frees. For each current header, native code captures begin at +4, reads
end at +8 only for nonnull begin, destroys the captured actual8h string range,
then frees the CURRENT reloaded begin. It always clears +4, +8 and +C in
that order, including null-begin elements, then advances by16. It does not
write +0, clear any string header itself or free the outer backing.

The complete per-header schedule equals accepted actual `00432440`, which
composes actual `004324A0`. The differing ignored words were checked rather
than treated as allocators: `0055A3A0` saves original first in a private scratch
slot and duplicates it as the second unused argument to `00432050`; the
other argument is the current header. Neither of the range entry's two
stacked arguments is read. Accepted `00432440` supplies its own header twice
instead. The complete `00432050` listing reads neither metadata word, so
the different duplicates have no observable storage effect in this Source
domain. Its actual pool getter/return schedule remains the accepted one.

The outer tidy captures begin at +4. Nonnull begin captures end at +8 and
passes that fixed16B header range to the new range service. Only after the
entire range returns does it reload CURRENT outer begin for free; it does
not free the captured original begin by substitution. Finally it always
clears +4, +8 and +C in order and does not write +0. Null begin skips end
read, range destruction and free, but still performs all three clears.
Endpoint and free captures deliberately differ. Raw header/backing aliases
or provider-side mutations are not rejected or restored.

Both inner and outer frees use accepted `singleton_lifetime_free`, whose
body calls current `std::free`. Original `00BF65AC` is a cdecl library thunk
to `00BF9DC8`; the existing correct `_free` name and host-CRT service boundary
are retained. No native heap/CRT binary identity is asserted by this reuse.

The first string/pool cleanup exception stops the current header before
remaining free/clears, stops later16B elements, and prevents outer free/clears.
Earlier effects remain. There is no local catch, rethrow, retry, validation or
rollback. A caller already handling an exception inherits the current Source
C++ cleanup-failure behavior, not a reconstructed native FH3/SEH personality.
The caller provides valid storage/pool/CRT lifetimes accepted by the children;
no owner, capacity, range, alignment, null or overlap guard is added.

The older `Fe3TemplateHost` projections in
`src/frontend_seg12_17_cc13.cpp` remain untouched. Historical outer tidy
returns early for null begin; historical range destruction continues past
null inner begin without clearing stale end/capacity. The new actual entries
follow the unconditional native clears and do not promote those projections.
Other bounded settings-storage projections and their fixtures remain separate.

Root-owned catch `0055B216` supplies ECX destination at `0055B219`, then
rethrows after tidy returns. The dispatcher at `0055AC60` and another tidy
at `0055B110` also demonstrate the captured range and dead stacked metadata.
These are interface checks, not completion claims for their callers.

Exact checked CALL instructions:

```text
0055A3BF CALL 00432050
0055A3C8 CALL 00BF65AC
0055B004 CALL 0055A3A0
0055B00D CALL 00BF65AC
00432454 CALL 00432050
0043245D CALL 00BF65AC
0055B219 CALL 0055AFF0
0055AC6E CALL 0055A3A0
0055B124 CALL 0055A3A0
```

Win32 Release and all three existing suites pass; CTest took7.31 seconds.
All nine report rows and nine documentation call claims pass live checks,
with no failures, unchecked claims or warnings. Results are recorded in
`reports/oct15_outer_string_vector_cleanup.json`. No new tests are supplied;
existing native math seeds do not exercise these entries. Added context,
by-value endpoints and current Source C++/CRT behavior do not prove original
entry/stack/register/private-frame ABI, original heap/exception/FH3/SEH
identity, fault addresses, original-game runtime or gameplay compatibility.
