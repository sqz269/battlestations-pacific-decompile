# Native vertex source generation

Addresses: 00b39110

`generate_native_shader_vertex_source_00b39110` reconstructs the complete normal
body into the existing actual B0h builder's pooled source at 4Ch. It calls the
accepted actual string, constant, field, struct, interpolator and sampler
implementations unchanged. It does not construct a semantic builder projection.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| B39110 | ECX actual B0h builder; no stack arguments; RET at B39872 | Complete normal body B39110–B39872, 594 instructions, 1,891 bytes; no Ghidra gaps |

The sole caller is B3B3C0 at B3BAB9: EDI receives incoming ECX at B3B3ED and is
never replaced before the call; B3BAB7 moves EDI to ECX. The caller first runs
B36800, then invokes this generator, then consumes the current builder50 source
at B3BABE. Its later B60F60 compilation and remaining B3B3C0 continuation are
outside this packet. This source interface is not a binary ABI replacement.

## Storage and order

The B354D0/B3B3C0 builder layout and the existing B43700 descriptor layout are
reused. B34E20 produces the 1Ch fields; B35930 copies descriptor D0 inputs to
builder04, B35BE0 produces system fields10, B36800 produces the distinct 1C/28
output lists, and B34AA0 produces mappings54/60. The generator neither aliases
these lists nor replaces them with host vectors.

Descriptor74 is the mode descriptor; descriptor70 is the base descriptor.
B45F70's mode loop writes mode ordinal10C after B43B00 parses it. B43B00 writes
ReceiveShadows at byte15, Constants at E8, and VS at F0. Builder78 is a different
effect owner and is not read by B39110.

1. Read source length. Only a nonzero length enters clearing. Capture its data
   and length, return a nonnull buffer with length+1, then write data=0 followed
   by length=0. A zero-length nonnull buffer survives this initial stage.
2. Read current descriptor74+10C once. Modes 0/3/7/6/1/4 emit respectively
   NORMAL/UNDERWATER/MAP/DRAW_SHADOW/REFLECTION/REFRACTION defines; all others
   emit no define. Run B38FF0 with explicit-register flag1.
3. Read current mode74 header dataEC, then current base70 dataEC, each with
   actual108D6F2 fallback. B35110 emits `\n%s\n%s` in base/mode order. Its four
   cdecl arguments are confirmed by ADD ESP,10h at B391F4.
4. Declare sVertexIn from04 with semantics1, sSysValues from10 with semantics0,
   and sVertexOut from1C with semantics0. All start at0 and disable vPos. Run
   B36E30(position1,fog1,vPos0), then the actual vertex sampler emitter B38080.
5. Re-read current descriptor74 byte15 for the optional shadow helper, then
   append the ambient/fog helpers. The existing `shader_vertex_literals.inc`
   matches the disk/live literals exactly, including the shadow early return.
6. Emit ShaderCode wrapper, counted current descriptor70+F0 body, EffectCode
   wrapper and counted current descriptor74+F0 body. Header C strings terminate
   at NUL; body counted strings preserve embedded NUL bytes. Run B35540 using
   the distinct fields28/maps54/60.
7. Emit main wrapper and SYS/OUT locals; B357D0 zeroes current lists10 and1C.
   B35820 decodes actual vertex inputs. Emit ShaderCode, EffectCode, position
   transfer, PackInterpolators return and final closing brace.

All direct numeric call rows appear in the JSON report, including the runtime
pool getter/return, resize and memmove sites. Struct calls have RET14h, zero
calls RET8, interpolator declaration RET0Ch, string-line calls RET4, and the
no-argument children plain RET. EBX is zero throughout the B39110 body; EBP
holds the builder until the final temporary captures its length there.

## Temporary and failure lifetime

Outer temporary storage is the same address-stable actual8h header in a
persistent operation. Each construction clears it, calls actual41DD40 with
preserve1, captures data and current length, and copies length+1 literal bytes.
The name/wrapper requested lengths 9/10/10/74/60/34/3/3 and brace length1 match
the original literals, including their exact terminators.

Every outer release uses captured data with the header's current post-child
length, except the final brace, whose data and length are both captured before
B35030. Released native headers remain stale. Completed child metadata may be
replaced only after its actual acquisitions have been released.

On a borrowed failure the exact outer header, captured preimages, partial
builder output and entered native child frame remain visible. Existing child
cleanup rules remain unchanged: an entered-but-not-returned helper header alone
does not prove its buffer is still owned. No original FH3 unwind, rollback or
retry is claimed. The caller retains all borrowed owners/context/scratch/domain
and excludes their retirement while running or failed. Destroying an unfinished
frame terminates; explicit diagnostic retirement requires temporary ownership
cleared and failed children retired, and does not complete generation.

## Validation boundary

The focused ignored fixture executes copied original B39110 against the new
source while both legs call the same accepted actual native children. It is a
composition differential, not an all-original dependency-chain execution.
Installed debugshader.shfx and lights/dummy.shfx pass through the actual
Lua/VFS descriptor reader. Actual B35930/B35BE0/B36800/B34AA0 producers, the
52-row B5BF70 registry, and the shared native string pool build fixture inputs.

The fixture compares full counted output, allocation/release sizes and bytes,
interpolator words, and surviving builder preimages; it targets live descriptor
replacement, zero-length buffer preservation, and counted embedded-NUL text.
One failure at the first struct child retains the outer sVertexIn name and
failed actual B35110 child; diagnostic cleanup and a separate destruction-guard
process exercise the boundary. Exact run/build/hash results are recorded in
`reports/native_shader_vertex_source.json` and the ignored final manifest.

Default Win32 Release `/W4 /WX` and both existing CTests passed after live/disk
seed verification. The live report check passed all 87 numeric call rows.
All eight render-mode cases and three targeted storage variants matched, as did
the retained-failure assertions and destruction guard exit77.

Both legs generated identical 4,412-byte installed-input HLSL. The installed
D3DX9_40 compiler with main/vs_3_0/flags1200 produced identical 432-byte bytecode.
This compiler check does not enter
the material compiler continuation or establish gameplay/render parity.
