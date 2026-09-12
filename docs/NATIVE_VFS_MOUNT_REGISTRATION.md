# Actual VFS mount registration

Addresses: `00BE1740`, `00BE1890`. Names are descriptive hypotheses, not recovered symbols.

This packet adds the complete logical bodies over the original manager, strings,
mount records and tree. Existing semantic fragments in `vfs_mount_registration.cpp`
and `vfs_provider_manager.cpp` remain separate consumers. These new interfaces are
not binary replacements, and no game startup or gameplay result is claimed.

| Entry | Coverage | Original ABI | Source interface |
|---|---|---|---|
| BE1740, 329 bytes through BE1888 | Complete within the stated service domain | ECX manager; provider, actual8h prefix, signed-priority bits, flags DWORD; RET10h | `register_native_vfs_mount_00be1740` |
| BE1890, 127 bytes through BE190E | Complete with finite BDB040 selector binding and explicit failure callback | ECX manager; system/virtual actual8h headers, priority, flags, device ID; RET14h; EAX provider/null | `mount_native_vfs_system_path_00be1890` |

BE1740 canonicalizes the input with BEE390, then calls 584110 using the verified
CE7898 `"/"` literal. The trim preserves a sole slash. It constructs a 10h-byte
payload whose string is at +0/+4, provider at +8 and low flag byte at +C; padding
is untouched. BDEEC0 consumes that value and constructs the first 14h-byte record.
BDCE40 copies it, and BE1330 inserts the copy into the actual tree at manager+3Ch.
The record prepends the priority DWORD; no provider AddRef, deletion or ownership
correction is added. Higher signed priorities precede lower ones and equal keys
retain insertion order, as established by the insertion producer.

The inline BF7680 call has overlap support despite its Ghidra `_memcpy` name.
The source uses `std::memmove` and current post-allocation header reads. It omits
a zero-byte library call, matching the existing actual-header source domain.

BE1890 captures the original manager and current vtable+18h entry before selecting
a factory. The finite binding accepts BDB040; its factory service consumes the
captured current factory target. A null provider reloads **0109CEEC**, then reads
that manager's **field +90h**. At BE18B8 the callback has no stacked arguments:
EAX holds the current publication and ECX holds the callback target. The explicit
failure dispatcher carries both values and supplies no no-op or null fallback.
The native manager constructor initializes that callback field to zero; actual
application callback installation is still a required surrounding contract.

On success, BE1890 writes the fifth argument to provider+10h, reads the two name
data fields, substitutes 0109CEF0 for null, and reaches the verified single-RET
4254B0 diagnostic with five arguments. It then calls BE1740 on the **original**
manager. If registration throws, the provider is not destroyed or rolled back.
The source's value arguments do not model arbitrary aliases into native compiler
stack spills, including mutation of its caller argument cells through a callback.

## Cleanup evidence

CC6898 loads FuncInfo E00FA0; its unwind map is E00F88:

| State | Action | Actual local |
|---|---|---|
| 0 -> -1 | CC6880 -> 41DD20 | canonical string, EBP-48h |
| 1 -> 0 | CC6888 -> BDB570 | first record, EBP-20h; string at +4 |
| 2 -> 1 | CC6890 -> BDB590 | copied record, EBP-34h; string at +4 |

State 0 is armed only after canonicalization returns; state 1 only after BDEEC0
returns; state 2 only after BDCE40 returns. BDEEC0 owns the by-value payload's
cleanup independently. Normal cleanup lowers the state first, then captures
current data and length+1, calls the canonical pool getter, and returns the block.
The order is copied record, first record, canonical string. The source uses the
canonicalizer's explicit getter/return services for this sequence, including
cleanup after a normal return operation throws. Nested `NativeStringStorage`
release remains noexcept. Original FH3/SEH and simultaneous cleanup failures
are not established by the C++ exception model.

## Evidence and validation

`reports/native_vfs_mount_registration.json` records all 17 CALL sites, complete
native-byte hashes, current installed-PE equality, profile bytes and cleanup
maps. The eight incoming BE1890 call sites are 73D6F9, 73D792, 73D829, 73CD6E,
73BE73, 99597A, 995C62 and BB5842. Their stack setup retains differing priorities,
ownership values and device IDs; none is hardcoded into the interface. BE1740's
only direct caller is BE1901. Current source support is documented in
`NATIVE_VFS_FACTORY_SELECTION.md`, `NATIVE_VFS_MOUNT_RECORDS.md`,
`NATIVE_VFS_MOUNT_INSERT.md` and `NATIVE_VFS_FACTORY_RUNTIME.md`.

Final integrated build and fixture results are recorded in the packet report and
`reports/native_aw_integration.json`; a reconstructed body, an isolated native
comparison and a game-validated route are distinct evidence levels.

Follow-up packets: recover the application's current failure callback installation
and remaining factory Create targets before wiring a complete game mount sequence.
