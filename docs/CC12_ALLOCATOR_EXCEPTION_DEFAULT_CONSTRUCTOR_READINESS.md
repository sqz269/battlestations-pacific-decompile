# Allocator exception default constructor: bounded readiness

This read-only packet resolves `00BF632F..00BF633F` as **17 bytes and five
instructions**, ending in plain `RET`. Live metadata, the complete live
listing, live bytes and the original PE agree. The current private
`construct_base` comment extends through `00BF6340`, one byte beyond this
body; that adjacent byte was neither read nor interpreted.

The original clears two fields with actual DWORD read-modify-write `AND`
instructions and returns the receiver in `EAX`. The current private C++ helper
expresses volatile assignments and returns `void`. This is a Source-contract
gap requiring explicit treatment before a raw adapter is admitted. No emitted
default-helper body was reviewed, so an emitted mismatch is not asserted.

Baseline: `0bd8828b70d1c543e9e934778699e90aaf23193e`, containing the published
cleanup admission `9f055bd7b42ef14366fa32b49805c21d41ff1adc`. The only packet
outputs are this document and its JSON evidence report. There are no Source,
CMake, ledger, Ghidra, build, probe or test changes and no admission credit.

## Gate and complete body

Each live query used `bsp.py ghidra`, whose client verifies project `bsp`,
program `/battlestationspacific.exe`, x86 language and image base before use.
The configured project remains `C:/Users/sqz269/bsp.gpr`.

Fresh metadata names `LIBCRT_unmatched_00bf632f`, with saved prototype
`undefined LIBCRT_unmatched_00bf632f(void)`. Its body is the inclusive
17-byte extent above, with five instructions, one block, no edges and no calls.
The decompiler infers a fastcall receiver but displays `void`; the complete
listing supplies the actual `EAX` result. No saved name or prototype changed.

| Address | Bytes | Complete operation |
| --- | --- | --- |
| `00BF632F` | `8B C1` | `MOV EAX,ECX` |
| `00BF6331` | `83 60 04 00` | `AND DWORD PTR [EAX+4],0` |
| `00BF6335` | `83 60 08 00` | `AND DWORD PTR [EAX+8],0` |
| `00BF6339` | `C7 00 70 93 D6 00` | `MOV DWORD PTR [EAX],00D69370h` |
| `00BF633F` | `C3` | `RET` |

Whole owned body: `8bc18360040083600800c7007093d600c3`.
SHA-256: `3aed2bc04e920f18daf703e4b76ea0d60eb59e4977578ab35b6403cd5fd6ca70`.
All five physical instructions match the visible listing; no listing or flow
gap, hidden additional register input, x87 operation or unresolved child exists.

## Field order, accesses and machine boundary

Let `D` be entry `ECX`. First `EAX=D`; then the complete DWORD at `D+4`
is read and written as zero, followed by the complete DWORD at `D+8` being
read and written as zero. Only afterward does the function publish the raw
profile DWORD `00D69370` at `D+0`. The profile write does not read its previous
value. There are no other direct receiver accesses or pointer dereferences.

The minimum receiver backing is the leading 12 bytes: writable `D+0..3`,
readable and writable `D+4..7`, and readable and writable `D+8..11`. Neither
old field value influences control flow, but the architectural prior reads
remain part of the two memory `AND` operations. No `LOCK` prefix or atomicity
promise is added. No existing message is freed or read through, and no
receiver-null, ownership, self-alias or heap-validation guard is present.

The first successful field operation precedes the second, which precedes
profile publication. A fault at either earlier memory instruction prevents
the later instructions from being reached; this instruction-order observation
does not establish exception delivery, hardware-fault or concurrency parity.
No original handler or surrounding caller was inspected.

On ordinary return, `EAX=ECX=D`; `ECX` and incoming `EDX` are unchanged.
`EBX`, `ESI`, `EDI` and `EBP` are untouched. There are no explicit stack
arguments, pushes, local frame, child calls or x87 operations. Plain `RET`
reads the return address and advances `ESP` by four; it has no immediate
stack-argument cleanup.

The second `AND`, at `00BF6335`, supplies the final arithmetic flags:
`CF=0`, `OF=0`, `ZF=1`, `SF=0`, `PF=1`; `AF` is undefined. The subsequent
profile `MOV` and `RET` do not change those flags. Other flag bits are not
modified by this body. These facts describe the complete instruction sequence,
not the behavior of a Source caller or a binary replacement.

The profile immediate remains opaque numeric data. This packet creates no
Native exception type, callable vtable, RTTI, static owner, slot, throw-info
binding or pointee identity and reads no profile data.

## Current Source comparison

The actual private helper in `src/native_legacy_exception_owner.cpp` remains:

```cpp
void construct_base(NativeLegacyExceptionStorage& owner) noexcept {
    // Complete library constructor 00BF632F..00BF6340.
    volatile auto& actual = owner;
    actual.base_message_04 = nullptr;
    actual.owns_base_message_08 = 0;
    actual.native_vtable_00 = exception_vtable;
}
```

It states the same final field values and publication order for the actual
typed owner. Its assignments do not explicitly encode the two prior DWORD
reads, machine flags or `EAX` receiver result. Whether a particular compiler
chooses memory `AND` instructions remains unreviewed for this default helper.
Its `noexcept` is existing Source policy, not evidence of Original EH parity.

`NativeLegacyExceptionStorage` is the actual 40-byte Source owner, with fields
at `+0`, `+4`, `+8` and a member at `+0Ch`, backed by Win32 size/offset asserts.
Such an owner provides the required leading 12 bytes. No raw 12-byte backing
was cast to the larger type and no public raw default-constructor interface
was created. A future primary-selected adapter must preserve the complete
machine schedule and return contract while keeping any typed-owner policy
explicit; this audit does not choose or implement that interface.

## Version-qualified surrounding receipts

The latest cleanup primary report records the registered normal MSVC Win32
build, three existing checks, the complete 23-byte/seven-operation Source
adapter, four actual typed calls, Core review and final UCRT `free` import.
Its six-byte `FF15` call explains the extra Source byte relative to the
22-byte Original cleanup. Retained typed `noexcept` produces current compiler
FuncInfo flags `5` and terminate references around the raw non-`noexcept`
declaration. These are reviewed Source compiler effects, not Original EH proof.

The earlier message-constructor, failure-object-constructor and copy receipts
retain their separate admissions of 24, 25 and 88 Original bytes. They do not
establish this 17-byte default constructor. No prior routine is counted again.

All 169 source/document references from the four receipts reproduce their
respective committed historical versions. Of those, 165 also match this
baseline. Four historical references differ currently: three `CMakeLists.txt`
references and one legacy-owner reference. The report retains both hashes and
the exact historical commit for each reference. All 54 latest cleanup-primary
references match the current baseline. Historical receipts are not silently
treated as current-source checks. Their artifact pins, machine execution and
builds were not rerun by this packet.

The report separately pins the current helper/type, receipts, admission
document, target configuration and verification code, including exact source
excerpts. Original placement/ABI, caller and exception-runtime compatibility,
startup and gameplay remain unproved. Source admission and all implementation
work remain primary-owned.
