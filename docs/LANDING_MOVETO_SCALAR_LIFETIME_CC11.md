# MoveTo root scalar and embedded adjustor, cc11

Two complete ordinary bodies now have conditional SOURCE interfaces connected
to genuine MoveTo construction, observer registration, cleanup and current CRT
release. Both return the SAME whole-root identity. The embedded callback entry
is an adjusting thunk, not a standalone callback destructor.

| Entry | Exclusive end | Bytes / instructions | Native contract |
|---|---|---:|---|
| `009C2B70` | `009C2B78` | 8 / 2 | ECX actual callback18; SUB ECX,18h; tail JMP root scalar; inherited stacked flags/RET4 |
| `009C3D10` | `009C3D2E` | 30 / 11 | ECX MoveTo ROOT; stacked DWORD flags; EAX captured ROOT; RET4 |

The complete 38 bytes match disk, live Ghidra and fixture literals. Adjustor
SHA256 is `c1b4a36c1867e4fa81138d76ef8a293e2a2851c4d890140cc26217f7c1796ec4`;
root scalar SHA256 is `d587dac263917d288c20204466ffd2b440fcb8d1426960e8137146ae4c9db2b1`.
Neither interval includes neighboring padding. Existing native
`CG_scalar_deleting_dtor_009c3d10` name/tag/library role is preserved.

## Complete root and callback contracts

Root `009C3D10` saves the original ECX identity in ESI, calls complete
`007B65E0` at `009C3D13`, THEN tests low-byte flags bit0 at `009C3D18`. When
set, `009C3D20` calls genuine CRT free with the captured ROOT; `009C3D25` restores
the stack. EAX is explicitly set to original ESI before RET4, including after
free. The old decompiler's extraout-EAX interpretation was not used.

`D20AEC` slot0 is that root scalar. `D20AD4` slot0 is the actual eight-byte
adjustor: `83 E9 18 E9 98 11 00 00`. It subtracts 18h from actual callback
ECX and tail-jumps at `009C2B73` to the SAME root scalar. Consequently flags1
frees ROOT and returns ROOT, never the incoming callback address. A standalone
callback object, whether 24 bytes or another size, does not satisfy this contract.

SOURCE borrows the established >=3Ch constructor view and its SAME root,
callback18 and vectorC aliases. The adjustor validates incoming callback and
root addresses only, then calls the complete root scalar. It reads no represented
profile/field, creates no translated cache or owner, and extends no lifetime.
Invalid aliases raise a SOURCE admission error; the native thunk has no guard.
The root captures identity before pure cleanup-view composition and genuine
cleanup, then tests the release flag after the helper, optionally frees ROOT,
and returns the captured identity without reading freed storage.

Complete existing `007B65E0` performs genuine `00695870` callback18 cleanup
before complete shared `007B45F0`. It retains actual provider-final fields:
callback `CE3CD4`, root `D056D0`, unchanged FIRST2C/owner4/payload/padding.
Freed-array metadata can remain dangling and is never dereferenced afterward.
The optionalfree is separate from both member operations. No embedded member
scalarflags1 or direct callback18 free is introduced.

## Native recovery and admission

Primary defined/saved/exported `009C2B70..009C2B78` and repaired the root's
three-byte free continuation `009C3D25..009C3D28` with supported locked tools.
`reports/moveto_scalar_boundaries_cc11.json` records that work; snapshot64718 and
both live flows now have zero CALL gaps. Workers performed only verified
read-only Ghidra queries against C:/Users/sqz269/bsp.gpr and
/battlestationspacific.exe. No class/profile mutation was made by this packet.

Actual root backing is >=3Ch, as established by complete constructor
`009C2AC0` and the native composite's approach+CC to next-state+108 interval.
The callback is an interior root+18 alias with an actual observer prefix; the
adjustor is not evidence of a separate allocation. flags0 permits borrowed
whole-root storage. flags1 requires a separately allocated complete actual CRT
MoveTo ROOT, NEVER task+4C4/approach+CC, callback18, a vector element, or any
interior arena/component allocation.

Stable live coherent actual storage, registrations/FIRST endpoints, actual
published `NativeObserverLifetime` manager/lock/dispatch context, successful
current same-CRT operations and established native vector ranges/profile domain
are mandatory. There is no default observer world, allocator, arena or profile.
Structural reentry/concurrency, invalid fields, allocation failure, faults,
private EH, original class ABI and game lifetimes are excluded. Raw
`D20AEC/D20AD4` remain UNCALLABLE SOURCE image identities; closing these direct
scalar interfaces does not create the other class methods or an executable table.
Whole lower/composite/approach/task construction and death lifetime stay unbound.

## Focused proof and reproducibility

One ignored connected fixture constructs actual 3Ch field-shaped roots through
the existing genuine constructor and registration, with real canonical CF7E64
edges, pending-slot suppression and a prepublished OS recursive section.
Source/full-original rootflags0 clean up borrowed roots. Source/full-original
callbackflags1 clean up and release separately allocated complete CRT roots.
The original eight-byte adjustor JMP targets the full copied 30-byte scalar.
Only that JMP operand and the two natural CALL operands are relocated, to
complete SOURCE cleanup/current-CRT boundaries. The adjustorflags0 route is not
separately exercised; both root release branches are exercised through the
stated direct/adjusted paths. No constructor sweep or new tracked test is added.

The executable's own CRTfree import is traced only while these bodies execute;
the captured genuine operation is always forwarded and the import restored.
Observation rejects callback18 free, verifies provider-final fields BEFORE
ROOTfree, then compares only numeric returned ROOT identity and independent
live endpoint/lock/dispatch storage. No receiver/view or freed array is read
after free. These are SOURCE observations, not historical CRT/loader/native
profile or runtime reentry proof. Manager lookup is unexercised in the owned
prepublished-lock domain. The copied owner token is no approach/arena claim.

Six fresh strict MSVC Win32 TUs compiled with `/EHsc /std:c++17 /MD /O2 /Gy
/DNDEBUG /W4 /WX`: scalar, constructor, observer_edges, observer_lifetime,
shared-state and fixture. `/MANIFEST:EMBED` produced an asInvoker/uiAccess=false
probe. Three current99bef9d8d libraries were uniquely frozen with
source-before/copy/source-after equality; all 30 source/header/library hashes
remained unchanged after the run. The probe passed **68 checks**. COFF confirms
root capture, complete cleanup, low-byte test, optional ROOTfree and captured
return; the compiler inlines that complete sequence in the valid adjustor path.
New C++ interfaces/guards/encodings are explicitly not original binary ABI.

`reports/landing_moveto_scalar_lifetime_cc11.json` contains native call receipts,
commands, hashes and limits. Ignored artifact paths share
`local/cc11_land_moveto_scalar_lifetime_`; the manifest is
`local/cc11_land_moveto_scalar_lifetime_manifest.json` and executable ends
`probe.exe`. Root owns CMake registration, independent probe/full main build
and integration. This packet changes exactly four owned files.

## Primary integration

Main `b02e3154636139d964b1e32f0f91b710dd71985e` passed the full Win32 build and all three existing CTests. Root independently compiled six actual translation units against 27 pinned Source/header/fixture inputs, 21 compiler includes and three current libraries, then reproduced 68 checks. All 38 original bytes match disk/live/fixture literals. The original callback adjustor reaches the copied original whole-root scalar; both scalar release branches run through different entry routes. Two direct calls and one tail jump passed static verification. Full fresh COFF review confirms cleanup before the low-byte flag test, root identity return, and the complete scalar body inlined into the valid adjustor path. Root fields are observed before free; after free only numeric identity and independent live owners are used. The existing CG scalar name and tag stay preserved; ABI and game qualification remain. The PE32 asInvoker manifest was verified. No tracked tests were added.
