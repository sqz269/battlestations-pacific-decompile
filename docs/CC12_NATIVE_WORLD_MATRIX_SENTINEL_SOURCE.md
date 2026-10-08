# Native World matrix sentinel producer

The approved Source implements the complete `004C3080` raw allocation leaf and one concrete allocator adapter. **Build and emitted-code qualification are pending Root CMake registration.** No World owner or runtime readiness is claimed.

The public entry is a no-argument CDECL naked function: Native consumes no incoming ECX/EDX or stack arguments, returns the real pointer in EAX, leaves ECX=pointer+4 on normal return, and uses plain RET. Both `004CB030` and `004C8B50` call the original helper; this is not exclusive to the World constructor.

The retained Native body is 26 bytes / 11 instructions. It requests 0x6C bytes, then writes the returned pointer only at +0 and +4. Bytes +08..+6B remain unspecified allocator contents. The design preserves all 22 bytes outside CALL operand [3,7), including both literal branches. A hypothetical null allocator result reaches a write to address4; it is not graceful null success. The fixed-size genuine provider returns storage or throws.

The private noinline CDECL adapter passes `SingletonAllocationRequest{object,bytes,bytes}` to the existing `singleton_lifetime_allocate`; the literal call-site size is 0x6C. It adds no registry, zeroing, class constructor, callback, catch, or World publication. The returned allocation transfers to the caller in the matching `singleton_lifetime_free` domain. Neither the adapter nor the leaf is noexcept.

This preserves the established host CRT boundary. The original allocator's static exception object/guard/atexit, Native exception vtable/throw metadata, global new-handler identity, and FH3 machinery are not reimplemented. Its whole105-byte body and the real Source allocator/throw/RTTI/import dependency graph were independently reviewed before Source admission.

The header and implementation are copied verbatim from the approved proposal. Root's design receipt SHA-256 is `bc0ec586ebb1c7dd85255cdccd48faf3039d5727215eb3100b38dd23c611f835`. Fresh helper/adapter and provider-graph machine-code proof will be recorded after the separately authorized normal build and its three existing checks. There are no new tests, probes, or target executions.

Only `native_world_matrix_sentinel.hpp`, `native_world_matrix_sentinel.cpp`, this document, and the accompanying report are owned. Root owns build registration. Actual World vtables, constructor/array unwind, normal destruction, caller publication, and game startup remain outside this leaf.
