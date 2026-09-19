# Type 55, nested message records and delegated creator (R192)

## Scope and boundary

Reconstruct eleven complete game-specific bodies (2,304 bytes), including the
full control flow of creator `008E1530`. Type 55 requires explicit list/vector
library providers. The fixture binds those providers to original helper bytes;
it does not furnish a production STL provider. The full `00768530` stream
factory, runtime composition and gameplay remain open.

Evidence comes from `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`: 19,462 live bytes matched against the original PE,
73 owned direct-call edges, and one separately recorded raw factory call.
Descriptive names are hypotheses, not recovered symbols.

| Address | Bytes | Reconstructed operation | Native entry/return |
| --- | --- | --- | --- |
| 008E0170 | 102 | Type-55 constructor | ECX this, RET |
| 008E0320 | 35 | Type predicate | ECX this, stack DWORD query, RET 4 |
| 008E0350 | 210 | Resolve pending received handles | ECX this, AL Boolean, RET |
| 008E0600 | 344 | Write nested records | ECX this, stack raw cursor, RET 4 |
| 008E1930 | 606 | Read entries and append receive records | ECX this, stack stream wrapper, RET 4 |
| 008E1480 | 132 | Message destructor | ECX this, RET |
| 008E1510 | 30 | Message scalar destructor | ECX this, stack flags, RET 4 |
| 008E08F0 | 96 | Entry constructor | ECX this, RET |
| 008DC100 | 265 | Entry destructor | ECX this, RET |
| 008DD440 | 30 | Entry scalar destructor | ECX this, stack flags, RET 4 |
| 008E1530 | 454 | Allocate/construct types 55–62 | DWORD selector in ECX, EAX pointer/null, RET |

Two missing functions were defined. Four bodies had returning-free flow gaps
repaired. `008DC100` also required restoring its tail through `008DC208` and
recreating its stored body; the earlier end at `008DC1AA` omitted string cleanup.
Library helper bodies are read-only evidence, not additional reconstructed game
functions. Their existing listing gaps were not changed under this packet.

## Actual storage and ownership

The 34h message has the established 18h base, a list at +18h and a vector at
+24h. List storage is `{retained, sentinel, count}`; 0Ch nodes hold next,
previous and a payload pointer. Vector storage is `{retained, begin, end,
capacity_end}`. Construction calls base `0075B430(55)`, sets delivery 1 and
profile `00D16124`, creates the sentinel through `008DB560`, clears list count
and the three vector pointers, and retains bytes at +18h/+24h and base padding.

The 2Ch entry uses profile `00D16038`, whose sole slot is scalar destructor
`008DD440` (the following bytes are strings, not more virtual slots). It owns
string headers at +4/+Ch, retains +14h, has DWORD fields +18h/+1Ch, and has a
position list at +20h. Construction clears both string headers, obtains the
sentinel through `008DB4F0`, clears count, and retains other payload fields.
Position records contain object pointer +0 and three float words at +4/+8/+Ch.

Receive-vector elements are 1Ch bytes: string header +0, Boolean +8, retained
byte +9, WORD handle +Ah, resolved pointer +Ch, and float words +10h/+14h/+18h.
The reader initializes only the temporary element's string header before
passing its 1Ch representation by value to vector resize. It subsequently
writes the Boolean and the selected branch's fields. The other branch's fields
and padding are not inferred to be zero.

## Writer and reader

The writer emits type (8 bits), low eight bits of the outer list count, then
walks actual links to the current sentinel. Each entry emits two owned strings,
two unsigned 2-bit DWORD fields and low eight bits of the inner count. Each
position emits a Boolean captured from its object pointer. If true, it reloads
that pointer and writes the WORD at object+174h in 12 bits. Otherwise it captures
the actual `00D7A248` scale through x87 FLD/FSTP and writes three numeric floats
through `00429790` with zero=0, signed=1 and width=32. Count headers do not bound
either linked traversal. Iterator validation calls may return; the source keeps
subsequent reloads and continuations.

The reader uses the wrapper's cursor at +4. It reads type and outer count,
resizes the list through `008E0EC0(list,count,nullptr)`, allocates a fresh 2Ch
entry for each node, constructs it and publishes the pointer. Existing payload
pointers overwritten or removed by resize are not implicitly deleted. It reads
the two strings and DWORD fields, then an inner count into a local DWORD. This
is not a second caller-supplied argument despite the initial pseudocode.

For each inner item, the reader appends to the existing receive vector through
`008E17C0`, rechecks the last index and copies the entry's first owned string.
The native copy uses `00BF7680`; source uses the established overlap-capable
memory-copy contract. A true Boolean reads 12-bit handle +Ah and clears resolved
pointer +Ch; false reads three numeric floats at +10h. Entry position lists stay
empty. Reading again appends receive rows; shrinking the outer list does not
clear previously received rows.

## Handle resolution and predicates

The type predicate matches full DWORD query 55, 54, or the zero-extended mutable
type byte. Fixed matches precede the object dereference.

`008E0350` first checks the full DWORD at current game `00E188A8` +21A4h. Zero
returns false without accessing the message. Otherwise it walks the receive
vector using signed byte-difference/28 size arithmetic and unsigned indexing.
Rows with false Boolean or an already resolved pointer are skipped. For the
others it resolves the full WORD handle through the two raw tables at
`00F89A54`/`00F89AA8`, using split `00F89A10` and bases `00F89A0C`/`00F89A60`.
Index shift/add wraps at 32 bits; ID zero has no special null rule. It stores
the result at +Ch and returns false on the first null result, preserving prior
resolutions. Exhausting the vector returns true, including an empty vector.

## Destruction and exception evidence

The message destructor stamps its own profile, destroys the captured receive
range through `008DE660` when begin is nonnull, reloads/frees the vector storage,
then clears all three pointers. It calls `008DB580` on the outer list and stamps
root profile `00CE4974`. List destruction frees nodes/sentinel and clears its
head/count; it does not delete entry payloads. Entry ownership is deliberately
not expanded in source.

The entry destructor frees each nonnull position payload and clears that node's
payload slot, resets sentinel links/count, frees nodes and sentinel, clears the
sentinel pointer, then releases strings at +Ch and +4. String header bits remain.
The entry retains its own profile. Both scalar destructors always clean up,
free through the actual singleton provider when flags bit 0 is set, and return
the captured address without inspecting freed memory.

| Owner | Handler | FuncInfo | Unwind map | Relevant cleanup |
| --- | --- | --- | --- | --- |
| 008E0170 | 00CA3923 | 00DD5E1C | 00DD5E0C | Active state 0 stamps root; metadata also contains a list action not activated by this body |
| 008E08F0 | 00CA3946 | 00DD5E50 | 00DD5E40 | Header +Ch, then +4 |
| 008DC100 | 00CA343B | 00DD5738 | 00DD5730 | State 0 releases only header +4 |
| 008E1480 | 00CA3A43 | 00DD60C4 | 00DD60B4 | List +18h through 008DBAB0, then root stamp |
| 008E1530 | 00CA3AA8 | 00DD60E8 | 00DD610C | Eight allocation guards, each frees captured allocation |
| 008E1930 | 00CA3B0B | 00DD61AC | 00DD61A4 | Release newly allocated entry if construction escapes |

Source `__finally` blocks preserve these specific cleanup boundaries. In
particular, entry destruction does not promise to finish the inner list or
release header +Ch after an earlier fault. Native FH3 dispatch is not proved.

## Delegated creator and library contracts

`008E1530` subtracts 55 from the full DWORD selector, rejects unsigned results
above 7 and dispatches through table `008E16F8`. It allocates sizes
34h/30h/24h/24h/2Ch/2Ch/2Ch/2Ch and calls the eight constructors. Allocation takes
one size argument; the decompiler's extra selector argument is spurious. Null
allocation returns null. The source now implements the entire dispatch and
construction/cleanup schedule with supplied profiles and contexts.

The full factory's eight slots at `0076A370..0076A38F` point to `00768C5E`, whose
CALL to the creator still has no stored Ghidra function membership. It remains
raw instruction evidence separate from the 73 ownership-checked edges.

`NativeMessage55LibraryCalls` requires real sentinel creation, list resize,
vector resize, range destruction and list destruction providers. No default
provider or success stub is supplied. `008E17C0` takes a 1Ch by-value element
and returns with RET 20h. `008DE660` takes begin in ECX and end in EDX, plus two
otherwise unused stack owner arguments, RET 8. Treating those registers as the
two pushed owner arguments gives the wrong destruction range.

The existing name `STL_xlen_throw_008de2a0` was misleading. The complete 147-byte
library helper normally checks unsigned `3FFFFFFFh - count >= increment`, then
adds to list+8 and returns with RET 4. Only its overflow branch constructs and
throws length_error. The corrected descriptive library name is
`STL_List_CheckedSizeAdd_008de2a0`; this is a library contract, not a new game port.

## Validation

Strict MSVC Win32 build and all three existing CTests pass. Final fixture
`local/session_message55_r192/fixture4` compares 1,055 original/source pairs and
1,634,733 observation bytes, using 29 original library helpers (3,337 bytes)
for both sides' container contracts:

- 56 constructor/lifetime cases with retained-byte guards and signed owner indices.
- 256 predicate cases spanning the mutable byte domain and full-width queries.
- 520 nested-record cases: all eight alignments; string lengths 0/1/148/149/150/
  255/256/511, embedded NULs, count high bits, repeated reads, list shrink to zero,
  vector append/growth through 255 elements, both numeric conversion selectors
  and four rounding modes. x87/MXCSR exception flags are compared.
- 132 handle-resolution cases, including zero process flag with null this,
  first-missing early return, cached pointers, ID 65535 and wrapping table bases.
- 91 creator cases covering all eight types, out-of-range DWORD selectors and
  seven owner indices; scalar release uses actual allocation/free providers.
- One source-only fault case restores private pool publication after a deliberate
  AV in header +Ch release. It verifies completed list cleanup, retained headers,
  and exactly the +4 return against actual pool state. This is not a native FH3 test.

Observations compare initialized fields and normalize heap identity. The native
reader leaves new receive padding and inactive branch fields indeterminate;
those bytes are not used as differential expectations. Library overflow,
allocation failure and invalid-iterator branches are outside the fixture domain.
The fixture guards selected library error paths and terminates if they are reached.

The first two fixture runs mistakenly classified all of `008DE2A0` as an
exception path from its existing name. Tracing located its normal count-add
call. Adding the verified original body corrected the fixture; production
source/library stayed unchanged. Fixture 3 passed before the focused shrink and
cleanup-fault checks were added; fixture 4 is the final result. All artifacts
are retained in the local evidence archives identified by the companion report.

## Remaining work

Supply and validate a production library-provider composition, bind the full
stream factory, then continue packet recorder/network/startup integration.
Fixture adapters and source profile contexts do not establish whole binary ABI,
original CRT/FH3 identity, arbitrary aliasing, concurrency or gameplay parity.
