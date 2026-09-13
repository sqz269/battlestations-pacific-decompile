# Actual surface string domain

Addresses: `00B3F630`, `00B3F4E0`, `00B3F5B0`. Descriptive symbols remain hypotheses. Original bodies and FH3 evidence are recorded in `docs/NATIVE_SURFACE_OWNER.md` and `reports/native_surface_owner_audit.json`.

`NativeSurfaceOwnerContext` can now borrow the actual native string-pool storage through `NativeSurfaceStringPool`. Existing semantic context initializers remain supported, while actual texture composition supplies the same `ActualNativeStringPoolStorage` used for texture names and the renderer cache. The adapter owns no pool, header, reference count or native allocation.

Surface constructor diagnostics, constructor unwind, destructor names and destructor unwind all forward allocation/release into the selected domain. Release preserves the native header words. The COM, renderer unregister, resource-support, tracking-counter and raw surface-pool ordering is unchanged. `actual_storage()` exposes the borrowed actual provider for identity checks.

This changes a host context field, not the `34h` native object layout or original ABI. `B3F630` uses ECX raw slot, stacked COM/flags/kind, EAX owner and RET0Ch; `B3F4E0` uses ECX owner and RET; `B3F5B0` takes flags and returns the original owner with RET4.

The Win32 build and both existing CTests passed. This is compilation and existing-check evidence for the adapter; actual pool surface lifetime behavior is to be exercised by the composed resource path. It does not establish full texture loading, surface rendering or gameplay.
