# Observer dispatch publication and owner lifetime

Addresses: 00693C70, 00693E10, 00695ED0, 00695F10, 00695F40, 00696360,
00696420, 00CCD6A0. Packet `orch6_observer_dispatch_owner_q`.

`00E198E4` aliases the embedded vector at **owning singleton +4**. The actual
14h allocation is published at **00E198DC**. Static initializer `00CCD6A0`
calls the owning getter, adds four to EAX, and stores that address in E198E4.
There is no second allocation or independently registered dispatch owner.
Owner destruction clears E198DC but **leaves E198E4 unchanged**.

The implementation reuses `NativeObserverDispatchStorage`,
`SingletonPointerSlots`, `SoundLifetimeAccess` and its captured manager lock.
It adds `NativeObserverDispatchOwner`: profile DWORD at +0 and the existing
dispatch storage at +4. Assertions establish the native 14h size. It adds
no endpoint/edge graph, private manager, default capacity or callback policy.

| Native routine | Inclusive native bytes | Source coverage |
| --- | --- | --- |
| 00693C70 | 00693C70..00693C80 | Complete base clear; ECX owner, RET |
| 00693E10 | 00693E10..00693E38 | Complete base scalar delete; ECX owner, flags stack, EAX original, RET4 at 3E36 |
| 00695ED0 | 00695ED0..00695F07 | Complete constructor normal stores; ECX raw allocation, EAX owner, RET |
| 00695F10 | 00695F10..00695F3D | Complete nondeleting normal destructor from verified bytes; Ghidra membership hole below |
| 00695F40 | 00695F40..00695F81 | Complete scalar normal destructor; ECX owner, flags stack, EAX original, RET4 at 5F7F; membership holes below |
| 00696360 | 00696360..0069641C | Complete normal getter/registration schedule; native no inputs, EAX owner, RET |
| 00696420 | 00696420..00696424 | Tail JMP to the same getter |
| 00CCD6A0 | 00CCD6A0..00CCD6AD | Complete static publication; CALL getter, ADD EAX,4, MOV E198E4, RET |

These source signatures add explicit borrowed publications/access. They are
not original callable ABI, FH3/SEH or hardware-fault replacements. Existing
`CG_scalar_deleting_dtor_00695f40` identity is retained; descriptive names
elsewhere are hypotheses, not recovered class symbols.

## Construction, registration and static ordering

`00695ED0` writes profile **00CF7E74**, then zeros owner +8, +C and +10.
Owner +4, the vector's unused +0 word, is preserved. There is no constructor
call, reserve, allocation or lock creation inside this constructor. Profile
CF7E74's slot zero contains **00695F40**. The separate base profile CF7E6C
points to 00693E10; the getter's constructor does not produce that profile.

`00696360` first captures E198DC and returns it if nonnull. Otherwise it gets
the actual lifetime manager through 00415350, captures manager+10 in ESI,
enters that section if nonnull, and increments its native +18 recursion word.
The lock remains this captured section across every later lookup/callback.
After a second E198DC check, it allocates **14h**, constructs if nonnull,
and publishes E198DC at 006963E1. It then resolves the manager **again** at
006963E6 before reading the current E198DC at 006963EB and registering that
value through **00BD0C30** at 006963F4. The slow return reloads E198DC after
decrementing/leaving the captured section. The fast return remains captured.

The borrowed `SoundLifetimeAccess` name is historical; its actual-storage
branch uses the already reconstructed raw manager getter and registration.
Registration validates before ignoring null and appends a nonnull object;
the owner adds no duplicate check, unregister, reorder or separate atexit
callback. The source allocator is the existing malloc/new-handler retry
family. The native null-allocation branch is preserved structurally, though
that source provider normally returns nonnull or throws.

The publication write was absent from Ghidra's initial E198E4 xrefs. A raw
scan found operand bytes at **00CCD6A9**, inside the previously undefined
initializer beginning CCD6A0. Its pointer occurs at **00CE2BAC**. This is
index **286** of the actual C++ initializer range **[00CE2734,00CE36E4)**:
`__cinit` at 00BFBC47 loads those bounds at 00BFBC90/97, skips null entries,
and calls each nonnull pointer at **00BFBCA7** in increasing address order.
The publication initializer itself makes no atexit call. The application
adapter must invoke this recovered publication at the corresponding startup
stage; this module does not install another host CRT initializer.

Native `ADD EAX,4` is retained as unsigned native32 address arithmetic. A
null getter result would publish address 4; no invented null guard is added.
Subsequent calls to 00696360/00696420 do not update E198E4, even if they
recreate the owning singleton after destruction.

## Destruction and integration contract

`00695F10` captures begin at owner+8 and frees that buffer if nonnull, zeros
owner+8/+C/+10, clears E198DC unconditionally, and writes base profile
**00CE3818**. It preserves owner+4. It neither deletes the pointed-to edges
nor unregisters the owner. `00695F40` inlines that sequence, tests flags bit0
after buffer free and before the zero/publication stores, then frees the
owner if requested and returns the original address bits. Higher flag bits
do not request deletion. `00693C70` only clears E198DC and writes the base
profile. Its scalar wrapper 00693E10 optionally frees the owner and never
destroys the derived vector.

Bind **CF7E74** in the actual manager's registered-deletion dispatch to
`delete_observer_dispatch_owner_00695f40`, passing the same live E198DC cell
and native flags1. Do not bind the E198E4 alias as another registered owner.
Existing `NativeObserverLifetime` should borrow that alias for pending-slot
invalidation. The vector must outlive those observer operations. After its
destruction the alias is dangling; native neither clears nor refreshes it.
The normal singleton drain's removal of a registration before invoking its
deleter is a separate manager responsibility. No concurrent shutdown safety
or complete process-wide destruction order is established by this packet.

## Ghidra and exception boundaries

The integrator defined CCD6A0, 696360, 696420 and 693E10 under the write lock;
CCD6A0's definition discovered 696360 automatically. Live verification then
confirmed the getter's exact 6360..641C envelope. The integrator also restored
three missing `83 C4 04` instructions after free calls. Full listings now
decode them, but Ghidra's body sets still exclude **00695F23..25** from
695F10 and **00695F53..55 / 00695F78..7A** from 695F40. Complete decoding and
control flow do not imply complete body membership. The source and probe
use the independently verified complete PE byte spans, with these holes
reported explicitly. No worker Ghidra mutation was made.

Constructor FH3 metadata DAB7D8 points to the base cleanup through C7EAF0.
Getter metadata DAB864 maps state0 to captured-guard destruction through
C7EB50 -> 00411EE0 and state1 to allocation free through C7EB58. That last
unwind function currently ends at its free CALL's last byte C7EB60; verified
POP ECX at **C7EB61** and RET at **C7EB62** remain outside its body. These
native unwind handlers are not executed by the normal-path fixture.
Registration failures in C++ still release the captured section through the
existing guard and retain the published owner. The constructor has only
stores and no regular C++ throw site; native hardware-fault cleanup is not
fabricated with a synthetic throwing constructor callback.

## Verification and retained artifacts

Win32 Release and both existing CTests pass. One ignored original-byte
fixture passes **41 canonical result words** over all eight owner spans
(**434 installed/live-verified bytes**). It covers construction over A5
backing, complete field preservation, nondeleting buffer cleanup, both base
operations, flags2 retention/flags3 deletion, the actual static initializer,
slow getter registration, fast getter/thunk reuse, balanced real Win32 locks,
and alias/no-unregister behavior after destruction. x87 TOP remains balanced.
The call-site gate reports 26 checked rows and zero failures. The two Win32
import identities and the CRT table's indirect target remain explicitly
outside its direct-target verification.

The native side relocates 11 relative transfers and 12 absolute operands.
The eight owner routines execute original instructions. Manager getter and
registration, CRT allocation/free and Win32 imports are shared concrete
external providers. The actual raw manager is allocated/registered into;
there is no fake pool count or callback pretending to be a manager. Its
original binary implementation and independent VS2005 CRT parity are not
executed/proven by this fixture. Profile DWORDs and native FH3 handler
immediates retain their original identities; handlers are not invoked.

Returned owner and embedded-pointer identities are canonicalized across
separate allocations. The getter's uninitialized owner+4 word is excluded
from cross-allocation equality; the preseeded constructor comparison proves
its preservation separately. Manager allocator metadata, unused manager+0,
OS-internal critical-section fields and freed allocation contents are not
compared. Cleanup explicitly removes the fixture's dead registration,
releases the raw manager section/vector and manager allocation, and frees
the executable mapping. This teardown is not mixed-profile manager-drain
proof. No game runtime, dispatch event producer or observer invocation is
implemented or validated here.

From the retained worker worktree:

```powershell
python tools/ghidra_export.py verify-seeds
./scripts/build.ps1
python local/prepare_observer_dispatch_probe.py
cmd /c local\build_observer_dispatch_probe.cmd
./local/observer_dispatch_probe.exe
python tools/verify_report_calls.py reports/observer_dispatch_owner.json
```

Run the probe only after compilation succeeds. The executable links with
`/MANIFEST:EMBED`. The preparer requires existing pefile/capstone and verified
live `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. The report hashes
the ignored source, manifests, bytes, logs, executable, and actual linked
`build/win32/Release/bsp_core.lib`. All dependencies and evidence stay under
this worktree; no retired worker tree is required.
