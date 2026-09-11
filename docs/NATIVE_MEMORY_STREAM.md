# Native memory stream operations and copy producer

Addresses: `00BEF4C0`, `00BEF590`, `00BEF600`, `00BEFA40`.

The original memory stream uses the existing actual-storage owners in
`native_retained_memory_owners.hpp`. This packet adds the three leaf operations
used by the Lua file-loading consumer, an explicit dispatcher for the original
numeric profiles, and the complete byte-copy producer. It does not replace the
semantic `MemoryStream` class or install a new callable vtable into native owners.

| Routine | Coverage | Native contract | Reconstruction |
| --- | --- | --- | --- |
| `00BEF4C0` | complete, 3 bytes; Ghidra definition pending | `MOV AL,1; RET`; no storage read | naked Win32 entrypoint |
| `00BEF590` | complete, `00BEF590..00BEF5E6` | ECX owner; destination/count/optional-count stack; `RET 0Ch` | naked Win32 entrypoint, shared CRT copy service |
| `00BEF600` | complete, `00BEF600..00BEF60B` | ECX owner; signed length in EDX:EAX; `RET` | naked Win32 entrypoint |
| `00BEFA40` | complete body and constructor cleanup semantics, `00BEFA40..00BEFAE5` | ECX bytes; low/high count stack; EAX new stream; `RET 8` | actual-storage C++ entrypoint with explicit owner context |

The first three C++ declarations use `__fastcall` with an unused EDX parameter,
so their machine interfaces accept the original ECX/stack inputs. The copy
producer and dispatcher have new C++ interfaces. CRT implementation identity,
MSVC 2005 exception-object identity, and game execution are outside the claim.

## Producer-established storage and table identities

`008D43C0` constructs a 10h backing: profile `00D15AD8` at +00, intrusive count
one at +04, allocated bytes at +08, stored signed-positive length at +0C. A
nonpositive input stores zero length and allocates one byte. Actual global
`0109DB98` increments; `0109DB9C` adds the original input bits. The existing
implementation borrows these counters through `NativeRetainedMemoryOwnerContext`.

`00BEF6D0` allocates 14h, writes profile `00D642C0` and count one, retains the
supplied backing at +08, sets cursor +10 to backing data, and sets end +0C to
data plus the current stored length. These are the producer proofs for the
offsets consumed here; no new owner struct or shadow lifetime is introduced.

Current Ghidra bytes for `00D642C0` establish:

| Slot | Original function | Use here |
| --- | --- | --- |
| +00 | `00BD30E0` | current zero-reference invocation, no decrement |
| +04 | `00BB8F90` | flag-one stream scalar deletion |
| +18 | `00BEF4C0` | AL open result always one |
| +24 | `00BEF590` | read |
| +30 | `00BEF600` | length |

`00D15AD8` has `00BD30E0` at +00 and `008D4470` at +04. The new dispatcher reads
the current numeric owner profile and borrowed table slots. Its input domain is
these verified immutable original profiles; arbitrary substituted table words
or other classes remain unresolved. It never rewrites an owner's table word.

`dispatch_native_memory_owner_zero_reference` does not decrement. Its caller
performs the real `InterlockedDecrement(owner+4)` and calls the dispatcher only
at zero. The dispatcher reproduces the +00 selection and BD30E0's current-table
reload for +04/flag one, then uses the already implemented BB8F90 or 8D4470.
BB8F90 invokes BEF9C0, which decrements the backing, invokes its current slot0
only at zero, clears stream +08 after successful return, writes D5C104 then
CEB130, and scalar-frees the wrapper. Backing deletion frees its data and owner
and updates the actual counters. A null dispatcher input follows BD30E0's null
branch; it does not permit null input to the read or length leaves.

## Observable operations

BEF590 takes the unsigned wrapped difference `end - cursor` and clamps the
requested DWORD with `CMP` / `CMOVNC`. Exactly four or two bytes use one load
then one store; every other count calls BF7680, including zero. It then reloads
and adds to the current cursor field, so a destination alias of that field
remains observable. The optional output count is written after the cursor
update. EAX ends as the optional-count pointer, not the byte count. There are no
new seek, bounds, initialization, null, or malformed-storage checks.

BF7680's body checks forward overlap at BF7694..BF769A and implements backward
copy at BF7844, including `STD; REP MOVSD; CLD` at BF785F..BF7862. The external
CRT binding is therefore `memmove`; using host `memcpy` would introduce undefined
overlap behavior. Accelerated CRT implementation details remain a service boundary.

BEF600 loads end, subtracts backing data as a DWORD, then executes `CDQ`.
Lengths whose subtraction has bit 31 set are negative; it does not return an
unsigned 64-bit size or read backing length directly. BEF4C0's bytes are
`B0 01 C3`; its descriptive "open" name comes from slot +18, while its established
behavior is simply true. The same three leaves occur in the second table
`00D64328`; the dispatcher supports the D642C0 owner producer only because the
other table's owner/deletion contract is not part of this packet.

## BEFA40 copy and ownership

Null source returns null before allocation. For a nonnull source, the routine
allocates a raw 10h backing, constructs it with only the low count DWORD, copies
exactly that low count through BF7680, creates a stream with BEF6D0, decrements
the temporary backing reference, and returns the wrapper. The stack high count
is never read. The only direct caller found, BF46B0 at BF4748, pushes stored
high +1C then low +18 and passes bytes +10 in ECX; BEFA40 removes eight bytes.

The constructor guard is proved by handler CC76DB -> FuncInfo E022D8 -> state
map E022D0 -> CC76D0. Its raw bytes are `8B45F0 50 E8D3EEF2FF 59 C3`: load the
saved raw owner, call BF65AC, pop the argument, return. The saved Ghidra body
currently ends at the call and needs the existing free fall-through repair.
State zero covers only 8D43C0 construction. State -1 is written before the copy;
there is no later backing rollback on wrapper allocation failure. The C++ guard
has exactly that scope. Negative low counts keep the original signed allocation
versus unsigned copy mismatch; this is not a safe arbitrary-byte-count API.

## Evidence and verification

Every live query verified project `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe` through `tools/bsp.py`. The installed image SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Reports carry exact direct call sites, caller attribution, table bytes, and
the pending three-byte Ghidra definition. Workers performed no Ghidra writes.

The isolated fixture maps the installed PE bytes at their original addresses
without imports or startup, then calls only these read-only leaves and their
actual small-copy BF7680 implementation. It compares the reconstructed leaves,
including stack cleanup, EAX count-pointer result, preserved ESI/EDI, 2/4-byte
paths, zero and clamped reads, backward overlap, and signed length. A composed
host path checks copy independence, ignored high count, immutable numeric table
identity, empty/null inputs, and actual stream/backing counters after release.
This is fixture and ABI evidence; it does not validate Lua loading or gameplay.
See `reports/native_memory_stream.json` for completed validation results.
