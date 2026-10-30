# Independent review: raw Text single-line geometry

Reviewed exact `0bc258c71f44b0729da801f3aa9b598856d2276f`: both production files, whole AB9FD0..ABA263 (660 bytes/195 instructions), all22 x87 forms and the required existing Mesh/material/mapping provider contracts. Only this document and its report are review-owned.

**No blocking Source finding was identified.** The library counterpart is ready for primary integration/common-build assessment within its stated Source domain. This does not close an actual nonempty Text caller, missing genuine providers, Original ABI/FP runtime parity or frontend activation. Earlier passed Source cohorts do not validate this new supplier.

## Actual homes, captures and loops

The first stacked UTF16-wrapper home is never read. Section home1 remains current at four distinct sites: initial two count stores, first texture binding, second texture binding after its callback, and final layout after both unmaps. Text+EC is separately read for vertex/index map counts and both section counts. Text+F0 is independently captured for measure and emit; the second loop does not reuse the first cursor or a copied string.

Both loops consume zero-extended UTF16 code units until NUL, with no length bound, LF/CR split, kerning, Unicode normalization or new range policy. Each font call reloads CURRENT Text+108. Advance is unsigned glyph+12 and uses CURRENT Text+1D8. Measure advances its cursor before FILD/FMUL/FADD/FSTP32 to Text+114. Emit keeps the captured glyph through returned quad, loads its advance and next UTF16 unit before the native FP pen-update phase, and retains each float32 spill and DWORD counter wrap.

## x87/control-word schedule

FNSTCW is after returned index mapping and before section stores. The width block FLDs actual Text+20, captures CURRENT Font, FMULs actual CEC380 under the original CW, then captures unsigned Font height and Text+F0. It loads savedCW|0C00 without changing masks, performs FISTP64, retains its low DWORD and restores saved CW ONLY after returned FISTP at the native phase. No fenv/RAII guard, finally reset, status clear or new failure termination is introduced.

Right/center alignment FILD signed width bits, conditionally adds the live2^32 float for unsigned correction, then subtracts current measured width. Center multiplies by the actual half double before its sole FSTP32 pen store, with no intermediate float/double spill. Other modes keep initial+0. All22 x87 opcode forms/counts match; their dynamic operand/spill paths were separately compared. This is static Source intent, not generated-code, unmasked-fault/status or Native private-frame proof. Later provider CW/status changes remain observable and are not reset.

## Quad arguments and existing providers

One retained float3 supplies homes1/4/5. Y/Z initialize once; X is copied from a separate pen only after all ten argument words are populated, immediately before quad. The homes remain mutable, including0/2/7. The caller keeps glyph, captured vertex stream and saved Font height independently, rebuilds argument homes for each glyph and never recovers parent state from callee-overwritten homes. Only a complete genuine quad return advances its loop.

The first emitted glyph supplies texture0 and material from the current section; after that callback, glyph+1C and section/material are reread for texture1. Existing raw unchecked B189F0 and SAME actual-owner registry are reused. B73260 reads the actual Mesh's raw vertex slot without a typed Mesh cast. Known B49980/B49B60 maps and B49C70/B49A80 unmaps use existing genuine mapping contexts; other captured current targets require explicit real Source providers. Index unmaps before vertex; then final CURRENT section and captured Mesh reach required B865A0.

Whole raw B74640, AD4480, AB98F0, other reached stream targets, B865A0 and actual F8BE50-role binding remain explicit provider cuts. The loader-zero preimage of F8BE50 is not a new local zero array or enduring-empty policy. No fake glyph/font/model payload, typed reinterpret, successful fallback or dormant bank appears.

## Failure and exact evidence audit

The default metadata destructor does no work. Entered failure retains mappings/partial geometry/argument backing under the caller contract; catch records the phase/exception and propagates. There is no automatic unlock, compensation, CW restoration, retry, rollback or process terminal policy. Existing provider-domain/fault qualifications remain inherited.

Both Source SHA/OID/LF-byte pins, all27 input pins and both raw saved-export SHA/length pins match. Installed PE SHA, the660-byte body hash, initialized CEC380/CE3978/D7A280 bytes and actual F8BE50 loader-zero bytes independently match their receipts. The report pins the complete producer evidence blob and exact reviewed Source. Raw export hashes are not silently normalized for line endings.

Only static Source/listing/PE/pin/diff checks were performed. No Source/CMake/ledger edit, Ghidra read/write, build, test, probe, execution or process action occurred. Actual caller/provider integration, wrapped path, glyph recursion, changed Text and activation remain unclosed. New Original/vendor/duplicate-body/CG/ABI/runtime/gameplay/activation credit is zero.
