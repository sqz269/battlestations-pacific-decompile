# Native particle parameter construction and loading

This packet reconstructs the complete AFBED0, AFBF60, AFC360, AFC470, AFC1B0 and AF4110 paths, plus their direct key construction, allocation, sorted insertion, coefficient and runtime segment helpers. The address-suffixed public functions in `native_particle_parameter_loading.hpp` are new C++ interfaces. Descriptive names are hypotheses, not recovered symbols. The report records all 20 reconstructed spans, last instructions, original stack cleanup, and native CALL/tail-jump rows.

The saved project is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, bridge `127.0.0.1:8089`. Full live spans were compared with `I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`. The installed executable SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. No game file was modified.

## Producer-established storage and ownership

AFBED0 clears only the temporary owner's first three DWORDs and calls AFBDB0 to reserve 32 records. Its fourth DWORD, kind at +0C, remains untouched. The actual owner is:

| Offset | Meaning established by producer |
|---|---|
| +00 | Record-array pointer |
| +04 | Signed live record count |
| +08 | Signed allocated record capacity |
| +0C | Conversion kind: 0 constant, 1 linear, 2 Hermite |

AFBDB0 grows only, with a minimum capacity of four. BF55BE allocates `capacity * 2Ch`, saturating the allocation request to FFFFFFFF on unsigned multiplication overflow. The native 403560 constructor iterator calls AFB670 on each new record. Existing records are copied forward in 11 DWORD steps; BF6989 releases the current old pointer; only then are the replacement pointer and capacity published. Allocation failure is not converted into successful empty ownership.

Each 2Ch record holds x and y at +00/+04, incoming tangent at +08/+0C, outgoing tangent at +10/+14, endpoint-kind DWORD at +18, and coefficients a/b/c/d at +1C/+20/+24/+28. AFB670 writes zero x/y and kind 1, copying two uninitialized native stack words into both tangent pairs; it does not initialize coefficients. AFC360 likewise forwards dormant endpoint tangent and coefficient scratch. The symbolic instruction kernels preserve these sparse stores. Those scratch values have no defined cross-call bit pattern and are not assigned invented defaults.

AFC260 finds the first key for which the new x is not strictly greater, then inserts before it. Equal and unordered x comparisons both stop the search. Capacity doubles only when count equals capacity. Its preliminary append, backward shift of whole records, and final copy retain the native ordering and current owner reloads.

AF4110 captures and releases the array pointer, then zeros the current owner +00/+04/+08. Kind +0C remains unchanged. Ghidra had incorrectly stopped this function and three other functions at BF6989. The integrator repaired AF4110 through AF4129, AFBDB0 through AFBE74, AFFD70 through AFFDE0, and AFC470 through AFCC9E before final evidence collection.

## Parsing and endpoint behavior

AFC360 inserts two keys into the existing array without clearing it or assigning owner kind. Their x values are zero and the current CE3D08 float. Their y values are the two caller arguments. AFBD10 normalizes the first outgoing tangent; AFBC90 normalizes the last incoming tangent. Each uses the original x87 comparison, clamp, multiply/add spill, canonical CRT sqrt, float spill and divide schedule. Required limits/clamps and the CRT exception-state binding come from the application.

AFC470 receives the actual four-byte pooled line owner. It uses the concrete AEE3C0 tokenizer and AEE2A0 cleanup through the application's existing `NativeStringStorage`. Case-insensitive grammar is:

| Form | Token sequence after form name |
|---|---|
| `Const` | value |
| `Linear` | key count, then x/y for each key |
| `Hermite` | key count, then x/y/incoming-x/incoming-y/outgoing-x/outgoing-y for each key |

Linear and Hermite require a signed count of at least two. After reading that count, the parser assigns kind and releases/clears the current array **before** validating individual records. The first x must equal the current D7A218 float; the last x must equal the current D7A220 double. In the inspected image these represent 0 and 100. Failure retains the new kind and whatever earlier records have already been inserted. Intermediate x values are sorted by AFC260; no additional monotonicity check is added.

Linear records get incoming `(current D7A260, 0)` and outgoing `(current D7A24C, 0)`. Hermite tangent pairs preserve native token ordering: obtain the y token, obtain the x token, convert y then x, release x then y. All parsed numbers are rounded to float at their original storage boundaries. Unknown forms return false without altering the builder. `Const` assigns kind zero and writes the first existing key's y; it does not create that key or change count/capacity.

Missing numeric tokens, null native allocation results and nonsensical caller states remain unsupported native fault paths. This reconstruction adds no successful fallback. C++ exception cleanup uses the concrete pooled-text owners; it does not claim the original FH3 exception ABI.

## Runtime parameter allocation and conversion

The pool is established from its producer, independently of the samplers. CD78B5 calls B004B0 with ECX = **F8D344**. B004FC publishes profile **D5DCBC** into that same pool. CD78BF calls BF6FF5 with destructor CE0BE0. B004A0 ignores its incoming size hint, loads F8D344, and tail-jumps to 9242F0. The binding therefore borrows the canonical `NativeWeakHandlePool` companion over that actual storage. It neither uses 0109CE94 nor initializes or owns another pool.

The allocated slot is 10h bytes physically, containing a 0Ch parameter payload and the slab index at +0C. AFF9B0 initializes only payload +04, +08, +09 and +0A. The multiplier at +00 and slab index at +0C remain untouched. Count and capacity at +08/+09 are **bytes**, and type at +0A is a WORD. The caller of AFBF60 is responsible for writing the multiplier later.

For kind zero, AFBF60 copies the first key's y into payload +04. For kind one, it allocates `(count-1 low byte) * 14h` and generates x-start/x-end/value/slope/integrated-offset segments. Slope uses the original x87 division without intermediate float rounding of the two subtractions. For kind two, AFBF20 first invokes AFB3A0 on every consecutive key pair, then allocates `(count-1 low byte) * 1Ch` and copies x-start/x-end/integrated-offset/a/b/c/d segments.

AFFD70 stores the capacity byte first. For supported types it releases an old segment array, clears its current pointer, and allocates the new array without resetting live count. B000A0 and B00120 integrate the previous segment up to its end before copying the new segment, write the accumulated offset into the new row, and increment the live count byte. AFFCB0 and AFFD20 use current double scales and retain the native x87 operation order and float spills. No clamp or protection is introduced around count truncation or unchecked writes.

The existing AFFDF0/B00090 definition teardown releases segment arrays for types 1/2 and returns the captured slot through this same physical pool. Unknown conversion kinds preserve the original behavior: a slot has already been allocated and is abandoned while the function returns null.

## Validation and integration

Strict standalone MSVC Win32 compilation passed with `/std:c++20 /O2 /MD /EHsc /W4 /WX /fp:strict`. The focused original-byte fixture passed 14 observations over two current-constant sets. It covered default endpoint insertion, Const, sorted Linear/Hermite construction, failed first/last endpoint checks, invalid count, unknown form, conversion, and builder release. It compared defined key bytes, complete emitted segment bytes and integrated offsets, actual pool slot/slab/freelist state, and the full array/text allocation-release trace. Twenty-one complete native spans and fourteen constant spans matched live/disk bytes; the extra function is the original 403560 constructor iterator used by the fixture.

The fixture uses one actual caller-owned parameter pool initialized through the canonical B004B0 implementation. Original B004A0 is relocated to that same storage, and original 9242F0 calls invoke its real canonical allocator. Pooled-text calls use the sibling's concrete implementation. Controlled allocator domains record the external array/string service effects. Native direct internal calls, branches and x87 instructions remain original bytes; external service targets and absolute data operands are relocated.

Only unused endpoint tangent scratch and coefficients not yet produced are omitted from key comparisons. Emitted runtime segment payloads are compared completely. Original throwing FH3 unwind, exceptional CRT sqrt handling, byte-count overflow, extreme signed arithmetic, invalid null-token faults, native drop-in ABI compatibility and gameplay are not claimed. Full repository build and any annotation/ledger integration belong to the primary integrator.

Scratch is preserved under `C:/Users/sqz269/bsp-ao-parameters`: `make_fixture.py`, raw live byte captures, `byte_evidence.json`, `call_evidence.json`, `original_bytes.hpp`, `bind_original.inl`, `parameter_probe.cpp`, `probe.cmd`, `fixture.log`, `fixture_result.json`, `generate_kernels.py`, and `finalize_report.py`. `probe.cmd` compiles this source and the pooled-text sibling source against the current primary `build/win32/Release/bsp_core.lib`, `bsp_lua511.lib`, `bsp_zlib121.lib`, plus `user32`, `ws2_32`, and `advapi32`. The exact library hash is recorded in the report. Probe executable names are neutral and link with `/MANIFEST:EMBED`.
