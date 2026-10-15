# Goal 15: pool storage cleanup

Addresses: 0041F820 (owned); 00419CC0, 00BD1510, 00BF65AC (read dependencies).

`cleanup_native_pool_storage_0041f820` reconstructs the entire existing-byte
record cleanup. The owning class and its construction path remain unknown.
The interface requires actual Win32 storage and the application's existing
`NativeStringRawPoolContext`; it introduces neither a shadow owner nor a host
method whose behavior replaces the pool.

| Routine | Native extent | Coverage | Original ABI |
|---|---|---|---|
| 0041F820 | 0041F820..0041F866 inclusive (71 bytes) | complete | ECX record, no stack arguments, plain RET; ESI preserved |

The new source function uses MSVC Win32 `__fastcall`: ECX record, EDX explicit
pool context. This extra source argument means it is not a drop-in binary
replacement. The native routine does not require EDX. No native exception frame
or original CRT heap identity is claimed.

## Recovered schedule

1. Read the pointer at `+10h`; if nonnull, call CRT free. The `ADD ESP,4` at
   0041F830 establishes its one cdecl argument.
2. Write zero DWORDs at `+10h`, `+14h`, and `+18h`, in that order.
3. Capture the pointer at `+8h`; a null pointer returns without reading `+4h`
   or resolving the pool.
4. For nonnull data, capture DWORD `+4h` plus one with unsigned 32-bit wrap.
   Push `(1, size, data)` and call the no-argument singleton getter. Move its
   EAX result to ECX and return the block through the sized-pool service.
5. Leave `+4h` and `+8h` unchanged, including their stale values. The routine
   does not free the outer record and adds no idempotence guard.

The scalar and pointer at `+4h/+8h` have the same access shape as the established
eight-byte native string header. The implementation reuses the complete raw
`destroy_native_string_header_0041dd20` body at the interior address `record+4`.
This is source composition, not a claim that the native routine calls 0041DD20.
No producer was found for this unreferenced routine, so the packet declares no
record struct, field names, vtable, or complete class layout.

## Dependency evidence

| Site within 0041F820 | Native callee | Read contract | Source composition |
|---|---|---|---|
| 0041F82B | 00BF65AC | cdecl CRT free thunk; one pointer, caller cleanup 4 | existing `singleton_lifetime_free`, a genuine `std::free` service |
| 0041F859 | 00419CC0 | no arguments, EAX current/lazily created pool, plain RET | existing raw-context header destructor resolves the actual publication cells for each nonnull release |
| 0041F860 | 00BD1510 | ECX pool, stack `(block,size,unused=1)`, RET 0Ch | existing concrete actual-pool return body; third argument is unread |

Both pool callee bodies and their assembly were inspected. The singleton's
caller-pushed arguments belong to the following return operation; they are not
getter arguments. The return body compares size unsigned against 96h, uses CRT
free for larger blocks, and applies its actual small-return gate and lock for
smaller ones. The source provider already implements that behavior. Getter
exceptions propagate after the three zero stores, without clearing the interior
string header or adding cleanup. The unused native third DWORD is omitted by
the established source provider.

## Verification and limits

Analysis used the configured `C:/Users/sqz269/bsp.gpr`, project `bsp`, program
`/battlestationspacific.exe`, x86 image base 00400000. The `bsp.py` live client
verifies that identity before each analysis/export batch. Exported pseudocode
and the complete assembly establish both null branches and every call site;
the three sites were checked with live containing-function queries.

`python tools/callsite_census.py 0041f820` found zero rel32 CALL/JMP targets and
zero literal address occurrences in its scanned `.text/.rdata/.data` sections;
live Ghidra xrefs also found none. That bounds the evidence: no reachable
caller or producer is asserted and no game behavior is claimed.

Worker checks passed: all three report call rows, `git diff --check`, and an
MSVC x86 C++17 `/c /W4 /permissive-` compile of the new source. That object-only
compile verifies this translation unit, not library linkage. The integrator
owns the required `scripts/build.ps1` Win32 build and existing CTest run. No new
tests were added for this small composition. This packet has no differential,
runtime, native ABI, or gameplay validation.

Integration requires adding `src/native_pool_storage_cleanup.cpp` to `bsp_core`
via the shared CMake registry. Existing providers are `src/native_string.cpp`,
`src/native_string_pool_owner.cpp`, `src/native_string_pool_storage.cpp`, and
the singleton lifetime implementation. The worker makes no shared registry or
Ghidra mutation; the integrator owns annotation and export refresh.

Primary integration update: the registered MSVC Win32 build and all three
existing CTest suites passed. Ghidra annotation/save and standard export
refresh are complete. See [the batch receipt](../reports/goal15_reconstruction_batch.json).
These checks do not execute or establish original ABI/gameplay equivalence for this entry.
