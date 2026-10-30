# Actual single-line Font selector composition

This packet starts at sealed `1a35a95f8fe44b0fe1c3f0324462abd7809fc9f8` and changes only the AB9FD0 parent Source pair and its four evidence files. The whole-parent Source `0bc258c71f44b0729da801f3aa9b598856d2276f` and independent review `f50237488def3c94c54c4cde97a63e2916176f58` retain exact historical Git-LF pins. That review applies to0bc; Root reviews this later composition separately.

Both mandatory Font virtual calls are replaced by direct `select_native_font_glyph_storage_00ad4480`. The unused Font virtual is removed from `NativeGuiTextSingleLineStorageBindings`; scoped exact-word searches of include/src/tests found no actual implementations or derived binding classes. No application, owner, adapter, default Font or glyph is introduced.

## Actual operand and lifetime

At measurement A0CA and emission A17B, the parent first captures CURRENT Text+108 Font, matching Native A0C3/A174, then stores the current zero-extended code unit's lowWORD in a genuine volatile `uint16_t` object before the direct call. Native reads no upperWORD. No typed DWORD is reinterpreted as a WORD and no unsigned UTF16 input is excluded.

`font_code_unit_home` and `NativeFontGlyphSelectionStorageScratch font_glyph` are appended to the existing nonmoving caller scratch without initializers or resource effects. Every old scratch field precedes them. The same actual WORD object reaches unchanged AD4480 and raw AD4370; its current reads remain possible after callback mutation. The selector's initial dispatch key does not replace that home during lookup.

Normal return permits the next reached selector call to overwrite the home and iterator output. Embedded SPACE/LF/CR arms do not read/write the iterator. A failed entered call retains the parent frame/home/iterator and genuine graph through the existing failure mechanism, and performs no advance, automatic cleanup, retry, rollback, retain or termination. Default destruction remains unchanged.

The parent's cursor/code-unit state remains separate from the selector home. Captured glyph, height, geometry counters, post-texture UTF16 reload and all ten quad homes retain their original schedule. Selector-home mutation is not copied back into those operands. Actual raw Font/head/node/pair/glyph storage must survive every reached current read, callback and failure.

## Concrete coupled Source

The unchanged sealed1a35 provider composes whole AD4480, finite private AD3E70, existing raw AD4370/AD4540 and the selected real SDK BF6713 transport. Embedded Font+4C/+6C/+8C returns and a NULL pair+4 payload propagate unchanged. Its existing SDK-handler/Watson/private-frame qualifications remain; this packet introduces no handler, no-return assumption, catch, FP environment policy or terminal action.

The [composition report](../reports/cc16_native_single_line_font_selector_composition.json) pins both current parent Source files, six unchanged concrete provider Source files, both sealed provider evidence files, all27 original parent inputs, six shared raw exports and the historical0bc/f502 receipts. Original parent inputs were rechecked against current files, and all15 Font input pins against sealed1a35; the old parent pair among those15 is intentionally superseded by the two current parent pins. The six concrete provider Source files remain identical to1a35. No live Ghidra analysis/export was needed.

An exact canonical Git-LF comparison permits only the two C++ call-site substitutions. Every other parent C++ byte is identical to0bc, including all five inline-assembly blocks, float spills, ambient-CW/status schedule, geometry, mapping, reloads, loops and quad call. The header permits only the concrete-selector include, unused virtual removal and appended home/scratch fields.

Static hash/diff and Source-schedule checks are the new evidence. No build, test, probe, game/process action, CMake/ledger edit or Ghidra write occurred. Root owns the joined MSVC Win32 build; earlier build/review receipts do not validate this later Source. Original/vendor/helper/duplicate-body, ABI, runtime, activation and gameplay credit are zero.

Actual populated Font and glyph production/lifetime, Model B74640, Quad AB98F0 binding, section-layout B865A0, F8BE50/current stream targets and the actual parent caller remain required. Wrapped ABA270, glyph recursion, whole ABA8D0, changed ABAED0 and frontend readiness/activation gates remain false.
