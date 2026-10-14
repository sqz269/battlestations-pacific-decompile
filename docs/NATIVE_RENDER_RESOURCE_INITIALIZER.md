# Native render-resource normal initializer composition

`initialize_native_render_resources_00b107f0` now runs the existing entry and
ten continuations in native order. Previously callers had to retain and pass
eleven separate acquisition records, and the application still stopped before
the `B107F0` work. The new operation keeps those exact records together without
allocating another native owner or acquiring another resource reference.

The original ABI is ECX = service, three DWORD stack arguments, and `RET 0C`.
This is an explicit C++ interface. It covers resource work through `B13010` and
the existing entry's already-initialized branch. It does not reproduce the
native register/SEH return sequence `[B13010,B13029)` or FH3 unwind handlers.

| Stage | Existing implementation |
| --- | --- |
| B107F0–B109BC | entry, including already-initialized return |
| B109BC–B11599 | targets/passes and actual `+70` producer |
| B11599–B118AF | post configuration |
| B118AF–B11D7E | DOF |
| B11D7E–B11E40 | second bloom |
| B11E40–B11EF4 | distortion |
| B11EF4–B120B9 | passthrough/dust |
| B120B9–B124A1 | `+658` |
| B124A1–B1297A | `+65C` |
| B1297A–B12C9A | `+664` |
| B12C9A–B13010 | wave/clear/focus tail |

Every stage borrows the same continuation context and actual service/argument
identities. Names, companion domains, imports and providers retain the existing
fragment contracts. A used operation cannot be replayed; an exception stops the
chain, preserves the original exception and all entered acquisition records,
and requires process retention. `active_stage()` identifies a stage entry,
not the precise failed CALL. The underlying records preserve their native sites.

`normal_work_complete` means the composed normal resource work returned. It
does not admit arbitrary partial-child profiles or bypass their retirement
contracts. Likewise the existing-init branch is not proof that a previous
interrupted initialization is safe. Keep the operation and borrowed contexts
alive through dependent use and genuine child retirement.

The application parent still must supply the actual shader/effect/material and
distortion/dust provider graph. This packet does not activate the renderer's
currently unbound phase-4 initializer or claim composed native execution,
binary ABI compatibility, full startup equivalence, or gameplay fidelity.
Validation is recorded in `reports/cc12_render_resource_normal_composition.json`.
