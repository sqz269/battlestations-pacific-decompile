# Native session record messages 04/05 (R179)

Fourteen complete normal bodies (1,666 bytes) supply the next two message-factory dependencies. The full factory and packet recorder remain incomplete.

## Reconstructed methods

| Native address | Behavior |
| --- | --- |
| 00429010 / 00429070 | Signed-byte / unsigned-DWORD stack-value bit writers |
| 004290D0 / 00429180 | Byte-length C-string / captured qword writers |
| 0075D5D0 / D660 / D7E0 / D650 / D970 | Type04 construction, write, read, fixed type test and scalar deletion |
| 0075D010 / D0E0 / D1D0 / D0D0 / D2C0 | Type05 corresponding methods |

Factory allocation instructions at 00768782 and 007687C5 establish C0h and 88h storage. Constructors retain payloads and padding, perform the actual current-game selection via signed index +18EC and table +18CC, then store literal mode1 and the respective D03048/D02FF8 profile. Field names denote storage offsets; semantic interpretations remain open. IsType compares the full query DWORD to fixed4/5 even if the serialized type byte changes. Scalar deletion stamps the translated root, frees only for flags bit0 and returns the captured identity.

## Wire order

Type04: type8; raw qword+20; signed+18 low6; unsigned+54 low2; strings+28/+48; boolean+58; unsigned+5C32; boolean+9E; bytes+60low5/+61low2; unsigned+5032; three byte spans +62[36], +86[16], +96[8]; signed DWORDs+A0[3]; split qword+B0; unsigned+B8/+BC32.

Type05: type8; signed bytes+18/+19 low5; string+1A; boolean+3B; signed bytes+3C/+3D low5 and+44/+45 low6; thirteen DWORDs+50 low4; float+40 using unsigned8-bit quantization with scale1 and no zero marker; split qword+48. The float writer retains FLD1/FSTP and FLD/FSTP payload loading, including signaling-NaN quieting before the shared numeric codec.

String writing scans through the terminator, truncates the length to its low byte, writes that byte and then exactly that many leading bytes. Length256 writes zero payload; length511 writes255 bytes. The original input pointer is retained across prefix writing, so overlapping output can alter the subsequently read input. Neither writer nor existing reader adds a clamp or a capacity check.

The raw qword writer captures two DWORD arguments into an eight-byte local. The classes' later split qword writers write low32, reload both words, then use the high32 produced by __aullshr32. Readers store low and zero high before reading the second word; __allmul(high,0,0,1) is high<<32, followed by the native low/high ADD/ADC stores. Source code preserves those intermediate accesses without reimplementing CRT library helpers.

## ABI and ownership

Native constructors use ECX object, no stack arguments and EAX identity. Virtual methods use ECX object plus one stack argument and RET4. Write methods receive a raw10h cursor; reads receive the18h wrapper with its cursor at+4. The translated five-slot tables are operational through ECX/stack adapters. Type05 appends **source-only** borrowed numeric context metadata after its five slots; native D02FF8 has only five slots. The caller owns profile/context lifetimes. Whole-module binary ABI and private EH parity are not claimed.

## Evidence and validation

- Existing bsp.gpr / battlestationspacific.exe verified. 10,924 live/PE bytes: 1,666 new bytes, 1,447 support bytes, 171-byte CRT converter block, 7,528-byte factory and112 data bytes. Original-side fixtures execute actual copied numeric and CRT integer helpers.
- Four missing Ghidra functions defined under the write lock; fourteen bodies and one parent dependency fragment named/commented/saved, prior values preserved, exports refreshed.
- 100 direct call sites pass stored function-ownership checks. Two raw factory calls, 00768799->0075D5D0 and007687DC->0075D010, have valid live instructions and PE bytes but absent stored function membership. They are recorded separately. Earlier R177/R178 factory gaps persist; no repair is claimed.
- Strict Win32 build and all three existing CTests passed; no permanent tests added.
- One local original-code fixture: **3,810 pairs and3,682,192 identical observation bytes**: 1,808 field-helper cases,320 string cases,1,680 constructor/virtual/scalar cases and2 freeing scalar checks.
- Coverage includes all eight alignments; string lengths0/1/7/31/32/127/255/256/257/511 with separate and overlapping output; signed byte/DWORD packing; captured qwords; empty and maximum field-sized strings; retained padding; invalid/valid game selection; and raw virtual entry calls after changing the type byte.
- Type05 numeric cases cover PC0/2/3, all four RC settings, both CRT conversion modes, signed zeros, subnormals, finite extrema, infinities and quiet/signaling NaNs. Cursor/buffer/object snapshots, x87 exception flags/control word/stack depth, and MXCSR flags/rounding match. Original pre-free root stamp/count are recorded; source retained stamps are checked without accessing freed storage.

## Limits and follow-up packets

Continue with the remaining factory classes, including type08 construction, owned storage and virtual methods around00765E10/D036D0. Do not substitute a generic message parser. The factory, packet recorder, network workers and ordinary game startup/gameplay remain unbound or unvalidated.

Class tests use strings fitting the observed field spans. Oversized/malformed class strings, arbitrary object/cursor aliasing, concurrent mutation, unmasked FP traps, exact x87 condition-code bits, allocator failure, native faults and original private EH are outside the fixture proof. Volatile intermediate stores follow the listing; final snapshots do not establish concurrency semantics. The report pins tested and integrated objects and immutable evidence archives; these checks do not establish game validation.
