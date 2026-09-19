# Native session messages 103 through 106 (R204)

Addresses: constructors 0075A1F0, 00728F20, 00728FA0, 00761710; predicates 0075A240, 00728F70, 00728FF0, 00761790; ordinary cleanup 0075A230, 00728F60, 00728FE0, 00761780; scalar cleanup 0075A270, 00729050, 00729070, 007617C0; codecs 0075C640/0075C660 and 007617E0/00761860; reused scalar codec 006D2F20/006D1550; partial factory 00768530.

## Scope and profiles

Twenty complete normal game bodies (854 bytes) and four five-slot profiles use existing base/header, bit, numeric, allocation and CRT providers. No library code was newly ported. Names remain descriptive hypotheses.

| Type | Allocation | Constructor | Profile | Fields after shared scalar20 |
|---|---|---|---|---|
| 103 | 28h | 0075A1F0 | 00D02D08 | boolean24, padding25..27 |
| 104 | 24h | 00728F20 | 00CFDE64 | none |
| 105 | 24h | 00728FA0 | 00CFDE78 | none |
| 106 | 38h | 00761710 | 00D03414 | signed24/28, float2C/30, signed34 |

Predicates accept their fixed type plus 98, 73 and 70, comparing the full DWORD and ignoring receiver/mutable type. Ordinary cleanup stamps root CE4974. Scalar cleanup stamps the root, frees through BF65AC if flag bit 0 is set, and returns captured this. Slot 4 reuses 004499C0.

Profiles 104/105 use the same source writer and reader adapter addresses as 99/100/102. The source context embeds the prior scalar context as its first member; a standard-layout assertion establishes that relationship. Profile wrappers for 103/106 recover the containing context to access their own providers. Five native slots precede the borrowed source-only context pointer. Full binary ABI is not claimed.

## Constructors

Type 103's 53-byte constructor begins with MOVSS from borrowed CF8B3C (normally 256). It clears fields08/0C, owner14, mutable type10, sender18 and flags1A/1C, sets delivery1, stores scalar20 and finally its own profile. It never reads Game. Boolean24 and padding retain their old bits.

Types 104/105 have 52-byte constructors. Each calls existing 75B430 with its fixed type, capturing Game's selected owner: index18EC in 0..7 reads owner18CC+4*index, otherwise NULL. After that call, MOVSS captures CF8B3C; sender18 and flags1A/1C are cleared, delivery1 is set, scalar20 is stored and the own profile is installed.

Type 106's 102-byte constructor inlines the same base initialization and owner capture with type106, then performs the same default load and final stores. Source reuses concrete 75B430 for the equivalent sequence; no additional native CALL is asserted. Its five payload DWORDs and padding retain their previous contents. Constructor MOVSS preserves raw signaling-NaN defaults and FP status.

## Shared scalar and message 103

All four types reuse the category98 scalar codec documented in [R203](NATIVE_SESSION_MESSAGES_99_TO102_R203.md). Presence includes unordered x87 comparisons. The writer multiplies a present scalar by four, uses the actual CRT conversion selector, then clamps signed low EAX to 0..255. The reader decodes quarter units or overwrites an absent value from borrowed D7A260 (normally -1).

Message 103's 31-byte writer calls the shared scalar writer, then writes byte24 as a boolean through 4290B0. Its 31-byte reader calls the shared reader, then reads a canonical boolean directly into byte24 through 428D70. Padding25..27 is retained. The writer takes ECX=this and a raw cursor on the stack; the reader takes ECX=this and an 18h stream with cursor at +4. Both return with RET4.

## Message 106 arithmetic and field order

The 118-byte writer calls the shared scalar writer, writes signed DWORD24 and DWORD28 at width32, writes floats2C and30 individually through 4295C0, and finally writes signed DWORD34 at width32. Each float uses zero0, signed1, max-finite scale D7A248 and width32. For each call, x87 loads/stores the borrowed scale first, then loads/stores the value. Separate FLD/FSTP copies preserve signaling-NaN quieting and exception status.

The 115-byte reader calls the shared scalar reader, reads signed DWORD24 and28, reads numeric floats directly into 2C and30 through 4293F0, then reads signed DWORD34. It separately copies the borrowed scale through x87 before each numeric read. Writes occur sequentially. Both routines use ECX=this and RET4; the reader uses the stream cursor at +4.

## Validation and native evidence

Strict MSVC Win32 compilation and all three existing CTests pass (`build2.log`). The cumulative ignored native differential fixture compares **49,927 original/source pairs and 47,871,349 matching bytes**. New coverage is 6,152 cases: 368 constructor/cleanup, 1,024 predicate, 24 scalar, 4,096 message106 wire, 512 message103 wire and 128 profiles104/105 wire cases. Five source-only cleanup faults are inherited coverage; no new exception claim is made here.

Message106 coverage uses 64 scalar/two-float variants, all eight bit alignments, both CRT selectors, four x87 rounding modes, signed integer boundaries, and x87/MXCSR exception flags. Repeated reads include an absent scalar while all extra fields continue to serialize. Constructors cover seven owner-index cases and 16 borrowed default patterns, including NaNs; type103 is exercised with a null Game cell. Fixtures also check storage/padding retention, full-DWORD predicates, scalar flag bits and shared adapter identity across packets. No new tests were added to the committed suite.

The collector compares 38,706 live/PE bytes, 22 owned CALL rows and 1,321 relocation records. Twelve missing functions were defined; all four scalar flows have zero gaps. Borrowed CF8B3C and D7A248 were independently checked against live Ghidra and the installed PE.

Table 0076A430 maps 103..106 to 00768EF2/00768F12/00768F32/00768F52, matching the constructors and sizes above. Raw CALLs 00768F06/00768F26/00768F46/00768F66 lack stored Ghidra function membership and remain separate from the 22 owned rows. Types107/108 route to 00768F72/00768F92 and remain follow-up.

The report records source/artifact hashes, native bytes, call checks, prior annotation values and immutable local evidence archives. Final integration checks 31 relevant COFF objects, ignoring only timestamp bytes 4..7, and repeats the fixture against the final merged library.

## Limits and follow-up

Full factory 00768530, recorder, networking, startup and gameplay remain open. Masked FP fixtures do not establish unmasked exception ordering, nondefault DAZ/FTZ/precision, arbitrary aliases or concurrent mutation. Allocation failure, malformed inputs and original CRT/FH3 behavior remain unproved. Borrowed contexts and cells must outlive their profiles and calls. The next bounded factory entries are 107 onward.
