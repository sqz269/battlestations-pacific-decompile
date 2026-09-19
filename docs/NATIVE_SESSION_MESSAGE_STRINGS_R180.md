# Native type08 message strings and lifetime (R180)

Six complete normal bodies (567 bytes) supply the message factory's type08 dependency. The full factory and packet recorder remain incomplete.

## Recovered behavior

| Native address | Behavior |
| --- | --- |
| 00765E10 | 91-byte constructor,2Ch allocation |
| 00765E70 | 13-byte fixed type8 query |
| 00765E80 | 80-byte writer: type8,signed10,two C strings |
| 00765ED0 | 128-byte destructor and ordered string returns |
| 00765F50 | Full225-byte reader through00766030 |
| 00766040 | 30-byte scalar destructor/free |

The constructor performs the real current-game selection, stores mode1 and profileD036D0, then clears the two eight-byte string headers at+18/+20. It retains padding and signedDWORD+28. Factory allocation at00768808 is2Ch. IsType compares the full query DWORD against fixed8 independently of the mutable serialized type byte.

Writing emits type8, low10 signed bits from+28, then the C strings pointed to by+1C/+24. A null pointer selects the actual zero byte atF1AF25. Stored string lengths do not drive writing; the existing C-string writer scans to NUL and writes length modulo256. The second pointer is loaded after the first string's write.

Reading consumes the same type/signed10 header and **both** strings. For each,428DA0 allocates a temporary from the wire's byte length and consumes the full payload. The reader then scans to the first embedded NUL, resizes the corresponding actual pooled header with preserve0, copies the current header length to its current nonnull data, and frees the temporary viaBF6989. Embedded NUL shortens the stored string without reducing wire consumption. An equal-length resize remains a no-op even with null data. No temporary guard is added around resize failure because the original reader has no local EH registration.

## Cleanup and Ghidra repair

The destructor stamps its own profile, returns the second nonnull string using length+1, then the first, resolving the actual419CC0 pool getter for each return. It leaves both headers untouched and finally stamps rootCE4974. Scalar deletion always destroys, then frees the object only when flags bit0 is set, returning its captured identity.

FuncInfoDB8870 points to unwind mapDB8860: state1->0 callsC88C08/41DD20 on the first string; state0->-1 callsC88C00/4499D0 to stamp the root. Source nested `__finally` blocks preserve that order for normal and exceptional exits. These are explicit source semantics, not a claim of original FH3/SEH binary compatibility.

The original stored reader ended at the first array-delete call. Both returning-free overrides were cleared, the verified tail was disassembled and the function recreated through00766030. The scalar's missing ADDESP4 after its free call was also recovered. The final reader/scalar listing checks report zero gaps. The fixed type test was defined. Existing names/comments and repair receipts are retained; six bodies and one factory fragment are named/commented/saved with refreshed exports.

## Interfaces and providers

The object is an explicit raw2Ch layout with no implicit string destructor. The translated profile has five ABI-visible adapters followed by **source-only** references to the caller's actual raw pool context and native empty-string backing. The originalD036D0 table contains only five slots. The caller maintains the profile and binding lifetimes. Writes take the raw10h cursor; reads take the18h wrapper whose cursor begins at+4. Original constructors/destructors take ECX with no stack args; the other virtual methods take one stack argument and RET4.

Production code composes existing actual raw string resize/destroy, getter, pool and allocation/free implementations. No semantic fallback pool, cached publication or invented process default is introduced.

## Validation

- Existing bsp.gpr / battlestationspacific.exe verified;9,013 live/PE bytes comprise567 new body bytes,803 support bytes,10 raw handler bytes,7,528 factory bytes and105 data bytes.
- 41 owned CALL/tail-JMP sites verified. Factory call0076881C->00765E10 has valid live instruction/PE evidence but absent stored function membership and is reported separately; earlier factory gaps persist.
- Strict MSVC Win32 build and all three existing CTests passed. No permanent tests added.
- One local fixture: **618 original/source pairs and4,129,336 identical observation bytes**:576 class/virtual/lifetime cases,40 embedded-NUL cases and2 freeing scalars. It covers all8 alignments, null/empty strings, lengths0/1/31/148/149/150/255/256/511, size reuse/replacement, the equal-length/null-data quirk, both small-return gate states and negative signed10 decoding.
- Pool observations include used arena/guard bytes, all head/tail values, every nonnull ring slot with pointer identity normalization, bump/live/peak and native/OS recursion state. Header/cursor/profile identities are checked before normalization. Original temporary allocation/free counts confirm both reader temporaries are released normally. No freed heap contents are inspected.
- The original side executes copied message/string code and relocated EH metadata. Pool getter/allocate/return ABI bridges forward to the same existing **actual** source raw-pool implementation with a real prepublished private owner. Allocation/free and overlap-admitting memcpy use current actual CRT providers. The installed CRT FH3 export is bound; normal fixtures do not invoke native exception handling.
- One **source-only** SEH case causes an access violation during second-string release (first string null). The exception escapes while the root profile is stamped and the header remains unchanged. This is separate from the original/source comparisons and does not establish original FH3 exception parity.

## Limits and follow-up packets

Continue with the remaining factory classes and their concrete serializers before binding the factory/packet recorder/network workers. Full ordinary startup, network exchange and gameplay remain open.

Lazy pool recreation, raw-manager registration, allocation/getter failure, native FH3 exception behavior, double faults during unwind, arbitrary aliasing and concurrency are unverified here. The underlying providers retain their documented limits. This packet does not claim whole binary ABI, original CRT identity or game validation. The report pins source/library hashes, integrated COFF comparisons and immutable evidence archives.
