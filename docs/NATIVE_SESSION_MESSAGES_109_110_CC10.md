# Native session messages 109 and 110 (CC10)

Addresses: constructors 00761A80/00761B50; ordinary cleanup 00761AF0/00761BC0; predicates 00761B00/00761BD0; scalar cleanup 00761B30/00761C00; profiles 00D03450/00D03464; reused codecs 006D2F20/006D1550.

## Scope and coverage

Eight complete normal bodies, 354 bytes, and two five-slot profiles are reconstructed. Both message objects are aliases of existing `NativeSessionMessage98ScalarStorage` (24h bytes); both profiles borrow the existing `NativeSessionMessage99To102Context`. There are no new codec, allocation, CRT, or library implementations. Names are descriptive hypotheses.

| Role | Type 109 range, inclusive | Type 110 range, inclusive | Coverage |
|---|---|---|---|
| Constructor | 00761A80..00761AE5 | 00761B50..00761BB5 | complete |
| Ordinary cleanup | 00761AF0..00761AF6 | 00761BC0..00761BC6 | complete |
| Predicate | 00761B00..00761B24 | 00761BD0..00761BF4 | complete |
| Scalar cleanup | 00761B30..00761B4E | 00761C00..00761C1E | complete |

| Profile | Delete | Write | Read | Predicate | Slot 4 |
|---|---|---|---|---|---|
| 00D03450 | 00761B30 | 006D2F20 | 006D1550 | 00761B00 | 004499C0 |
| 00D03464 | 00761C00 | 006D2F20 | 006D1550 | 00761BD0 | 004499C0 |

The source profiles copy the actual writer/reader adapter addresses from `NativeSessionMessage100Profile`, preserving adapter identity with 99/100/102/104/105. Five native-shaped slots precede the borrowed source context pointer; full original binary ABI is not claimed.

## Construction and retained storage

Both constructors take ECX=this, return the captured pointer in EAX, and end in RET. They first capture ECX in EAX, save EBX and zero EBX, then store delivery04=3, fields08/0C=0, base profile D02C68 and one byte of mutable type10 (109 or 110). They read Game E188A8, then signed index18EC: 0..7 selects owner18CC+4*index; otherwise owner14 becomes NULL.

MOVSS then captures borrowed CF8B3C (live bytes `00008043`, 256.0). After this load, sender18, relay1A and flag1C are cleared, delivery04 becomes 1, scalar20 receives the raw captured bits, and the final profile is installed. MOVSS preserves signaling-NaN payload bits without x87 conversion. Source composition calls existing concrete 0075B430 for the identical inline base stores and owner capture; this is not an additional native CALL row.

The constructors preserve padding11..13, 1B and 1D..1F, and have no payload beyond scalar20. Reused readers modify only their documented header/scalar fields. All unrelated storage remains intact.

## Predicates, cleanup, and shared codec

The predicates compare the entire stack DWORD against their fixed type, 98, 73 and 70. They ignore ECX and mutable type10; high-bit or type+256 requests are not narrowed. Both branches return with RET4; the final RET4 starts at 00761B22/00761BF2.

Ordinary cleanup writes only root profile CE4974 through ECX and returns. Scalar cleanup tests bit 0 of the low flags byte, captures this in callee-saved ESI, stamps CE4974, optionally calls existing `_free` BF65AC, and returns captured ESI in EAX. Each free CALL has one stack argument, confirmed by ADD ESP,4 at 00761B46/00761C16. Scalar methods return with RET4 at 00761B4C/00761C1C. The exact ESI writes and both constructor EBX writes were checked across their whole bounded listings.

The shared scalar codec is the existing implementation documented in [R203](NATIVE_SESSION_MESSAGES_99_TO102_R203.md): extended header, flag1C, unordered-or-above-threshold presence; present values use x87 multiply by four and the existing CRT selector before signed-low-EAX clamping. Absent reads overwrite scalar20 from D7A260. No shared algorithm or provider contract changes in this packet. Writer ECX=this plus raw cursor returns RET4; reader ECX=this plus stream uses cursor+4 and returns RET4. Slot 4 reuses the inspected 004499C0 always-true body.

## Factory and body ownership

Live bytes at 0076A448 map 109/110 to raw entries 00768FB2/00768FD2. Each allocates 24h through BF681B, with ADD ESP,4 at 00768FB9/00768FD9, and calls the matching constructor at 00768FC6/00768FE6. Live Ghidra reports no containing function for either constructor call; these remain separately recorded unowned edges. No factory implementation or parent ownership is invented.

The four predicate/scalar methods originally lacked function definitions. The integrator defined them under the shared write lock; the worker then verified the exact live bounds, zero listing gaps and both owned CALL rows. The report retains exact starts, last instruction starts, last inclusive bytes, end-exclusive bounds and native bytes. This worker makes no Ghidra mutations; integration records the definition receipt separately in `reports/native_session_messages109110_cc10_definitions.json`.

## Validation and limits

`./scripts/build.ps1` passes the strict MSVC Win32 build and both current CTests (`reconstructed_math`, `tool_tests`). The final core-library hash equals the library used by the differential fixture.

The isolated differential fixture passes **50,763 original/source pairs and 47,991,805 matching bytes**. It derives from R204, verifies the installed PE hash against the historical support image, and adds 836 cases / 120,456 matching bytes: 184 constructor/ordinary-cleanup, 512 full-DWORD predicate, 12 scalar cleanup, and 128 wire/profile cases. Constructor cases cover seven signed owner-index cases, four retained-storage patterns, and 16 raw borrowed defaults, including NaNs. Wire smoke coverage includes all eight bit offsets, both CRT selectors, and a repeated absent read. Shared writer/reader adapter identity is asserted explicitly. Five source-only cleanup faults are inherited coverage; this packet makes no new exception claim. No tests are added to the committed suite. `reports/native_session_messages109110_cc10.json` records the results, artifacts, hashes and merged-library rerun instructions.

The local collector compares 470 fresh live/PE bytes: all eight bodies, both profiles, the default cell, dispatch entries and two factory entries. The source/report distinguish complete body reconstruction, strict compilation, differential evidence, and game validation. No game was launched and no startup, networking, full factory, gameplay, or full binary-ABI claim is made. Borrowed contexts/cells must outlive calls and profiles. Original CRT/FH3 behavior, allocation failure, malformed streams, arbitrary aliases, concurrent mutation, unmasked FP exceptions, nondefault DAZ/FTZ/precision remain unproved; historical support fixture coverage is not new packet evidence.
