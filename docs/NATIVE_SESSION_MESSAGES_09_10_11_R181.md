# Native session messages 09/10/11 (R181)

Sixteen complete normal bodies (924 bytes) supply three more message-factory dependencies. The full factory and packet recorder remain incomplete.

## Reconstructed methods

| Class | Native routines | Allocation/profile |
| --- | --- | --- |
| 09 | 007661D0 constructor;766230 type test;766240 write;766300 read;7662A0 destructor;7663D0 scalar | 30h / D036F8 |
| 10 | 0075E540 constructor;75E5C0 type test;75E5D0 write;75E5F0 read;75E610 scalar | 18h / D03138 |
| 11 | 0075E630 constructor;75E6B0 type test;75E6C0 write;75E6F0 read;75E720 scalar | 1Ch / D0314C |

The factory's allocation and constructor instructions establish these layouts. Constructors use the actual current-game publication and signed owner index0..7, else null; they store literal mode1 and their profile. Type09 clears only length+18/data+1C, retaining+20/+24/+28/+2C. Type11 retains payload+18; all preserve base padding. Fixed IsType methods compare full query DWORDs to9/10/11 irrespective of the mutable serialized byte.

Type09 wire order is type8, unsigned low8 fromDWORD+20, the C string at+1C (actual emptyF1AF25 when null), then low/high32 of the qword at+28. Writing reloads the qword after its low half is written, matching __aullshr32. Reading allocates the byte-length temporary, consumes the entire payload, scans to its first embedded NUL, resizes the actual pooled header with preserve0, copies current length to current nonnull data and frees the temporary. It **then** reads the qword: low and zero-high stores precede the second read; __allmul(high,0,0,1) supplies the high half followed by the native ADD/ADC stores. Equal-length/null-data resize remains a no-op; no temporary exception guard is invented.

Type10 writes/reads only the type byte. Type11 adds all32 bits ofDWORD+18. Writes receive the raw10h cursor; reads receive the18h wrapper with cursor at+4. The real five-slot profiles use ECX/stack adapters. Type09 appends explicitly **source-only** raw-pool/empty-string binding metadata after its slots; the caller maintains those lifetimes. Native profiles contain only five slots, and whole binary ABI is not claimed.

## Lifetime and Ghidra evidence

Type09 destruction stamps D036F8, captures the nonnull string and length+1, resolves actual419CC0 and returns throughBD1510, then stampsCE4974 without clearing the header. FuncInfoDB88C8/mapDB88C0 directs state0->-1 throughC88C40/4499D0 to the root stamp on unwind. A source `__finally` preserves that order. Its scalar destroys before optional flags-bit0 object free. Type10/11 scalars stamp the root, conditionally free and return the captured identity.

Nine missing functions were defined. Type09's reader was initially truncated at the temporary free; the returning-free override was cleared and its verified full198-byte body recreated through007663C5. The scalar's missing ADDESP4 was restored. Both final listing checks report zero gaps. Sixteen bodies and one factory dependency fragment are named/commented/saved with prior values and repair receipts retained and exports refreshed.

45 owned CALL/tail-JMP sites pass exact stored ownership checks. Factory calls0076883C->0075E540,0076885C->007661D0 and0076887C->0075E630 have valid live instructions/PE bytes but absent stored function membership; they are recorded separately. Earlier factory metadata gaps persist.

## Validation

- Existing bsp.gpr / battlestationspacific.exe verified;9,413 live/PE bytes:924 new bytes,814 support bytes,10 raw handler bytes,7,528 factory bytes and137 data bytes.
- Strict MSVC Win32 build and all three existing CTests pass; no permanent tests added.
- One fixture: **1,182 original/source pairs and4,102,056 identical observation bytes**:576 type09 cases,560 type10/11 cases,40 embedded-NUL cases and6 freeing scalars. All8 alignments, valid/invalid game selections, retained payload/padding, fixed type queries after changing the type byte and scalar return identities are covered.
- Type09 tests include string lengths0/1/31/148/149/150/255/256/511, both small-return gate states, fresh/same-length/replacement/null-data storage, embedded NULs, packed-byte truncation and exact post-free qword consumption. Pool used arena/guard bytes, all head/tail values and nonnull ring slots, bump/live/peak and native/OS recursion match. Freed heap contents are never inspected.
- The original side executes copied message/string/CRT-integer code and relocated EH metadata. ABI bridges forward to the existing **actual** source raw-pool providers with a real prepublished private owner. Current allocation/free, overlap-admitting memmove and FH3 exports are explicit provider boundaries; original CRT identity is not asserted.
- One **source-only** access-violation case confirms root stamping and header retention when type09 release fails. Normal original comparisons do not invoke EH, so original FH3 exception parity remains unproved.

## Limits and follow-up packets

Continue through the remaining concrete factory classes before binding the full factory/packet recorder/network workers. Ordinary startup, network exchange and gameplay remain open.

Lazy pool recreation/raw-manager registration, allocation/getter failure, original private FH3/SEH, double faults during unwind, arbitrary aliasing and concurrency are outside this fixture. Source bindings and final snapshots are not whole binary ABI or game-validation evidence. Tested/integrated object hashes and immutable archives are recorded in the report.
