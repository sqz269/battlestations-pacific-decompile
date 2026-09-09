# System constant header generation

`00b38c60` emits one constant declaration; `00b38ff0` walks the system constant
registry and assigns optional register annotations. Both are now reconstructed
through explicit typed records in `shader_source.hpp/.cpp`. Complete bodies and
the four record getters match original executable bytes in saved Ghidra.
Assembly, hashes and formatting literals are in `reports/shader_constant_header.json`.

The constant record is 20h bytes. Getters confirm its name string is at +14h
(length) and +18h (data), not the beginning of the record. Dimensions are +Ch
and +8h, and array count is +10h. For dimension values A at +Ch and B at +8h,
the declaration is floatAxB when unsigned B>1; otherwise floatA when unsigned
A>1, otherwise float. Array brackets appear only when unsigned array count>1.
The variadic dimension/count format is signed `%i`, despite unsigned selection
comparisons. The typed emitter preserves full-DWORD signed formatting, including
malformed HLSL for unusual descriptor values, rather than imposing a new type
validation policy. Names use native `%s` termination. A nonnegative signed
register argument appends ` : register(cN)`. Every declaration ends with `;\n`.
Native ABI is thiscall with constant pointer and register index, RET8.

The header routine has one flag argument, RET4. It gets the singleton list via
`00b5b890`, walks 20h records in order and begins with unsigned cursor zero.
The starting cursor is annotated only when the flag is true and the cursor is
below global `00e13078`. The emitter further suppresses negative signed register
values. Cursor advance is B*array_count with 32-bit multiplication/addition wrap;
A does not affect it. This is a starting-slot check, not a range-fit test. Records
past the limit still produce declarations, with compiler-assigned registers.
The new API passes a vector and explicit register limit instead of reproducing
singleton registry ownership, mutation during emission or allocator/exception ABI.

The existing shader probe now checks exact source for float4x4 ProbeTransform
at c0, a two-element float4 ProbeTint at c4, and an unannotated scalar ProbeBias
at a six-register limit. All three are used by its diagnostic vertex main,
compiled as vs_2_0 along with the reconstructed interpolator packer. Real shader
creation/binding, the full D3D9 probe, Win32 Release build and both existing
CTest checks pass. No new test target was added. This does not validate constant
values by executing the generated shader, native compiled-bytecode identity,
or game rendering.

The full vertex/pixel generators call this header routine. Reconstructing the
system constant registry and loading actual descriptor values remains necessary;
the complete material generator and runnable game are still unfinished.
