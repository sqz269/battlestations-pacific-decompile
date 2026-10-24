# Oct24 primary model identifier audit

This packet audits 28 currently unknown rows from the parent's 499-unknown report: 13 in `scoring_bindings.cpp`, 12 in `engine_seg56_cc13.cpp`, and three in `post_effects_cc13.cpp`.

`config/wire_identifiers_primary_models.json` adds 25 executable-model mappings:

- Thirteen scoring rows use explicit `MissionScoreMap` selectors. Five getters select Mission/Action/Ship/Plane/Command; six ship/plane/command setters/adders preserve cache invalidation; the ActionMedals setter preserves its lack of cache invalidation. The `008bb530` ActionMedal binding is an explicit shared implementation of `lua_binding_scoring_set_medal_008b97f0`, with `shared_implementation: true` and `MissionScoreMap::ActionMedals` recorded.
- Nine engine rows identify the existing Effect, moving-camera pose, scripted input, active-dialog IDs, navigation-point initialization and objective models. The objective message model remains a header-only projection: native `0075B430(id)` base construction is outside its body. The mapping identifies the existing model and does not upgrade its coverage.
- Three post-effect rows identify named-index search and `MitBlockPool` allocate/free. The pool bodies model slot order only; slab memory, pointer decoding, allocator registration and locking remain outside this interface, and locking belongs to the caller.

The `PerfTimingRegistry` constructor (`008d1a10`), destructor (`008d1ab0`) and singleton getter (`008d1b80`) have only a header lifetime contract and implicit map-member initialization/destruction. The source defines start/end/query methods but no native lifetime bodies. Their three function records now use `metadata_only_no_function_body_verified_oct24`, preserving previous status, coverage and original evidence. All edits went through the ledger CLI with address leases.

Validation: 25 names match executable definitions using the parent's latest parser; the single different-address suffix alias is explicit; all three exact ledger replacements match the written shard; mapping/correction address sets are unique, disjoint and cover all 28 inputs; no new mapping overlaps the parent's existing mapping files. The index refresh was run after the ledger edits. No source/header changes, Ghidra mutation, build or game run was needed. These are model-identity and metadata checks, not native ABI or runtime/gameplay proof.
