# Pending entity lock deletion binding

Addresses: 009256D0, 00D190C4, 00F899E8.

The finite singleton deletion map now dispatches the actual `D190C4` profile
to `delete_native_pending_entity_lock_009256d0`, passing the same process-static
`F899E8` publication used by the process getter. No additional binding field or
private manager is introduced. `D190C8` starts a distinct owner's table.

Native scalar deletion clears this publication unconditionally, even when it
does not name the popped owner. Explicit-publication fixture APIs remain useful
for isolated reconstruction checks, but this process map always uses its real
process cell. The component's native-byte and exception limits are in
[NATIVE_PENDING_ENTITY_LOCK.md](NATIVE_PENDING_ENTITY_LOCK.md).

A focused composition probe checks actual getter publication, fast reuse,
single raw-manager registration and finite-map drain back to a null publication.
This uses compiled providers; the independent component fixture compares original
PE entries. Empty mission queues do not prove pending-lock use during gameplay.
Validation is recorded in `reports/pending_entity_lock_binding.json`.
