# Oct18b nC low rows (segments 60..64)

The exact assignment contains 30 newly defined functions from `cc16-progress/local/codex_new_nC.txt`.
This packet reconstructs 21 game bodies and classifies 9 compiler/STL rows as
`library_instantiation_out_of_scope`; no rows remain open. All descriptive names are hypotheses.

The code reuses the existing `support_seg61_cc13` block-buffer/singleton family,
`cc13_seg49` tickable-entity family, `cc13_left61` single-effect entity family, and
`cc13_seg49_64_78` EventParam/RangeFilter/WarningObject family. New call interfaces extend
those families for explicit external operations. No game host wiring, global remapping,
startup source registration, or unrelated class implementation is changed.

The four singleton deleting destructors clear their own publication global and restore the
base vtable. Three parameter deleting destructors restore their EventParam base identity.
Those seven stateful class bodies are retained. Seven pure `SUB ECX; JMP deleting-dtor`
adjustors and two standard-library instantiations receive no game reconstruction credit.

The wreck update uses the full assembly, not the erroneous apparent arguments in the
pseudocode. Its x87 matrix-row sum remains x87, with the observed float rounding before
adding the stored half-height. Bounds calls, settings reads, counted releases, both spray
updates, debris generation and shape flag clearing preserve the native order. Numeric
constants were checked against live image bytes. External calls remain address-qualified
contracts; this work makes no claim of runtime, visual, gameplay or binary ABI equivalence.

The single-effect update consumes an unused stack argument (`RET 4`). It retains the
definition passed by value, assigns and releases the temporary result, updates the live
matrix, adopts null after completion and kills with reason 2 when no live effect exists.
The ordinary path is reconstructed; native SEH metadata and exception-path equivalence
are not supplied.

The kill-warning duplicate override receives `self` in ECX and `other` on the stack
(`RET 4`). Type 3 stores the entity directly in its +6Ch word; its class at entity+538h
is used only after the kind-6 check. The two always-false warning overrides also use
`RET 4`, although their decompiled prototypes show no argument. The range predicate
uses inclusive, ordered comparisons, so a NaN fails.

`reports/oct18b_nc_low.json` records per-address ABI, evidence and uncertainty, plus 48
exact direct/tail-call sites for `tools/verify_report_calls.py`. Ghidra mutations use the
annotation tool and write lock; that tool verifies the existing bsp project and program,
records previous names/comments, preserves comments and saves before refreshed exports.

Validation: PASS: MSVC Win32 Release scripts/build.ps1; new source explicitly included in bsp_core. PASS: existing CTest suite; no new tests or native/game executions.

Native ABI, fixture, startup and gameplay validation: not established; no game runs.

| Address | Result | Source symbol / classification |
| --- | --- | --- |
| `00924150` | game body | `bsp::support_seg61_cc13::pending_init_lock_scalar_delete_00924150` |
| `009241d0` | game body | `bsp::support_seg61_cc13::pending_entity_lock_scalar_delete_009241d0` |
| `00924250` | game body | `bsp::support_seg61_cc13::deferred_event_lock_scalar_delete_00924250` |
| `0092a010` | out of scope | `CG_DeletingDtorAdjustor_0092a010` |
| `0092b920` | game body | `bsp::cc13_seg49::sinking_wreck_update_0092b920` |
| `00940da0` | game body | `bsp::support_seg61_cc13::sound_request_queue_scalar_delete_00940da0` |
| `0094f700` | game body | `bsp::support_seg61_cc13::block_buffer_append_text_0094f700` |
| `0094f9e0` | game body | `bsp::cc13_left61::single_effect_entity_update_0094f9e0` |
| `00950900` | out of scope | `STL_CheckedVectorIteratorAdd_00950900` |
| `00963be0` | game body | `bsp::cruiser_class_name_00963be0` |
| `00963e10` | game body | `bsp::submarine_class_is_kind_of_00963e10` |
| `00968090` | game body | `bsp::cc13_seg49_64_78::event_param_kind_00968090` |
| `009680f0` | game body | `bsp::cc13_seg49_64_78::event_param_kind_009680f0` |
| `00968130` | game body | `bsp::cc13_seg49_64_78::event_param_kind_00968130` |
| `00968170` | game body | `bsp::cc13_seg49_64_78::event_param_kind_00968170` |
| `009682b0` | game body | `bsp::cc13_seg49_64_78::event_param_scalar_delete_009682b0` |
| `009682d0` | game body | `bsp::cc13_seg49_64_78::event_param_scalar_delete_009682d0` |
| `009682f0` | game body | `bsp::cc13_seg49_64_78::event_param_scalar_delete_009682f0` |
| `00968440` | game body | `bsp::cc13_seg49_64_78::range_filter_matches_00968440` |
| `0096ac60` | game body | `bsp::cc13_seg49_64_78::kill_warning_is_duplicate_0096ac60` |
| `0096b280` | out of scope | `STL_ListDestroy_0096b280` |
| `0096f230` | game body | `bsp::cc13_seg49_64_78::warning_default_lifetime_0096f230` |
| `00973b50` | out of scope | `CG_DeletingDtorAdjustor_00973b50` |
| `00973c60` | out of scope | `CG_DeletingDtorAdjustor_00973c60` |
| `00973cf0` | out of scope | `CG_DeletingDtorAdjustor_00973cf0` |
| `00973dd0` | out of scope | `CG_DeletingDtorAdjustor_00973dd0` |
| `00973e50` | out of scope | `CG_DeletingDtorAdjustor_00973e50` |
| `00973ed0` | game body | `bsp::cc13_seg49_64_78::confirm_warning_is_duplicate_00973ed0` |
| `00973ee0` | out of scope | `CG_DeletingDtorAdjustor_00973ee0` |
| `00976290` | game body | `bsp::cc13_seg49_64_78::string_list_warning_is_duplicate_00976290` |

Ghidra closeout: all 30 reviewed names/comments applied and saved; prior names and comments
are archived in the JSON report. All 30 affected exports were refreshed. The final ledger
diff contains exactly 30 name rows and 30 function rows, all within the assigned addresses.
