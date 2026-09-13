# Actual material-program child composition

Addresses: 00B43B00, 00BDF4C0, 00B26500, 00B265C0, 00B26680, 00B3C3A0

`NativeMaterialEffectNativeChildren` supplies the actual descriptor reader,
VFS resolver and three state caches required by the existing B45EE0/B46950
program loader and B5F6A0 finalizer. This is host composition of separately
reconstructed bodies, not six newly recovered functions or a new native ABI.

The constructor checks the supplied effect lifetime, descriptor strings, VFS
strings and state-cache canonical registry share their domains. Use that same
effect lifetime in the enclosing `NativeMaterialEffectProgramsContext`; all
contexts must remain alive for retained failed operations. The individual
implementations retain their further runtime and profile checks.

Each concrete call requires an empty child slot, creates stable operation
storage and publishes it before any native allocation, lookup or ownership
change. Successful child destruction is metadata cleanup. A failed child stays
with its parent; it is not replaced, replayed or rolled back. The existing
program loader already stores mode and shadow names in its persistent frame.
Descriptor reads use the original caller's stable descriptor and name addresses.

`NativeMaterialEffectCompiler` remains a required B3C3A0 implementation. It
must publish its own acquisitions, return a real canonical held pass or the
original compiler-null result, and preserve the existing child contract. No
default success, semantic pass conversion or unavailable-child null is supplied.

Evidence comes from `NATIVE_SHADER_DESCRIPTOR_READER.md`,
`NATIVE_VFS_NAME_RESOLUTION.md`, `NATIVE_MATERIAL_STATE_CACHE.md` and the existing
`NATIVE_MATERIAL_EFFECT_PROGRAMS.md`. Individual original/source and installed
asset fixtures do not establish a complete cold effect load through this
adapter. Source registration/build status is recorded separately in the report.
