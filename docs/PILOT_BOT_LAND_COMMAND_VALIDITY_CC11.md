# Retained land command validity source

`cc11_land_command_validity` adds complete conditional normal source for the
land task's retirement predicate `009B3560`, its paired approach validation
`009B34D0`, and their real descriptor-copy provider `007EEDC0`. These callers
use a particular retained task's live cached-squadron and target slots. The
source can supply that task's +40h observation to the existing active-owner
facade once actual adapters exist; it supplies no task arena, runtime owner,
observer lifetime, or Boolean `GameUnitsHost` binding.

The four tracked files are the existing squadron host header/source, this
document, and `reports/pilot_bot_land_command_validity_cc11.json`. Worker
Ghidra access remained read-only. BSP verified project `bsp`, configured
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language
`x86:LE:32:default`, and image base `00400000` before live batches.

## Native coverage and interfaces

| Entry | End exclusive | Coverage | Original interface |
| --- | --- | --- | --- |
| `009B34D0` | `009B3552` | Whole normal approach-validation body | ECX=approach, plain RET; preserves EBX/ESI |
| `009B3560` | `009B35DD` | Whole normal task +40h predicate | ECX=task, EAX=2/1/0, plain RET; preserves ESI |
| `007EEDC0` | `007EEE00` | Whole 19-instruction descriptor-copy body | ECX=squadron, stack output pointer, EAX=output, RET4 |

Primary recovered and defined `007EEDC0`, matched its disk/live 64-byte window,
saved the project, and refreshed its export in main `4e404e4ce`. The worker
checked its complete saved listing and live body membership before porting.
No new register ABI, arithmetic, function definition, or annotation was
inferred by the worker. The source signatures/providers are new C++ interfaces
and are not drop-in native entries.

The established squadron profile `00D087C0` has +174h cell `00D08934` pointing
to `007ED580`, which obtains squadron+348h and tail-calls `0071BE40`. Its
+178h cell `00D08938` points to `007EEDC0`. The land task profile `00D1FFA0`
+40h selects `009B3560`. Unsupported profiles require their actual complete
contracts; these addresses cannot be applied to an unrelated host controller.

## Retained fields and predicate order

`LandTaskCommandValidityView` borrows the slots themselves:

| Task offset | Approach offset | Native use |
| --- | --- | --- |
| +404h | +0Ch | Cached retained squadron |
| +424h | +2Ch | Air-operations block pointer |
| +428h | +30h | Retained target identity |
| +42Ch | +34h | Third pointer field cleared on invalidation; meaning not renamed |

The bounded producer at `009AFEB6` stores `006C0B50`'s return into the
third field. This does not establish that record's ownership or lifetime.

The initial cached pointer is loaded once for the null guard and first token
call. Every later actual call reloads its own cached receiver. No current
plane+9D4 lookup, command-class latch, target name, or return-to-base census
replaces these retained identities.

`009B3560` returns 2 for a null initial cache or a null **first** +174h result.
Otherwise it reloads +404h for a second +174h call. A second result unequal
to interned `land` `00E08FA0`, including a newly null token, returns 0.
Only when that token is `land` and the current task+424h is nonnull does it
reload +404h, call +178h with the local descriptor output, and pass the
**returned mutable descriptor identity** to `00521EA0`. The resolver result
is compared with a fresh task+428h read **after** that call. Equality returns
1, otherwise 0. It does not recheck +424h after descriptor/resolver calls or
clear any task fields.

`009B34D0` also preserves the two token probes and a fresh approach+0Ch
receiver for descriptor copying. A resolved target mismatch, missing cache,
missing command, or wrong token reaches the common clears at
`009B3543/46/49`: approach+2Ch, +30h, +34h become null in that order.
For a matching target, it captures the then-current block+2Ch, checks it and
block+7Ch owner for null, then tests the actual owner's byte+5Dh against zero.
If those gates pass, the captured block is ECX for `006C4790`, with a **fresh**
approach+0Ch squadron argument. True AL returns without source clear writes.
Any matching-target admission failure first writes null at `009B3540` and
then executes the common +2Ch write at `009B3543`, followed by +30h/+34h.

Pointer-field references are volatile to preserve the observed fresh loads
and separate clear publications. The compiled source retains two +2Ch
writes on that failure arm. This is not a fault/concurrency guarantee or a
claim that volatile supplies ownership, atomicity, or native exception state.
It does not impose a new retirement/drain schedule. Invalidation remains a
three-field clear, with no immediate dequeue, task deletion, hook, or drain.

## Descriptor and required service contracts

`SceneCommandTarget` is asserted to be 18h bytes on Win32, with kind/flag at
0/1, ID at 2, object pointer at 4, three floats at 8/C/10, and trailing float
at 14h. `007EEDC0` executes the actual `0071EB60` acquisition against the
retained squadron's +348h controller, then copies WORD0, WORD2, DWORD4,
followed by four separate pointer FLD/FSTP pairs in offset order. Before the
last FSTP the native moves the destination pointer into EAX. The source
returns that output pointer and preserves the copy kernel's operation order;
it does not use struct assignment or a raw 24-byte copy.

The required descriptor provider must return the live actual mode-1 +58h,
mode-2 +18Ch, or actual lazily initialized empty singleton selected by
`0071EB60`. Its initialization effects and native controller lifetime remain
external. Existing `GameCommandsHost::active_command_descriptor_0071eb60`
substitutes an empty mode-2 descriptor, and its queue conversion omits real
override storage; those projections are not complete providers here.
The repeated token observations reuse `command_queue_current_command` only
against the required actual head/override/mode state, as already verified for
`0071BE40`; they never compare a command-class enum with an interned pointer.

The required mutable resolver must execute complete `00521EA0`. Kind 0
returns null without changing the object field. Other kinds use a nonnull
cached object+4 directly, or resolve the uint16 ID through actual globals
`00F89A10/00F89A0C/00F89A54/00F89A60/00F89AA8`, select the 10h-stride table
entry's +0Ch object, and **write it back into this returned descriptor+4**.
A null table result is also written and returned. A by-value descriptor,
one-based host index, or detached target result would lose this side effect.
The existing pure handle-slot helper is not a complete table/cache resolver.

Block-owner and owner-byte providers must faithfully read block+7Ch and
owner+5Dh, without invented callback effects or a single-player false default.
`006C4790` must implement its actual service: null squadron is refused, and
the block's list at +B4h is walked for matching stored squadron identities.
A match returns false; reaching its end returns true. A presumed empty list
is not supplied. Corrupt iterator checks/private library fault behavior stay
outside the admitted ordinary-return domain.

All pointer mappings, fields, receiver storage, descriptor storage, actual
handle tables, and block/owner/list objects must be valid when each operation
reaches them. Later token/descriptor receivers have no native null guards.
The final admission service explicitly admits a null squadron argument and
must refuse it. Actual service calls can change retained slots between calls;
source fixture replacements demonstrate observation order, not native
reentrancy or observer validity. Mapping reuse, structural reentry, concurrent
mutation, exceptions/faults, and unproved death-time lifetimes are excluded.

The fixed x87 copy admits masked exceptions, no pending unmasked exception,
and an available x87 stack slot. Each load/store pair balances the stack and
keeps ambient precision/rounding/status behavior, including NaN quieting and
denormal effects. It changes no FP policy. Native ABI/profile dispatch,
instruction/data address registers, partial-fault effects, and the source
temporary's initialization are not made native-identical by this normal
source facade. Source temporary defaults are overwritten by all 24 copied
bytes and are not a descriptor-provider default.

## Focused validation and remaining binding

MSVC 19.51.36244.0 x86 compiled the actual changed TU with
`/EHsc /std:c++17 /MD /O2 /DNDEBUG /Iinclude`. The existing ignored hook probe
was extended, linked with the changed object and main
`build/win32/Release/bsp_core.lib` using
`/MANIFEST:EMBED /INCREMENTAL:NO /OPT:REF`, and passed. Existing retained-hook
and cruise checks remain present.

One focused validity scenario changes source receivers A->B->C->D across the
provider observations. It checks null-cache/first-null-token result 2 versus
second-null-token result 0; a real fixture mode-2 descriptor; resolver mutation
of its copied local descriptor; a changed target slot read after resolution;
admitted field preservation and subsequent list-driven refusal/clears.
The copy quiets source sNaN `7F801234` to `7FC01234`, retains negative-zero,
denormal and qNaN output bits, and leaves the source descriptor unchanged.
The fixture saves/restores its FP environment and checks x87 control/TOP,
invalid/denormal status and unchanged MXCSR. Result:

```text
PASS retained hook/cruise; validity fresh probes/result2vs0; mutable resolver; ordered x87 descriptor copy; admission clears
```

Object disassembly proves the WORD/WORD/DWORD/four-pair copy kernel and two
separate +2Ch stores at source-function offsets DD/E6 before the remaining
clears. The report's four direct native call rows pass; six native virtual
calls are explicitly excluded from direct-call verification. JSON and
whitespace checks pass. These are source fixture/object/native-call receipts,
not actual controller/table/list execution, cached lifetime proof, native ABI,
or original-game gameplay validation. The primary owns full main build and
integration. Task construction/profile/arena, owner scheduler binding,
deferred hook/BE routing, and observer/death contexts remain separate and
unbound; existing Boolean host flows are unchanged.

Primary review/integration: 3f07b3b71709ebe6afcda21be1ece878da5a871a; MSVC Win32 Release and all three existing CTests passed. Actual retained hook/cruise TU PASS; fresh command probes, result2 versus0, mutable resolver/fresh target, ordered x87 24B descriptor copy, sequential admission clears. Executable SHA256 086aa79f815f76f700754e3f6cac0f98506332a1879367e49fb6670bcad7fa23. Native ABI and game validation remain unclaimed. Build receipt: J:\PROG\battlestations-pacific-decompile\local\cc11_strings_validity_integrated_build.log.
