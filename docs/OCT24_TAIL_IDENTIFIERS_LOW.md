# Oct24 low-address identifier audit

This lane audits the original 513 `unknown_no_identifier` rows below `00800000` whose recorded source does not contain `/game_`. It adds no reconstructed C++ bodies and changes no source/header files.

The final `config/wire_identifiers_low.json` contains 152 address-to-body mappings. Every record names an executable definition and carries source/address evidence. Fourteen deliberate aliases to functions bearing another address suffix have `shared_implementation: true` and explain the shared behavior. The overloaded `004f8f20` method records its exact `(bool, bool, GuiWidgetOwner&)` signature and the limits of simple-name use evidence. The destructor mapping uses `~LuaMachine`; implicit destructor reachability remains conservative.

The 322 ledger corrections preserve the original evidence and prior status:

- 316 use `metadata_only_no_function_body_verified_oct24`. These include trivial-slot/class tables, aggregate/default/layout/schema records, documentation-only addresses, and fragments embedded in larger routines. Their evidence distinguishes existing partial semantic code from a complete callable body.
- Six use `library_instantiation_out_of_scope` for stock MSVC STL template bodies. Standard-library bindings do not receive game reconstruction credit.

The 39 untouched rows are accounted for as follows:

- 25 already carry library/vendor/standard-library reuse statuses and require the parent reporting filters.
- 11 have existing naked assembly or constructor initializer-list bodies and require the parent parser fixes.
- Three locale-node-pool addresses (`00435a50`, `004360e0`, `00436230`) belong to the separate pool audit.

The parent integrates these mappings and filters, rebuilds the index and runs the whole readiness/board reports. This lane's counts describe its input-row accounting; they are not a final reachability result.

Validation: all 152 mappings match the parent's updated function-definition parser; all 14 suffixed aliases have explicit shared-body flags; addresses are unique and disjoint from the 322 corrections; all 322 exact ledger replacements were checked against the written shards. The index was refreshed after the edits. No build or game run was needed for these metadata changes. Body inspection and prior evidence do not establish ABI compatibility, runtime parity or gameplay validation. In particular, movie-action/keyframe/visual-class semantic models retain their documented raw vtable/layout omissions.

Commits: `462779fc2` introduced the first 82 mappings; `b474df7b1` corrected the first 188 table-only claims. The final batch adds 70 mappings, shared-alias flags and 134 further ledger corrections.
