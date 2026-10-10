# ExplosionType: primary Source review

Accepted worker f38eed81ed154713b098cd574cf9b3dcf83b3f41 supplies the interior
0087CCE2..0087CD4C fragment, ending before CD4D. It borrows the actual descriptor,
current Lua row/owner, still-live Unique94h and reusable fresh field44h. The
complete 0087CA80 reader remains Source-absent; no whole original function was
added. This is an ordinary C++ interface, not a Native callable entrypoint.

The first protected lookup enters state8, runs the genuine integer-number
predicate, saves the result and lowers to state1 before actual probe destruction.
False returns without changing descriptor+40h. True performs a fresh protected
lookup after destruction, enters state9 and converts that new value without a
second predicate. Raw int32 publication at +40h precedes state1 and final cleanup.
Lua callbacks can change the second value; neither value nor tracked index is
cached. Unique cleanup remains the enclosing owner's responsibility. Earlier
writes survive later cleanup failure; normal cleanup is never retried.

The mandatory live Source bool alias is forwarded to the existing conversion
provider and read only after the second real Lua numeric read and x87 float32
narrowing/reload. It can change during callbacks. No default, temporary, snapshot,
enum mapping or reinterpretation of Native DWORD0109EEA4 as bool was introduced.
The actual DWORD binding/application adapter remains held.

Root replayed all107 Native bytes/26 instructions and all858 parent starts
against the unchanged installed PE, frozen saved listing and worker fresh live
listing. The registered normal fragment object has379 fully decoded code bytes
in five physical code sections; its root is202 bytes/62 instructions. Six REL32
calls resolve four genuine Lua definitions: protected lookup, tracked destructor,
integer predicate and integer accessor. Three further calls resolve the real
51-byte ordered predicate,54-byte live-mode conversion and117-byte naked x87
kernel. All selected bodies, both conversion paths, both special-path x87 stores
and physical Source EH/safe-handler records were reviewed. State8/9 guard cleanup
lowers before destruction; extra ordinary C++ spills/EH remain explicit limits.

Normal MSVC Win32 Release build passed all three existing CTests. Source599
contains599 selected project inputs,73 whole Core objects,three whole App objects
and139 unique positive Core definitions. All70 prior selected Core objects and
allthree App objects are byte-identical to Source595. The new scope includes
existing GUI/render numeric provider objects and their Source; it is not a whole
application, compiler/SDK/preprocessing or environment closure.

All599 frozen Source595 pins and45 worker pin occurrences were verified and
copied before worker reuse. Root appended only this fragment ledger and parent
name evidence. The locked annotation preserved the old name/comments verbatim,
recorded prior values, saved the project and refreshed the parent export. No
prototype/body/flow repair occurred. Target project/program checks come from the
configured existing client; absolute runtime GPR-path identity is not separately
attested by that client.

No new test, fixture or runtime run was added. Source595's earlier production
VFS-phase run failed before device/Present with FMOD61/78/37; it supplies no
ExplosionType execution evidence. Full reader Source, Native register/stack/FH3,
DWORD binding, FP status/traps/fault/NaN identity, SEH/longjmp, double-exception
identity, receiver binding, successful startup and gameplay remain held.
