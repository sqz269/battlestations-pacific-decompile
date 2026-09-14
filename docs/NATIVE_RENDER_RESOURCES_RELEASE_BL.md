# Native render-resource release closure

Owned analysis addresses: 00B14F60, 00B151C0, 00B0F6E0, 00B52270.
This packet adds evidence and proposed work packets only: no native body,
C++ interface, build, fixture or runtime credit.

BK commit `2eea9361f82614818242f31f518b3d24ca4285a9` constructs the actual
6ACh service. Its cleanup closure requires six currently unreconstructed
bodies. Four were the assigned candidates; the other two are the CCh child
destructor and deleting wrapper reached through its actual D62074 profile.

| Entry | Actual inclusive body | Bytes | Proposed descriptive name | Evidence coverage |
| --- | --- | --- | --- | --- |
| B14F60 | B14F60..B15085 | 294 | BSP_RenderResources_Destroy | Full body; one three-byte listing gap recovered from original bytes |
| B151C0 | B151C0..B151DD | 30 | retain existing scalar deleting destructor role | Full body; one three-byte listing gap |
| B0F6E0 | B0F6E0..B0FBF8 | 1305 | BSP_RenderResources_ReleaseTransientMembers | Full 561-instruction body |
| B52270 | B52270..B522FE | 143 | BSP_RenderServiceTextures_ReleaseAuxiliaryReferences | Full 64-instruction body; auxiliary meanings remain provisional |
| B52400 | B52400..B52543 | 324 | BSP_RenderServiceTextures_Destroy | Full original body; Ghidra currently ends at B524CB |
| B52840 | B52840..B5285D | 30 | retain existing scalar deleting destructor role | Full body; one three-byte listing gap |

The live BSP CLI verified `bsp`, `/battlestationspacific.exe`, base00400000,
the configured `C:/Users/sqz269/bsp.gpr` project and port8089. The report pins
the original PE, all six spans, BK producers and existing source providers.
No Ghidra flags, names, bodies, comments, prototypes or saves were changed.

## Original entry and release contracts

B14F60/B0F6E0/B52270/B52400 take the actual receiver in ECX, have no stacked
arguments and return with plain RET; no semantic EAX result is established.
B151C0/B52840 take ECX plus one DWORD flags, destroy first, free the captured
allocation only if flags bit0, return that captured address in EAX and RET4.
An escaping destructor exception prevents free. All preserve nonvolatile
registers; neither deleting wrapper establishes another FH3 frame.

B0F6E0 first captures receiver+34 into ECX and unconditionally calls B52270.
There is no null check. B52270 operates on the actual CCh child, not on service
offsets. It visits child+30,+68,+88,+B0 in that order. B52550 writes all four
to zero on construction; a concrete nonzero producer/profile contract has
not been established here.

B0F6E0 then visits 42 service fields, 41 distinct offsets, in this order:

```text
050 1D0 044 048 04C 02C 650 658 65C 664 018 020 024 028
038 03C 040 1CC 1D4 1C8 1D4 070 074 080 01C 060 064 06C
068 654 078 07C 030 660 010 66C 670 674 678 054 05C 058
```

Finally B0FBEF clears byte+1C4. This helper has no FH3 frame and no own
cleanup after a child failure. A returning callback may change later fields;
the second +1D4 visit follows +1C8 and must not be eliminated. The public
thunk B0FC00 tails into this helper. Settings_ApplyAll calls that thunk at
008D606C with current F8D39C, so its use is broader than final destruction.

B14F60 writes service profile D5E480, arms state3, calls B0F6E0, then visits
+34, +668 and +0C. Each visit captures the pointer, skips it if null,
decrements actual captured+4, dispatches the captured object's CURRENT slot0
only on zero, and clears the service field only after that call returns.
Clearing after a callback deliberately overwrites callback writes to the same
field. No second decrement belongs in the terminal dispatch.

Each routine captures CE2220 once into a nonvolatile register, not once per
field: B52270 at B52271 into EBX, B0F6E0 at B0F6F1 into EBP after B52270,
B14F60 at B14F96 into EBP after B0F6E0, and B52400 at B52436 into EBP after
B52270. The captured stdcall target receives captured-owner+4 and consumes
that one stack word. Current profile/slot0 loads occur after the zero result.
BD30E0 then reloads current profile/slot4, calls it with flags1 and performs
no decrement. A future context must preserve these capture epochs.

## Concrete current profiles and missing children

| Constructed field/domain | Producer | Current terminal path | Source status |
| --- | --- | --- | --- |
| service+1D4 actual40h frame target | B14D89/B14D92, B1FBB0 | D5E600: BD30E0 -> B1FCF0 | Existing native_frame_target_owner.cpp, actual surface context and shared CRT free |
| service+34 actualCCh child | B14EDD/B14EF0, B52550 | D62074: BD30E0 -> B52840 -> B52400 | Both terminal bodies and B52270 are missing |
| service+668 first default texture | B14E20/B14E22, B319B0 | In admitted 2D domain D61948: BD30E0 -> B3F590 | Existing native_texture_2d_owner.cpp; requires canonical pool, renderer, source and surface contexts |
| service+0C actual24h helper | B14F29/B14F2E, B3C800 | D61854: BD30E0 -> B3C6C0 | Existing native_cockpit_helper_lifetime.cpp plus BK's authoritative companion |
| CCh child+18,+3C,+70,+98 | Four B52550 cache loads | Admitted 2D profile as above | Existing texture terminal, but parent B52400 missing |
| CCh vectors+A4,+8C,+7C,+24 | B52550 initializes raw headers | B52170/B521E0 resize0 then BF6989; EH B523C0/B523E0 | Existing complete texture-vector providers |
| service+69C,+68C,+684 and base | BK initialized members/base | 432050/B14500/419CC0+BD1510/B0F0C0 | Existing raw string/vector/base providers |

B52400 writes D62074, arms state4, calls B52270, and releases the four loaded
textures in order +18,+3C,+70,+98. It destroys vectors in reverse member
order +A4,+8C,+7C,+24, then BD30F0 restores CEB130. Native vector cleanup
shrinks count to zero and frees the current pointer without inventing pointer
or capacity clears. BF6989 is a returning jump through BF65AC to the shared
CRT free domain. B52840 frees the actual CCh allocation only after success.

Only the observed current profile domains above have concrete providers. The
remaining service members and nonzero child auxiliary references require
producer/profile evidence before unrestricted cleanup can be implemented.
A generic abstract release callback or a hardcoded zero assumption would not
close this gap. B0FD70's existing analysis identifies service+66C as a B319B0
remap-texture load; that producer is still absent from reconstructed source.

## Native unwind maps and listing defects

B14F60's handler CBC432 loads FuncInfo DF4640, whose map is DF4620:

| State | Next | Funclet | Action |
| --- | --- | --- | --- |
| 3 | 2 | CBC424 | 4324A0 service+69C |
| 2 | 1 | CBC416 | B14590 service+68C |
| 1 | 0 | CBC408 | 41DD20 service+684 |
| 0 | -1 | CBC400 | B0F0C0 service base |

Normal code disarms each member before its explicit destruction: state2
before +69C, state1 before +68C, state0 before +684, then -1 before the base.
It does not release later raw resource members after an earlier terminal
throws. Native B151C0 does not free after a throwing destructor.

B52400's raw handler CBFF6A loads DF8A94, mapDF8AB8. States4,3,2,1,0 map
through CBFF5C, CBFF4E, CBFF43, CBFF38, CBFF30 to child+A4,+8C,+7C,+24 and
BD30F0 respectively, each transitioning to the preceding state. Normal code
disarms each vector before its explicit resize/free. Earlier texture failures
leave later texture references untouched while this vector/base cleanup runs.

Three listing holes are exactly `ADD ESP,4`: B15021..B15023,
B151D5..B151D7, B52855..B52857. More seriously, B52400 ends at its first
returning BF6989 call, and `ghidra flow` incorrectly reports zero gaps because
the missing fallthrough lies beyond the stored function body. Original bytes
prove B524CC..B52543 is the continuous 118-byte tail, ending RET before INT3.
CBC432..CBC43B and CBFF6A..CBFF73 are raw handlers with no Ghidra function.
All raw ranges use inclusive bounds in the report. No repair was performed.

## Ownership limits and next packets

The native constructor leaves service+70 as a preimage, but B0F6E0 releases
it. Destruction immediately after BK therefore requires a separately valid
+70 value/domain; adding a constructor zero or skipping release would change
native behavior. Conversely, neither B14F60 nor B0F6E0 releases service+67C,
the second default texture loaded by BK. This closure proves its reference
has no explicit matching release here and its field remains untouched; it does
not establish whether another path retires it or whether the game leaks it.

BK binds exactly one helper reference over the actual +4 counter. Final zero
must use that companion's existing terminal path so retirement is recorded;
direct owner deletion while the companion is bound violates its contract.
After native retirement, helper companions and the camera/viewport block
remain alive until their independent host-quiescence checks can be satisfied.
If other references keep the helper alive, service destruction must retain
those host companions. A helper_binding_failed BK attempt was never published
at service+0C, so ordinary B14F60 cleanup cannot discover or retire its saved
completed helper. That explicit external obligation must remain visible.

Suggested bounded followups, each with disjoint source ownership:

1. **CCh child lifetime:** B52270, B52400..B52543, B52840. Own new
   `native_render_service_texture_lifetime.hpp/.cpp` and evidence. Compose
   existing vectors, 2D texture terminal and CRT; first establish nonzero
   auxiliary-reference producers/profiles or state a narrower admitted domain.
2. **Service member release:** B0F6E0 (optionally B0FC00 thunk). Own new
   `native_render_resources_member_release.hpp/.cpp` and evidence. Preserve
   all 42 visits and current captured-IAT semantics. Depends on packet1 and
   concrete current-profile providers for every admitted nonnull field;
   +70 is an explicit admission/producer blocker.
3. **Derived lifetime composition:** B14F60 and B151C0. Own new
   `native_render_resources_lifetime.hpp/.cpp` and evidence. Depends on1/2,
   canonical BK helper identity and separate persistent retirement/quiescence.
4. **Saved-analysis repair:** primary-only mutation packet for the three
   holes, full B52400 body and two raw handlers; preserve old annotations,
   refresh exports and then repeat the mechanical call audit. No native body
   reconstruction credit belongs to this packet.

The report retains every numeric call site, including raw-tail calls, rather
than mislabelling them indirect to satisfy an older verifier. The current
59e84616 verifier lacks `no_ghidra_function` support; its expected raw-boundary
failures are reported explicitly: 141 total rows, 35 numeric rows checked,
26 passed and nine failed at the seven raw-tail calls and two raw-handler
jumps. The remaining 106 indirect rows are skipped by this verifier and rely
on the complete listing/provider evidence. This design packet makes no build, runtime,
native ABI or whole-game cleanup-completion claim.
