# Canonical GUI relative bounds and current58

Addresses: 00AC06C0,00AC0820,00580820,00AA7970,00ACF120,00ACEB30,00AA8240.

The relative-bounds operations use the existing GUI owner, Text lifetime, FrameBox state and native Model node. They preserve the native order around callbacks and explicit x87 float stores. Names are descriptive hypotheses; these C++ interfaces are not binary replacements.

AC06C0 takes ECX source, EDX size output and offset, padding and width on the stack (RET0C). It captures both source-size sums before output stores and preserves padding/output alias effects. For actual type3 Text it reads the same Text lifetime, uses the ordered zero predicate to select measured width, and calls AB6BD0 for height. The native vertical non-center branch tests the saved horizontal alignment for2; that apparent bug is preserved. Constants are required volatile aliases.

AC0820 takes ECX source, EDX destination and stack width (RET4). It spills a copy of width through x87 before measurement, while retaining the original value for the later comparison and MOVSS override. It invokes actual current58, then reloads both owners' sizes, pivots and source position. Four products, two differences and two offset sums keep the native float stores. Existing AA8240 inverse-parent arithmetic publishes through the same owner recomposition and bounds update.

Current58 uses verified tables: Group, Screen and Listbox reach AA7970; Icon reaches AB1EF0; Text reaches ABBF30; FrameBox reaches ACF120. AA7970 stores two lanes sequentially and recomposes without a bounds update. FrameBox performs base58 first, reloads stateEC, copies the borrowed size into actual live records when state is not-1, then invokes current88 ACEB30. Its verified19-byte body zero-extends stateEC and invokes current80 AD0D80. Section's distinct ABE670 is not treated as the base operation.

580820 takes ECX screen and the selected objective stack pointer (RET4). It reloads highlight294 after visibility and fitting, reads width and height separately, adds live CEE4E8 to height and dispatches current58 on the fresh highlight owner.

The focused local probe compares the original installed/live-matched AC06C0 non-Text branch against actual Group owners:30,720 bit comparisons across12 x87 precision/rounding controls and five alias modes. It also checks same-owner fitting, current58 callback reloads, highlight size and scalarAC storage. The callback check injects an observer at the documented attached+A0 notification boundary of the actual node; it does not validate an installed attachment implementation. No permanent tests were added.

Text/FrameBox resource effects, full menu execution, signaling-NaN parity in inherited resolved_position and original ABI remain unvalidated. Pending Text raises the existing exception and stops placement; resuming Text alone does not resume the outer operation. All borrowed owners, state records, input storage and type profiles must survive callbacks. See reports/gui_widget_relative_bounds.json and reports/orch5_menu_size_dispatch.json for evidence and detailed scope.
