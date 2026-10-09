# Qualified Source observer member initialization

Candidate for `007F0F80..007F0FDB` (92 bytes, 28 instructions). The complete
saved listing and raw decode have the same 28 starts; the Root Astra gate
records matching original PE/live bytes and a full body read. The worker read
all 28 instructions. The qualified Source entry is
`initialize_native_observer_member_007f0f80`. This packet does not register or
compile it; independent Root build and admission remain required.

## Actual storage and ordered effects

Use the existing 16-byte `NativeObserverOwnerStorage` at the actual member base
M, a separate reference to the actual volatile byte M+10h, and a separate
reference to the actual volatile pointer cell M+14h. The API introduces no
18h member type, backing allocation, default values, endpoint copy or service
cell. These references must identify those parts of the same live member;
the function does not derive offsets or validate the caller's binding.

The incoming argument is a reference to an actual pointer cell. Three separate
volatile DWORD stores clear the actual array data/count/capacity at M+4/8/Ch
before one volatile read captures that cell. A caller-side pointer copy would
erase this ordering and is outside the intended binding. Ordinary C++ lifetime,
type and valid-access requirements apply; this does not promise arbitrary
Native stack or incompatible typed alias behavior.

After capture, a private guard retains the actual owner and lifetime. The
function writes profile DWORD `00CF6494` at M, the captured pointer at M+14h,
then byte 1 at M+10h. It does not write M+11h..13h. A null captured pointer still
gets all six member writes. A nonnull captured pointer is passed directly to
the existing free function
`register_observer_pair_00694a60(*captured, actual_owner, retained_lifetime)`;
M+14h is not reloaded for registration. The same actual endpoint/owner address
may be used if all reached existing provider contracts are satisfied. The
guard is disarmed before returning the actual owner reference. There is no
promise that callback effects leave all initialized fields unchanged.

## Failure policy and retained providers

The public function may throw. Guard construction is nonthrowing and occurs
after the incoming-cell read, before profile/endpoint/flag publication. The
guard destructor explicitly uses `noexcept`; when armed it calls only the
actual `NativeObserverLifetime::destroy_callback_owner_00695870(owner)`.
Successful registration is followed by guard disarm. An exception from
registration instead reaches owner destruction, preserving the prior stores
and any partial registration effects for that provider to observe. There is
no added unregister, whole-member reset, rollback, array repair or retry.
If owner cleanup throws during unwind, current C++ policy terminates; no
later work is guaranteed after termination. Native faults are not thereby
converted into C++ exceptions.

The actual cleanup provider writes base profile `00CE3CD4`, obtains its outer
and nested current section, reads the current owner count, detaches when
nonzero, and frees the late-loaded backing after callbacks/unlock. Its catch
path frees then rethrows. The initializer neither substitutes the normal
member-cleanup wrapper nor calls pair release before this provider. Existing
registration retains its outer lock and nested find/create behavior, including
same-owner and partial-append effects. This Source composition does not repair
those states or establish their Native failure equivalence.

Retain the actual owner/endpoint storage, borrowed sound lifetime access,
observer domain, publication-cell identities, lock/dispatch cells, service
provider and callback targets for all reached normal/failure calls. Existing
`GameObserverRuntime`/singleton retention is available Source infrastructure;
this entry adds no selected production caller, lifetime binding or original
global-address binding. Nested provider lookups remain independent.

## Native evidence and limits

Native entry takes M in ECX and one incoming stack DWORD; it returns M in EAX
and uses `RET 4`, subject to compatible child return and current stack contents.
The body publishes an FS exception frame, saves ESI, zeroes M+4/8/Ch, then
reads the incoming pointer into ECX at `007F0FA8`. The compare at `007F0FAC`
survives the state/profile/endpoint/byte stores to select the branch. The
full DWORD state-zero write at `007F0FAE` precedes profile publication. The
nonnull child call at `007F0FC3` passes captured A in ECX and actual M in EDX
without pushed arguments. Its complete 135-byte Native boundary and actual
Source free-function identity are independently recorded in the registration
ABI readiness report.

Accepted EH metadata selects handler `00C8F398`, descriptor `00DBFDC8`, map
`00DBFDC0` state zero, and action `00C8F390` tailing to `00695870`, with previous
state -1. The action reloads the current frame owner word at EBP-10h. That
metadata/action selection under compatible frame premises is evidence for the
chosen cleanup target; it is not proof that the Original interpreter matches
the C++ guard's captured references, state storage or `noexcept` policy.

The Source interface has five explicit references and a current compiler
exception policy. It does not reproduce Native ECX/EDX/stack cleanup, ESI save
slots, current stack aliases, FS publication/restoration, final flags, CRT
exception types or an Original callable ABI. There is no startup, gameplay,
fixture or binary replacement credit. Profile constants remain opaque values,
not installed C++ virtual tables.

## Prior evidence and this packet's validation

The current accepted shared Source97 receipt is
`reports/cc12_native_squadron_launch_task_cleanup_primary_review.json`, also
referenced by the string-fields cleanup primary report. It records the normal
Win32 build ending 2026-10-09 11:00:05 -07:00, three existing checks, 97 inputs,
23 whole objects and 27 positive public Core definitions. Its predecessor
Source88 is historical. These are existing build facts, not compilation of
this candidate. The registration provider's object/Core membership is separately
recorded in the accepted 135-byte registration audit and lies outside the
Source97 whole-23 review.

The JSON report pins this packet's source/doc, current provider files, selected
accepted reports and Root gate/capture. Worker validation is limited to full
owned Native instruction inspection, selected current Source inspection,
pin replay, JSON parsing and diff/scope checks. No CMake, ledger, Ghidra,
consumer, build, test, probe or fixture execution changes are part of this
four-file candidate. Independent Root must register, compile and inspect the
actual compiled entry/guard and provider reach before Source admission.
