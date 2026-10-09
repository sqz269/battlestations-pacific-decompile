# CC12 tick subnode reparent unwind contract readiness

The Native reparent handler now has a concrete descriptor-to-action edge to
the known guard destructor. Its one state record targets `00C964E0`, whose
eight bytes compute ECX=`EBP-14h` and tail-jump to `00411EE0`. This establishes
the encoded edge, with runtime-provided EBP still a condition. No Native
exception dispatch, Source adapter, ABI or gameplay credit is applied.

## Exact owned bytes

All live bytes equal the configured Original PE. The 24-byte code diagnostic
`00C964E0..00C964F7` contains the complete eight-byte action, complete ten-byte
handler thunk and six trailing CC bytes. The latter are preserved diagnostic
padding, not an invented function body. There is no saved Ghidra function at
the handler entry; this packet does not create one or change any listing,
AddressSet, flow, prototype, name or comment.

The action is `LEA ECX,[EBP-14h]; JMP 00411EE0`. It pushes no argument and
does not alter arithmetic flags or EAX/EDX in its owned operations. The
handler thunk is `MOV EAX,00DC864C; JMP 00BF6B43`. That target's current
metadata name is `FID_conflict:___CxxFrameHandler3`; its 54-byte body is
unopened. A metadata name is not a recovered interpreter implementation.

The 44-byte data range `00DC8644..00DC866F` contains exactly the selected
eight-byte state record and following 36-byte descriptor. Its raw descriptor
DWORDs are `19930522, 1, 00DC8644, 0, 0, 0, 0, 0, 1`. They match the current
installed MSVC x86 absolute FuncInfo layout: magic, one unwind state, map
pointer, zero try/map/type-list fields and EH flags one. The selected record
is `toState=-1, action=00C964E0`. Raw data and direct branch targets are
verified; state interpretation and dispatch remain runtime qualifications.

## Guard coordinate and active interval

The accepted 243-byte reparent body registers its three-word frame at
`S-0Ch`, where S is its entry ESP. Its eight-byte guard is at `S-14h`, with
profile `00CE37FC` and captured section at +4. The state-zero write follows
getter, optional Enter/depth, the late node-input read and oldParent load/TEST.
No later explicit state reset occurs before its normal release and FS restore.
The parent body is referenced from its accepted receipt and is not reread.

Current installed `ehdata.h` describes the x86 three-DWORD registration node,
FRAME_OFFSET=12 and PRN_FRAME=registration+12. Its current trnsctrl source
also computes the target frame coordinate that way when an explicit frame
is not used. Under this convention the selected registration gives EBP=S,
so the action addresses exactly `S-14h`. These local compiler sources are
pinned layout/convention comparisons, not Original compiler provenance or
proof of the unopened Native frame-handler register setup or OS delivery.

## Current Source guard and adapter constraint

The actual current Source `destroy_native_singleton_guard_00411ee0(void*)`
borrows eight bytes. It captures guard+4 before writing the raw guard profile,
then on a nonnull captured section decrements the current DWORD at +18 and
calls LeaveCriticalSection. It does not clear/reload that pointer or declare
noexcept. Existing fixture/Original-ABI claims were not replayed in this audit.

Normal reparent cleanup uses the captured EBX section directly. Its encoded
unwind action instead addresses the actual stored guard, whose +4 value can
be changed by raw aliases. An adapter must preserve this distinction and
the state-zero interval; using the same automatic guard destructor for both
paths would lose it. Mutation rollback, successful release, caught exception
sets and Native failure policy are not inferred from the descriptor alone.

This two-file evidence packet reads no Native guard, frame-handler, caller,
neighbor or profile body and runs no build, test or probe. Every Ghidra query
verifies `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Actual Source
consumer/storage/profile bindings, runtime EH and startup/gameplay remain open.
