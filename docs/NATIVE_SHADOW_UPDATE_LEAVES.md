# Native shadow-update leaves

This source supplies three complete native leaf behaviors, covering 41 original bytes. It adds an explicit current-word getter and two literal MSVC Win32 naked setters. The parent AD7A30 routine and its missing AD7360/AD7440/AD72A0 children remain separate work.

| Address / native range | Coverage and original ABI | Source interface |
|---|---|---|
| 00B7AAE0..00B7AAE5, 6 bytes | Complete: `MOV EAX,[0109019C]; RET`; no receiver or public stack inputs | `get_native_override_context_word_00b7aae0(const volatile uint32_t&) noexcept`; explicit borrowed-cell C++ ABI |
| 00B400C0..00B400CC, 13 bytes | Complete: ECX actual receiver; first public DWORD low byte; RET4 | `set_native_material_effect_byte_00b400c0(void*, void* unused_edx, uint32_t)`; naked fastcall |
| 00B40820..00B40835, 22 bytes | Complete: ECX actual receiver; public DWORD index then binary32 bits; RET8 | `set_native_material_effect_float_bits_00b40820(void*, void* unused_edx, uint32_t index, uint32_t value_bits)`; naked fastcall |

The getter reads the caller's current volatile DWORD exactly once. The existing binding is `LightTypeBootstrapStorage::directional_0109019c.own_id`: `own_id` is the descriptor's first DWORD, and bootstrap writes that same cell. `GameNativeTypeStorage::light_types()` exposes its stable binding. Callers supply that existing cell without initializing it, caching its value, substituting a default or introducing another global. The source getter changes the native no-argument ABI; neither six-byte code identity nor direct original-image binding is claimed. Preserve the established Ghidra name `BSP_Render_GetOverrideContextWord`.

The byte setter loads only AL from `[ESP+4]`, stores AL to `[ECX+13Ch]`, and executes RET4. Upper EAX bits remain untouched by the load. A full public DWORD keeps the native stack contract; narrowing it to bool would incorrectly normalize values. The indexed setter uses EAX for the unchecked index, MOVSS to copy the binary32 bits through XMM0, and native DWORD address arithmetic for `[ECX+EAX*4+140h]`. It performs no x87 arithmetic or float conversion. The public `value_bits` parameter is deliberately a raw DWORD. Both source interfaces reserve unused EDX so the original public arguments retain their stack positions.

These functions borrow valid actual storage. They allocate nothing, retain nothing, invoke no callbacks and add no null or bounds checks. The semantic `MaterialEntryEffect` projection is not used as the native object's backing storage. The field offsets and names describe observed operations; they do not establish a complete effect class layout.

## Native caller evidence

Both setters have only AD7A30 as their direct native caller. At AD7AC8 / AD7AE1 / AD7B09 it writes indices 0 / 6 / 2 using separate reads of current F8C210+8. The caller, not the setter, performs FLD32/FSTP32 conversions; the third value additionally uses FMUL64 half and a binary32 spill/reload. At AD7B16 it passes low byte 1 to B400C0. Preserve that caller arithmetic if the parent is later reconstructed.

The third spill at AD7AFC overwrites the **first camera argument's backing slot**, not the second incoming float. With S=AD7 entry ESP, baseline ESP is S-28; the outstanding PUSH at AD7AEF makes `[ESP+24h]` equal S+4. The captured camera remains in EDI, while the second incoming slot S+8 is never read. EG retains the complete 82-instruction derivation; EI does not implement this parent.

A8F3B0 calls B7AAE0 at A8F81A between capture of the current light's profile and reloading its receiver. The returned current DWORD is passed to the captured profile's +0C virtual slot at A8F826. A8F3B0's relationship to the setters is transitive through AD7A30. This leaf implementation adds no parent call site or initialization ordering.

## Evidence and validation

The accepted EG admission at `f93dfb935198d3a1a56374fece7d343f303b9b59` compared all complete leaf bytes against both live Ghidra and original PE SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Its JSON, native capture and helper hashes are pinned in the report. Current source begins on accepted EH source commit `96907b532a143c218af01812a78a0332659b877f`; EH source remains unchanged for the joint build. Shared startup registration is an isolated append under `docs/COORDINATION.md:72`, which forbids whole-file registry leases.

Before build, the required generated-code check is complete 13-byte and 22-byte COFF section equality with the native bodies, zero relocations and original RET4/RET8 stack cleanup. The getter requires its own source/object check for one current borrowed volatile DWORD read; native six-byte identity is not expected. The joint build must pin every tracked build input and actual ignored seed before/after, all four libraries, the EI object, and EH's `native_shadow_job_lifetime.obj` and changed `native_singleton_destruction.obj` at one exact clean source commit.

**Current validation: source implemented; primary review and the single coupled Win32 build are pending.** No new tests or fixture have been added. The report will record the actual existing CTest count and final generated proof after that build. This does not claim original-game execution, visual rendering, unmasked FPU-fault behavior or completion of the parent update routine. The package supplies three bodies / 41 native bytes; only the two setter names are new review candidates, while the existing getter name is preserved. Ledger and Ghidra updates belong to the primary integrator.
