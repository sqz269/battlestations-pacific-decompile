# Opaque vector-storage tidy alias

Address: 00468020.

| Native address | Source API | Native span | Coverage |
|---|---|---|---|
| 00468020 | tidy_native_vector_storage_00468020 | 00468020..00468049, 42B/13ops | complete |

Independent full-listing and live/installed-byte review establishes the same
complete schedule as accepted 00468840: capture DWORD+4 exactly once, conditionally
free that captured pointer if nonzero, then store zero at+4, then+8, then+0Ch.
Both returning paths perform all three stores. Neither reloads+4 after free,
directly accesses+0, touches an element, or invokes a diagnostic/destructor.
No caller or producer establishes an element type or semantic field roles, so
the actual16B storage remains opaque. Correct native STL library names remain.

The complete42B bodies are byte-identical after masking only CALL operand
bytes12..15 (zero-based). Both independently decoded rel32 values target actual
00BF65AC. The alias's sole native edge is `0046802B CALL 00BF65AC`, with one
captured pointer stack argument and ADD ESP,4. Native00468020 does not call
00468840. Source delegation is reuse of that already reconstructed, integrated
implementation, not an invented native edge or unresolved callback. The real
provider Source/header at `eab43f2bf` were already in this worker branch before
the alias draft; no stub or narrowed positive-only implementation is supplied.

Native inputs are ECX opaque storage, unused incoming EDX, no stack arguments;
ESI is preserved and return is plain RET. Native null-path EAX is0; after free
it is ambient callee state, with no established semantic result. Source uses
fastcall void and makes no EAX value promise. Current-CRT free remains the
accepted actual singleton_lifetime_free/std::free service boundary. Every taken
raw access and release still requires valid backing and heap compatibility.

Live incoming xrefs are empty and no existing whole reconstruction/Source was
found. Exact typed metadata confirms one complete42B returning/non-thunk body,
ordinary free CALL/no override with target00BF65AC and fallthrough8030, and the
conditional branch to8033/fallthrough802A. No flow or prototype repair is made.
All42 live bytes match the installed PE, with the full equality/target receipt
recorded in JSON. Guarded batches verify C:/Users/sqz269/bsp.gpr and program
/battlestationspacific.exe, x86 image base00400000.

Owned annotation records old values, preserves correct names/comments, takes
the write lock, saves the project, refreshes the export and verifies live
readback. The reviewable Source draft awaits its normal Win32 build and existing
checks. No new routine tests are added. Source reuse/static/compile evidence do
not establish original private-frame/register/flags/mangling/heap/CRT/FH3/SEH/
fault/binary ABI, direct native differential, application admission or game
behavior.
