# Native material and cache admission (CL)

Addresses: 00b18d60, 00b407a0, 00535320, 00b2ebb0. Compiler support:
00cbc570, 00cbc578, 00cbc583, 00cbc591, 00cbc599, 00c6c240, 00c6c248,
00cbd740, 00cbd748, 00cbd750, 00cbd758, 00cbd760, 00cbd76b, 00cbd773,
00cbd77b, 00cbd783, 00cbd78b.

CL connects the existing effect constructors and material factory to the actual
numeric texture/effect caches. The four ordinary bodies were already reconstructed;
this packet adds composition and canonical material admission. Evidence and every
direct/indirect transfer are recorded in `reports/native_material_cache_admission_cl.json`.

## Behavior and ownership

The additional B18D60/B407A0 interfaces capture the current renderer and dispatch its
numeric D5F0A8/slot64 B319B0 through `NativeTextureCacheContext`. Construction and
cache contexts must borrow the same actual string adapter and renderer publication.
Texture and effect owners must use the same canonical lifetime domain. Each call
has a fresh retained texture-acquisition frame. The returned fallback reference
transfers directly to effect+98. The temporary `error.tga` name is returned before
sampling the full DWORD serial; serial publication wraps modulo 2^32.

B18D60 exception states own only completed names and reference-base cleanup. They
do not release fallback+98 or acquired array owners. The derived constructor sets
descriptor+C4, retained+138, byte+13C, profile D61A00 and fourteen words+140, but
**does not initialize C8..137**. Existing callable interfaces remain available to
callers with actual relocated virtual code.

B2EBB0 now retains a separate constructor texture frame alongside its persistent
program and recursive fallback frames. It allocates host continuation metadata
before native storage. Constructor failure frees only the raw178h allocation;
failed child/provider state remains inspectable in the retained caller frame.
Successful construction still calls B46950, assigns the original name and admits
the completed creator under the existing program ownership contract. Failed frames
are not retryable and must remain alive; no implicit native rollback is added.

The additional 535320 interface executes actual renderer48 B318B0, B18780 material
allocation and B18900 construction. The cache and material owners must be identical.
Its sole native exception state returns a raw material slot after constructor
failure; it does not release the temporary effect on that exception path. Normal
completion releases the temporary effect after construction (or a null slot result).
The call sites are 535345, 53534E, 535364 and 535377; zero-reference terminal dispatch
is 535387. The per-call frame records outstanding native acquisitions.

`GuiNativeGeometryOwners::register_native_material_creator` admits a completed
factory result without retaining it. Failure while preparing host metadata leaves
the native creator unchanged. A transactional canonical bind failure preserves the
stable unbound companion and metadata entry in the acquired frame. Explicit later
retirement can release it. Frame identity fields are inspection values and may
become stale after external retirement; they must not be reused as new creators.

## Evidence and validation

Saved Ghidra `/battlestationspacific.exe` and the installed PE agree across 1,353
ordinary bytes and all compiler supports. The three FH3 maps confirm base-name
cleanup, the factory raw-slot-only unwind, and the loader's raw allocation state4.
Three missing dispatch handlers were defined. CBD760's returning-free continuation
was restored without clearing bytes or changing global no-return annotations.
Existing function names and comments are preserved and extended; names remain
descriptive hypotheses. The report distinguishes each original ABI from these
new C++ interfaces.

The tracked MSVC Win32 build and both existing CTests pass. One ignored focused
probe composes an actual RTX5090 D3D9 texture, native string/material pools, numeric
texture and effect cache hits, and the shared canonical owner registry. It checks:

- Base/derived constructor fallback transfers, serial wrap, and poison preservation
  in unwritten fields. The derived test explicitly retires only its constructed
  base; it never invents valid pass pointers or claims a complete shader effect.
- Material creation from a real constructed base effect in an explicitly seeded
  effect-cache alias, including material+7C, effect dirty+B4 and exact references.
- Normal canonical admission and an injected transactional bind exception. Failure
  retains material and effect references until explicit fixture retirement.
- Returned material slots, empty canonical registry/allocator list/texture cache,
  and final D3D9 device/API reference counts of zero.

Frozen source, libraries, probe artifacts, build logs, byte audits and hashes live
under ignored `local/native_material_cache_admission_cl/registered/`; the tracked
integration report records their receipts. No new CMake entry is needed and no
lease is taken on `cmake/startup.cmake`.

## Limits and follow-up packets

The hot effect-cache fixture uses a fully constructed base effect, not a synthetic
successful B46950 output. B2EBB0's new constructor wiring is build-checked only.
Actual descriptor B43B00, program compiler B3C3A0, state/pass cache children and
their lifetime composition remain prerequisites for cold effect loading. Unused
fixture providers throw if reached; none supplies fake successful native storage.
This packet does not establish original full-parent execution, binary ABI/FH3/SEH
equivalence, complete material/subset parsing, render parity or gameplay validation.

Next work must audit the complete B941D0 material/subset parent and its cleanup,
then B944E0/B94710 and the registered parser wrappers after their dependencies.
The agent branch remains independent of unreviewed moving `main` deltas.
