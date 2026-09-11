# Actual Text content composition

Address: `00ABA8D0`, `BSP_TextContext_UpdateUtf16Geometry` (descriptive hypothesis).

`build_gui_text_content_00aba8d0` joins the existing actual-owner content stages
to the actual single-line and wrapped builders. It keeps the native temporary,
captured main section/material and saved shader-selection byte across the
selected call at `00ABAB6A` or `00ABAB78`. It reads the saved selection rather
than rereading the live multiline byte after callbacks.

Unchanged and changed-empty inputs complete through the existing prefix.
Nonempty input runs preparation, the selected builder and then the color,
shadow-stream, material, position and parenting tail. The tail runs only when
the selected builder reports completion, including its real stream unlocks.
This entry does not include a subsequent virtual50 from an outer caller.

The outer pending frame owns one separate UTF16 temporary. All canonical Text,
font, mesh, stream, mapping, resource, service and D3DX module lifetimes remain
borrowed. The same service domains are required across all stages. Both builder
frames retain the native float3 argument aliases independently of pen locals;
moving the outer frame preserves those allocations. No parallel widget/resource
hierarchy or success callback is introduced.

Optional glyph children still stop inside `00AB98F0`. A caller may use the
conditional resume entry only after that actual child tail has completed with
the saved arguments. The missing child factory/callee continuation is not
implemented here. Undefined wrapped alignment or vertex-format cases remain
pending and the child resume entry rejects them without consuming the outer
frame. Dropping a pending frame does not unlock its mappings or complete the
operation; the caller must retain it and every borrowed owner.

This is a new C++ interface over the supported valid ownership and allocation
domains described in `GUI_TEXT_CONTENT.md`, `GUI_TEXT_SINGLE_LINE.md`,
`GUI_TEXT_WRAPPED.md`, `NATIVE_VERTEX_POSITION_READ.md` and
`GUI_TEXT_CHILD_LIFETIME.md`. Original native string/pool/SEH ABI, exception
rollback, destructive reentry, factory enablement and gameplay are not claimed.

Evidence: both selected CALL instructions and their native targets are checked
in `reports/gui_text_runtime_content.json`; the complete existing content call
audit remains in `reports/gui_text_content.json`. Batch build and verification
results are recorded in `ORCH5_TEXT_BUILDERS_BATCH.md`.
