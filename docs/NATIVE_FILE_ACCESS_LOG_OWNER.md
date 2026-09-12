# Native file-access-log owner lifetime

The actual object published in `0109CEE8` is a four-byte vtable holder. Its
derived virtual0 is `00737CC0`, which calls `00737540` and then frees the captured
owner only when flags bit0 is set. There is no retained subject, stream, or file
handle in these complete lifetime bodies. Descriptive names are hypotheses.

| Entry | Inclusive end | Coverage | Original ABI |
|---|---|---|---|
| `007374A0` | `00737530` | complete | ECX owner; RET; EAX owner |
| `00737540` | `007375D8` | complete | ECX owner; RET; no semantic result |
| `007376A0` | `007376BD` | complete installed body; stored listing gap B5..B7 | ECX owner, stacked flags DWORD; RET4; EAX original owner |
| `00737C40` | `00737CA5` | complete | ECX owner, stacked native8h subject value; RET8; EAX owner |
| `00737CC0` | `00737CDD` | complete installed body; stored listing gap CD5..CD7 | ECX owner, stacked flags DWORD; RET4; EAX original owner |

## Producer, publication, and destruction

The sole `737C40` caller is application initialization `73D410`, at `73D9D3`.
`73D99E` pushes4 for allocation, `73D9A7` cleans four argument bytes, and
`73D9C2` reserves the eight-byte subject argument. `73D9CC` constructs that
argument, then `73D9D1` supplies the captured allocation in ECX. `737C40` consumes
both subject words with RET8; its base call takes no subject arguments.

`7374A0` stores `CFEA6C` in the owner's first word, gets the existing lifetime
manager, and captures that manager's section at +10h in ESI. A nonnull section is
entered and its current DWORD+18h incremented. The constructor publishes the
captured owner at `7374F6`, calls the manager getter again at `7374FC`, and only
then reloads the **current** `0109CEE8` at `737501` for registration. Registration
is `BD0C30`, RET4. The captured first section is decremented and left, even if a
provider changes a publication. The constructor returns the captured owner.

`737C40` captures the subject data pointer after the base returns, stores derived
table `CFEAE0`, disarms its subject cleanup, then, for nonnull captured data,
reads length+1 with DWORD wrap and releases through the existing pool. `419CC0`
takes no arguments and leaves the already-pushed return arguments intact;
`BD1510` consumes `(data, length+1, 1)` with RET0Ch. No subject byte is retained or
changed. Source accepts the actual eight-byte argument header explicitly;
`ActualNativeStringPoolStorage` supplies the established pool implementation.

`737540` first stores `CFEA6C`, captures/enters the first manager's section,
gets the manager again, and unregisters the **current global**, which need not
equal the captured owner. It then clears `0109CEE8` unconditionally, leaves the
captured section, and stores singleton-base table `CE3818`. `BCFCA0` consumes one
stack argument with RET4, clearing only the first matching lifetime-vector slot.

Live table bytes establish `CFEA6C[0]=7376A0` and `CFEAE0[0]=737CC0`. Both
deleting wrappers capture owner in ESI, directly call `737540`, test the low
flags byte's bit0, and optionally call CRT free `BF65AC`. The omitted stored
bytes are `83 C4 04` (ADD ESP,4). Execution then reaches MOV EAX,ESI and RET4 in
both cases. The decompiler's `extraout_EAX` return is wrong. `BF65AC` itself is a
five-byte jump to CRT free `BF9DC8`. No replacement function start is invented at
either gap. The primary integrator may repair the erroneous no-return call
overrides at `7376B0` and `737CD0`, then refresh exports.

All nine static references to `0109CEE8` were queried. The two writes and two
lifetime argument reads are above. Consumers are `BE1F87` (capture, load current
vtable0, invoke with flags1 at `BE1FA1`), `73D996` (creation guard), `4E55AF`
(startup guard), `BDEAC4` and `BDEB58` (nonnull logging gates). A full body review
of `BDEB40` also finds only builder construction/appends/destruction after its
gate; its full reconstruction is outside this packet. Other virtual methods
and game execution of the log object are not claimed here.

## Exception states and shared services

Constructor handler `C86190` uses FH3 metadata `DB56F8`, map `DB56E8`.
Destructor handler `C861B0` uses metadata `DB572C`, map `DB571C`.
Both state0 entries tail-call `412430`, which writes `CE3818`; state1 first
destroys the captured guard through `411EE0`, then proceeds to state0. State1
is armed only after entry/increment. If a second getter or register/unregister
throws, source therefore releases the captured section and restores `CE3818`
without undoing the publication or inventing an unregister on constructor
failure. Destructor unpublication happens only after unregister returns.

Derived constructor handler `C86248` uses metadata `DB5810`, map `DB5808`, with
one cleanup `C86240` tail-calling `41DD20` on the current stacked subject header.
State0 is armed before base construction and reset to -1 before normal release.
No derived-object rollback is armed around that final release. The established
`NativeStringStorage::release` is noexcept and covers a returning pool getter;
throwing getter/free, hardware-fault cleanup, original FH3/SEH register and spill
identity, and binary calling-convention replacement remain outside this API.

`NativeFileAccessLogLifetimeBindings` borrows the application's actual global
cell and existing `SoundLifetimeAccess`; `CapturedSoundLifetimeSection` preserves
the captured section. Its name is historical: this existing bridge supports
both the canonical actual manager cell and the existing semantic fixture domain.
No private pool, manager, dispatch registry, publication, or log sink is added.
Object release uses the existing `singleton_lifetime_free` CRT boundary.

## Historical corrections and verification

The `APP_INIT_BOOTSTRAP.md` description that this object is "never stored in a
global and never freed" is superseded by producer `7374F6`, consumer `BE1FA1`,
and deleting wrappers above. Historical semantic names/records are preserved;
new actual-source records state their separate scope.

All five complete installed bodies and their next16 bytes, tables, three FH3
maps/handlers, and consumed guard/base/free evidence were compared with saved
Ghidra bytes. The report retains 15 matching spans totaling832 bytes and the
installed PE SHA256. The Ghidra project/program were verified by every CLI
analysis batch. This worker did not mutate Ghidra or the game installation.

The ignored native fixture maps the frozen installed image at50000000 and
executes these five bodies. Attempt02's original00400000 reservation failed with
Win32 error487 before any native owner ran. Attempt01 failed with empty output;
its exact stage is unlocalized, and associating it with the reservation failure
is an inference from attempt02. Both failed attempts are retained. The
fixture changes only eight verified global/IAT DWORD operands in the bodies:
7374F8/737503/73759D/7375AB and7374E9/73751A/737589/7375BE. Native table tokens,
relative branches/calls, stack instructions, and FS:[0] operations remain
unchanged. This explicit instrumentation is not general PE relocation or
original exception-handler execution. It bridges the two getters,
registration, unregistration, pool return and free to established source
providers, and binds the two IAT section calls to actual Windows APIs. Its
fixture executable has fixed high image base30000000 and an embedded manifest.
It uses the same actual manager/global/pool cells for source and native runs;
the borrowed manager has preallocated raw slots and an optional real section,
and the actual pool is preconstructed. Lazy domain creation/drain is not tested.

Six scenarios compare constructor/destructor tables, lifetime slots, current
global unregistration (including a publication different from owner), flags
0/1/2/101h, returned owner, section depth, untouched subject header and actual
small-pool return. This is a normal-path instruction fixture, not original FH3
execution or a game run. No permanent test is added. Exact results, frozen
inputs and build evidence are recorded in `reports/native_file_access_log_owner.json`.
