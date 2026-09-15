# Raw reader path-vector construction and native unwind boundary

This read-only packet recovers the register ABI and exceptional control flow behind `00BEA150` without implementing source or changing Ghidra. It starts at `45980f8a3a0512d10cf4f874fff2fde1eb59ee5e` and preserves the BL discovery at `10e4b0ad9d15ca0f6531cae06b9aed38945eb60a`. The prior report SHA-256 is `c82587cf310498b67844c2201275412859ba5bc390d511510f7ab1d73d02bcfc`; its 32 local artifacts were copied and independently checked with SHA-256 and SHA-512.

**The complete `BEA150` construction operation is source-ready in its valid, nonfaulting caller-owned `70h` storage domain.** Its exact base and header constructors cannot throw a C++ exception in that domain, and the existing actual `415270` source provider can be called directly ten times. Generic unwind or terminate callbacks are unnecessary for this concrete construction. The generic iterator's C++ exception cleanup order is separately established, including the outer constructor's exception eligibility gate. Full native SEH/CRT identity and asynchronous-fault parity remain separate work; a `catch (...)` around the constructor is not evidence of native outer cleanup behavior.

The machine-readable evidence is [native_raw_reader_vector_unwind_bm.json](../reports/native_raw_reader_vector_unwind_bm.json). Every captured code/data span matches both the installed PE and live Ghidra bytes. The PE SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Each live query uses `bsp.py`, whose client verifies project `bsp`, program `/battlestationspacific.exe`, x86 language and image base against configuration. The configured project is `C:/Users/sqz269/bsp.gpr`; the Java process command line does not independently expose the opened project path.

## Callback and frame contracts

`BEA150` receives ECX = caller-owned writable `70h` storage and returns that address in EAX by plain `RET`. `BF09A0` first writes zero at `+0,+4,+8,+C`. It then calls `BF7CD1` with destination `reader+10h`, stride `8`, signed count `10`, constructor `415270`, and destructor `41DD20`. Only after this call succeeds does it write `+60=0`, `+64=FFFFFFFF`, `+68=0`, `+6C=0`.

`BF7CD1` is the existing CRT `eh_vector_constructor_iterator`, with five stack arguments and `RET 14h`. Its indirect constructor call at `BF7CF5` has ECX = current header and no explicit stack argument. `415270` writes zero length then null data, returns the header in EAX, and has no calls. After the callback returns, the iterator advances its cursor by stride and increments the completed count. The failing element is never counted.

The iterator's EBP-relative state is:

| Location | Meaning |
| --- | --- |
| `EBP-20h` | Success flag, initially zero; set to one only after the loop |
| `EBP-1Ch` | Number of completed constructors |
| `EBP-4` | SEH state, zero while constructing, `-2` after success |
| `EBP+8` | Mutable current cursor, just beyond the completed prefix |
| `EBP+Ch` | Element stride |
| `EBP+10h` | Signed requested element count |
| `EBP+14h/+18h` | Constructor/destructor addresses |

`BF7D1E` is a finally funclet using **that inherited EBP**, despite its current `undefined ...(void)` signature. On success it returns immediately. Otherwise it calls `BF7C10(cursor,stride,completed,destructor)`. `BF7C10 __ArrayUnwind` has `RET 10h`: decrement count; exit if the sign flag is set; subtract stride from the cursor; publish it; call the destructor with ECX = that element. It therefore destroys only the successfully constructed prefix, in reverse order. The actual caller uses ten elements; arbitrary invalid counts, including `INT_MIN` and machine overflow behavior, are outside this valid-prefix contract.

`41DD20` first captures the header data pointer. A null pointer skips even the length read and all providers. Otherwise it captures wrapping DWORD `length+1`, pushes `(unused=1,size,data)` for the subsequent release, calls the **argumentless** `419CC0` getter, sets ECX to the returned actual pool, and calls `BD1510`, whose `RET 0Ch` consumes those arguments. It never clears the header. The existing raw-pool overload of `destroy_native_string_header_0041dd20` preserves the getter's throwing boundary; the `NativeStringStorage` overload is `noexcept` and is a different exception contract. Completed headers in this particular constructor contain null data, so their native destructor path calls neither provider.

## SEH metadata recovers the unlisted entry

`C07C00 __SEH_prolog4` puts the registration at `EBP-10h`, exception-pointers slot at `EBP-14h`, saved ESP at `EBP-18h`, encoded scope pointer at `EBP-8`, and state at `EBP-4`. The actual `E15590` security cookie encodes the scope pointer and protects the frame.

| Scope table | GS offset/xor | EH offset/xor | State 0: enclosing, filter, handler |
| --- | --- | --- | --- |
| `E02D50`, iterator | `-2,0` | `-30h,0` | `-2,0,BF7D1E` — finally |
| `E02D10`, array unwind | `-2,0` | `-34h,0` | `-2,BF7C33,BF7C5C` — exception filter/handler |
| `E03558`, terminate | `-2,0` | `-28h,0` | `-2,C07A95,C07A99` |

The saved and live listing omit `BF7C33..BF7C5E`, although all 44 bytes match the PE. These bytes are entered through the scope table, not through normal fallthrough after the loop's backward jump.

At `C07CFC`, the actual EH4 handler writes its `EXCEPTION_POINTERS` address into registration minus four, which is the parent's `EBP-14h`. At `C07D19`, it calls `_EH4_CallFilterFunc C0DCB6` with ECX = filter and EDX = parent EBP. That helper loads EBP from EDX before its indirect call. Consequently `BF7C33` reads a real parent-frame exception-pointer slot; the decompiler's phantom stack argument is not an ABI.

The recovered filter dereferences `EXCEPTION_POINTERS.ExceptionRecord`, compares its first DWORD with `E06D7363`, and returns EAX = zero for every other code. For `E06D7363` it calls existing `terminate C07A75` at `BF7C57`. Thus a C++ exception during array cleanup terminates. Other SEH exceptions continue searching; remaining element cleanup is not guaranteed. There is no returning positive-filter path. The table still identifies `BF7C5C`, which restores ESP from `EBP-18h` before the normal epilogue.

The iterator's exception path is also explicit: EH4 calls `_EH4_LocalUnwind C0DD00`, which installs the parent EBP and invokes `__local_unwind4 C0DBC4`. That function moves state to the enclosing state before invoking a null-filter finally entry. Its `C16898` dispatch is literally `CALL EAX; RET` and leaves the parent EBP available to `BF7D1E`. The success flag suppresses cleanup when the funclet is called normally at `BF7D11`.

The `terminate` wrapper obtains actual CRT thread data through `C0522E`, invokes the nonnull pointer at `+78h`, then calls `_abort C04DE2` if the callback returns or is null. Its separately unlisted seven-byte filter/handler sequence at `C07A95..C07A9B` returns filter value one, restores ESP, and reaches abort if the configured callback raises an exception. `E03558` independently identifies those entries. This does not reconstruct CRT thread-data ownership, abort, or replace them with invented providers.

## Outer C++ unwind and its eligibility gate

`CC7138` has no current Ghidra function. Its ten native bytes load EAX = `E01B58` and jump to `BF6B43`. The latter's saved name is `FID_conflict:___CxxFrameHandler3`; this packet preserves that ambiguity. Its body forwards the four OS stack arguments, EAX metadata and three zeros to existing `C07991 ___InternalCxxFrameHandler`. Matching call shape or metadata does not select a new runtime provider identity.

The full 36-byte FuncInfo at `E01B58` has magic `19930522`, maximum state one, unwind map `E01B50`, no try blocks, no ESTypeList, and EHFlags one. `E01B50` is the single pair `{-1,CC7130}`.

The assembly at `C0799C..C079D4` matters: when TLS `+20Ch` is zero, the exception code is neither `E06D7363` nor `80000026`, masked metadata magic is at least `19930522`, and EHFlags bit zero is set, the handler returns disposition one without entering outer frame unwind. Therefore an ordinary asynchronous fault does **not** justify an unconditional outer base-cleanup claim. TLS state is an actual runtime input. A source-only fault probe cannot establish this native behavior.

For an eligible C++ unwind with flags `&66h`, maximum state one and the zero nested-handler argument, `C079F9` calls `C069A2 ___FrameUnwindToState` targeting state `-1`. That routine publishes the next state before `C07B10 __CallSettingFrame@12` invokes the action. `C07B10` sets action EBP = registration plus `Ch`.

Let entry ESP of `BEA150` be `S`. Its registration is `S-Ch` and saved reader is `S-10h`. The action's inherited EBP is therefore `S`; `CC7130` loads ECX from `EBP-10h` and tail-jumps to `BF09B0`. This destroys the base once on the qualified C++ unwind path, after the iterator has handled completed path headers. The failing constructor element and the reader's caller-owned allocation are not freed by these actions.

At this point the stream-reader base remains four zero DWORDs. The complete `BF09B0` body skips stream reference release, invokes `BF0700(base+4,0)`, then calls actual CRT `_free BF6989` with null. The zero count/capacity in `BF0700` skips all growth and element-release branches and writes count zero. This precisely establishes the constructor's zero-base cleanup schedule; general attached-stream destruction and `BF0700` growth are outside this packet.

## Listing repair proposal, not an applied repair

`bsp.py ghidra flow BF7C10` reports the gap after `JMP BF7C31` and deliberately leaves it alone. Its call-fallthrough repair is not applicable.

Before a separate repair, reclaim the relevant code and metadata addresses, acquire `coordination.ghidra_lock`, reverify project/program and PE/live hashes, and record code-unit definitions, exact body AddressSets, comments and flow properties. This packet records current function names/signatures/extents, listings, bytes, absent entries and existing comments. Extents are not a complete AddressSet, and exact flow/no-return flags were not queried or changed.

Explicitly decode the two entries `BF7C33` and `BF7C5C` over `[BF7C33,BF7C5F)`. Preserve the real loop jump, CRT names and terminate behavior; do not manufacture normal fallthrough or assign ordinary stack arguments to an inherited-EBP funclet. Add verified scope references from `E02D24`, `E02D28` and `E02D68` where supported, with appended evidence comments. If separate function ownership is required, review the actual AddressSets before splitting anything. The raw metadata entries currently have no code xref to `BF7C33`.

The optional `C07A95..C07A9B` repair has the same metadata-entry issue and should receive its own explicit scope. `C069A2` also has unlisted exceptional-handler bytes; only its relevant state publication/action dispatch is semantically qualified here. No transitive CRT-runtime completeness follows from a complete byte capture.

After a future mutation, preserve old comments, save the project, refresh only affected exports and rebuild the index. This packet performed none of these mutations.

## Minimum next source packet

The specialization is supported by concrete bodies and alias/publication order. `BF09A0` has only register operations, four zero stores and `RET`; `415270` has only `MOV EAX,ECX`, two zero stores and `RET`. Neither performs an external data read, allocation, call or C++ throw operation. Valid writable storage excludes hardware faults in these stores. The actual callback is fixed to `415270`; there is no user-supplied constructor callback in `BEA150`.

The base interval `[0,10h)`, header interval `[10h,60h)` and final-field interval `[60h,70h)` are disjoint. Header `i` receives only `reader+10h+8*i`, and there is no aliased input buffer. No reader pointer is published to a global or another object. `+60`, `+64`, `+68` and `+6C` are stored in that order after all ten headers, followed by returning the saved reader pointer. Concurrent mutation, replaced callback hooks and asynchronous faults are outside this source domain. Fresh caller-owned construction storage is required: this operation overwrites old bytes without destroying a live reader.

An existing actual provider is already present: `initialize_native_string_header_00415270`, declared in `include/bsp/native_renderer_worker_lifetime.hpp` and implemented at `src/native_renderer_worker_lifetime.cpp:64` as naked assembly with exactly the original four instructions. Reuse that function ten times. Its source and header are retained in local evidence. The unrelated generic header-array helper in the same file uses a catch-all; it is not needed for this specialization and supplies no proof of native exception identity.

Use new `native_raw_reader_path_storage.hpp/.cpp` files anchored at `BEA150` and `BF09A0`, with `415270` as an existing source dependency. Preserve base-first, ten-header and final-field ordering, and return the same pointer. No reader allocation, stream attachment, host vector, pool context, destruction callback or injectable provider is needed. Because the exact base and header bodies supply no source-domain C++ throw, the prefix and outer-base exception cleanup paths are unreachable in this concrete operation. This closes construction within that declared domain without pretending to implement an arbitrary throwing-callback CRT iterator.

The source packet must retain the exception boundary above. To claim generic callback exception parity, separately implement or bridge the Win32 SEH iterator finally, the precise `E06D7363` array-unwind filter, and the real terminate path. For any native outer-unwind claim, preserve the `C07991` eligibility gate and the zero-base cleanup schedule, including the real `BF0700(...,0)` and CRT free boundary. Reuse the existing raw-context string destructor if a nonnull cleanup contract is in scope; its potentially throwing getter cannot be replaced with the `noexcept` host projection.

Run the required Win32 build and relevant existing checks only after the next source change. No source, tests, executable probes, runtime tests, game tests, shared export edits, merges or pushes were performed here. The report inventories every retained local artifact with both hashes and records exact matched spans, qualified direct/indirect call instructions, old metadata, proposed repair steps and remaining uncertainty.
