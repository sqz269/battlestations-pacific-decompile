# Mission entity lock in the raw singleton domain

Addresses: profile 00CE7548, native scalar deleter 004C4890, process publication 00F878FC.

The finite raw-singleton deletion map now routes CE7548 to the complete mission-entity lock scalar deleter using the same process F878FC publication used by its getter. CE7548 has one slot, 004C4890; the adjacent CE754C begins a distinct owner profile. The popped owner is passed directly even if publication changed after registration. No private context, manager, cleanup registry or automatic shutdown is introduced. Owners constructed against another publication are outside this process binding.

One focused composition probe creates the owner through its process getter and the canonical raw manager, checks fast reuse and exactly one registration, then replaces the published pointer before actual manager drain. The finite map deletes the original popped owner, clears the same process cell, preserves the replacement object and empties the manager. The retained probe uses the compiled production libraries. This is source composition evidence; original-byte execution, native exception transport and gameplay binding remain separate.

The selected MSVC Hostx64/x86 toolchain, compiler-discovered headers, searched libraries, object/executable, embedded manifest and output are retained by `local/mission_lock_binding_proof_x/manifest.json`. Exact compiled revision and hashes are in `reports/native_mission_entity_lock_domain.json`.
