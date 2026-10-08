# Complete raw weapon-accuracy initializer

`raw_initialize_native_weapon_hit_accuracy_00836ef0` provides the complete independent Win32 register entry for Original `00836EF0..00836F77`. It initializes all `58h` bytes without reading the receiver or calling another function. The existing typed initializer and other module functions retain their complete compiled bodies.

The entry receives writable storage in ECX, copies that pointer to EAX, and returns with plain `RET`. The literal 27 instructions preserve the Original 22-store order: `100.0f` at `+0`, `200.0f` at `+4`, then `0.5f` at `+30,+8,+34,+C,...,+54,+2C`. ECX, EDX, flags and ESP are untouched; XMM0 finishes with scalar `0.5f` and zero upper lanes. There is no x87 arithmetic, stack frame, allocator, callback or exception helper.

Admit valid writable caller-owned `58h` storage, disjoint from the immutable code and constants. This write-only service does not need a gameplay-object producer. It provides no profile, class, enclosing settings object, ownership, publication, Lua load or teardown. The Original constructor calls the initializer four times at settings `+240,+298,+2F0,+348`; that placement is evidence for the role, not execution of the complete constructor.

The current MSVC I386 COFF body is exactly 136 bytes and 27 instructions. Every byte matches the installed Original after replacing only the three genuine DIR32 constant operands at bytes `4,18,31`. Their resolved object payloads match Original `00CE3D08=0000C842`, `00CE386C=00004843`, and `00CE3800=0000003F`. All five prior complete module functions retain their bytes and relocation structure; only anonymous translation-unit path names differ between the main and integrator object identities. The entire production object appears once, byte-for-byte, in the retained complete `bsp_core.lib`.

`./scripts/build.ps1` completed Win32 Release and all three existing CTests. No new test or Native/Source fixture was added for this branchless initializer; its register shape and instruction identity are static qualifications. App linkage and runtime execution are not claimed. The first ignored sealing attempt failed while parsing a short final hex-dump line, before any execution; the parser was corrected and the complete gate passed without a Source change.

The [JSON report](../reports/cc12_weapon_hit_accuracy_raw_defaults.json) records the full Original and COFF instructions, three real relocations/constants, exact archive member, current build and explicit limits. `local/cc12_weapon_defaults_primary/` retains 12 selected input copies, the whole current and baseline objects, the complete current archive, and live pre/post Original byte captures. This is a selected physical evidence set, not a claim to copy every compiler or SDK input.

The complete `76Ch` gameplay-settings constructor, Lua loader, lazy global owner, recursive shutdown and gameplay remain separate dependencies. This added entry creates no new Original function count.
