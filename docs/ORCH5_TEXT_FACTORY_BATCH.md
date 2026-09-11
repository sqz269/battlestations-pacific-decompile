# Canonical Text factory and glyph-child integration

This batch connects the actual Text factory, Model/mesh cloning and optional
glyph-child creation through the existing canonical widget and resource owners.
Four new sources are registered in the normal Win32 `bsp_core` build:
`native_mesh_clone.cpp`, `gui_widget_attach.cpp`,
`gui_text_runtime_factory.cpp` and `gui_text_glyph_child_runtime.cpp`.
The content driver composes them with the existing real builders and submission
continuations; it does not invent an executable host or a rendered menu.

The Text default factory associates one opaque pool allocation with one existing
widget owner and one Text lifetime. Actual type hooks include properties,
construction/loading, clipping, alpha and full bounds/height behavior. Pending
content and child clipping retain their exact continuation. Flags0 deletion
retains raw allocation ownership until explicit return; pending scalar deletion
is rejected before native effects. Native allocation-before-base callback order,
copy construction and raw object/vtable/SEH ABI remain outside this C++ interface.

The flags26h/parent0 Model clone allocates from the actual Model pool, reads the
current source name after allocation, registers the destination in the same
runtime, copies the supported base state, clones actual mesh/section/material
owners and performs late geometry/retained-owner/pose writes. Partial creator
references are retained at explicit boundaries. The mesh route includes physical
LOD arrays, streams, section copy construction, actual material cloning and
pooled bone names. Positive actual point-light arrays remain a boundary.

The `AB98F0` child tail starts after the existing mapped glyph prefix. It creates
the unbound Text, publishes the exact child, clones and transfers its Model,
sets live font/shadow/shader state, uses an actual pooled UTF16 header, completes
recursive content and final color, then attaches, pivots and positions the child.
Same-frame guards prevent callback reentry from repeating cleanup or entering
moved builders. Missing services and unsupported wrapped format/alignment stay
pending. The explicit service accessor keeps locale comparison adapters and
the concrete child-deletion owner in the same actual deletion domain.

Independent reviews found and corrected an unconditional x87-half load, nullable
parent-node handling, deletion-service identity, pending scalar-deletion order
and reentrant UTF16 cleanup. The full bounds logic preserves the assembly's
unusual horizontal/top/bottom versus vertical/center conditions and float spills.
The reviews do not constitute runtime or rendering evidence.

The combined Release MSVC Win32 build passes both existing tests. Seven reports
contain 72 numeric call rows with zero failures: 65 direct rows and seven resolved
indirect rows whose targets require separate profile evidence. These counts
include repeated sites across reports. A focused original-byte `B85EF0` probe
matches all 100 destination-slot bytes, retained counts and x87 status, including
signaling-NaN quieting; subsequent host destruction balances references. Its
481 original bytes match the installed PE and use private constant/import
relocations. This test does not execute the native glyph tail or game callbacks.
No permanent tests were added.

Fifteen reviewed names/comments were applied under the Ghidra write lock and
the project saved. Before-values are archived in
`reports/orch5_text_factory_annotations.json`; affected exports are refreshed.
Details, source hashes and verification logs are recorded in
`reports/orch5_text_factory_batch.json`.

## Follow-up packets

- Recover positive point-light-array ownership for Model base copying before
  broadening the explicit flags26h clone domain; coordinate `B6F150` leases.
- Retain the outer page-loader frame across property suspension before treating
  all Text load paths as complete; coordinate existing GUI loader ownership.
- Supply and validate executable factory/font/material/mapping services and
  capture real Text rendering. Build and section-copy results are separate from
  menu, visual, native ABI and gameplay validation.
- Recover nonzero timed-entry deletion and non-Text current70 profiles only
  after inspecting their actual owners and call sites. Existing empty-header
  and unsupported-profile boundaries remain explicit.
