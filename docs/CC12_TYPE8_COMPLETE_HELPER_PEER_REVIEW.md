# Type-8 complete-helper correction: independent artifact review

The peer review passes for Root's sealed `local/t8p2` correction. A new parser
independently checked the actual COFF objects, linker map, PE image, `gate.bin`
and serialized observations. Every one of the 29 retained functions emitted
by the three translation units has a complete gate span. All 33 spans agree
with the linked bytes, and all 280 COFF relocations resolve exactly.

**Source credit remains 0 for this review.** Root controls admission and
integration. This is an artifact review; no compiler, native executable,
accepted Root helper, prior phase or Ghidra query was run. No C++ source was
semantically queried or changed. Hash reads preserve the explicitly requested
older files without reopening their implementation contracts.

## Independently parsed code coverage

The review uses direct Python `struct` parsing for PE, COFF, linker-map and
gate serialization, with Capstone for complete instruction decoding. It does
not use Root's serialized COFF/static-gate/coverage results as the checker.
The gate header, 33 variable-length code spans and four 104-byte CRT records
consume all 11,031 bytes of `gate.bin`, with no trailing bytes.

| Emitting translation unit | Retained functions |
| --- | ---: |
| Constructor and its private adapter | 2 |
| Canonical provider and retained support | 9 |
| Probe, verifier and serialization helpers | 18 |
| Total retained functions | 29 |

Each retained COFF function starts at its declared section offset, has the
same complete extent in the gate and linked PE, and resolves every DIR32 or
REL32 relocation against an unambiguous map symbol. Every other byte is
literal. Gate spans are distinct and nonoverlapping. Whole main has 6,083
bytes, 1,549 instructions and 192 relocations.

The four additional spans are the complete 14-byte security-cookie helper,
6-byte memcpy import thunk, 6-byte cold throw thunk and 48-byte map-owned
chkstk extent. The cookie includes its successful RET and failure tail JMP
to a separately named external function. Chkstk includes its probing loop,
RET and five alignment INT3 bytes; the map exposes `__alloca_probe` and
`__chkstk` as aliases of the same address. No external failure or EH body was
expanded or admitted. Other system/library boundaries remain outside the
claim of 29 retained TU bodies plus these four named spans.

Whole constructor `[008EF2F0,008EF360)` is 112 bytes and 34 instructions.
SHA256 is `4c3786af3a642703dc38315ae63d9dee0468552f47a03bc1405fc0e90ab5c928`.
Only CALL operands `[47,51)` and `[62,66)` differ between Original and linked
Source/bound Original. All 104 other bytes match. Both calls resolve to the
same actual private adapter and memcpy thunk. The first instruction tests
the low flag byte before root writes; both exits use `RET0C`.

Source is at `37001040`, its actual CDECL adapter at `37001000`, canonical
allocation at `370011B0`, canonical free at `37001210` and memcpy at
`3700462C`. The adapter's relocation resolves to canonical allocation;
canonical allocation/free resolve to their actual malloc/new-handler/free
IAT slots. Cold support remains gated as bytes without EH admission.

## Captures and complete storage

The Type-8 entry order is **data at T+4, byte count at T+8, full flag DWORD at
T+C**. The peer did not apply Type-9's count/data order. Actual capture records
give R before argument pushes `0012906C`, Q immediately before CALL
`00129060`, and T on entry `0012905C`. Returned ESP equals R.

The complete raw caller is 191 bytes/68 instructions. Its three pushes,
independent Q/T capture and indirect call agree with the physical slots.
After return, GPR and all three dead-slot observations precede PUSHFD;
only MOV/LEA instructions occur before that flags capture. Current seed
values are recovered from the binary: EBX `E2734A91`, EDI `B6D815E3`, incoming
EDX `F43B9726`.

| Case | Byte count | Full flag word | Flags under mask 8D5 |
| --- | ---: | --- | --- |
| Source retained | 5 | `A791BD00` | `044` |
| Original retained | 8 | `43D8F600` | `044` |
| Source copied | 8 | `FC268701` | `000` |
| Original copied | 5 | `67B9A0FE` | `000` |

All four 140-byte captures pass guards, full argument DWORDs, stack guards,
EAX identity, nonvolatile registers, ESP transport and DF-clear checks.
Borrowing returns ECX=root/EDX=input; CMP defines AF zero and mask `044`.
Copy flags independently evaluate the actual final ADD:
`00129044 + 10h = 00129054`, giving mask `000`. Copied ECX/EDX residuals and
ES observations are recorded without preservation assertions.

All four complete 56-byte roots match phase/tag 8, zero fields, actual pointer,
exact byte count and byte `+2C=1`. All 27 preserved bytes, including DWORD
`+30`, retain the case poisons `A9/73/D4/6E`. Both borrowing cases retain their
exact input address. Byte `+2C` does not distinguish ownership.

The complete 96 guarded input bytes were checked. The two 16-byte payloads
were independently reconstructed from unique groups of four consecutive
`MOV DWORD [ESP+offset], immediate` instructions in actual main. They are
`B84200D76C91E32500AF7934C25EF618` and
`4DE97300C628B15A00F3976E24D841AC`. The saved copied children equal the full
8-byte and 5-byte input prefixes. All six recorded allocations, four capture
buffers and both guarded inputs are pairwise disjoint and nonwrapping.

Main's linked control flow places code/provider gates before its first root
allocation (`3700236A`) and target call (`37002889`). Captures/payload saves
precede child free loop `37002FD4`, then root free loop `37002FF2`. Provider
checks and code gate `37003214` follow frees and RX release. The serialized
four CRT specifications agree with PE imports and Root's runtime records.
These are checks of recorded evidence and linked control flow, not a fresh
live-provider attestation or another observation of allocations/frees.

## Preservation and stopped attempts

Root's exact 404-file inventory is sealed by SHA256
`edae817c0fbc4d710e620dd54f02a547aca71f3e269ebc8ee5319360c0c241d7`.
All 403 listed artifacts plus the manifest, and all 18,537 older pins, were
checked. The peer's own new family records terminal checks and its exact
recursive seal; no ignored failure or pycache path is omitted.

Stopped peer parser attempts remain in the fresh family. The initial map
parser rejected repeated `.idata$6` section labels; the next classified
chkstk by only its first alias. A capture draft also used older EBX/EDI
seeds. Each was corrected in a new parser file. A planned contiguous-payload
assertion was stopped before execution when preflight showed separate DWORD
initializers. Successful code/capture checks were not rerun. None of these
stops identifies a defect in Root's immutable correction artifacts.

No shared metadata, credit, source or original analysis was changed. Native
private CRT/EH, class behavior, startup and gameplay remain unadmitted.
