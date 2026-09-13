# Native field zeroing and vertex input decoding

This packet supplies actual-storage B357D0, B35820 and B34E90 implementations for the remaining source-generator reconstruction. It operates on the existing B0h builder, 1Ch field and 0Ch pointer-array layouts and the same pooled eight-byte strings. The old semantic emitters in `shader_source.cpp` remain separate; whole vertex/pixel generator wiring is still outstanding.

Source and contracts are in `src/native_shader_field_initialization.cpp` and `include/bsp/native_shader_field_initialization.hpp`. `reports/native_shader_field_initialization.json` records numeric calls, byte/literal hashes, ABI, validation and limits. Names are evidence-backed descriptive hypotheses, not recovered symbols.

| Original body | Native ABI | Behavior |
| --- | --- | --- |
| B357D0..B3581C | ECX builderB0h; stacked instance8h/list0Ch; RET8 | Append two tabs, `instance.field=0;` and a newline for every current field. |
| B35820..B35924 | ECX builderB0h; no stack arguments; RET | Emit vertex scale/offset decode assignments for a captured input prefix. |
| B34E90..B34F07 | ECX field1Ch; stacked fresh output8h; EAX output; RET4 | Clear output and select x/xy/xyz/xyzw for unsigned width1..4; otherwise leave a null header. |

All three saved assembly bodies reach their RET and have zero post-call gaps. Register inputs and variadic arguments were checked in assembly; the zero-emitter pseudocode loses its field-name argument. Five full original bodies, including unchanged B35110 and 419CA0 dependencies, total 815 bytes and 20 direct calls. The four switch targets at B34F08 and seven literal/table regions are separately pinned through the verified `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.

The code bytes and file-backed literals match the installed executable. Empty literal108D6F2 lies in the PE's virtual zero-fill area: the evidence checks its zero value in saved Ghidra and its virtual section extent, without claiming that a raw disk byte exists. The worker performs read-only Ghidra analysis; the integrator handles annotation, comment preservation, saving and refreshed exports.

## Native storage and read order

Existing B354D0 produces the actual builder. Source4C is its eight-byte string, input array04 has data04/count08/capacity0C relative to the builder, and descriptor70 points to the actual 110h descriptor. Existing B34E20 produces the field's owning name8h, scalar08, width0C, mask10, semantic14 and index18. Existing B34680 produces the native pointer-array allocation. This packet never translates them into a host vector or `ShaderSourceBuilder`.

B357D0 captures a current field's name data, using empty108D6F2 when null, **before** calling the existing 419CA0 getter for the instance. The getter's empty fallback is a different original literal, E17654. `NativeShaderFieldInitializationContext` therefore borrows that separate pointer alongside the unchanged constant-header context. Each iteration calls the unchanged actual B35110 formatted-line helper, then rereads count04 and data00 of the passed list. Type, width, mask and semantic do not filter zero assignments. Existing source text remains a prefix.

B34E90 first clears output without freeing prior storage; its original output is fresh. It reads the current width DWORD, subtracts one with DWORD wrap and uses the unsigned switch range0..3. Only those four branches call the unchanged 41E350 assignment helper, with x, xy, xyz or xyzw. Other widths leave length0/data-null and allocate nothing. No valid HLSL fallback is invented for malformed widths.

B35820 captures descriptor70, tests its byte1F, and computes the smaller unsigned value of descriptor20 and builder08 **once**. That gate and prefix length remain captured throughout the loop. Before each row it reads current builder04 and calls B34E90 on that field. After the allocating swizzle helper returns, it captures the swizzle text and rereads builder04. It then reads the right-hand and left-hand field names in native order from the same current table.

Each line is exactly:

```hlsl
	IN.Name=IN.Name * cVtxElemScale[i].swizzle + cVtxElemOffset[i].swizzle;
```

B35110 adds the final newline and preserves its native pooled temporary schedule. B35820 releases the swizzle using its current header only after that child returns. A changed table can affect the name on the current row and subsequent field selections; a changed count or descriptor does not change the captured prefix limit. All entries reached by that captured limit must remain readable. Constant declarations, registry array sizes and generator ordering remain the caller's responsibilities.

## Failure and ABI limits

The implementation reuses existing 41E350, 419CA0, B35110, native string resize/destruction and actual pool services unchanged. It does not add semantic callback results or unresolved-callee shortcuts. Host CRT formatting and existing noexcept string release remain explicit boundaries.

`NativeShaderFieldInitializationOperation` holds stable continuation metadata outside the native owners. A failed decode retains the actual swizzle header, current field/descriptor/list references, captured bound/names and the existing failed B35110 child with its acquired line storage. Replaying an operation is rejected before native work; destruction of a running or failed frame terminates. The caller must also keep the actual input owners, strings, scratch and domain alive and exclude their teardown; those external owners do not receive automatic guards here.

The existing assignment and formatting helpers keep their own failure behavior. Entering an assignment is distinct from its normal return. The frame does not promise native FH3 unwinding, rollback, retry or recovery. Explicit diagnostic cleanup must resolve all retained native acquisitions, including those in the child, before acknowledging frame retirement.

These C++ interfaces are not original thiscall/SEH binary replacements. Readable actual pointer graphs and sufficient shared scratch0108D6F8 are required, including a valid descriptor70 even to test the decode gate. The native shared-scratch reentry/concurrency restriction remains. No null-pointer substitute or new malformed-width behavior is introduced.

## Validation scope

The report records the default MSVC Win32 Release `/W4 /WX /fp:strict` build, both existing CTests after `verify-seeds`, live numeric-call verification and one ignored original/source fixture linked only against the registered libraries. Registration appends one deferred source line after a transaction checks the shared registry is unleased, before building.

The fixture executes five copied full original normal bodies. Verified E8 operands are redirected to copied originals or established actual-storage/CRT bridges; only the B34ECF absolute switch-table operand is relocated to the same four branch offsets. Branches and original FH3 registration instructions remain intact. Failure probes invoke reconstructed C++ only; they do not invoke relocated native handlers.

The fixture uses B34E20-produced fields, B34680 pointer-array allocations, a B354D0 builder, actual pooled strings and normal B3A7E0/same-domain shutdown. It compares exact text and allocation/release traces for all swizzles, invalid widths, empty fields/instance strings, zero list bounds and current list/count/instance replacement. Decode checks cover unsigned clamping, disabled/zero limits, current table replacement during swizzle allocation with preserved limit/gate, malformed-width text, and untouched fields and unrelated builder bytes. A focused failure check retains both the swizzle and formatted-line child plus the output preimage, rejects replay and verifies failed-frame retirement exit77 before explicit cleanup.

This establishes bounded source and fixture behavior. Whole source-generator composition, shader compilation, rendering and gameplay remain unproven by this packet.
