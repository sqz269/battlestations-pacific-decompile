# Oct24 GUI identifier/body audit

This follow-up audits all 87 original unknown rows recorded in `src/gui_widgets_cc13.cpp` (50), `src/native_gui_widget_pools_cc13.cpp` (14), and `src/engine_seg49_64_78_cc13.cpp` (23) assigned by the parent after the low-address batch.

`config/wire_identifiers_gui.json` adds 70 executable-body mappings. It covers current type-id/name/lineage operations, guarded type initialization, widget factories and scalar deletion, fixed-pool release/allocation stubs, the shared sentinel shape, singleton clearing and the rain-pool free thunk. Each mapping carries the table/header/address association and the actual implementation file. Two deliberately shared suffixed aliases have explicit flags. Type-initializer return variants preserve their documented interface qualification: the generic initializer exposes the target through its reference argument, without reproducing native EAX return/stack ABI. Generic type predicates have overloads, so simple-name reachability remains approximate.

Seventeen records had no qualifying game body and were corrected through the ledger tools, with original evidence and status retained:

- Two are stock vector size/resize instantiations and now use `library_instantiation_out_of_scope`.
- Fifteen use `metadata_only_no_function_body_verified_oct24`: twelve subscription aggregate/default lifetimes; the implicit `WarningEventRecord` destructor; the rain-model scalar deleting wrapper whose conditional pool-free operation is missing; and the Section first-vertex-stream wrapper that delegates the entire traversal to a required host contract.

Existing partial/typed models remain documented. No table accessor or unimplemented host callback is promoted to a whole native function body. No C++/header/Ghidra mutation or game run was made.

Validation: all 70 mappings match the parent's updated executable-definition parser; both suffixed aliases are explicit; all 17 exact ledger replacements match the written shards; the mapping/correction address sets are unique and disjoint and account for all 87 input rows. The local index was refreshed after these changes. Whole-readiness and board counts are the parent's integration checks. This audit does not establish native ABI compatibility or runtime/game parity.
