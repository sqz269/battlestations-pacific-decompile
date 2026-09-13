# Native shader field and struct declarations

This packet reconstructs B34F20, B35030, B385B0 and B38B50 over the actual eight-byte pooled string headers, 1Ch fields, 0Ch field-pointer list and B0h material-program builder. These supply actual-storage children for the remaining source-generator reconstruction. Full vertex/pixel source-generator wiring is still outstanding.

The source is `src/native_shader_struct_declarations.cpp`, its public contracts are in `include/bsp/native_shader_struct_declarations.hpp`, and machine-readable evidence is in `reports/native_shader_struct_declarations.json`. Descriptive function names are evidence-backed hypotheses, not recovered symbols.

## Original bodies and ABI

| Address | Original interface | Reconstructed operation |
| --- | --- | --- |
| B34F20..B3502F | ECX destination8h; stacked C string; RET4 | Append a pooled C-string copy, release it, then append/release a separate pooled newline. |
| B35030..B3510B | ECX destination8h; stacked source8h; RET4 | Append source with length captured before resize and data reloaded afterward; append/release a separate pooled newline. |
| B385B0..B38A41 | ECX field1Ch; stacked output8h and semantics low byte; EAX output; RET8 | Clear output, append type, optional unsigned width, two tabs, name, optional semantic/index and pooled semicolon. |
| B38B50..B38C54 | ECX builderB0h; stacked unsigned start, name8h, list0Ch, semantics low byte, allow-vpos low byte; RET14h | Append struct opening, each current field declaration, optional vPos and closing text. |

All four saved assembly listings reach their RET instructions and have zero post-call gaps. Some pseudocode still reports unreachable blocks; the complete saved assembly is authoritative for the pooled newline/semicolon schedules. The existing 711370 unsigned helper contains three unused bytes at 7113BD..BF after an unconditional jump; these are alignment, not a missing return path.

Live bytes from the verified `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, match the installed game executable. Six complete original normal bodies, including unchanged B35110 and 711370, total 2,496 bytes and 94 direct calls. The eleven DWORD switch destinations at B38A44 are separately pinned. The worker uses read-only Ghidra queries; annotation, saved comment preservation and refreshed exports belong to the integrator.

## Storage and temporal behavior

Existing B354D0 produces the actual builder. Only source string4C changes here. Existing B34E20 produces the same 1Ch field layout: name8h, scalar08, component count0C, mask10, semantic14, index18. Mask10 is not read by B385B0. The passed list has pointer data00, count04 and capacity08; this routine neither owns nor modifies it.

B34F20 constructs its C-string temporary before touching destination storage, then captures that temporary's length and data across destination resize and release. This permits a C-string argument to refer to the destination's current characters. B35030 instead captures the source length first, resizes the destination, then reloads source data. Its source and destination headers can be identical. Both routines create a separate one-character pooled newline; B385B0 creates the semicolon with the corresponding explicit resize/copy/captured-release sequence.

B385B0 clears the output header without releasing a prior buffer, matching its fresh-output ABI. Scalar0 produces `float`, scalar1 produces `int`, and other values omit the type. An unsigned component count greater than one adds the unsigned decimal count before two tabs. The branch test precedes tab construction; the numeric value is reloaded after that construction. B38631 and B38954 call **711370**, not signed decimal constructor 5F1840.

With a nonzero semantics low byte, the formatter appends two tabs and ` : `, then the token selected by the current semantic DWORD: POSITION, COLOR, TEXCOORD, NORMAL, BINORMAL, TANGENT, BLENDINDICES, BLENDWEIGHT, FOG, INDEX or VPOS. An unknown semantic omits the token but still appends the current unsigned index. Scalar and semantic unknown cases follow the original switch behavior without adding validation or fallback enum values.

B38B50 emits `\nstruct %s\n{` through the unchanged actual B35110 formatted-line API, then reads current count04 and data00 for each row. It retains the actual field output until its tab-prefixed line returns and releases that field output before the next row. The native semantics argument stays constant across the loop. After all rows, allow-vpos gates current builder70 and then current builder74 reads at byte30; the second descriptor read is short-circuited. A high byte at word30 does not enable vPos. Closing text is `};\n` passed to B34F20, yielding two trailing newlines.

## Existing dependency and failure boundaries

The implementation calls the existing actual-string 41E870 constructor, 41E350 assignment, 4261A0 concatenation, 425E10 append, 41DD40 resize and 41DD20 destructor. It reuses `construct_native_material_program_number_00711370` and the existing constant-header context and B35110 operation unchanged. There are no semantic formatter callbacks or successful unresolved-callee stand-ins.

`NativeShaderStructDeclarationOperation` is stable host continuation metadata. It records actual temporary headers, current field/list/builder, active native site, captured buffer preimages and the actual B35110 child. A failure retains this frame and cannot be replayed. Frame destruction while running or failed terminates. The caller must also keep all raw owners, strings, context and shared scratch alive and exclude their retirement; this interface does not install automatic guards on those external owners.

Existing helper cleanup remains a distinct boundary. In particular, 4261A0 frees its partial output if the append fails after its initial copy, and 711370 cleans its internal temporary on an output-copy failure. An entered-but-not-returned helper header can therefore describe freed storage. The operation distinguishes helper entry from normal return and never promises that every nonnull retained header is owned. Diagnostic cleanup must resolve only the acquisitions that remain live; acknowledging cleanup is not native completion.

The new C++ interfaces do not reproduce the original private FH3 handlers, SEH registration or binary calling conventions. Existing `NativeStringStorage` release remains noexcept. Readable original pointer graphs and enough actual shared scratch at 0108D6F8 are required; native scratch reentry/concurrency restrictions remain. Invalid pointer graphs and overflowing extents are not converted into host-friendly substitute behavior.

## Validation

The default source registry includes this translation unit after a transaction checked that `cmake/startup.cmake` was unleased. Validation results and hashes are recorded in the JSON report: standard MSVC Win32 Release `/W4 /WX /fp:strict` build, existing checks after `verify-seeds`, and a single ignored original/source fixture linked only against the registered libraries.

The fixture relocates six full original normal bodies, rewrites the verified relative CALL operands to their corresponding copied bodies or established actual-string/pool/CRT bridges, and relocates only the absolute B387F3 switch-table operand to the same eleven branch offsets. The original branch layout remains intact. Read-only installed literal pages and the actual pooled storage owner are used. Failing tests invoke only reconstructed C++; they do not invoke relocated original FH3 handlers.

Comparisons cover exact emitted text and pooled allocation/release order, existing output prefixes, C-string aliasing, header self-append, empty input and a source pointer changed during resize. B34E20-produced fields cover all semantic tokens, unknown scalar/semantic values, zero/scalar/vector/UINT_MAX widths, unsigned indices and low-byte flags. Struct checks use the actual builder layout and verify start bounds, current list data/count replacement, post-row descriptor replacement, byte30 vPos gating and unchanged unrelated builder/field bytes. Focused failure checks verify retained temporary/output preimages, replay rejection, the existing concat cleanup boundary, explicit diagnostic cleanup and failed-frame retirement exit77.

This is source reconstruction and bounded fixture evidence. Full source-generator composition, shader bytecode generation, rendering and game validation remain unproven.
