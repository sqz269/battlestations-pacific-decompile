# Actual material program compiler continuation

Addresses: `00B3B3C0` (full native body `00B3B3C0..00B3C390`).

`NativeMaterialProgramCompilerActual` supplies the substantive tail required by
the existing raw compiler prefix. It resumes the same caller frame, builder and
local name at `B3B513` or `B3B536`. Together with the existing prefix it covers
the normal native function. The complete source implementation links with real
providers. Whole compiler execution and original FH3 unwinding remain untested.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| B3B3C0 | ECX actual B0h builder; stack effect, root descriptor, mode descriptor; EAX pass; `RET 0Ch` at B3C38E | Existing prefix plus complete normal actual continuation; source failures retained, original FH3 not emitted |
| B3B513 continuation | EDI same builder, EBP zero, original local name live | B35BE0, B36800(null,null,1C), B34AA0(1C), B372D0 before common allocation |
| B3B536 continuation | Same frame, builder, local name; no vertex-input replay | Common pass allocation and all cached/generated branches through B3C390 |

The complete 4,049 bytes (1,282 instructions) match the read-only installed PE
and live original project. SHA-256:
`190350a4fbf442dd6820d2b5caad70f55f4f9b2eb1be3bbb97c5b944a1ed0757`.
The report records full direct-call rows, indirect operands, profiles, handler,
11-state unwind map, and cleanup actions. Existing prefix/source foundations are
consumed from published `4812aa79`; per-file provenance is recorded in the report.

## Native storage and providers

The context borrows the application's canonical raw string pool, current
renderer/cache publications and state/retained-owner domains. Its three required
registration functions only bind host metadata for each newly produced actual
pass/reflection/shader owner; they must not retain, mutate or substitute it.
All domains and current publications in the borrowed child contexts must refer
to the same native world. There is no default provider or typed-pass conversion.

The B0h builder comes from existing B354D0, not a projected compiler request.
B41820 and the distinct numeric-profile B44B10 overload construct the real 88h
pass. B44B10 reaches the complete B319B0 white.tga cache through current
D5F0A8+64. Two raw 88h metadata objects use the existing B3B3C0 initializer:
reference count one, numeric D61810 profile, FF bytes08..3D, zero74/78/7C/80/84,
and untouched 3E..73/75..77 preimages. The pass receives them at70 then74.

Cached VS/PS paths build local-name `.vso`/`.pso` keys and call actual B34890.
They dereference the returned record, reflect its current bytecode, reload the
renderer device and call real COM CreateVertexShader/CreatePixelShader. A null
cache record is a native invalid dereference, not a graceful compile failure.

Generated paths use the existing actual field/source builders and B60F60/B61280
against the original root descriptor's profiles38/40 and current source4C.
The preliminary pixel compilation uses both inputs from1C and fills ten
TEXCOORD/two COLOR DWORD masks. Its successful COM result is deliberately
orphaned: native later overwrites that stack cell and never releases it.
Filtered VS assembly clears28, selects usage, resets54/60, appends28 mapping,
clears/rebuilds1C and generates the VS. Final PS uses1C/28 and supplies the same
DWORD containing500 to both usage outputs. GetFunction size, allocation,
GetFunction data, reflection, optional cache writes and free preserve ordering.
The two cache gates and each stream reload remain at their native positions.

Each shader wrapper uses real B5FAF0/B5F9B0 construction and canonical binding,
then temporary COM Release, real raw pass slot assignment, and raw owner release.
Unsupported vertex-texture format71 with a vertex sampler skips compilation
but still constructs the native wrappers around null COM pointers.

After both shaders: capture root descriptor, zero builder8C/90/94; B3B280 root
then mode; B34920 root then mode; capture pixel metadata84 and prune16 slots;
optional capability3D alpha-to-coverage state; effect-only ShadowMap then
ShadowTexture searches (each repeated), adding captured base pixel-sampler
count; finally ShadowMap MAG, MIN, MIP, addressU, addressV with current78 reloads.
The two B3B280 contexts must supply their distinct caller scratch preimages and
post-call residue evidence, using the same actual owner/pool/renderer domain.

## Ownership, cleanup and ABI limits

The actual tail frame is attached to `parent.tail_child` before its first native
effect. Every retained child frame is attached before the child call. A failure
keeps acquired owners, names, COM pointers, buffers and child operation state
alive. Running/failed frame destruction terminates; there is no acknowledgement,
disarm or rollback to make a failed operation appear complete. Normal null
compilation returns release only the original local name: previously acquired
pass/metadata/COM objects remain native orphans. Normal string returns leave the
raw header stale. Bytecode pointers remain recorded after their native free.

The original B3B3C0 FH3 handler is CBEC93, descriptor DF73F0, unwind map DF7414:

| State | Previous | Native action |
| --- | --- | --- |
| 0 | -1 | CBEC20: local name destruction |
| 1 | 0 | CBEC2B: substring destruction |
| 2 | 0 | CBEC33: raw pass pool return B41040 |
| 3 | 0 | CBEC46: generated VS suffix |
| 4 | 3 | CBEC51: generated VS joined name |
| 5 | 0 | CBEC3E: cached VS suffix |
| 6 | 0 | CBEC59: raw VS wrapper free |
| 7 | 0 | CBEC72: generated PS suffix |
| 8 | 7 | CBEC7D: generated PS joined name |
| 9 | 0 | CBEC67: cached PS suffix |
| 10 | 0 | CBEC85: raw PS wrapper free |

Source records reached state/site but does not pretend the host C++ exception
ABI is that FH3 table. Original private stack aliases, incidental registers,
arbitrary invalid pointers/extents, concurrent mutation of private scratch,
other dynamic profiles and failures inside imported D3DX/Win32 remain explicit
execution boundaries. Inline array reset retains native negative-capacity
reserve/copy/free and negative-count wrapping writes; safe extents are required
for a defined C++ execution. This is not a drop-in machine-code replacement.

## Validation

Eight original math seeds match; strict MSVC Win32 `scripts/build.ps1` and both
CTests pass. The Win32 `/MD /fp:strict /W4 /WX` fixture exports a factory whose
linked vtable contains the complete actual continuation and its concrete
providers. The link map confirms both the parent and numeric B44B10 provider;
there are no unresolved/default symbols.

The focused original/source fixture executes only BE4460, BE40D0, original and
source physical WriteFile, and B1FF50. Two cache writes cover nonempty/empty
names, exact prefix/payload bytes, output-count overwrite, null output and
64-bit position carry; three getter inputs include zero and pointer wraparound.
The original code copies relocate only the fallback literal/import-pointer
operands and callable vtable addresses; all original stream bodies execute.
An initial fixed-address fixture failed before execution due to an occupied
mapping; its log is retained separately. Whole compiler generation/cache/pass
execution, FH3 failure behavior and gameplay have not been runtime validated.
Final immutable source/compiler-read/link/tool/runtime closure is in the report.


## Independent ordering review and correction

The independent full-body review is frozen at
`orch5-material-compiler-review/local/material-compiler-review.json`, SHA-256
`03398ee054fa4af04839f54dc52b11e8c80925ed68084e17dd8da2caa24ed56e`.
It checked all 1,282 exported lines, all four companion bodies, and all 7,655
upstream frozen artifacts without altering the original compiler archive.

Source correction `d61b6329ab12ba7688a500eb2abb2a0b90ecae24` changes only
private helpers and the normal continuation's ordered reads:

- B3C2AC captures the first ShadowMap sampler before builder+AA; the first
  MAG call uses it, and the remaining four state calls reload current pass+78.
- Cached reflection reads current pass+70/+74 before cache row+8.
- Cached COM creation captures the actual device table before row+8, then
  loads its current +16C/+1A8 entry and calls the original stdcall signature.
- Inline array clearing captures capacity once for both tests and count once
  for the negative-count offset. Live loop-count reloads remain in place.
- String literals and normal releases keep native null guards before current
  length reads. Repeated shadow lookup releases use the pointer captured
  before the literal copy, with length read only at the guarded release.

No public API, native address ownership, registration policy, child adoption,
null-return retention or exception cleanup policy changed. The compiled Win32
object confirms the corrected capture/load order and COM stack signature.
Initial flag capture around private mask initialization and pixel cache versus
private-size staging remain within the documented private-scratch timing limit.
The correction's checks and final frozen closure are recorded separately from
upstream evidence in the report; whole compiler/B44B10 execution remains open.
