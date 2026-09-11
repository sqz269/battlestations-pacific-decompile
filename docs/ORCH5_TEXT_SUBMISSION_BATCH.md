# Canonical Text submissions and remaining prerequisites

This batch registers seven sources in the normal Win32 bsp_core build. Source
submission now uses the existing canonical content, font and material owners,
retains outer strings across pending glyph work, and runs the final current
color operation only after the inner content completes. Size-triggered rebuild
preserves native pair-copy aliasing and callback order.

The batch also adds main/shadow clipping refresh, resource-name invalidation,
the single-code-unit native string producer, raw Text pool ownership, the
derived property-reader continuation, and canonical glyph references with the
observed offset producers and final positioning fragment. Base/child property
traversal remains a prerequisite; it is not repeated by the derived reader.
Full glyph-callee inspection establishes that stack arguments5/6 are unused;
the stable argument2 position survives through its late x read.

The pool uses one actual slab table/free stack, allocator-list membership and
Win32 critical section. Raw slots remain distinct from canonical C++ widget
owners. The focused existing probe passes 65 allocations, empty-slab trimming,
occupied-slab relocation, payload preservation, hidden slot identity repair,
LIFO reuse and explicit destruction. It was linked against this combined
worktree build; it does not execute the original game allocator.

Timed-entry storage is documented as backing pointer, signed count and signed
capacity. Its nonzero ownership path remains unimplemented. The original raw
delete profile and pool startup initializer are now defined in Ghidra. Seven
false-free continuation gaps were repaired locally: one in timed reserve and
six across five pool functions. All report zero remaining gaps after calls;
alignment gaps after jumps were left alone. No global no-return flag changed.
Thirty-one reviewed names/comments were saved, with old values archived in
reports/orch5_text_submission_annotations.json; affected exports were refreshed.

The normal Release MSVC Win32 build and both existing math tests pass. Eight
reports contain 526 numeric call rows with zero failures; the JSON separates
direct calls from resolved indirect targets and symbolic visitor/profile rows.
The source-submission caller audit includes 313 function-attributed call sites
and separately records two raw unassigned calls. These are evidence checks,
not claims that the callers are reconstructed. Independent cross-reviews of
the property and reference packets found no actionable issues in their stated
domains. No permanent test suite was added.

The complete Text factory, remaining type/current70 wiring, earlier optional
child construction/Model clone/content steps, nonzero timed entries, original
string/Lua/SEH ABI and game execution remain unfinished. Existing actual
material-clone behavior is available; composing the full mesh clone and its
canonical owner registration is still required. Build and pool-probe results
do not establish rendered Text or gameplay correctness.
