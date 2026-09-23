# Native session messages 107 and 108 (CC10)

Addresses: 007618E0, 00761950, 00761960, 00761990, 007619B0, 00761A20, 00761A30, 00761A60. Profiles: 00D03428, 00D0343C. Reused codecs: 006D2F20, 006D1550.

## Scope and boundaries

Eight complete normal game bodies (354 bytes) and two five-slot profiles are reconstructed. The 24h storage and scalar context reuse [R203](NATIVE_SESSION_MESSAGES_99_TO102_R203.md); no library or shared codec implementation changes are made. The factory, networking and gameplay paths remain outside this packet. Source names are descriptive hypotheses. The existing `BSP_ShipMessage_SetEngineJamFailure_Construct` name at 007619B0 is preserved.

| Type | Routine | Start | Last instruction | End exclusive | Bytes | Coverage |
|---|---|---|---|---|---|---|
| 107 | constructor | 007618E0 | 00761945 | 00761946 | 102 | complete |
| 107 | ordinary cleanup | 00761950 | 00761956 | 00761957 | 7 | complete |
| 107 | predicate | 00761960 | 00761982 | 00761985 | 37 | complete |
| 107 | scalar cleanup | 00761990 | 007619AC | 007619AF | 31 | complete |
| 108 | constructor | 007619B0 | 00761A15 | 00761A16 | 102 | complete |
| 108 | ordinary cleanup | 00761A20 | 00761A26 | 00761A27 | 7 | complete |
| 108 | predicate | 00761A30 | 00761A52 | 00761A55 | 37 | complete |
| 108 | scalar cleanup | 00761A60 | 00761A7C | 00761A7F | 31 | complete |

The two predicates and two scalar cleanups initially lacked Ghidra function ownership. Their inclusive final bytes are 00761984, 007619AE, 00761A54 and 00761A7E respectively. Raw PE disassembly and matching live bytes establish the exact bounds; the primary integrator handles definitions and annotations under the shared write lock. This worker performs no Ghidra writes.

## Constructors, ABI and retention

Both constructors receive the message in ECX, capture it in EAX, preserve EBX and return EAX with plain RET. The complete EBX provenance is PUSH EBX, XOR EBX,EBX, zero stores, POP EBX. Initial stores set delivery04 to 3, clear DWORD08/0C, install base profile D02C68, and write only the type byte10 (107 or 108). Game E188A8 supplies signed index18EC: indices 0..7 read owner18CC+4*index, all other values store null at message14. Source reuses concrete 0075B430 for this equivalent inline sequence; neither native constructor contains a CALL.

MOVSS then captures borrowed CF8B3C (normally 256), before clearing WORD18 and bytes1A/1C. Delivery04 becomes 1; MOVSS stores the captured bits to DWORD20, and the final store installs the own profile. The default bit copy preserves signaling NaNs and FP status. Bytes11..13, 1B and 1D..1F retain prior contents. There are no fields after scalar20 in these 24h allocations.

All observed constructor call sites supply the message through ECX. Factory sites 00768F86/00768FA6 have no stored containing body. Other 108 sites are 00813530 in 008132C0 and 00827D65 in 00827B90, each preceded by LEA ECX of stack storage. Their callers are analyzed only; no wider engine-failure behavior is reconstructed here.

## Profiles and cleanup

| Type | Profile | Slots: scalar cleanup / writer / reader / predicate / true |
|---|---|---|
| 107 | 00D03428 | 00761990 / 006D2F20 / 006D1550 / 00761960 / 004499C0 |
| 108 | 00D0343C | 00761A60 / 006D2F20 / 006D1550 / 00761A30 / 004499C0 |

The source profiles use the same writer/reader adapter addresses as profile100 and borrow the existing `NativeSessionMessage99To102Context`. The inherited source-only context pointer follows the five slots. The context and its pointed-to cells must outlive profile calls. Full drop-in binary ABI compatibility is not claimed.

Each predicate takes a full DWORD at stack+4 and RET4, ignores ECX and mutable byte10, and accepts exactly its own type, 98, 73 or 70. Values differing only above the low byte are rejected. Both ordinary cleanups receive ECX and stamp only profile CE4974, then RET. Scalar cleanup tests flags bit0 at stack+4 before saving ESI; ESI captures ECX, the root profile is stamped, and only a set bit0 calls existing `_free` BF65AC. CALL sites 007619A1/00761A71 each push one pointer followed by ADD ESP,4. The captured object identity is returned in EAX; RET4 consumes flags. No CRT internals are newly implemented.

The table at 0076A440 maps 107/108 to raw blocks 00768F72/00768F92. Each requests 24h via BF681B, cleans the cdecl argument with ADD ESP,4, and calls its constructor. Allocation CALLs 00768F74/00768F94 and constructor CALLs 00768F86/00768FA6 remain separately recorded unowned edges. No parent function is fabricated.

## Validation and limits

Strict MSVC Win32 compilation succeeds; the two configured CTests (`reconstructed_math` and `tool_tests`) pass. The extended native fixture compares **50,763 original/source pairs and 47,991,805 matching bytes**. The 836 new pairs comprise 184 constructor/ordinary-cleanup cases, 512 predicate cases, 12 scalar-cleanup cases and 128 shared-wire cases. Five source-only cleanup-fault cases are inherited fixture coverage; no new exception coverage is claimed. All four report CALL rows pass (two owned cleanup calls and two analyzed external constructor calls).

The report records the current PE hash, exact native bytes, function bounds, call rows, build result and isolated differential result. The historical R204 fixture is read-only; its captured PE hash is rechecked before extending a copied fixture in ignored `local/cc10_messages107108`. New fixture cases exercise constructor padding and owner boundaries, 16 borrowed default patterns, full-DWORD predicates with null receivers, scalar flags and shared codec adapter identity. Shared wire checks include all eight bit alignments and both existing CRT selectors; earlier cumulative scalar tests remain intact.

The native comparison substitutes the existing host providers and does not establish original CRT/FH3 behavior, arbitrary aliasing, concurrent mutation, unmasked FP exceptions, malformed inputs or allocation failure. No gameplay execution, networking behavior or whole-factory completeness is claimed. No committed tests are added.
