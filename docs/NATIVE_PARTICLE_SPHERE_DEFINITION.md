# Native sphere particle definition

Addresses: `00B02BC0`, `00B02CE0`, `00B02F30`, `00BD2F40`.

The sphere definition now provides allocation, position/velocity generation,
and generation with matrix selection over the actual native objects. A shared
unit-float random entry uses the application's existing `RandomThreads` domain.
These are new Win32 C++ interfaces with a borrowed EDX access pointer, not
drop-in vtable entries or a reproduced compiler exception ABI.

| Entry | Proposed descriptive name | Inclusive end; bytes | Native ABI | Coverage |
|---|---|---|---|---|
| B02BC0 | SphereEmitter_CreateParticleRecord | B02C30; 113 | ECX definition, stack(model,time), RET8, EAX record | Complete normal body through required allocator and AFE0A0; compiler EH frame excluded |
| B02CE0 | SphereEmitter_GeneratePositionVelocity | B02F26; 583 | ECX definition, stack(parent,position,velocity), RET0C | Complete |
| B02F30 | SphereEmitter_GeneratePositionVelocityMatrix | B02FA8; 121 | ECX definition, stack(parent,position,velocity,matrix), RET10 | Complete through captured current virtual0C |
| BD2F40 | RandomThreads_NextUnitFloat | BD2F84; 69 | ECX stream, no stack arguments, ST0 float, RET | Complete through canonical state lookup/draw |

Names describe established behavior and remain hypotheses, not recovered symbols.
The final instructions are B02C2E/RET8 (3 bytes), B02F24/RET0C (3),
B02FA6/RET10 (3), and BD2F84/RET (1). Live Ghidra and installed PE bytes match
all four complete ranges. The integrator created the missing B02F30 function
under its write lock; a subsequent flow audit finds no gaps in these bodies.

## Storage and producers

AF9FB0 chooses `SphereEmitter`, allocates 8Ch and invokes B02B90. B02B90 calls
AFA280 and installs D5DE88; slots08/0C/10 are B02BC0/B02CE0/B02F30. AFA280
stores its mode argument at definition70. The B02FD0 reader writes the
`EmittedSpeed`, `InnerRadius`, and `OuterRadius` curves to definition80/84/88.
Those constructor/parser producers are evidence only and belong to other work.
No competing definition, curve or particle layout type is introduced here.

B02BC0 requests exactly108h from the real BF681B allocation domain. Its
AFE0A0 dependency is the shared `construct_native_particle_record_00afe0a0`:
it establishes parent modelA4/definitionA0, time30, sparse zero fields, and
A8/AC/B0/B4 from the current one word. AFE1A0 is a different record initializer
and cannot replace this constructor. Untouched bytes stay untouched.

## Preserved behavior and required services

`NativeParticleUnitRandomAccess` borrows the canonical random owner and current
CE3978/D63B80 addresses. BD2F40 selects the current thread/stream state, retains
the pre-draw null-write guard, calls existing BA2C20, converts signed EAX with
FILD, adds the current float unsigned correction for negative EAX, multiplies
by the current double scale and spills/reloads float32. It retains the native
unchecked stream indexing contract; no new random seed or fallback is added.

B02CE0 performs two initial primary-stream draws. Its x87 schedule includes
the intermediate float32 spills, square-root CRT binding, FSIN/FCOS spills,
and the FCOMIP clamp to the current percent-limit word. Inner/outer/speed curve
type0 reads the stored constant, type1 calls AFFA70, and other types call
AFFAE0. An ordered zero radius difference skips the third draw; an unordered
difference takes it. The original FUCOMIP/LAHF/TEST/JP sequence is retained.
Position components are calculated into original stack scratch before their
forward stores. The speed curve pointer is captured before those stores;
later curve reads remain after them. Input/output overlaps preserve this order.

B02F30 captures the current definition vtable0C target before invoking the
required dispatch callback. It reloads parentA0 after that call. Definition70
selects parent60; otherwise parentA4 selects modelB0 when model1B0 is nonzero,
or modelF0 after a required B6DB70 refresh if flag5C bit2 is clear. All matrices
use the existing sixteen forward x87 copies at004134F0.

| Original callee | Required implementation/contract |
|---|---|
| BF681B at B02BDE | Required real cdecl allocator, one size argument; ADD ESP,4 |
| AFE0A0 at B02C06 | Shared sparse108h constructor, three stack words; RET0C |
| BD2F40 at B02CE9/B02D04/B02E3C | Shared canonical unit random entry |
| BF7030 at B02D67 | Existing `native_crt_sqrt_st0_00bf7030`, borrowed actual `CameraAxesCrtAccess`, operand/result ST0 |
| AFFA70/AFFAE0 at six sphere sites | Existing linear/cubic evaluators, time on stack; RET4 |
| Captured vtable0C at B02F45 | Required same-object exact-target dispatch; three stack words, RET0C |
| 004134F0 at B02F5B/B02F7F/B02FA0 | Existing native matrix copy; source on stack, RET4 |
| B6DB60/B6DB70 at B02F75/B02F90 | Existing local-matrix getter/world refresh on actual model storage |
| BD2ED0/BA2C20 at BD2F41/BD2F62 | Existing canonical thread lookup/random-state draw |

Compiler unwind metadata is outside the allocation wrapper's new ABI. The
original CBB5A0 cleanup was truncated in Ghidra after its BF65AC call: disk
CBB5A9/POP ECX and CBB5AA/RET complete the11-byte funclet. CBB5AB..CBB5B4 is
the handler entry using DF365C. These were reported to the integrator for
separate repair; allocation exceptions/unwind-handler identity are untested.

## Verification and limits

Both production objects compiled with MSVC Win32 `/W4 /WX /fp:strict /O2 /MD`.
One temporary original-byte differential executable passed 32 unit draws
including current-scalar variants and the actual guard fault, six generation
scenarios, four matrix paths, and allocation/null paths. It compared float
bits, floating-point status, complete canonical random state, parent/output
overlaps, world-cache writes and sparse108h constructor output. Original calls
were relocated to the same canonical dependencies; this does not independently
revalidate those dependencies or the whole original executable.

Scratch commands, original-byte generator, probe source and log remain under
`C:/Users/sqz269/bsp-an-sphere/`; no permanent test was added. Build integration
and game execution are the integrator's responsibility. There is no gameplay,
rendering, binary-vtable, allocation-exception or concurrent-mutation proof.
