# Parent header callback readiness (CC12)

This read-only audit establishes a complete ordinary raw-storage contract for
callback `004B 7EC0`: return the incoming receiver and zero its three DWORDs.
The result is compatible with the accepted empty 12-byte list-root layout under
fresh, unowned storage preconditions. **Source 0 / registered ready packets 0**;
the bounded leaf proposal awaits Root's independent review and registration.
Owning parent, iterator exception handling, sentinel, and class lifetime remain
unadmitted. No Source, build, fixture, native execution, or Ghidra mutation occurred.

Baseline: `92ca45788a7a8e84328e61296be7552a51849638`. One whole callback and one bounded actual
iterator invocation were inspected. The graph contains 11 nodes and 12 edges,
23 of the 24-node/edge budget. The former owner audit is frozen provenance;
its whole body was not queried again.

## Complete body and physical ABI

Complete half-open range `[004B 7EC0, 004B 7ECD)`: **13 bytes / 6 instructions**.
SHA-256 `1e09d34a9de7a48bd061703ef73f341df8596595468b282c5f19c36b6bf0b98d`.
Hex: `8B C1 33 C9 89 08 89 48 04 89 48 08 C3`.

| Address | Actual instruction |
| --- | --- |
| `004B 7EC0` | MOV EAX, ECX |
| `004B 7EC2` | XOR ECX, ECX |
| `004B 7EC4` | MOV DWORD [EAX], ECX |
| `004B 7EC6` | MOV DWORD [EAX+4], ECX |
| `004B 7EC9` | MOV DWORD [EAX+8], ECX |
| `004B 7ECC` | RET |

Entry ECX supplies actual writable 12-byte storage. EAX returns that full root;
ECX returns zero. EDX, EBX, EBP, ESI, and EDI remain unchanged. There are no stack
arguments, temporary stack stores, calls, heap providers, branches, or EH frame.
Plain RET at `004B 7ECC` yields return ESP equal to entry ESP plus four.

XOR is the only flag writer: CF=0, PF=1, ZF=1, SF=0, OF=0. Defined arithmetic
mask `0x8C5` yields `0x44`; AF is undefined and excluded. DF remains unchanged,
including DF=1, because this body has no string or CRT operation. Allocation and
node-provider phases separately require their valid ordinary DF=0 contract.
The stores use the actual writable DS receiver; no ES/string requirement is added.

All 12 bytes are written, in DWORD order `+0`, `+4`, `+8`, each with zero.
There are no untouched bytes inside the requested extent. Static whole-body
stores establish the write extent; this audit has no runtime adjacent heap
guards or heap-metadata claim. Resetting an already populated owning root would
discard links without disposing nodes and is outside the proposed fresh/unowned
contract. The callback itself allocates, owns, traverses, and frees nothing.

The historical endpoint `004B 7ECC` is inclusive. The first 12-byte record omitted
RET; it is retained as an incomplete prefix, with a separate complete 13-byte
record and extent correction. The live proto has a six-instruction function.
Old metadata saying no Ghidra function remains a frozen historical association,
without being presented as current state. No listing repair was needed.

## Genuine callback invocation

One live window `[00BF 7CE8, 00BF 7D03)` in the actual constructor iterator is
**27 bytes / 10 instructions**, SHA-256
`bd78961b5fb5c760a76724cdb9bcc4173415a74c8885f39051af695a881fbcaf`. It contains:

| Operation | Actual evidence |
| --- | --- |
| Count guard | MOV EAX,[EBP-1C]; CMP EAX,[EBP+10]; signed JGE BF7D03 |
| Receiver | MOV ESI,[EBP+8]; MOV ECX,ESI |
| Invocation | `00BF 7CF5` CALL DWORD [EBP+14], bytes `FF 55 14` |
| Next element | ADD ESI,[EBP+C]; MOV [EBP+8],ESI |
| Next index | INC DWORD [EBP-1C]; JMP BF7CE8 |

The CALL begins at window offset 13 and occupies `[13,16)`; its EBP displacement
byte is offset 15. It has no direct-target DWORD relocation. No argument PUSH
occurs between receiver load and CALL. The callback's plain RET requires no
stack cleanup, and the iterator ignores EAX while preserved ESI supplies the
next receiver.

The saved matched-library signature identifies the frame slots as destination,
stride, count, constructor callback, and destructor callback. The actual live
operand establishes `[EBP+14]`; the SEH prolog's construction of EBP is outside
the witness. This slot association is qualified historical signature evidence,
without whole iterator ABI or exceptional-cleanup admission.

The immutable owner descriptor supplies destination parent `+0x18`, stride
`0x0C`, count `0x61` (97), actual constructor `004B 7EC0` pushed at `004C B065`,
and actual destructor `004C 2D30` pushed at `004C B060`. Element index 1 is parent
`+0x24`. Callback order follows those physical pushes. No parent buffer or
semantic World substitute was fabricated, and no callback was executed here.

## Proposed bounded Source boundary

The proposed hypothetical interface is:

```cpp
void* __fastcall initialize_native_parent_list_header_storage_004b7ec0(
    void* actual_storage_root, std::uint32_t unused_edx) noexcept;
```

Both formals stay in registers; there is no stacked value/owner argument. A
literal 13-byte x86 body can preserve the actual GPR/flag/RET behavior without
globals, fake phase dispatch, heap bindings, or callback shims. Root alone may
register this ordinary supplied-storage leaf after independent review.

Accepted current append/erase/removal require an actual coherent 12-byte
`{count+0, head+4, tail+8}` root. These zeros supply its empty raw state. A future
connected family may obtain actual 12-byte roots from the canonical current
allocator, invoke this initializer, produce every node through real accepted
append Source, and dispose nodes through real accepted erase/removal Source.
Borrowed payloads retain their own live identity. Only an actual root that is
empty after node dispositions may be explicitly freed; freed nodes are never read.

Required future gates include whole Original/COFF/unique linked 13-byte equality,
zero calls/relocations, real ordinary and raw ECX/no-stack/plain-RET callers,
actual loaded I386 UCRT/IAT/export/physical-file identity before allocation,
strict current toolchain/header/library/Source pins, full 12-byte results,
EAX/ECX/EDX/nonvolatiles/ESP/defined flags/DF and external canaries, plus all
other live root/payload identities. No future recipe or executable was created
or run in this audit, and no old accepted family was replayed.

The whole iterator's `00C0 7C00` prolog, `00C0 7C45` epilog, `00BF 7D1E` cleanup,
and actual destructor failure invocation remain named Astra/EH prerequisites
if a complete iterator/owner binding is required. Owner sentinel `004C 3080`,
handler `00C6 5651`, original private heap, deleting lifetime, subject linkage,
registration, observers, and game phase remain unadmitted. These gaps do not
introduce a hidden call inside the complete ordinary 13-byte callback.

## Immutable evidence

All **9,621 prior pins** remain unchanged: the owner audit's 211 actual files
plus its 9,410 older pins. All 24 current inputs, 16 frozen historical files,
and two additional frozen iterator references match their bookends. Old
original-path/hash metadata remains historical provenance without repinning.
Each live batch verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
and installed PE SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Whole callback and invocation bytes match live/PE before and after.

The retained prefix and syntax-invalid `propose.py` are included. A new
`propose_v2.py` recovered the JSON-only error without query or native replay.
The ignored family `local/ph12` has **133 listed artifacts
plus two seals, 135 actual files**. Exact-root
exclusions are only the receipt and manifest; nested historical seals remain
included. Postprocessing consoles are outside the sealed root.

| Sealed file | SHA-256 |
| --- | --- |
| `proposal.json` | `78a474d57dc6886f68fc4263980132d625639cddc168fbe5ddfabf01beb6791b` |
| `readiness_receipt.json` | `a84da02497b92f34ed49ffde3bc2e8394913c8756d19db3fd43230dd12412a1f` |
| `artifact_manifest.json` | `1808a008f0f64f6c2c71a55bbbca4f74656f73a428fb8f8d884df4703fdfe1af` |

This evidence is static/read-only. Source compilation, fixture ABI/runtime,
whole native parent/EH/class ownership, startup, and gameplay are unvalidated.
