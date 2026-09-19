# Native session messages 23, 24 and 25

Addresses: 00429AC0, 00429F20, 0075E000, 0075E090, 0075E0A0, 0075E0E0, 0075E120, 00766060, 007660C0, 007660D0, 00766110, 00766150, 007661B0, 0075DAA0, 0075DAC0, 0075DAD0, 0075DB10, 0075DB50, 00768530

## Scope and native layout

R185 reconstructs 18 complete normal bodies (980 native bytes): three message classes and the owned-string bit codec. The factory remains a dependency fragment. Names describe recovered behavior and are not recovered symbols. Source interfaces are explicit C++ interfaces; profile context extensions are source-only metadata.

| Type | Native size/profile | Payload | Wire after type byte |
| --- | --- | --- | --- |
| 23 | 50h / D030E8 | fourteen DWORDs at18..4C | fourteen unsigned4 fields |
| 24 | 24h / D036E4 | length18, pointer1C, signed DWORD20 | byte-counted owned string, signed32 value20 |
| 25 | 20h / D03070 | signed DWORDs18,1C | two signed6 fields |

The verified factory instructions allocate50h at7689F0 and call75E000 at768A04; allocate24h at768A10 and call766060 at768A24; allocate20h at768A30 and call75DAA0 at768A46 with stack type25. These establish the allocation layouts. Their containing factory has a verified7528-byte range, but these three instruction sites lack stored Ghidra function membership. They are recorded separately from ownership-checked call rows.

Each constructor uses the existing base behavior: mode3, zero08/0C, base profileD02C68, typebyte, one current-game E188A8 capture and signed selection18EC in0..7 from the owner array18CC, else null. Then mode1 and the final profile. Type23 zeroes all fourteen payload DWORDs. Type24 zeroes only length/data; value20 remains untouched. Type25 calls the actual base constructor with its arbitrary DWORD type argument, stores its low byte and leaves both payload words untouched. Padding11..13 is retained throughout.

Each predicate compares the entire query DWORD with the fixed class tag. This remains true for type25 constructed with a different typebyte. All three profiles have scalar-delete, writer, reader, predicate and the existing always-true4499C0 slots. Type24's source profile adds borrowed actual pool context and fallback backing after those five visible slots.

## Owned-string codec

429AC0 takes ECX=cursor and one stack pointer to the actual eight-byte string header, returning with RET4. It captures only the low byte of stored length, writes that8-bit prefix, then reloads data+4. Null uses actual E17669 backing. It writes captured-length times8 bits without strlen. High length bits are ignored, and embedded NULs are transmitted. The pointer reload happens after output may have modified an overlapping header. There is no native capacity clamp.

429F20 takes the same cursor/header ABI. It reads a local unsigned byte length, consumes exactly that many bytes into a256-byte stack buffer, appends NUL and scans the first NUL. It calls existing41DD40 to resize the actual destination header with preserve=false. It reloads data after resize and, when nonnull, copies the header's current length through the existing BF7680 overlap-admitting copy contract. The source uses memmove. There is no temporary heap allocation or local temporary cleanup. Existing resize behavior is preserved, including equal-length/null-data returning without allocation.

Type24 writes type8, that header codec and then full signed32 value20. Its reader consumes the full wire string, even when storage truncates at an embedded NUL, before reading value20. It therefore differs from the C-string writer used by type9: the type24 writer uses stored low length, not a terminator scan.

The actual fallback's first255 bytes were captured from live Ghidra and matched to the PE. The fixture binds that backing instead of inventing an empty string. The source requires the actual backing to remain valid for the requested count.

## Other wire and lifetime behavior

Type23 writes fourteen low nibbles in ascending field order; its reader zeroes each full DWORD before reading four unsigned bits. Type25 writes signed6 values, whose reader sign-extends to full DWORDs. Writers receive raw10h cursors; readers receive18h stream wrappers with the cursor at+4. Stream profile, ownership byte and padding are retained.

Type23/type25 scalar methods stamp rootCE4974, free through actual BF65AC only for flags bit0 and return object identity. Type24 scalar always invokes its full destructor before applying the same free rule. Destructor766150 stampsD036E4, captures nonnull data1C and length18+1, calls actual419CC0 pool getter and BD1510 return with trailing1, then stamps rootCE4974. Header fields and value20 remain unchanged.

Native FuncInfoDB889C references unwind mapDB8894. State0 to-1 invokes C88C20, which jumps to root-stamp4499D0. Source __try/__finally preserves that cleanup on a pool-return escape. Original handlerC88C28 and metadata are relocated for normal native fixture calls with the actual CRT FH3 export. Original exception dispatch compatibility is not established by the source-only fault check.

## Evidence and validation

Seven missing functions were defined under the Ghidra lock with prior state recorded. The type24 scalar's missing ADD ESP,4 after BF65AC was repaired without changing the callee's no-return annotation; its listing now has zero gaps. Confirmed names/comments retain prior values and evidence; affected exports are refreshed.

- 9,763 live bytes matched the PE, including all new/support bodies, complete factory bytes, profiles, fallback and EH metadata.
- 48 direct CALL/tail-JMP rows have verified Ghidra ownership. Three factory sites remain explicitly separate raw evidence.
- Strict MSVC Win32 build and all three existing CTests passed.
- 2,001 original/source cases matched5,333,708 observation bytes:105 constructors,736 string-class cases,624 other class cases,528 standalone writers,2 aliases and6 freeing scalars.
- Coverage includes all8 bit offsets; nonzero output patterns; signed6 limits; stored lengths0,1,31,148,149,150,255,256,511; high-length DWORD bits; null fallback; embedded NULs at0,7,148,149,254; empty/equal/different/equal-null resize; small-return gate0/1; retained fields and actual private pool arena/ring state.
- One focused source-only access violation verifies root cleanup and retained header state; the original FH3 exception path is not compared.

The two alias cases use owned mapped backing. One prefix changes the header pointer before the required reload; the other changes stored length after its low byte was captured. The initial probe expectation missed the bit writer's carry-byte assignment. Correcting the probe's backing and expectation made both cases pass without source changes; failed probe artifacts are retained in the local evidence archive.

The fixture uses actual raw pool getter/allocation/return implementations and a prepublished private pool. It does not fabricate provider results, touch peer gameplay, or inspect freed heap contents. Lazy pool recreation, allocation failure, concurrency, general aliasing, original CRT identity and whole binary ABI remain open.

## Follow-up packets

Factory continuation includes type26 at768A66 calling75DB70 and type27 at768A86 calling75DC20. Claim their functions and profiles before reconstruction. The full factory, packet recorder, network workers, ordinary startup, network exchange and gameplay still require composition and runtime validation. This packet alone does not establish a runnable game rebuild.
