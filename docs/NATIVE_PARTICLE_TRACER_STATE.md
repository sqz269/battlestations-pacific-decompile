# Native particle Tracer state

This packet reconstructs the complete normal bodies of seven routines over actual
particle, resource and scene storage. The large initializer retains every normal
branch through its required application calls; unavailable mesh/material services
are explicit boundaries. The focused caller differential stops at the first such
boundary, `AF3440`, and does not establish a successful Tracer attachment or update.
The interfaces are new MSVC Win32 C++ entry points, not binary replacements.

| Native inclusive body | Reconstructed operation | Native ABI |
| --- | --- | --- |
| `B0B6A0..B0C80E` | Tracer virtual18 initialization | ECX definition; stack state, record; RET8 |
| `B0A840..B0A91A` | Tracer virtual1C cleanup | ECX unused; stack state, force DWORD; RET8, AL Boolean |
| `B0B5D0..B0B68C` | Current Tracer resource singleton | no arguments; RET, EAX current publication |
| `B0AB90..B0AC9B` | Actual18h resource construction | ECX storage; no stack arguments; RET, EAX storage |
| `AF3430..AF3439` | Traceline slot allocation thunk | no arguments; overwrites ECX with F8C288, tail JMP AF32F0 |
| `AFDA80..AFDABC` | Actual108h record matrix selection | ECX record; RET, EAX borrowed matrix |
| `858260..858325` | Traceline model base construction | ECX raw node; stack counted8h name; RET4, EAX raw node |

Names are descriptive hypotheses. In particular, the existing
`CG_static_dtor_stub_00af3430` label is incorrect: all ten bytes form an allocation
thunk. It is not a destructor, and its callers' ECX value `1B8h` is discarded.

## Storage and producer evidence

`B0A0B0` constructs the Tracer definition through `B01150`, installs `D5E048`,
zeros `+98/+9C/+A0`, and writes kind3 at `+10`. The actual `B0AD50` parser produces
the Tracer fields consumed here: widths `D8/DC/E0`, outer/glow/tail color triples
`B4..D4`, light radius/fade `8C/94`, tile `E4`, maximum segment count as a float at
`80`, minimum length `84`, segment life `88`, bullet life `90`, tail length `AC`,
atlas pointers `A4/A8`, and show-head byte `B0`. See the existing
[loading evidence](NATIVE_PARTICLE_OBJECT_TRACER_LOADING.md).

`B0CA40` produces the actual6Ch state, including definition `+64`, emitter `+68`
and initially null light `+60`, then captures and calls the current virtual18
at `B0CADF`. Tracer initialization publishes the owned80h payload at state `+34`
and the actual Traceline at `+30`. Cleanup receives this same state through the
current virtual1C selected by `B04F00` at `B04F2A`. The caller handles light `+60`
separately after subtype cleanup.

Record allocation callers `B03970`, `B01CE0` and `B02BC0` allocate `108h` and pass
the incoming actual model plus their emitter definition to `AFE0A0`. Its stores
at `AFE0D8` and `AFE0E6` establish record `+A4` as model and `+A0` as definition.
The alternative `AFE1A0` producer writes the same fields at `AFE237/AFE258`.
`AFDAC0` only initializes both to null, and `AFCF50` copies the actual record,
including its matrix at `+60` and these pointers. No new packed record owner or
semantic replacement is introduced.

The real Traceline pool is the static object `F8C288`: `AF32F0` uses a tracked
critical section, 32-slot slabs, `1BCh` stride and the allocation ID at slot `+1B8`.
`AF19D0` produces that slab layout. `858260` constructs the actual same-slot
`B75030` model base, installs `D0C8C8`, and performs its sparse tail stores and
child mask propagation through `7099C0`. The caller then installs `D0C928`.
`+190` is deliberately untouched by `858260`; `AF3440` later produces the point
array/count there. The existing `BAD6F0` Tracer pool (`7B0h`, ID `7AC`, `D63FA0`)
is a different object and is not used.

## Preserved behavior and application contracts

The initializer preserves native instruction and load order, all x87 evaluation
and spill points, SSE bit copies, parameter-kind branches, signed random factors,
the distinct unsigned-corrected `BD2F10/BD2E60` range draw, packed color conversion,
actual array growth/free branches, interlocked resource references, name lifetime,
and current virtual-target captures. It borrows `NativeParticleTypeStateAccess`
as its first member and uses the same `RandomThreads`, current scalar addresses,
curve helpers and matrix helpers. Its final branch either gets the record matrix
or constructs the native identity matrix before current virtual28 with five stack
words `(state, 0.0f, 0, matrix, 0.0f)`, matching native `RET14h`.

Cleanup captures the section returned by actual `72B740`, enters it and increments
the actual depth before dereferencing state `+30` and node `+190`. Nonzero count
with a zero low force byte sets bytes `+195/+1A8` and returns zero. Otherwise it
calls payload's captured deleting virtual00 with flags1, clears state `+34` only
after that call, reloads state `+30`, executes canonical `B6DFA0` unlink/release,
and clears `+30` only after release. All operations remain under the captured lock.
The payload is not a second refcounted object: `86ADE0/86AD50` destroy its actual
array and retained `+40/+44` resources, then conditionally free the payload.

Resource construction uses the counted name `pf43uf43ccuf44uf41.mvfm`, calls the
current renderer `+38`, destroys the actual name, and calls current renderer `+40`
with the actual14h declaration key. The key has one declaration at `+00`, count1
at `+10`, and three incoming stack-residue words; the access object supplies those
words explicitly. Known `B2F710` uses the existing actual hardware-layout
factory/tree/owner domain. Each atlas lookup reloads the current atlas manager.
The singleton getter retains its double check, captured manager lock, publication,
manager reload and actual `BD0C30` registration ordering.

`NativeParticleTracerApplication` deliberately requires real services:

- `AF32F0` must allocate in the actual `F8C288` pool, not a malloc substitute.
- Renderer `+38` (`B317E0`) must use its actual declaration registry/decode flow.
  An alternate current renderer `+40` target also remains a required service.
- `prepared_model` must return the prepared companion for this exact raw slot,
  the same actual string storage and the same node lifetime domain. It must not
  bind a base `NativeModelReference` whose deletion would return the wrong pool.
- Node `+5C` (`D0C928 -> AF3440`) consumes the actual80h payload and the root from
  record `+A4 -> +A4`, with native RET8. Its mesh/material services are unresolved.
- Definition `+28` (`D5E048 -> B0A110`) requires the real initial update, including
  current clock and `AF2630` processing. It has five native stack words, RET14h.
- Payload current virtual00 must perform its real deletion. Retained resources
  resolve through the existing actual owner domain and actual atomic `+04`.
  Traceline cleanup requires the existing derived node lifetime; its terminal
  destructor and physical pool return remain external obligations.

PointLight allocation/construction uses the existing physical pool and complete
`B7C710`, followed by `adopt_constructed_native_point_light` in the same canonical
owner/reference domain. There is no duplicate node, reference count or transform.
Current numeric native profiles are resolved through a pure `table_slot` service;
that lookup must not allocate, mutate storage or disturb floating-point state.

Original FH3/SEH registration and unwind are not reproduced by the new naked
initializer entry. Normal native frame space and stack cleanup are preserved;
source adapters add the access binding. Host contract failures throw or stop,
and are not reported as successful native execution. C++ cleanup guards do not
establish binary exception-unwind parity or rollback of every partial initializer.

## Verification and limits

Read-only queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, bridge8089. The installed binary SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All 15 copied function spans and 21 scalar/profile spans match live Ghidra bytes.
The PE has no relocation directory; the scratch fixture relocates 230 decoded
instruction operands, including absolute operands and relative calls/tail jumps.

The scratch fixture is `C:/Users/sqz269/bsp-ar-tracer-state/probe.exe`, built with
MSVC Win32 `/MD /W4 /WX /O2 /fp:strict /MANIFEST:EMBED`. Its original-byte side
executes the real `AF32F0/AF19D0` pool, curves, range RNG, resource fast getter,
Traceline constructor and payload destructor. It shares the canonical actual
string, RNG, PointLight, node, scene and reference domains with the source side.

- 21 paired caller comparisons pass: seven scenarios at x87 control words
  `007F/027F/037F`, including constant/linear/cubic parameters, dynamic light,
  NaNs, signed zero, a current-global alias, and color overflow. Complete state,
  definition, record, payload and Traceline bytes, array contents/allocation events,
  resource counts, pool/lock depth and RandomState match up to the actual `AF3440`
  dispatch. Pointer fields are normalized only for corresponding actual objects.
- Five complete standalone cleanup comparisons pass: deferred count, low-byte
  force `100h`, forced release and empty counts. Real payload deletion, canonical
  logical node release, field bytes and lock ordering agree. An external actual
  reference keeps the terminal derived destructor unreachable; it is not mocked
  as successful destruction. The node count is an explicit standalone input.
- Four complete `AFDA80` comparisons pass: record matrix, local matrix, cached
  world matrix and lazy world refresh, including resulting actual node bytes.

The caller deliberately raises the same explicit stop at `AF3440` on both sides;
it never returns fake attachment success. Thus `B0C74D..B0C80E`, actual `B0A110`,
resource-constructor/slow-getter behavior, free-growth branches, final derived
destruction and original exception unwind have static/source evidence only here.
The scratch original handler uses continue-search and `/SAFESEH:NO` solely to
observe that stop. The x87 checks compare control/status and a pre-existing 80-bit
stack sentinel for these cases; they are not an exhaustive floating-point claim.
There is no gameplay, visual or binary drop-in validation.

`scripts/build.ps1` passes the worktree's Win32 Release build and existing
`reconstructed_math` test. The new source is compiled by the strict scratch
fixture; the primary integrator owns its CMake registration. The report checker
passes 101 numeric call rows with zero failures: 96 direct/tail transfers checked
and five recognized indirect virtual calls; two open current-owner virtual00 rows
remain explicitly indirect.

## Read-only analysis corrections for integration

No Ghidra mutation was made. All seven entry points already have function bodies;
none require a new function. False no-return analysis after `_free` hides two
executable three-byte ranges inside the initializer: `B0BF1D..B0BF1F` and
`B0BFCB..B0BFCD`, each `83 C4 04` (`ADD ESP,4`). The installed-byte reconstruction
includes both. `B0BF3D..B0BF3F` is skipped alignment, not missing executable flow.
The integrator can repair those two flow gaps after the worker lease is released.
All call-site rows and full inclusive ranges are in the accompanying report.

## Correction from AR integration

Both returning-free ADD ESP,4 continuations at B0BF1D and B0BFCB are repaired; alignment at B0BF3D stays data. AF3430 already has its complete ten-byte body; its allocation-thunk interpretation replaces the misleading destructor name.
Saved annotation/export and final build evidence are recorded in the report.
