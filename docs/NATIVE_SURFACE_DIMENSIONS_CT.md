# Public raw D619A0 surface dimension leaves

The installed `battlestationspacific.exe` SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Live `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr` and the PE
agree on both complete bodies: `00B3CD10..00B3CD13` is `8B 41 1C C3`
(width) and `00B3CD20..00B3CD23` is `8B 41 20 C3` (height). Each takes the
actual surface address in ECX, returns the **current unsigned DWORD** in EAX,
and uses plain `RET`. The new one-argument Win32 fastcall interfaces keep
that physical register and stack behavior; no bounds, null, profile or value
repair is added. The actual `NativeSurfaceOwnerStorage` layout puts width at
`+1Ch` and height at `+20h`, distinct from its COM surface at `+2Ch`.

The immutable concrete D619A0 profile stores `00B3CD10` in slot `+1Ch`
(`00D619BC`) and `00B3CD20` in slot `+20h` (`00D619C0`). Ghidra reports
these two data references and **zero direct callers** for each leaf. The
post-effect submission path calls these slots indirectly on the first and
second current surface respectively; its composition and final pass are
separate work. This packet does not edit or implement that parent or any
leased pass/draw provider.

Both addresses already have full-function reconstruction records sourced
from private `surface_width_00b3cd10` and `surface_height_00b3cd20` helpers
inside `native_post_effect_construction.cpp`; the analogous 20h constructor
also contains private helpers. Those records and their prior body/byte credit
remain intact. This packet only exposes independent callable raw leaves for
composition and claims **zero new body/byte credit**.

`reports/native_surface_dimensions_ct.json` pins live/PE bytes and hashes,
vtable cells, complete direct-caller evidence, source/provider hashes, and
generated Win32 COMDAT bytes/relocations. Profile validity is a caller
precondition; these leaves themselves read unchecked actual storage. An
identical object body establishes source/ABI evidence for these leaves, not
whole-program binary replacement, material-pass execution, GPU output, or
gameplay validation.

`./scripts/build.ps1` completed the Win32 Release build and its existing
`reconstructed_math` CTest (1/1). Dumpbin and direct COFF reads show two
four-byte COMDATs identical to the PE bodies, each with zero relocations.

## Primary integration validation

Exact source `3a92529df59b7dbfa8ae1f6e7a82f648570f323c` passed MSVC Win32 Release and both existing math CTests with 2601 unchanged tracked inputs and a clean tree before/after. Both complete4-byte COMDATs equal installed/live body and worker objects, zero relocations: MOV EAX,[ECX+1Ch/20h]; RET. Public raw wrappers preserve actual pointer storage and original register/stack behavior. No additional runtime probe was needed for these exact native leaves. Existing full-function body credit remains unchanged.
