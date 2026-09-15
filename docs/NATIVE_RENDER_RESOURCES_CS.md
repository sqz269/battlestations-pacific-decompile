# Native render-resource construction composition (CS)

CS runs the complete source constructor `00B14A10` with actual D3D9 textures,
frame surfaces and a camera/viewport, then explicitly retires its children.
The tracked Win32 build and both existing CTests pass. This is a bounded source
composition fixture; application startup and the full parent destructor remain
unexecuted.

## Source consolidation and credit

The preceding orch4 CR packet duplicated a constructor already implemented on
orch3's branch. CS replaces the CR interface with the earlier reviewed BK source
from `7142a2f142eaf8991eff9533bcb5b5a7fc26123c`. Its persistent cockpit admission
and companion records now provide the common implementation. CS retains CR's
additional check that the frame-surface and texture contexts share the actual
renderer publication and raw string storage. Header documentation names this
report; the remaining imported constructor code is unchanged.

Texture-helper lifetime and the placement construction of its live atomic count
come from orch3 BL, `9335da3246f979e042a4286c8f4ad42b19ca69b5`. The imported
BL document/report are historical evidence, including their original limitations.
The CS report records the original blob hashes and exact constructor deltas.
These are integrations of prior implementations, with **zero new unique function
or native-byte credit**. CR's existing source/evidence receipts remain immutable;
its earlier exception fixture does not validate this replacement interface.

## Native evidence and contracts

| Body | Bytes | Contract |
| --- | ---: | --- |
| `00B14A10` | 1356 | ECX actual 6ACh receiver, no stack arguments, EAX receiver, RET |
| `00B52550` | 752 | ECX actual CCh receiver, no stack arguments, EAX receiver, RET |
| `00B52270` | 143 | ECX helper, no stack arguments, RET; auxiliary release |
| `00B52400` | 324 | ECX helper, no stack arguments, RET; native member destruction |
| `00B52840` | 30 | ECX allocation, stack flags, EAX captured allocation, RET4 |

The five bodies total 2605 bytes. Sixteen constructor/destructor compiler support
spans add 177 bytes. All 783 decoded instruction owners were checked in the
existing `bsp` project, `/battlestationspacific.exe`, and bytes compared with the
installed executable (SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`).
The audit includes 64 direct transfers and 31 indirect calls.

The fresh audit found `00B52855 ADD ESP,4` outside the saved function body after
the returning `_free` call at `00B52850`. A leased, locked repair cleared only
that call's flow override and regenerated the body. Its existing name and
comments were preserved. No executable bytes or global no-return policy changed.
Five source evidence comments are appended and saved, with affected exports
refreshed. Descriptive names remain hypotheses rather than recovered symbols.

BK preserves ordered volatile loads/stores, receiver preimages, actual
40h/CCh/24h allocation requests, nine native unwind states, temporary names and
the unterminated four-byte marker. Admission precedes native base publication.
The completed cockpit helper and its source metadata remain persistent; a host
binding failure after successful native construction is recorded separately from
native allocation unwind. Explicit host quiescence controls companion disposal.

BL captures one decrement-IAT epoch before auxiliary processing, then a fresh
epoch after capturing the first texture. It uses the actual captured owner count
and current terminal profile, clears fields after returned release, and destroys
vectors/base in native order. The canonical companion borrows the count begun
at the original B52578 count-one store; it adds no reference or second decrement.

## Current execution evidence

One extension to the existing retained D3D9/mesh probe uses only the three current
tracked libraries. It is compiled with MSVC Win32 `/O2 /MD /fp:strict /W4 /WX`
and an embedded manifest. The controlled child exits zero.

Six actual native D3D9 texture owners are explicitly inserted into the native
cache: black.tga (1x1), noise.dds (4x4), kosz_01.tga (8x4), szor_01.tga (12x3),
csikok.tga (10x2), and splotch.tga (9x3). Constructor loads return these exact
owners and retain them. This exercises real cache hits; the cold resolver is a
throwing guard and is not reached. Texture pixel content is not checked.

Actual backbuffer/depth surface owners are retained by the frame-target child.
The camera uses the same actual type counter/root table as the application's
resource type fixture and the same raw name pool. Its real camera allocation,
companion and 64x64 viewport register successfully. The D0 constant band is
explicitly mapped for the camera's far-distance constant.

Assertions cover completion/publication, child identities and reference counts,
texture-derived ratios (2, 4, 5, 3), the XXXX marker, the CE74F8 alias observing
the preceding receiver84 store, untouched receiver70 and other preimages, and
the live camera/viewport registration.

Explicit caller cleanup invokes the BL texture-helper terminal, retires the
cockpit/camera/viewport, frame and surfaces, drops all six cache creator owners,
and uses the existing service member/base cleanup. The texture-helper auxiliary
fields are zero from actual B52550. The existing three full mesh parsers and
focused failure probes also pass. All 64 canonical mesh/texture companions retire;
camera/helper/frame/surface lifetimes are checked separately. Native manager and
fixture resources drain. No new test suite is added.

## Remaining boundaries

Nonzero D61EC8 auxiliary producer/terminal composition, cold texture/archive
loading, shader compilation and original-machine differential execution are
unvalidated here. Canonical terminals are noexcept; unrestricted original
FH3/SEH and binary ABI equivalence are not claimed.

`00B0F6E0`, `00B14F60` and `00B151C0` are not called by this fixture. In particular,
the parent release walk visits receiver70, which B14A10 leaves untouched, and
does not release the noise owner at67C. Recover the complete initialization and
consumer ownership before using that parent cleanup. Explicit child retirement
does not establish full parent destruction.

No CS application run or native renderer startup wiring is claimed. Gameplay
and a runnable complete game remain outstanding. Main integration requires its
own review and build; CS is published on the orch4 agent branch.

Machine-readable evidence: `reports/native_render_resources_composition_cs.json`,
the CS flow/annotation/integration reports, and immutable local receipts under
`local/render_resources_composition_cs/registered/`.
