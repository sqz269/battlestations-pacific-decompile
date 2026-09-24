# Native session messages 111 and 112 (CC10)

Addresses: 00761C20, 00761C80, 00761C90, 00761CB0, 00761CD0, 00761D30, 00761D90, 00761DF0, 00761E10, 00761E20, 00761EC0, 00762640. Profiles 00D03478/00D0348C. Factory table cells 0076A450/0076A454 and raw routes 00768FF2/00769012 are evidence only.

## Scope and boundaries

Twelve complete normal game bodies (805 bytes) provide two five-slot message profiles. Existing base/header, bit cursor, numeric and CRT providers are reused; no library code is newly ported. All names are descriptive hypotheses. The report records each exact native byte span, last instruction, inclusive end and exclusive end.

| Type | Routine | Inclusive native range | Coverage |
|---|---|---|---|
| 111 | Constructor | 00761C20..00761C78 | complete |
| 111 | Ordinary cleanup | 00761C80..00761C86 | complete |
| 111 | Predicate | 00761C90..00761CAF | complete |
| 111 | Scalar cleanup | 00761CB0..00761CCE | complete |
| 111 | Writer | 00761CD0..00761D2E | complete |
| 111 | Reader | 00761D30..00761D8B | complete |
| 112 | Constructor | 00761D90..00761DE9 | complete |
| 112 | Predicate | 00761DF0..00761E0A | complete |
| 112 | Ordinary cleanup | 00761E10..00761E16 | complete |
| 112 | Writer | 00761E20..00761EBC | complete |
| 112 | Reader | 00761EC0..00761F52 | complete |
| 112 | Scalar cleanup | 00762640..0076265E | complete |

The readers, predicates and scalar cleanup functions were absent from Ghidra at worker start. The primary integrator defined those six under its own lease and write lock (definition receipt `reports/native_session_messages111112_cc10_definitions.json`, commit `5ff6f4f9e`). The worker performed read-only Ghidra queries. The next constructor at 00761F60 is outside this packet. Type112's scalar cleanup is the nonlocal 00762640, established by its profile slot, not adjacency.

## Layout and construction

Type111 allocates 38h: the existing `NativeMessage75ExtendedHeader` occupies 00..1F, followed by six DWORDs at 20/24/28/2C/30/34. Type112 allocates 2Ch: base 00..17, WORD18, byte1A, retained byte1B, WORD1C, retained bytes1E..1F, and three raw float DWORDs at 20/24/28. Its WORD1C is not the existing extended-header byte1C flag.

Both constructors take ECX=this, return with plain RET and leave EAX=this. The full EBX provenance is PUSH, XOR EBX,EBX, read-only uses, then POP. They store delivery3, clear DWORD08/0C, install base D02C68, and store only the fixed type byte10. They capture the Game pointer once from E188A8, then select owner14 from Game+18CC+4*index for signed index18EC in 0..7, otherwise null. No null-Game branch exists.

After capturing the owner, both clear WORD18 and byte1A. Type111 clears byte1C; type112 clears WORD1C. They then store delivery1 and finally their own profile. All payload DWORDs and unused padding bytes retain their old contents. Source calls existing 0075B430 for its equivalent inlined base initialization; no native CALL to that provider is claimed.

## Profiles, predicates and cleanup

| Type | Profile | Slot0 | Slot1 | Slot2 | Slot3 | Slot4 |
|---|---|---|---|---|---|---|
| 111 | 00D03478 | 00761CB0 | 00761CD0 | 00761D30 | 00761C90 | 004499C0 |
| 112 | 00D0348C | 00762640 | 00761E20 | 00761EC0 | 00761DF0 | 004499C0 |

The predicates ignore receiver and mutable type byte, compare the full stack DWORD, and use RET4. Type111 accepts 111, 73 and 70; type112 accepts only 112 and 70. Values with matching low bytes and nonzero high bytes are rejected. Slot4 reuses the established always-true provider.

Ordinary cleanup writes root CE4974 to the profile word and uses RET. Scalar cleanup captures this in ESI, writes the root profile, frees through BF65AC if flag bit0 is set, and returns captured this in EAX with RET4. The native free has one pointer argument: ADD ESP,4 at 00761CC6 / 00762656. Both 11-instruction scalar bodies include the return path after free. The source uses the existing real CRT free provider; original allocator identity is not asserted.

Source profiles use ignored-EDX fastcall bridges for the native ECX/stack placement. A source-only borrowed context pointer follows the five slots. This does not establish whole binary ABI compatibility.

## Codec order and arithmetic

Both writers take ECX=this and a raw 10h cursor on the stack; both readers receive an 18h stream and capture its cursor at +4. All four use RET4. Source reuse of the common header codec represents the same inlined native sequence: type byte10 at width8, WORD18 at width12, then byte1A normalized to one boolean bit.

Type111 then writes byte1C as a boolean and exactly six sequential DWORDs at width2. Its reader zeroes and reads the type/WORD fields, canonicalizes both boolean bytes, then zeroes and reads each complete destination DWORD. The total wire width is 34 bits. Direct native rows in the report include the loop call once, with six executions documented here.

Type112 writes WORD1C at width12 followed by three numeric floats at 20/24/28. Each numeric call has zero0, signed1, scale borrowed from D7A248 and width32; both numeric providers clean five stack arguments with RET14h. The total wire width is 129 bits. For every writer numeric call the native x87 FLD/FSTP copies the scale first, then the field. Separate source x87 copies retain signaling-NaN quieting and masked exception status. The reader separately copies the scale before each call, then writes numeric output directly to the destination field. The borrowed scale is loaded anew each time; it is not hardcoded or cached across fields. The verified original value is max-finite float.

## Evidence and validation

Live Ghidra `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, agrees with the installed PE for 921 bytes: 805 owned code, 40 profile, 4 numeric scale, 8 dispatch table, and 64 raw factory-route bytes. The report carries all native bytes and hashes. All 26 owned CALL rows pass `verify_report_calls.py` against the live containing bodies.

Table cells 0076A450/454 resolve to routes 00768FF2/00769012. Their allocation pushes are 38h/2Ch, followed by allocator CALL and ADD ESP,4. Constructor CALLs 00769006/00769026 have no stored containing Ghidra function and remain separately labelled unowned raw edges; they are not attributed to 00768530.

Strict MSVC Win32 `/W4 /WX /MD /fp:strict` compilation passes, along with both current CTests (`reconstructed_math`, `tool_tests`). The ignored local fixture is `local/cc10_messages111112/run_probe.ps1`, copied from R204 and extended with these exact native bodies and 42 relocation operands. It uses original machine code and existing concrete support providers, writes original/source observation streams, and links `/MANIFEST:EMBED`. Its 492 native/support arrays match 31,067 current PE/image bytes, including 16 zero-initialized BSS bytes. No new committed tests are added.

The new differential coverage is **6,596 original/source pairs and 2,656,828 matching bytes**: 56 constructor/ordinary-cleanup cases (seven owner indices and four storage patterns), 512 predicate cases, 12 scalar-cleanup cases, 512 type111 wire cases, 4,096 type112 wire cases, 512 raw numeric reader cases, and 896 changed-scale cases. The wire cases cover all eight alignments; type112 includes 64 three-float patterns, both CRT selectors and four x87 rounding modes, with masked x87/MXCSR exception flags. Raw readers preserve patterns that a writer could quiet. Changed scale cells include both zeros, plus/minus one, infinity, quiet NaN and signaling NaN. Repeated reads overwrite prior values and retain unrelated bytes. Full-DWORD predicate queries include high-byte variants and null receivers. Scalar flags include nonzero values without bit0.

The cumulative fixture passes **56,523 pairs and 50,528,177 matching bytes**. Five source-only cleanup faults belong to inherited fixture coverage, not these two messages. The report records the library, executable, source and fixture hashes. For a merged-library rerun, copy final `probe.cpp`, `original.inc` and `run_probe.ps1` into the same relative ignored directory and select a fresh `-OutputDirectory`; the runner links the current checkout's `build/win32/Release` libraries.

## Limits

Networking, full factory, recorder, startup and gameplay remain open. Isolated codec comparisons do not prove game behavior or whole ABI compatibility. Original CRT identity/FH3, allocation failure, malformed input, arbitrary aliasing, concurrent mutation, unmasked FP exception ordering, nondefault DAZ/FTZ and x87 precision are unproved. Borrowed context, numeric cells and profiles must outlive their callers.
