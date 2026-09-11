# Native gamepad force event owners

Addresses: 00869010, 008690F0, 00869290, 00873450, 00873560, 00873750,
00873530, 00873720, 00873860, 00872150, 00872160, 00872180, 00872C90.

Packet `orch3_gamepad_event_owners_ad` and the separately leased base destructor
extend the typed request behavior in `GAMEPAD_FORCE_EVENTS.md` with actual20h
event storage and canonical intrusive ownership. The live project is
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; installed PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Names describe recovered behavior; they are not recovered symbols.

## Storage and constructor order

| Offset | Actual member |
|---|---|
| 00 | primary table: D0DCD0 constant, D0DD08 fading, D0DD40 alternating |
| 04 | sole32-bit atomic reference count, native constructor writes1 |
| 08 | secondary table: D0DCCC / D0DD04 / D0DD3C |
| 0C | active byte1; bytes0D..0F remain untouched |
| 10 / 14 | borrowed subject / component definition; no reference operations |
| 18 | type6 |
| 1C | request handle; untouched until A95BF0 returns |

All three constructors initially write CEB130 and D0C8C0, then publish their
concrete tables before request allocation or spatial callbacks. Native ECX is
the actual20h owner; stack arguments are definition and subject; RET8; EAX=this.
Complete byte ranges, including every RET immediate, are recorded in the report.
The factories use ECX=definition and one subject stack slot, RET4; allocate20h,
return null for a null allocation, or call the corresponding constructor.
The production allocator follows the established BF681B CRT allocation adapter
(malloc/new-handler/retry/throw); a normal failed allocation throws.

The shared request helpers borrow individual component fields. Fading's radius
is read after source/target refresh and distance calculation; channel, amplitude
and duration are consumed after request allocation. Alternating retains the
actual selector byte, including255. Existing typed parameter entrypoints remain
available. Native request storage itself still crosses the existing typed
`GamepadForceRequest` interface: its vtables, allocation sizes, spare bytes and
SEH ABI are not claimed to match the original request allocations. The typed
inner allocation still uses `new(nothrow)`, as documented in the earlier packet.

## Lifetime and actual dispatch

`NativeGamepadForceEventReference` borrows the atomic object at the owner's+04.
Repeated binding returns the same companion without writing or retaining any
native field. A single application's `NativeGamepadForceEvents` domain owns
only host companions, not another event counter or request registry. All
terminal releases of bound events must use that domain and it must outlive them.
Factories allocate their host companion memory before entering the native
factory; successful submission has no later host allocation failure boundary.
This host preparation is a new C++ composition requirement, outside the original ABI.

The domain checks CURRENT primary table words for generic zero dispatchBD30E0
and the relevant scalar wrapper. Those identical43-byte wrappers restore
D0C88C/D0C888, invoke BD30F0 (primary becomes CEB130), and free only when flags
bit0 is set. Count, handle, active byte and borrowed pointers are not cleared.
**Destruction does not cancel the independent request.** Explicit cancellation
872160 captures the current handle, invokes A957F0 if nonzero, then clears+1C
after the callback. Completion872180 passes the actual+1C address to A957D0 and
tests its resulting value. The genuine noop872150 consumes two unused stack
arguments (RET8), touching no event bytes.

Correction to the older packet's slot labels: live table bytes put noop872150
at primary+28, and cancel872160 at primary+30, not+2C/+34. Completion is+08.
The native secondary table pointer at owner+08 is a separate interface.

`GameplayPointRumbleComponents` composes the three known component factories
with the required remaining component dispatcher. `GameplayPointRows` still
loads the current component virtual+18; unknown functions require their actual
implementation. The returned nonnull companion participates in existing
6FBEB0 assignment and6CF070 temporary release, leaving exactly one row reference.
Full point teardown releases those real event counts and leaves requests alive.

## Exception evidence

Constructor FuncInfo DC8240/DC826C/DC8298 each has one unwind state. Maps
DC8238/DC8264/DC8290 transition0 to-1 through C961F0/C96210/C96230, recovering
the owner from EBP-10/EBP-28/EBP-10 and tail-calling872C90. That18-byte base
destructor restores D0C88C/D0C888 then jumps to BD30F0. There is no handle cancel
or request rollback. Factory cleanup funclets C95260/C952A0/C95300 read EBP-10
and call BF65AC, after constructor cleanup. Handlers C9526B/C952AB/C9530B select
DC7168/DC71C0/DC7238. The new C++ guard/free ordering follows this evidence;
the original C++ exception dispatcher and hardware SEH were not executed.

## Validation and limits

Strict Win32 build and both existing CTests pass. One ignored focused probe
compares complete original constructor/factory/scalar and control/base bodies,
checks ESP balance, full20h preimages (including untouched padding), real request
registry behavior, selector255, repeated binding, actual04 retain/release,
no cancellation on terminal and C++ base-only failure cleanup. Original request
submission is bridged into the same existing typed request registry. The native
fading comparison uses the absent-target path; distance behavior continues to
use the earlier verified helpers and is exercised with real pose bindings below.

The existing AB full-chain fixture was extended with three actual constructed
rumble components alongside shake: real definition/weak cache -> admission ->
8689C0 -> complete8680B0 -> real node/string pools -> three nonnull20h event
owners -> current component factories -> actual point teardown. Each event
ends construction at count1, borrowed pointers match its actual row and point,
fading evaluates0.6 for distance2/radius10/amplitude0.75, and all three requests
survive point/event destruction. Existing name-reentry and failure paths pass.
Fixture device outputs are controlled observations, not physical rumble proof.
There is no game-runtime or original vtable/exception ABI validation here.

## Follow-up packets

Bind remaining component factories to their real owners and establish the
component terminal domain (the full-chain fixture retains one explicit component
reference). Connect current event update/cancellation consumers before claiming
their native call path is integrated. Recover actual request object layouts if
binary storage parity at that boundary becomes necessary.
