# Raw particle parameter properties and bounds

Addresses: 00B00980, 00B001A0, 00AFFE90, 00AFFE20, 00B08870.

This packet adds actual-domain overloads to `native_particle_type_loading.hpp`.
The existing legacy parser and helper signatures remain available. The common
property dispatcher and three numeric kernels are shared implementations.

## Context and ownership

`NativeParticleTypeParameterRawContext` borrows the same
`NativeParticleParameterRuntimeRawContext` used by AFBF60 conversion and the
actual CRT square-root access. Its five numeric pointer members are D7A358
percentage scale, D7A220 end time, D7A210 bound base, D7A2B0 derivative scale,
and D7A328 discriminant scale. All are double inputs. The context owns nothing
and introduces no allocation callback or legacy owner adapter.

The property is an actual four-byte pooled-text header. B00980 only reads its
string, so it needs no string-pool context. A raw parser passes its existing
header from `NativeStringRawPoolContext`; the caller retains and destroys it.
Runtime properties allocate through the same actual F8D344 parameter pool,
with the fixed source CRT segment-storage provider documented in
`NATIVE_PARTICLE_PARAMETER_RUNTIME_RAW_ORCH4.md`.

| Original body | Bytes | Original ABI | Coverage |
|---|---:|---|---|
| B00980–B00C04 | 645 | ECX definition; stack property, builder, percentage; RET0C/AL | complete |
| B001A0–B002BC | 285 | ECX parameter; RET/ST0 | complete |
| AFFE90–B00036 | 423 | ECX 1Ch segment; RET/ST0 | complete |
| AFFE20–AFFE89 | 106 | ECX/EDX roots; stack a,b,c; RET0C/ST0 | complete |
| B08870–B088AA | 59 | ECX Sprite; stack parameter; RET4 | complete |

B00980 publishes BornRatio+24 and TerminateAfter+20 through one FLD32/FSTP32
pair; animation frame limits +50/+54 retain CVTTSS2SI. Its ten runtime
properties use conversion before loading the current percentage scale, storing
the multiplier, and publishing the pointer. Repeated properties do not release
the overwritten parameter. The first five comparisons reload the actual header
and use source CRT `_stricmp`; the remaining comparisons use AEDF80. Unknown
names return false. No null-input policy is added.

The original property call sites AF8F3A, B068BA, B0803A, B08D9B and B0B163
all pass the same three stack words after a percentage FLD32/FSTP32. Their
containing Object, Axial, Floating, Sprite and Tracer bodies and the RET0C
cleanup were checked. B08F47 passes the converted parameter to B08870.

## Numeric binding and publication order

The bound view holds addresses of the original context's numeric pointer
members. Each native constant load reloads that pointer cell. In particular,
a CRT precision callback can rebind end time/base and the next segment's
derivative/discriminant inputs. The same correction also applies to legacy
bindings. All 269 original-address-tagged kernel instructions are unchanged;
the added binding instructions only manipulate integer registers and stack.

The raw unary and quadratic entries preserve the lower x87 stack and add no
floating-point return spill. The original discriminant stores, root order,
aliased-root behavior, polynomial evaluation, comparisons and NaN behavior
remain in the shared kernels. Negative/unordered discriminants leave both
root outputs untouched. Linear/cubic scans retain their unchecked stop rules;
the count byte is not used as a safety limit.

B08870 required an overload because its previous entry required the legacy
loading domain. Its raw entry writes +88 before calling B001A0, retains the
original result FSTP32 followed by MOVSS publication to +8C, and writes positive
zero for null. The domain-free AFC1C0 integer helper gets no extra wrapper.

## Validation and limits

`reports/native_particle_parameter_properties_raw_orch4.json` contains all five
complete live/installed spans (1,518 bytes), exact direct calls, original ABIs,
prior full annotations and ledger records, and artifact hashes. None of these
five bodies establishes an EH frame. Conversion's existing constructor-only
FH3 state remains in its unchanged provider; the probe also archives its
current handler/map bytes. Ghidra was read only; no flow or body repair was
needed.

The strict MSVC Win32 Release build and all three CTests passed. Ignored probes
linked to that final library passed:

- 571 raw bound/Size comparisons, including three x87 precision settings,
  current constants, aliased roots, SNaN/QNaN, full output/status/control/lower
  sentinel state, and a genuine CRT precision callback that rebinds numeric
  pointer members before later native loads.
- 570 existing legacy bound/Size comparisons.
- 480 raw property observations: all 14 recognized names and one unknown,
  Const/Linear/Hermite/SNaN Const builders, four x87 rounding modes and two
  percentage scales. Both sides used the same actual parameter slot and raw
  string pool. Comparisons cover the entire 98h definition, defined key data,
  segment bytes, 904h slab/freelist, 9,098,372-byte string arena/ring image, and
  x87 exception flags/TOP/lower sentinel. Segment addresses alone are normalized;
  unproduced builder coefficient scratch is excluded.

These are reconstructed, build-tested and bounded native-fixture-tested C++
interfaces. Source CRT locale/error behavior is a provider boundary. Original
FH3/SEH transport, malformed storage, unmasked faults, binary replacement and
gameplay remain unvalidated. No permanent tests were added.
