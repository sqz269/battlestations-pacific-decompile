# Dormant raw locale manager constructor

This library supplies normalized ordinary Source interfaces for the complete
normal `00736540` registration base and `0073C960` constructor. It is dormant:
`source_ready=false`. It creates no cell/global/semantic owner/callback, performs
no receiver allocation, and installs no caller, deletion profile or startup path.
The existing `gui_startup` void/noexcept derived-field fragment is unchanged.

`NativeLocaleManagerConstructorContext` borrows the application's actual
`01090AA0` manager and `00F8BC4C` locale publication cells as distinct
`void* volatile&` references. Their bindings stay fixed; values remain CURRENT.
The caller supplies valid four-byte-aligned `4040h` receiver storage, disjoint
stable cells/context and actual raw manager/vector/section/provider storage
through every reached operation and cleanup. An initialized C++ projection or
zero-filled image is not a substitute. Install real profile/lifetime handling
before invoking a constructor which registers; this packet provides none.

The base captures the incoming receiver, arms state0 and writes `CFEA5C`.
Its first genuine `00415350` lookup captures that manager's section at `+10`.
If nonnull it calls actual `EnterCriticalSection`, then increments the current
physical DWORD at section `+18`. It then arms state1, including the null-section
skip path, and publishes the original receiver in the actual locale cell.
Only after the second genuine manager lookup does it reload CURRENT locale and
call raw `00BD0C30` exactly once. Normal release decrements/leaves the FIRST
captured section, remains armed through Leave, then returns the captured receiver.
No duplicate check, refcount, second registry/domain or semantic manager is added.

Saved FuncInfo `00DB5388`/map `00DB5378` proves state1 action `00C85F48`
(`LEA ECX,[EBP-14]; JMP 00411EE0`) followed by state0 `00C85F40`
(`MOV ECX,[EBP-18]; JMP 00412430`). Only those proven actions are represented.
Before guard arming an ordinary C++ escape resets the original receiver profile
through genuine `00412430`. Afterwards genuine `00411EE0` receives the actual
local eight-byte guard, then root reset follows only if cleanup returns.
There is no unarmed unlock. If normal Leave escapes after its preceding decrement,
state1 is still armed and guard cleanup decrements that captured section again;
this recovered behavior is not repaired. A second escaping C++ cleanup terminates.
Publication and any completed/partial registration effects remain on failure:
no clear, unregister, receiver free, rollback or retry is invented.

The full derived constructor calls the base before any derived writes. It stores
`CFF140`, zeros `+8/+C/+10`, memsets exactly `+14..+4013`, zeros count/language
`+4014/+4018/+401C`, then `+4024/+4028/+402C` and `+4034/+4038/+403C`.
It does not touch the inherited words `+4/+4020/+4030` or zero the whole image.
The original complete derived body retains EH state `-1`; its five unarmed map
entries do not justify additional member/base cleanup. It returns the original
receiver, not a later publication value.

The held `37ce9b606` version required live C++ DWORD/pointer fields in raw
storage. The bounded revision removes that restriction: receiver fields,
manager `+10` and section depth `+18` use four-byte `memcpy` loads/stores with
MSVC compiler fences. The captured section DWORD converts through full-width
`uintptr_t` to the actual Win32 pointer; depth arithmetic remains unsigned
wrap before its store. Only the local eight-byte guard and real borrowed cell
references remain typed objects. Alignment, byte addressability, real native
backing and all alias/retention obligations still apply. Compiler fences are
not synchronization or a new concurrent-writer guarantee. EH/control/order
and the untouched words are unchanged; no object-lifetime placement is invented.

Both Original bodies consume ECX, return captured EAX and plain RET; no EDX
payload or stack argument is observed. These Source APIs add explicit receiver
and context arguments. Profile DWORDs are identities, not callable C++ vtables.
Original entry/FH3/SEH, mutable private spill aliases, fault timing, cookies,
provider throw/register identities and general second-failure parity are open.
No vendor CRT/library body is reconstructed or credited.

Receiver allocation/null/caller-unwind policy is separate: the startup window
uses `BF681B(4040h)`, skips the constructor on null and frees only its captured
allocation on state24 failure. This library does not reproduce or repair that
caller policy. Every actual published/registered/current graph must remain
available for caller containment after an entered escape; ordinary raw drain
cannot be inferred safe from a failed constructor or null/non-null cell alone.

`007365E0`, full raw `0073C9F0`, scalar deletion, canonical host cell/dispatch,
loading/parser/map/sidecar ownership and uppercase/Text consumers are excluded.
Construction supplies genuine initial state only, not a loaded locale. In
particular this cannot waive the frozen ac60 Viper19/Font48 admission HOLD.
Root owns later CMake/ledger/integration and activation. No runtime, Original ABI,
full-caller, loaded/uppercase/Text readiness or production claim follows.

Evidence: saved complete `00736540..007365D0` (145B) and
`0073C960..0073C9EC` (141B) assembly, prior accepted locale-constructor readiness
EH captures, and the same genuine providers already used by
`native_allocation_stats_constructor`. Compilation evidence is recorded in
`reports/native_locale_manager_constructor.json`; an object-only check does not
link, activate or validate a game path. No new tests, probes or executables.
