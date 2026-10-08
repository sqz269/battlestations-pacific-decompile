# Raw property key hash readiness (CC12)

Whole Original `0043B760` is established. Its contract is **ready for a separate, whole borrowed raw Source service on valid ASCII keys with the genuine current C-locale CRT boundary**. A current `0043B760` Source function is still absent; this audit implements nothing. General Original locale behavior, binary ABI and game admission remain unclosed.

The worker started clean and refreshed main, then refreshed again while still clean as main advanced to `33d1086d39a2f9fb4de98129d7fd187e09fdea42`. The resulting worker baseline is `0f847c24d855edc2f3e86507c12a151f9de3b2e6`. The prior lookup audit `8a1476f` is present on the worker and under Root review; it was not yet on main at the Source receipt. Selected current Source files match main. Only this document and `reports/cc12_property_hash_readiness.json` are owned outputs; only `0043B760` was claimed.

## Complete native body and actual caller binding

The actual body is `0043B760..0043B7B8` inclusive (`0043B7B9` exclusive): 89 bytes, 39 instructions, one direct call, no indirect calls and no instruction gaps. Complete saved pseudocode and assembly were inspected. SHA-256 is `3f92060d0b946ebf8440f1edb1b763ea9cc898f2950e34a9a9da297fcb32455f`. Installed PE bytes at file offset 243552 in `.text` match saved Ghidra. The entire installed executable is 12223752 bytes with SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Each read-only live wrapper batch verifies `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language and image base. Assembly resolves the signed byte promotion, separate hash/remaining registers, unsigned branches and physical return cleanup. No listing/prototype repair was required or applied. Ghidra's stored `undefined FUN_0043b760(void)` prototype is not the physical ABI.

The routine takes one stack DWORD pointing to an actual `8h` header: length at +0, data pointer at +4. It returns the DWORD bucket in EAX and uses `RET 4`; EBX, EBP, ESI and EDI are saved/restored. It never reads incoming ECX or EDX as an argument. ECX/EDX/flags have no general preservation promise. No receiver, map, class object or allocator is required by this body. Its input must be nonnull and valid; there is no input guard.

Actual lookup calls it at `0043B8BA` after pushing the query pointer. ECX still holds the map, which the hash ignores. The callee's `RET 4` now independently confirms the prior lookup stack inference. The other xref is `008F1920` in `008F18B0`; that caller body was not expanded or admitted from its name. The sole owned-body call is `0043B789 -> 00BF924E` (`_toupper`): a signed byte is pushed as an integer, and the caller restores its four bytes with `ADD ESP,4`.

## Whole schedule and arithmetic

The routine loads the data pointer first, then length, once each. EBP is the data cursor. ESI is the evolving unsigned DWORD hash; EDI is the fixed step; EBX is the separate remaining count.

1. Set hash and remaining to length. Set `step = (length | 20h) >> 5` using a logical shift. The step is always at least one.
2. If data is null, select the actual property cell `00E177E4`. This happens even at length zero, but length zero performs no byte or CRT read.
3. While unsigned remaining is at least step, sign-extend the byte at the cursor and pass it to genuine `_toupper`. Retain only its low eight return bits.
4. Update the DWORD hash as `hash ^= byte + (hash << 5) + (hash >> 2)`, with native modulo-2^32 addition and logical right shift. Subtract step from the separate remaining count, advance the cursor **one byte**, and repeat.
5. Return `hash & 3Fh`, restoring all saved registers.

Thus the iteration count is `floor(length / step)`, and the routine reads that many consecutive prefix bytes. A valid 64-byte key reads its first 21 bytes: step is three, but cursor advance is one. Empty keys return zero without touching pointed data or invoking CRT. The body writes no input/global/output storage, allocates/releases nothing, and returns no ownership. It contains no REP/x87 instruction and adds no DF requirement. It does not normalize or mutate a string, look up a map, publish a bucket, insert a node or destroy a record.

## Genuine CRT and input admission

Current `src/native_enum_dictionary_lookup.cpp` contains the same complete arithmetic and signed-byte promotion, using real `std::toupper`. Its Source and header hashes exactly match the existing production compiler input receipt. The actual I386 COFF helper is 85 bytes/38 instructions and contains an `__imp__toupper` relocation. That is a genuine current CRT import, not a Source callback, handwritten ASCII table or uppercase hook. The helper's distinct enum address/fallback does not establish a property function's identity; this packet independently recovered the entire property body before comparing the algorithms.

Original calls the internally linked native `_toupper` body at `00BF924E`, rather than a PE IAT entry at that site. Existing verified enum evidence contains its complete 39-byte body with SHA-256 `3158c03a0ab276bb10aad57b86e81c495f0343f5f9feb34720cab39390b07d20`; those bytes still match the installed PE. Its provider/fixture evidence was reused, not re-executed or expanded into another native body. Existing Original qualification admits the image-initial zero-locale branch only. Saved Ghidra and PE initial virtual zero-fill at `0109DE1C` are still four zero bytes; this is not captured live game locale state. The named `__toupper_l` dependency and nonzero-locale graph remain unexpanded/unbound here.

The bounded whole Source domain is stable actual Win32 `8h` headers, closed NUL-free ASCII keys with lengths matching genuine buffers, null data only at length zero, and current CRT C locale held stable throughout the call. On that domain every sampled byte is a valid nonnegative CRT argument. Original uses sign extension: bytes `80h..FEh` become negative values outside the standard `toupper` input contract; `FFh` becomes EOF. No non-ASCII or locale-change parity follows from the ASCII proof. There is no fault, concurrent mutation, native SEH/reentry or current game-locale claim.

Inputs need no manufactured packed map. Current `construct_native_string_header_0041e870` with `NativeStringRawPoolContext` genuinely produces fresh actual headers/buffers through existing raw string/pool services. It zeroes both words and uses the actual raw resize/copy path. Successful valid ASCII inputs provide both empty and nonempty hash admission; allocation failure and constructor ownership/EH remain that producer's existing limits. The hash only borrows its already-live input and invokes no allocator.

The property fallback `00E177E4` has an initial NUL byte in `.data` raw file bytes, installed PE and saved Ghidra. No current property Source cell was manufactured. A future Source interface must expose its distinct role explicitly. For valid null/empty headers the selector runs but the selected cell is never dereferenced or returned, so a property cell's absent publication does not block the complete hash service's valid-input result. Null/nonempty headers remain excluded. Actual property lookup comparison does dereference its fallback on appropriate paths, so its missing binding stays a separate lookup closure issue. This readiness covers the complete nonempty loop and valid empty path, not a null-only subset.

## Precise next implementation packet

Propose a separately authorized packet owning only `0043B760`, `include/bsp/native_property_key_hash.hpp` and `src/native_property_key_hash.cpp`, with the new interface:

```cpp
std::uint32_t native_property_key_bucket_0043b760(
    const void* actual_key_header, const char* actual_empty_00e177e4);
```

It must read the actual Win32 header offsets in native order, preserve the entire unsigned loop/signed CRT promotion, call genuine current CRT `toupper`, keep the property fallback role explicit, and borrow all memory without guards, allocation, semantic containers or uppercase substitutes. It must not silently delegate to an enum-address service or claim existing hash Source now implements `0043B760`. CMake/ledger integration remains an integrator-owned separate action.

This is a new borrowed C++ Source interface, not Original's one-argument `RET 4` binary entry. Any native qualification must keep the complete 89-byte Original hash and genuine Original CRT provider under the already approved ASCII/initial-zero-locale constraints, with no Source hooks replacing calls. Compilation and source/static equivalence do not establish native execution or a drop-in entry. This audit ran neither a new qualification fixture nor a build.

Even after a hash service is implemented, `0043B8B0` whole lookup still needs its own current Source implementation, genuine property fallback binding and populated-map admission. Whole `008F28F0`/`008F33F0` insertion and replacement paths retain record profile/lifetime closure; the admitted type-6 chain `004E6730 -> 008F0DE0 -> 008F0640` is separate. Map construction/insertion, recursive destruction, class/EH/factory/world, binary ABI and gameplay are not closed by this leaf.

Evidence is in `local/cc12_property_hash_readiness/`: complete listing/pseudocode/bytes/flow/xrefs, two direct call rows and prior genuine lookup binding, full installed PE identity, selected current Source pins, reused original CRT receipts, actual imported-CRT COFF evidence and its compiler input hashes, and the artifact manifest. Only these two audit files are committed. No Source/native execution, build, fixture replay, new test or Ghidra/shared metadata/CMake/config/export write occurred.
