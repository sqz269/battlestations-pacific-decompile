cc13-stlreview1 pipeline (docs/CC13_STL_PROBABLE_REVIEW.md). Run from the repo root, in order:
  1. stlr_feat.py      index features for every stl_probable row -> local/stlr_feat.json
  2. stlr_refs.py      Capstone absolute-reference census over the disk bytes
  3. stlr_bucket.py    first-pass buckets (named_tmpl / named_game / raw_*)
  4. stlr_decomp.py raw_pure,raw_mixed,named_tmpl,named_game,raw_game 600   (read-only live Ghidra, local/stlr_decomp/)
  5. stlr_norm.py      normalised-pseudocode clusters; labels in stlr_labels.py + stlr_labels_extra.txt
  6. stlr_final.py     decisions; 7. stlr_export.py -> tools/template_shapes/stlr_decisions.txt
  8. tools/template_shapes/stlr_review.py [band prefixes]   (writes config/tags, local/stlr_ghidra.json)
  9. stlr_ghidra_apply.py   (Ghidra renames + bookmark deletes; takes the write lock)
stlr_fix_assign.py was the one-off STL_Tree_Assign -> STL_Map_Subscript correction. stlr_view.py prints a saved decompile.
LEASES: stlr_export.py hard-codes the ranges leased to other sweeps on 2026-10-12 (EXCL, SINGLE); edit them before re-running.
cc13-stlreview2 (doc section 9): after steps 1-5 and stlr_final.py, run stlr2_final.py (read-based overrides, EXCL = sweep67 lease only) -> tools/template_shapes/stlr2_decisions.txt, then stlr_review.py --decisions tools/template_shapes/stlr2_decisions.txt --source docs/CC13_STL_PROBABLE_REVIEW.md, then stlr_ghidra_apply.py.
stlr2_final.py --s67 writes tools/template_shapes/stlr3_decisions.txt for the rows cc13-sweep67 released (local/s2_s67.txt lists them).
