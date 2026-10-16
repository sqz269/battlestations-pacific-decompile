# Actual Screen reader-entry fragment, packet 79

`continue_native_gui_page_reader_00ac67cc` reconstructs only the 58 bytes
`[AC67CC,AC6806)` of Original AC6600. It constructs the actual globals/reader
on packet72's SAME still-open Lua owner, enters actual `GuiScreen`, and returns
pending properties at AC6806/EH5. The page remains unfinished. There is no
property dispatch, node-root propagation, reader destruction, Lua close or
page publication in this packet.

Readiness77 is commit `8c09e86aed44c6c4c4f2ae14e5661e3fdfeb83ac`.
The real72 dependency is `d12e5827c435f4b6f12c26d90b9f3bfd4f8fb83a`,
using actual71 commit `76eb01c2bdc9375ad92517a426ee3c363ca7b88a`.
No substitute Lua backend, script, table owner, tracked slot or runtime fixture
was introduced to make the fragment appear executable.

## Evidence and exact argument transfer

Verified existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe` through the canonical live wrapper before the
bounded analysis/annotation batches. AC6600 is `[AC6600,AC6879)`,633B,
Original ECX actual124h page, stacked name-header pointer/node pointer/flag DWORD
(low byte), EAX same page and RET0C at AC6876. The new function is a typed C++
fragment API, not a callable Original subfunction or constructor ABI adapter.

| Original site | Implemented action |
|---|---|
| AC67CC..D5 | Persistent actual14h globals-argument storage replaces the parent's `SUB ESP,14h` temporary.14h is20 decimal bytes, not20h. |
| AC67DA | Existing B67980 fills that SAME storage: owner, kind1, indexFFFFD8EE, opaque0, tracked0; padding remains unchanged. No Lua call. |
| AC67E3 | Existing4425C0 receives that SAME object by reference in the Source API, adopting the native by-value argument's cleanup. No extra copy/ref/registry projection. |
| AC67E8..F5 | Capture actual D5CB24=`GuiScreen`; read CURRENT reader profile, then CURRENT table+04. |
| AC67FC/6804 | Set EH5 before genuine BD8E20(kind0,actual key address). |
| AC6806 | Return with SAME reader and Lua live, awaiting the genuine property tail. |

The existing constructor consumes the actual argument on normal return and
ordinary C++ failure. Its own vector/argument cleanup remains authoritative;
the parent never repeats it. The frame records adoption before constructor
entry. Enter uses the existing BD5790 lookup and442220 vector path; its SAME
persistent fresh14h scratch is first initialized by the genuine B65F50
prelude. Opaque/padding bytes remain unchanged. The reader-header comment was
corrected to describe the already-existing fresh-or-unbound scratch contract;
no reader implementation changed.

## API, identities and retained lifetime

[The context and frame](../include/bsp/native_gui_page_reader_continuation.hpp:12)
borrow actual D5CB24, SAME `NativeLuaServiceBindings`, and REQUIRED genuine
CE44FC table words (at least two). Supported dispatch is explicitly
CE44FC/current+04=BD8E20. The current profile is read before the current slot;
an unsupported profile/target throws under sticky entered retention. There is
no table cache/default, arbitrary-profile equivalence, numeric-address call or
invented callback. Table/literal authenticity and SAME service ownership are
caller obligations, not authenticated from the supplied pointers.

The metadata-only72 `prefix_identity()` accessor permits
[fresh preflight](../src/native_gui_page_reader_continuation.cpp:58) to reject
a foreign script/prefix pairing before entry. Then the persistent frame enters
BEFORE borrowing72's actual4C8h owner or activating services. The same14h
globals, aligned14h reader backing and14h scratch already exist. The
user-provided constructor adds no blanket native-byte initialization, including
the reader's untouched proxy04 and object padding. Existing enum values are
preserved by appending `awaiting_properties_continuation` only.

[The destructor guard](../src/native_gui_page_reader_continuation.cpp:32)
checks sticky entered/failed metadata FIRST, before reading potentially
interrupted native bytes. On entered interruption, retain the whole frame,
72 owner/prefix, slot, names and providers; host must `_Exit` before any member,
dependency, manager or CRT drain. No retry/rollback or destructor cleanup replay
is offered. A callee may already have consumed or cleaned its local state on
failure; retained identities do not promise that all native contents stay live.

Normal pending destruction is refused while the raw reader lacks its genuine
retired CE374C stamp or SAME owner.state04 is nonnull. A genuine future tail
must independently guard the entire graph BEFORE
[borrowing pending reader/Lua](../src/native_gui_page_reader_continuation.cpp:49),
retire the reader exactly once, close the SAME Lua owner, and quiesce every
borrow before eventual disposal. The72 frame and its owner must outlive79's
borrow. These cell checks express a caller lifetime contract; they do not
authenticate that close/retirement ran. There is no forced settlement, abandon
or mark-complete API. External GUI serialization includes callback reentry.
Source `Activation` restores binding selection, not Lua C frames/handlers or
interrupted native FH3/SEH state. Native-private stack/scratch placement parity
is outside this typed fragment's proof.

## Remaining tail and validation

The later115B `[AC6806,AC6879)` still needs genuine CURRENT page+18 AC4C50
and its raw AAA710/AA6560 dependencies. Worker75 closed exact readiness in
`77c767156a2b13f5b9bc6ca8b1b98edf568e24b9`: AC4C50 whole216B is
`[AC4C50,AC4D28)`; AAA710 still lacks507B `[AAACD4,AAAECF)` and the semantic
seven-case AA6560 factory cannot substitute. Later reader+08 must reload after
properties; node+4C reload follows leave. The EH6 stamp/vector-cleanup seam
identified by77 also remains, followed by CE374C/EH1/actual Lua close. This
packet does not bypass any of that with a callback or double cleanup.

Strict direct MSVC Win32 `/std:c++17 /EHsc /MD /O2 /W4 /WX` compile passed for
the new TU and the72 TU with the updated headers. `scripts/build.ps1` passed
the existing worker build and2/2 CTests; existing LNK4006 duplicate-symbol
warnings remain. The new TU is not registered in this worker's CMake; Root owns
that registration and combined build. No new tests or fabricated runtime
composition were added. No native differential, actual script/backend run,
Screen completion, Original ABI/FH3 parity, normal-host activation, visual or
gameplay proof is claimed. Exact Source/object/build/annotation receipts are in
[the report](../reports/cc12_native_gui_page_reader_continuation_79.json).
