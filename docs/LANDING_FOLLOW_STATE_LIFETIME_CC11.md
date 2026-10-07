# Follow state-root lifetime — CC11

The complete ordinary Follow cleanup and profile scalar cleanup now have
conditional Source providers in `native_land_follow_state_lifetime.hpp/.cpp`.
They borrow one actual Follow receiver, use the already complete callback18 and
shared-state providers, and preserve optional **root** release after both finish.
No constructor, task arena, executable profile or observer world is supplied.

## Native bodies and actual consumer

All ends are exclusive. Disk/live bytes match for both bodies: **172 bytes,
46 instructions**. Five direct calls are checked in the accompanying report.

| Entry | Exclusive end | Bytes / instructions | Original interface |
| --- | --- | --- | --- |
| `007B6630` | `007B667C` | 76 / 20 | ECX actual Follow root; RET, saved ESI/native SEH frame |
| `009C2A60` | `009C2AC0` | 96 / 26 | ECX root, stacked DWORD flags; EAX original root, RET4; saved ESI/native SEH frame |

Ordinary calls at `007B6658/007B6667` target `006CDD70/007B45F0` in that order.
Scalar calls at `009C2A88/009C2A97` target the same providers, then `009C2A9C`
tests low-byte flag bit0. The optional `009C2AA4` free targets `00BF65AC`.
Primary repaired the returning `ADD ESP,4` at `009C2AA9..009C2AAC`, saved and
refreshed the export, with zero remaining call gaps. See
`reports/follow_state_scalar_flow_recovery_cc11.json`; no worker mutation or
body-extension attempt occurred.

The actual D20AB8 table has slot zero `009C2A60`; native constructor `009C2980`
stamps it at `009C29B5`. That complete scalar normal path is the natural provider
boundary. The 44 known direct `007B6630` callers are private-unwind funclets:
they establish reachability, not a normal caller/private-EH binding. The profile
is **raw and uncallable** in Source; no fabricated table or class adapter is
introduced. Native `009B2C80` also spells the member/shared sequence inline but
is not newly reconstructed or admitted here.

The shared provider retains its established distinction: full ordinary
`007B45F0..007B4644` is independently proved by 84 disk/live bytes and known
boundaries, while stored Ghidra function metadata still ends inclusive `4629`.
This packet does not silently promote that stored body to complete metadata.

## Same actual root and storage domain

Cleanup accesses the root profile, vector at `0Ch`, callback at `18h`, and its
FIRST-endpoint14 at root `2Ch`: a 30h reached prefix. That prefix is **not** a
whole Follow object. A real receiver needs at least **98h** backing: actual
entry `009BEDA0` stores DWORD root94, and composite `009B2ED9/2EE4` constructs
Follow at approach108 before the next state at approach1A0 (a 98h interval).
The established outer Follow identity is task500. Constructor initialization
through8C does not justify allocating a smaller stand-in.

`NativeLandFollowStateCleanupView` retains the original root identity, actual
shared profile00/vector0C aliases, and the existing callback18/watched2C observer
view. Its pure admission function compares only addresses and extent, then
composes existing references. It checks the existing entry4/18/2C/6C/84/85/88/
8C/90/94 aliases against the **same root**. It performs no represented field
read, callback, native call, allocation, defaulting or pointer translation.
Null, undersized or mismatched views are Source admission errors, not native
recovery. Direct aggregate callers must satisfy the same contract.

State2C must already be the actual `006952A0` FIRST-endpoint identity. There is
no whole-unit or endpoint-offset assumption. Required actual observer manager,
current lock/dispatch, edge services and endpoints remain borrowed and live.
The genuine shared provider's actual24B/CF5C94/count/capacity/CRT contracts apply;
the unrelated 90h node API is not used.

## Destruction and optional release

Ordinary Source calls complete `destroy_native_land_follow_observer_006cdd70`
on callback18, then complete `destroy_native_land_state_007b45f0` on the same
root. The callback's final profile comes from genuine `00695870` (CE3CD4), and
the root's final profile from shared cleanup (D056D0). The member14/root2C cell,
padding and tail remain as those native providers produce them. There is no
CF8900/D20AB8 restore, first-cell clear or dangling-header reset.

Scalar Source captures the original **root** identity before the helpers,
completes the same sequence, then observes low-byte flag bit0 and optionally
uses canonical CRT free on that root. It returns the captured identity without
post-free field/view access. It does not call member `006CDDF0(flags1)`.
Flags1 requires a **separate complete actual same-CRT Follow receiver >=98h**,
never embedded task500, callback18, a vector element, enclosing storage or the
whole task. Flags0/ordinary cleanup may borrow the actual embedded receiver.

Native scalar inlines both cleanup calls. The new C++ interface may inline its
ordinary helper and uses a qualified volatile flag observation; it is not the
native TEST/RET4 or class-profile ABI. Source COFF confirms captured root at0A,
callback provider at10, shared provider at1D, then low-byte load22/mask29 and
optional root free33, followed by captured EAX identity3C. Ordinary COFF calls
the callback provider at0D before shared at1A.

Admit coherent live actual backing/pure aliases, stable real endpoint/vector
contexts, nonwrapping ranges and ordinary successful same-CRT operations.
Structural reentry, alias/header mutation, concurrency, invalid storage/profile,
allocation/fault/private-EH recovery and owner/death/queue lifetime guarantees
remain excluded. Full constructors, state entries, approach9B2C80, arena,
executable profiles, actual manager/game binding and native ABI remain unbound.
Exit `009BDE40/007B8A90` is a separate scope and is not adopted.

## Focused verification and handoff

Two fresh strict MSVC Win32 TUs (`/std:c++17 /EHsc /MD /O2 /Gy /DNDEBUG /W4
/WX`) and an embedded asInvoker manifest probe passed **78 assertions**. One
actual98h Follow fixture binds its existing entry/shared/observer views to the
same storage. Real duplicate callback registration forces unregister then base
detach; a real24B vector element supplies the second cleanup identity. Actual
pending slots and a captured recursive Win32 section verify suppression and
depth restoration. Final fields are inspected while their containing storage
is still live, with no dereference of freed backing.

Source scalar0 retains an embedded borrowed root; the complete original76B
ordinary body executes equivalent nonrelease cleanup. Source and complete
original96B scalar1 use a separate actual98h CRT receiver with null2C. Only
**five natural CALL operands** are relocated to complete genuine observer,
shared and CRT Source ABI bridges. No fields, branches, constants or profile
table are patched. The original96B scalarflags0 branch is not separately run;
its complete byte/control-flow evidence is retained. No extra sweep or tracked
test is added.

Ignored instrumentation traces this executable's own CRTfree import, always
forwards the captured real operation, snapshots live final fields **before**
root free, and restores the import/protection. It is not a production provider,
behavioral reentry or historical CRT/loader/native runtime proof. Original SEH
frames execute only successful ordinary paths. Fixture preimages are not full
constructor or state-entry invocations. The actual manager lookup is unexercised
under the owned prepublished Source lock; runtime world/lifetime binding remains
required.

The report is the manifest for 40 pre/post-equal inputs, two fresh TU commands,
three support libraries frozen from the current `d670125fc8` main build with
original/copy/after equality, artifacts, native calls and exact body hashes.
Probe paths are the ignored `local/cc11_land_follow_state_lifetime_probe.cpp`
and `.exe`; Source object is `local/cc11_land_follow_state_lifetime.obj`.
Root owns CMake registration, full main build, annotations, metadata and
independent integration. Workers made no shared or Ghidra writes.

## Primary integration

Main `a121efaa03744877e75c4ef324181d9744d863f0` passed the full Win32 build and all three existing CTests. Root independently rebuilt five actual Follow-state/Follow-observer/shared-state/observer-lifetime/fixture TUs, verified 43 current Source/header/fixture inputs (35 compiler includes), three current libraries and the PE before/after, and reproduced 78 assertions. Both complete bodies (172 bytes) match disk/live/fixture literals; five direct calls passed. Fresh emitted helper, post-helper low flag, root free and returned identity ordering was inspected. Original scalar flags0 remains unexecuted, while original ordinary and scalar flags1 paths passed; all constructor/manager/EH/ABI/game qualifications remain. The PE32 asInvoker manifest was verified. No tracked tests were added.
