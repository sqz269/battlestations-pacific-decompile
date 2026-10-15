# Loaded sampler provider diagnosis

The real cached GuiFont path rejected the installed Win32 provider revisions.
Its visible admission metadata was valid, but D3D9, NTDLL and Kernel32 all had
different PE identities from the previously supported triplet. The private
compiler frame, compiler object and complete 96-byte bridge were unchanged.

Source `a893f091c4b1b3c61acb8d493e66399cde537e54` adds failure-only loaded module
headers/paths, export allocation identities and existing admission metadata.
The canonical Win32 build passes all three existing checks in 7.10 seconds.
A fresh windowed GUI attempt reaches the same unknown `B-16` guard and exits 1,
preserving the native graph through process exit.

The recorded flags include nonnull context/calls/owner/shader, registered pass
and shader, variant/source modes zero and the same cache. These observations do
not independently prove every private-object predicate. The DLL identity
mismatch is itself sufficient to reject the legacy qualification.

The loader reports System32 paths in the Win32 process. Passive file analysis
selects the SysWOW64 files whose I386 machine, timestamp and image size match
the loaded headers; it does not use the 64-bit files reached by a 64-bit path
reader. The in-memory optional-header ImageBase observation is kept distinct
from the on-disk preferred base used to qualify HIGHLOW relocations.

The subsequent additional profile and successful runtime cases are documented
in [CC12_SAMPLER_PROVIDER_SUPPORT_29.md](CC12_SAMPLER_PROVIDER_SUPPORT_29.md).
The diagnosis and rejected attempt remain separate evidence in
[`cc12_sampler_entry_profile_diagnostics_28.json`](../reports/cc12_sampler_entry_profile_diagnostics_28.json).
No module, bridge, caller word, reference count or Original installation was
patched to make that first attempt pass.
