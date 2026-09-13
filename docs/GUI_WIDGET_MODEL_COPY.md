# Canonical widget Model copy binding

`GuiWidgetModelCopyRuntime` connects AA9520's actual Model current10 call at
`00AA96FC` to the reconstructed B752B0 composition. It forwards the live flags
read by the caller: `0x3E` copies index and vertex streams; `0x26` preserves the
existing shared-stream path. Both established routes use parent zero.

The adapter borrows the same widget, Model, mesh, material, stream and canonical
reference-owner domains. It returns the acquired Model creator for the caller's
existing transfer into the destination widget. It supplies no alternate pool,
render device or retained-object registry, and never substitutes flags26 after
an interrupted flags3E operation. Partial creator and mapping state stays with
the constructor operation.

This is a new C++ binding for the established native caller, not a recovered
symbol or original ABI replacement. Complete copied Text execution additionally
requires a real retained Text identity for cursor material parameters, concrete
renderer/declaration/material/layout services, and the retained derived copy
continuation. The raw Text pool allocation remains opaque transport until its
actual native identity/lifetime producer is established. Build and focused
validation are recorded separately in the batch report.
