# Native pixel source generation

Addresses: 00b39880

`build_native_shader_pixel_source_00b39880` reconstructs the complete normal
body using the actual B0h builder, actual descriptor/field/list headers and
the same pooled source at builder+4C. This is an explicit C++ interface; the
native calling convention, private FH3 frames and game execution remain outside
this packet. Names are descriptive hypotheses, not recovered symbols.

| Routine | Original ABI | Coverage |
|---|---|---|
| B39880 | ECX builder; stack declaration-list0Ch, unpack-list0Ch; RET8 | Complete normal 3,449-byte body, 1,018 instructions, inclusive B3A5F8 |

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Live/disk bytes agree; the stored body has no
instruction gaps. `reports/native_shader_pixel_source.json` carries every
numeric call row, literal, byte hash and helper/caller contract. There are
163 calls in B39880, 32 calls in unchanged copied fixture line helpers and
two caller rows. All 197 rows pass the live report verifier. No new function
definitions or Ghidra changes were made.

The source reuses the existing native constant header, struct, sampler,
interpolator struct/unpack, shadow and zero-field helpers. Reviewed shadow
dependency `790cc4b00498e97b82aaec44a257fd03e1fbbe3d` supplies B38230/B382B0.
The existing `shader_pixel_literals.inc` is unchanged and its bytes were
checked against the original image. There is no `ShaderSourceBuilder`
conversion, semantic emitter callback or generated-text substitute.

The caller at B3B688 passes builder+1C twice. At B3BDF2 it pushes builder+28,
then +1C: declarations use +1C and unpacking uses +28. EDI supplies ECX at
both sites. B39880 preserves those separate identities. Its B39963 formatted
call has four cdecl arguments and `ADD ESP,10h` at B39968; the earlier EBX push
is a saved register. Mode common data is captured first, then base common data,
and the emitted order is base followed by mode. Copies at BF7680 use its
overlap-capable `memmove` contract.

Storage comes from the established B354D0/B3B3C0 builder, B43700/B43B00
descriptor, B34E20 field, B36800 selection, B34AA0 mapping, B372D0 system-field
and B5BF70 registry producers. This packet introduces no owner layout. The
field-initialization context preserves the distinct native empty cells
108D6F2 and E17654; the same 108D6F8 formatting scratch and live registry/
register-limit cells are supplied to the existing helpers.

Read and cleanup order matters:

- A nonzero old source length captures the old buffer and length for return,
  then clears data and length. A zero length leaves the existing data word.
- Pixel/SYS names and ShaderCode punctuation capture data and length before
  child calls. EffectCode punctuation captures data but returns current length.
- EffectCode captures A8 before constructing its suffix, then reads current
  A4 for the unsigned 711370 constructor. Both header concatenations and all
  five pooled temporaries remain real operations in their original order.
- Main captures A4 and A8 for its signature/initialization branch. Counts
  outside 1..4 skip those lines and continue emitting malformed source. The
  later locals read A4 after their suffix allocation. Their prefix returns
  captured data with current length; other concatenation headers return current
  data and length. SYS name and final punctuation return both captured values.
- The mode+10C is reread after the first color transform. A changed descriptor
  can therefore cause both transform policies to emit. Alpha choice A9 is
  retained across Color0/fog appends, while current mode+31 is read afterward.
  No-fog premultiplication reads current base+32 after the alpha append; the
  fog path never premultiplies. Each later COLOR output independently rereads A4.

`NativeShaderPixelSourceOperation` is persistent host metadata. A failed call
retains the partially written actual output, entered/returned temporary-header
state and exact failed native helper child. It cannot be replayed or retired
without diagnostic cleanup. Owners, replaceable lists/descriptors, strings,
scratch and context must remain alive and retirement must be excluded by the
caller. Existing constructor/concat cleanup remains its own boundary:
entered-but-not-returned headers do not prove live ownership. No rollback or
native exception emulation is added.

This differs from the earlier semantic projection described in
`SHADER_PIXEL_COMPOSITION.md`: that interface rejects unsupported MRT counts
and publishes only on success. It remains a separate interface. The native
storage implementation preserves incremental writes and the actual malformed
count path.

The default MSVC Win32 C++17 `/W4 /WX` build and both existing CTests passed
after seed verification. All ten original/source comparisons passed: eight
MRT/depth branches, unsupported MRT0 and a bounded chain of descriptor/alpha/
count mutations. Text, allocation/release traces and unrelated builder bytes
agree. Failure retains the actual B34F20 child and output after reset; replay
is rejected, premature retirement exits77 and explicit cleanup reaches
canonical registry/string-pool shutdown.

Installed D3DX9_40 compiles the fixture's no-fog depth source as `ps_3_0`,
flags1400, using the actual generated source and a descriptor PS body that
initializes Depth. The fog fixture produces identical X3004 `cFogDirColor`
errors from original and reconstructed text. Its actual registry has
`cFogDirColor4`/`cFogColor`, while both descriptor E8 strings contain only
comments. B43B00 reads the Lua `Constants` property into E8 at B44480/B44499/
B444B9, so wider descriptor/pipeline context may supply an expected definition.
A bounded installed shaderfx scan found the token in `shaders.bin` and no
textual descriptor definition. This is an unresolved context boundary, not a
claim that native fog shaders are defective. Both error logs and exact inputs
are preserved separately from the successful compile evidence.

Validation results and transitive fixture artifacts are recorded in the report.
The focused local executable compares relocated original B39880 and native
source with copied original B34F20/B35030/B35110. The remaining dependencies
are the unchanged actual native helpers in the same pool/registry domain;
only direct CALL operands are relocated. Its inputs use actual registry,
field, system-field and mapping producers. Source-only failure checks retain
the child frame and reject replay/retirement. This does not establish original
FH3 compatibility, material execution, draw/readback or gameplay behavior.
