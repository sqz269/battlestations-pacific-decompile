# Raw singleton vector allocation and length-error throw

This packet reconstructs the complete logical bodies **00BCFEB0..00BCFF04 (85 bytes)** and **00BD0590..00BD05F8 (105 bytes)**. Their exported source entries retain the allocation register/return contract and no-input throw contract. Allocation uses the repository's actual shared malloc/new-handler/free service. The length error carries the actual reconstructed legacy string and exception payload in an owning C++ transport. Original static-CRT and exception-runtime identity remain explicit service boundaries.

The four owned files are `src/native_singleton_vector_allocation.cpp`, `include/bsp/native_singleton_vector_allocation.hpp`, this document and `reports/native_singleton_vector_allocation_audit.json`. No manager, domain, publication, registration callback or invalid-parameter handler is introduced. Append/register BD0BC0/BD0C30 are separate incomplete dependencies, not names for this allocation packet.

## Native evidence and source contracts

Fresh guarded `tools/bsp.py ghidra` queries verify the existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 language and 00400000 image base before every analysis batch. Both complete owned bodies match the installed executable byte-for-byte and decode completely. Their saved Ghidra signatures omit the real register/result contract; assembly controls the declarations. The audit links the immutable capture and source/build pins.

| Native entry | Source entry and ABI | Behavior |
| --- | --- | --- |
| BCFEB0 | `void* __fastcall native_singleton_pointer_allocate_00bcfeb0(uint32_t count, void* unused_edx)` | Count in ECX, EDX unconsumed, no stack inputs, EAX allocation, plain RET. |
| BD0590 | `[[noreturn]] void __cdecl native_singleton_length_error_00bd0590()` | No consumed input or normal return; throws the owning source transport described below. |

BCFEB0 first tests ECX. Zero proceeds with zero bytes. Positive count executes unsigned `FFFFFFFF / count >= 4`; rejected counts throw bad_alloc, admitted counts allocate `count*4`. Thus count 3FFFFFFF and its byte request FFFFFFFC are admitted. The source comparison against 3FFFFFFF is equivalent for every uint32 input and avoids an undefined zero division. It adds no pointer-count or CRT-maximum cap and no count-one shortcut. No storage initialization occurs here.

Native overflow calls BF6340 with a pointer to a null message pointer, installs D6923C and calls BF6885 with throw metadata E03CC0. The source uses real `std::bad_alloc` at this explicit CRT-exception boundary. The host exception's message, RTTI, throw metadata, allocation internals and catch identity are not claimed to be the original object.

Native BF681B passes its original byte argument to malloc BF9F1A and, on failure, to C055B1 `__callnewh`; a nonzero handler result retries malloc. It initializes library-owned bad-allocation state 109DD68/109DD74 and an atexit action on the exhausted path. Each C055B1 invocation rereads encoded 109DE44 and decodes/calls the current handler. BF9F1A also has heap-mode/newmode behavior and an oversized-request handler call of its own. These are **actual native dependencies**, not reconstructed by replacing the call with a presumed complete CRT.

`singleton_lifetime_allocate` is the concrete source provider: real `std::malloc(request.host_bytes)`, real `_callnewh(request.host_bytes)`, retry for nonzero, otherwise real `std::bad_alloc`. Both request sizes are the same 32-bit byte value for this raw entry. Its allocations pair with the existing `singleton_lifetime_free`/`std::free` source domain, also used by the raw vector leaves. There is no extra owner domain. Host heap modes, maximum-size handling, errno, zero-size allocation identity, failure callback counts, handler encoding and old CRT globals are outside the parity claim. Arbitrary callbacks that discover and mutate a caller's stack argument/request record are outside the source ABI contract.

## Length-error ownership and read/write schedule

BD0590 reserves an uninitialized 1Ch-byte temporary string and 28h-byte exception payload. It writes only temporary capacity+18h = 15, length+14h = 0, then first inline byte+4 = 0; the leading DWORD and unused bytes remain uninitialized. It calls completed 00408720 with the literal at CE37E0 and count 18 (`vector<T> too long`, excluding the terminator). Only after assignment returns does it arm state 0. It calls completed 00411700 on the exception payload using that temporary, then publishes D69260 and calls BF6885 with D83F98.

Handler CC5478 selects FuncInfo DFF4BC. Its one-entry unwind map DFF4B4 sends state 0 to -1 through CC5470, which addresses the temporary at `[EBP-50h]` and tails 004072D0. There is no synthetic catch/rethrow and no temporary cleanup armed during the initial string assignment. A constructor failure cleans the completed temporary; native constructor 00411700 separately owns its established base cleanup. The current source preserves this division using a guard created after counted assignment.

`NativeSingletonVectorLengthError` owns exactly one `NativeLegacyExceptionStorage` (28h). Construction calls 00411700 and then stores D69260. Copy calls completed 00411940, whose successful copy also publishes D69260. Destruction calls completed 00411780, which destroys the contained string and base ownership. Raw payload default-initialization writes nothing. D69260 remains an address identity stored as data, never a callable rebuilt vtable. The source transport has a **new C++ class/catch type, RTTI and EH ABI**; callers must catch this transport, not assume `std::length_error` or original game exception matching.

The existing SBO provider covers allocation failure and returning CRT services, including its full 304-byte native growth body 004089E0..00408B0F and both catches. Ghidra splits that logical body after 00408A4F, so the evidence separately captures the entire logical range and its FuncInfo/maps. Initial growth failure is governed by that provider; this packet does not invent a caller cleanup before BD0590's native state 0 becomes active. Legacy owner/string services remain their published new C++ interfaces, with real source CRT allocation/copy/free and source exception machinery.

## Verification and limits

The strict Win32 Release build passed using the unchanged scripts/build.ps1 and an ignored source-registration hook. Both existing CTests passed, and all eight native math seeds matched disk. These checks are separate from native-byte, emitted-COFF and archive verification. The emitted allocation/throw entries, owning transport constructor/copy/destructor, cleanup funclets, throw metadata and actual source-provider relocations are retained in the immutable evidence. Existing math tests do not exercise allocation failure or vector length exceptions. No routine tests were added.

This is a complete logical reconstruction under the documented allocation/string/exception service contracts. It is not a reconstruction of the full original CRT, original exception catch/RTTI/FH3 identity, arbitrary stack-spill alias mutation, hardware-fault timing, source register-clobber identity or game runtime behavior. Register/stack shape for the raw allocator alone does not establish drop-in binary compatibility for its exception paths. The packet does not complete reserve/insertion, the raw manager, registration or the canonical manager's registered-owner dispatch.
