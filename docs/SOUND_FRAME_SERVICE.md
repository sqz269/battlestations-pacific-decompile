# Sound frame and transition callers

`sound_frame_service.hpp/cpp` reconstructs the complete control flow of
`00735B50`, `004BBD00` and `004CD610`. These are typed C++ entries with explicit
services, not native ABI replacements. Their sound path calls the existing
`0068A670` raw listener provider and the current sound-manager update profile.
The transition's screen, input and cinematic operations remain required
dependencies; this packet does not reconstruct those subsystems.

Analysis used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, verified before live batches. The three complete
listings, direct-callee bodies, receiver producers and incoming call sites were
read. Ghidra was not modified. Names are hypotheses rather than recovered
symbols. `reports/sound_frame_service.json` contains every direct CALL row and
separate indirect dispatch rows for the mechanical verifier.

| Routine | Inclusive native body | ABI | Coverage |
| --- | --- | --- | --- |
| `00735B50` | `00735B50–00735C20` | No effective input; ECX ignored; RET | Complete |
| `004BBD00` | `004BBD00–004BBDCF` | ECX job ignored; unused stack DWORD; RET4 | Complete |
| `004CD610` | `004CD610–004CD6CC` | ECX actual game; stack byte; RET4 | Complete control flow through required dependencies |

The RET4 at `004BBDCD` proves that the queued entry consumes a stack word.
Its decompiler prototype incorrectly lists an EDX input and misses that word.
There is no native read of the entry's ECX, EDX or stack argument. `0068A670`
uses ECX plus two stack output pointers and RET8; the EDX value shown by the
decompiler is unused. The three sound dispatches pass a matrix pointer and
three copied float words, with RET10 in the concrete manager update bodies.

## Scheduling and publications

The inline entry snapshots the raw bits of `D7A24C` once, then captures
`E198C4`. It writes the 64-byte identity matrix and three positive-zero velocity
words before testing the captured interface. It skips `0068A670` only when
that pointer is null. The job entry makes the same initial stores but calls
the listener unconditionally. Neither entry invents a null sound manager or
muted fallback. Both reload `F8BBD8` after listener callbacks, then read its
current virtual slot +4.

The existing canonical source manager supports D5B44C -> `00A87BF0` and
D5B000 -> `00A7E630`. The latter obtains its timestamp through the current
clock virtual +14; the real application binding uses `FrameClock.current`
through `00BEE050`. The frame caller supplies no dt and never updates the clock.
Unknown manager profiles and a missing current source manager are outside this
typed binding's domain and produce an error rather than a success substitute.

`bind_game_sound_frame_service(core, actual_interface_word, listener_context)`
borrows the core's existing publication and update context. It creates no
second system or singleton domain. The listener context must contain the actual
raw interface/game/camera/unit storage used by `0068A670`; a semantic
`InGameInterfaceManager` companion is not a native receiver. Callers must keep
all borrowed services alive and obey the existing sound-runtime serialization
and shutdown contracts.

The inline entry is called at `004CA5D4` in game render and `0045F684` in
`0045F600`; both load `E1AE90` into ECX, which the callee ignores. The queued
entry is the primary virtual of the native 8-byte job produced by `004C0960`:
its primary profile CE753C points to `004BBD00`, and the second word is the
registered singleton base. Existing frame-job code supplies the unused DWORD
(normally zero). This packet does not change job scheduling or rendering.

## Transition sequence and required operations

`004CD610` writes raw game+7184 to 1, captures `E18D48` into ESI, then writes
that screen's wanted/applied bytes +4/+5 to 1. Visibility commit may alter
publications; the following virtual +18 still uses the same captured screen
and its current vtable. If the caller's input byte is nonzero, it obtains the
input singleton and applies `(index=10h,value=5)`. The two values remain on the
native stack across the getter's RET and are consumed by `A933F0`'s RET8.
It then calls cinematic mode on the original captured game with `(1,0,1)`,
whose RET0C confirms all three values.

Five iterations follow, each calling the listener on current `E198C4`, reloading
the sound manager for slot +4, and finally calling the captured Sleep import
with 10 milliseconds. The fifth Sleep is present. The matrix and velocity
scratch are initially unwritten and reused; the transition creates no identity
fallback or interface guard. `004CD66D–004CD66F` contains `8D 49 00`
(`LEA ECX,[ECX]`) skipped by the preceding unconditional jump. It is alignment,
not an omitted executable branch or missing tail. There is no local EH state:
a dependency exception leaves earlier native stores/effects in place.

| Call site | Native operation | Contract / boundary |
| --- | --- | --- |
| `00735BE2` | `0068A670` | Existing concrete listener, conditional on captured interface |
| `004BBD8F` | `0068A670` | Existing concrete listener, unconditional |
| `004CD62E` | `004F83B0` | Required actual-screen visibility commit: slot24 child list, each child slot34 receives current screen+5; retains allocation/EH behavior |
| `004CD63A` | Captured screen virtual +18 | Required current dispatch; constructed movie profile CEAE94 points to `004F8940` |
| `004CD647` | `004BEC00` | Required actual24h input singleton, EAX result; getter ignores the two pending stack arguments |
| `004CD64E` | `00A933F0` | Required input-context write/max/record walk with index10h and value5; arbitrary index/value preserved by service interface |
| `004CD65B` | `004CD0F0` | Required existing cinematic body, including its sound-class/input effects, with all three bytes |
| `004CD680` | `0068A670` | Existing concrete listener, five unconditional passes |
| `00735C1B`, `004BBDC8`, `004CD6BA` | Current sound virtual +4 | Concrete current D5B44C/D5B000 dispatch through the borrowed update context |
| `004CD6BE` | Captured `[CE2230]` Sleep | Real Win32 Sleep(10), no clock synthesis |

The screen receiver producer `004F8AF0` installs CEAE94 in the actual 34h
MoviePlayer. Its +18 entry is `004F8940`, verified as byte C3 followed by CC
padding, but the current Ghidra database lacks that function start. This is a
documented dependency gap, not an address leased or modified by this packet.
The raw game+7184 flag is also used by existing frame/movie completion code;
this service writes the real byte rather than a separate projected state.

The input producer `00A93DA0` initializes the actual 24h manager with profile
D5B630, its vector header at +10/+14/+18, +1C byte1 and +20 word1. `A933F0`
writes that vector at the supplied index, recomputes the unsigned maximum
starting at 1, and calls `A93020` to walk the 30h records at input+4/+8 and
invoke `A92D40` with the context vector, +1C byte and +20 maximum. The detailed
`A92D40` operation is unread in this packet. That record processing and the
native singleton/registration path remain the required input provider's work.
Shared dependency interfaces retain receiver and complete argument domains;
this packet does not narrow them to values from a single unrelated caller.

Both direct incoming `004CD610` sites pass byte1: `004D7F72` in mission-completion
checking and `004E476C` in state-request draining. Their ECX comes from the
captured game ESI. The source also preserves the zero-byte branch, which skips
only the two input operations.

## Verification boundary

MSVC Win32 Release built using `scripts/build.ps1`; seed verification preceded
the two existing CTests, which passed. The report-call verifier checked 11 direct
rows (seven owned calls and four incoming calls), with zero failures; five
indirect rows are explicitly separate. It checks instruction addresses against
current function membership and exact CALL targets. There
are no new permanent tests and no changes to application/render/menu hosts.
This packet does not make an executable-frame, audible-playback or native ABI
claim. The application can attach the concrete frame binding when it has actual
interface/game storage; transition execution additionally requires the real
screen/input/cinematic services described above.

## Primary integration evidence

The primary integrator defined the one-byte RET dependency 004F8940 through
the locked definition tool and saved its hypothesis name and evidence. This
closes the analysis database gap above; it adds no reconstructed C++ routine.
`reports/sound_frame_service_function_definitions.json` and
`reports/sound_frame_service_prototypes.json` retain the old values and readback.

An ignored manifested probe linked against the primary's actual core library
also exercised the genuine null-interface arm of 00735B50. It used the core's
same sound-manager publication, update context and frame clock. Identity
matrix/zero velocity, A87BF0 previous-listener copies and the alternate owner's
real timestamp update were verified. Real FMOD GetChannelsPlaying,
Set3DListenerAttributes and EventSystem_Update calls returned zero. Teardown
left zero errors, pending file adapters and tracked strings. The probe did not
execute 004BBD00 or 004CD610, whose real interface and transition services are
still required. Its worker runner is `local/run_sound_frame_probe.ps1` and its
log is `local/sound-frame-integrated-probe.log` in the frame worker worktree.
