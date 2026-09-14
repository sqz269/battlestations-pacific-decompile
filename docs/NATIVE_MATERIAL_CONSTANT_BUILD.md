# Actual material constant build

`build_native_material_constants_00b42350` reconstructs the complete normal
schedule at **B42350–B4340F (4,288 bytes)** over the actual pass and entry.
It borrows the same VS bank `0108EBF4` and PS bank `0108DBEC` used by pass
execution; each native header is four bytes before its bank. The original
ABI is ECX=pass, stack=entry/unused override, RET8. The new C++ interface adds
`NativeMaterialConstantBuildContext` and a persistent `NativeMaterialConstantBuildFrame`.
It is not a binary replacement. The earlier typed implementation remains a
separate projection; this upgrade claims no additional B42350 native body.

The source-only commit is `6336f48698718c2fa5164b9ffb2fa48e0f0ae322`, based on
`812b6c0e3be89fed52316ac0076721c69ed66b34`. Its exact dependency pairs are in
`reports/native_material_constant_build.json`. The public interface is independent
of the pass execution header and uses its agreed function signature.

## Native schedule and storage

1. Iterate pass binding pointers from `+5C` with the current unsigned `+6C`
   count. Capture the source owner's current virtual `+2C`, then the current
   renderer publication, obtain the texture, and call that captured renderer's
   current `+130`. Reload the binding/owner and pass PS/VS descriptors before
   source virtual `+30(entry, VS, PS, VSheader, PSheader)`.
2. Obtain the actual material table at `entry+04 -> section+20 -> material+80`.
   Capture `entry+10 -> state+198` as the register-array type. Parameters are
   actual 84h records: source `+08`, word count `+0C`, matrix byte `+10`, signed
   VS registers `+14`, signed PS registers `+4C`. Table iteration uses pointer
   inequality against current count at table `+80`. VS matrix width uses the
   last matching register in the current shader's 20h constant rows (`+78/+7C`),
   reloading the shader/data/count during that scan. Matrix source is reloaded
   afterward. PS matrices always write four transposed rows.
3. VS byte `+30` emits object bone transforms through actual `B8FF00/B90620`.
   VS byte `+32` uses captured object animator `+130` and current virtual `+0C`
   with the actual `010900FC` type. A successful optimized predicate uses
   object palette `+190` and captured animator `+34 -> +08` count; null palette
   emits nothing. The fallback uses current `object+184` 60h records, each
   node world matrix and inverse bind at record `+20`, and actual inverse world
   `B6E0D0`; the two `413920` multiplications retain their order.
4. VS bytes `+20/+21` select decode scale/offset banks. The budget comes from
   the effect descriptor's `+20` or `+24`, selected by instance type 2. Current
   section streams/count are `+3C/+4C`. Capture each stream's `+50` decode
   presence, call current virtual `+24`, then `B47900`, and use the signed
   minimum of declaration count and register gap. The two register cursors
   advance by that whole minimum before emitting rows; budget/remaining are
   separate unsigned counters and only zero-tested at the next stream.
   **B42A4F branches to B429C5, bypassing the one-time LEA at B429C2. ECX remains
   1 in the absent-decode loop; the decoded route also restores 1.** Each output
   row therefore decrements budget/remaining by one. Current `B61E10` returns
   `stream+50 + index*32`. The second stream may consume more than the remaining
   counter; native wrap is preserved.
5. Emit VS and PS world matrices, scalars, threshold fade, VS inverse world,
   then PS/VS diffuse rows. World widths are captured before a reached refresh,
   while their register bytes are reloaded after it; the PS world path uses
   **VS byte +3E**. Inverse width/register are both captured before the getter.
   Fade uses actual entry `+00`, entry owner `+08`, actual fraction `D7A238`,
   and the substantive `B73770` provider with its current threshold constants.
6. Bind pass `+7C` service shadow texture, then pass `+78` first-light shadow or
   pass `+84` fallback, preserving captured renderer/profile versus later slot
   reads. The first-list sentinel diagnostic calls the existing host CRT
   invalid-parameter boundary and continues if it returns. `B7AAB0` reads light
   `+174`; the observed shadow profile `D5B5D8/+08` dispatches to raw `A8FCF0`.
7. Always obtain object point-light array `+164`, then gate on current VS byte
   `+35`. Clamp signed count to at most 4, convert its unsigned bits using the
   native x87 bias path, and emit two float4 rows per light from `+1EC/+184`
   when current byte `+36` permits. Negative counts retain the native unsigned
   tail behavior; no bounds repair is introduced.

Width-2 matrix packing preserves MOVSS bits through an eight-word scratch copy.
Widths 3/4 preserve individual FLD/FSTP crossings and their ordered writes.
Raw parameter/palette copies use overlap-safe `memmove`: the captured full
869-byte `_memcpy` body at BF7680 also implements backward overlap. Native
private stack aliases and incidental registers are not represented by the new
source ABI. Valid raw extents, original object identities, clear DF, and the
ambient native floating-point environment remain caller requirements.

## Concrete providers and failure state

The new raw leaves are B17390[7], B5B880[4], B8FF00[7], B90620[16], B75E50[6],
B47900[4], B61E10[13], and B782D0[40]. B782D0 preserves the AL-only result and
RET4 while scanning the same three current type-ID cells. The original optimized
animator constructor B79BC0 writes D62EB0 at B79D9E; its `+0C` slot is B782D0.
The full constructor body and focused profile-writing context are archived,
without claiming reconstruction of that constructor.

Existing actual factories BBC6F0/BBC810 construct raw 34h texture-source owners
and publish D64478/D644B4. Both profiles' `+2C` is the new C302F0[19] getter:
test current count `+14`, then return `array+10[index+08]`, or null when empty.
D64478 `+30` is BBCC40[103], whose actual VS `+1E/+1F` writes target the passed
VS header `+04` bank. The rate row stores Y before X, followed by zero Z/W;
the color row reads each current source word in order. D644B4 `+30` is literally
BBCBD0[3], RET14. Its empty body is established by bytes and profile provenance.

Current renderer D5F0A8/+130 uses the existing real B24710 texture-binding
context and owner domains. Logical vertex D61D6C/+24 uses actual B48CE0.
Unknown source, renderer, animator, logical-vertex or shadow profiles fail at
the reached source dispatch. No numeric game token is called as a host pointer,
and no callback supplies a substitute texture, animator result or successful
write. The source frame records fresh/running/completed/failed plus the last
original callsite. A failed frame cannot be replayed. It does not acknowledge,
disarm or discard underlying persistent child owner failures; earlier native
effects remain visible. Native B42350 has no FH3 frame. Source exceptions are
not claimed to reproduce original hardware-fault or native exception ABI.

## Verification and limits

- `python tools/ghidra_export.py verify-seeds`: eight current native seeds match.
- `./scripts/build.ps1`: MSVC Win32 Release and both CTests pass.
- COFF inspection captures all twelve compiled entry bodies and relocations;
  nine raw leaf/provider bodies match their complete original bytes exactly.
  B42350, B75E50 and B782D0 have the documented new source interfaces.
- `local/constant-build-validation/run_probe.ps1`: `/MD`, `/fp:strict`,
  `/MANIFEST:EMBED`; two variants execute the relocated full 4,288-byte B42350
  body and its native arithmetic/leaf closure, then compare both 4,096-byte
  banks byte-for-byte with source. The variants cover fallback and optimized
  skinning, matrix widths 2/3/4, signaling-NaN MOVSS versus x87 behavior,
  overlapping raw copies, world/inverse refresh, both decode routes and budget
  wrap, fade/diffuse, and four point lights. Native/source source-getter and
  caustics-writer comparisons use owners from the actual existing factories.
  A separate mode within this fixture checks visible earlier writes, retained
  source failure and rejected replay at an unsupported second stream profile.
- 48 full native/context spans totaling 12,176 bytes match the installed PE
  and live Ghidra through verified BSP wrappers. Captures include full owned
  bodies, arithmetic providers, original profiles, the full animator producer,
  factory bodies and their allocation-unwind/FH3 records. Definition events
  for the four previously undefined entries use the official locked tool,
  exact byte ends, save, and refreshed exports; no names/comments were changed.

The fixture's B42350 runs have no pass source bindings and no shadow texture
binds. Those complete source routes are compiled but are not full renderer/COM
runtime proof. The logical declaration getter and CRT memmove/invalid-parameter
boundaries are shared providers in the native harness. The fixture does not
prove unknown derived profiles, animator construction/animation, terminal texture
ownership, native exceptions, parent draw integration, rendered output, or gameplay.
The archive pins gather dependency 80027137 for its raw shadow/light leaves;
the parent's later private captured-profile correction 1bbc1662 is separate
and does not change those reached leaf bodies or this public API.

The immutable archive, its manifest hash, exact build/link/runtime closure and
source hashes are identified by the final handoff, not by a mutable log inside
the captured folder. The report contains the exact numeric native callsite rows.

## Primary descriptor-search capture correction

The original B42421..B42431 reads shader+7C count before its+78 descriptor base and captures that initial end without another base read. B42449..B4245A reloads the current shader, count and base after each match check. The primary source now makes these reads explicit, removing the extra initial base read and dependence on C++ function-argument evaluation order. Frozen worker source6336f486 remains unchanged; combined validation follows the correction.
