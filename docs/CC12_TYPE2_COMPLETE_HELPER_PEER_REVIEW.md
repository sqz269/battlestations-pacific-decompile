# CC12 Type 2 complete-helper artifact peer review

The independent artifact review passes for Root's sealed `local/t2p5` run of
`008ef1b0`. This peer review adds **0 Source** and performs no target execution,
compilation, native or provider query, Ghidra mutation, or ledger change. Root's
separate Source admission is outside this peer's authority. The machine-readable
result is `reports/cc12_type2_complete_helper_peer_review.json`.

## Inputs and independence

Root's family contains exactly 127 artifacts plus `seal.json` (128 files); its
seal SHA-256 is
`77695fa9e0f34908b7b6d90221f3c688a564a33c5b43691d06c11525ec068eb3`.
Before and after the review, fresh hashes verify all 128 files, all 20,715
historical pins, and the unchanged Type 5 draft (20 files) and Type 9 draft
(16 files). The union contains 20,879 distinct files.

New peer-owned Python parsers read raw COFF structures, the linker map, the PE32
image, the packed gate, serialized captures, and sealed frozen provider DLL
bytes. Capstone independently decodes the linked instruction bytes. Root's
recipe, manual/decode scripts, serialized COFF/static-gate conclusions, and
previous accepted helper implementations were not imported or executed. Root's
four fresh object files were inspected as data, never linked or executed here.
The peer family retains its parsers, receipts, full decoded listings, errors,
and logs under the named worktree's ignored `local/t2peer` directory.

## Complete linked code and frontier

The four fresh translation units define 32 physical bodies: constructor 1,
duplicate 2, canonical adapter 9, and probe 20. Actual COFF weak externals add
two alias names, yielding 34 logical function symbols. All whole linked sizes,
bytes, symbol resolutions, and relocation operands agree with the raw objects,
map, PE, and dynamically packed gate. There are **274 distinct physical
relocation operands and 280 symbol-level operand checks**; six checks repeat
through the two deleting-destructor alias pairs. The first peer parser stopped
on its own incorrect expectation of 280 distinct sites. Its failed script and
logs are retained; a separately retained recovery checks both counts. Root's
sealed artifacts were not changed.

All 47 code spans are nonoverlapping and byte-exact. They cover the 32 bodies,
the complete 14-byte security-cookie helper, the complete 43-byte stack probe,
and 13 complete six-byte import thunks. CFG ownership excludes the cookie's two
and stack probe's five trailing `CC` padding bytes. Duplicate `.idata$6` SECTION
labels are metadata; executable and IAT symbol resolution remains unambiguous.
The whole `main` is 3,301 bytes / 928 decoded instructions and the whole raw
capture wrapper is 140 bytes / 52 instructions. Ordinary, observer, gate reader,
hash, prefix-relocation, and provider-verifier bodies are included in the gate.

The only explicit unexpanded direct-code frontier is the cookie failure tail
to `___report_gsfailure` and the canonical deleting bodies' calls to sized
`operator delete` (`??3@YAXPAXI@Z`). The latter callers are exactly the
`??_E`/`??_G` alias pairs for `std::exception` and `std::bad_alloc`. Their complete
caller bodies are checked; the external sized-delete and GS-failure bodies,
exception paths, and EH behavior are unadmitted. Imported target bodies are not
implicitly admitted by validating their six-byte thunks.

The actual constructor is 60 bytes / 20 instructions. Only its call operand
at byte offsets 39..43 changes to reach the physical 57-byte / 30-instruction
duplicate helper. All other constructor bytes match the sealed original.
The duplicate's instruction stream, allocation/copy call binding, argument
cleanup, and return behavior are independently checked against its qualified
original bytes. The constructor returns with `RET 4`.

## Recorded invocation and observation evidence

The actual whole `main` verifies code and all four providers before the first
domain entry, observes each result before its child free, checks five entries /
three returned children / three canonical frees, and verifies all code and all
four providers afterward. The provider loops, actual call sites, full raw and
ordinary adapters, and observer code are decoded and gated, including the
zero-entry precondition for domain work.

All four serialized 80-byte captures are independently decoded into the actual
eight before registers, eight after registers, flags, argument slot, and value.
Captured `Q=00579364` is ESP before CALL; captured ESP after `RET 4` is
`R=00579368=Q+4`. The pre-argument R and target-entry `T=00579360=Q-4` are inferred
from the decoded wrapper, not falsely described as separately captured values.
For nonnull input, final defined arithmetic flags come from
`ADD(Q-44,16)`; the independent `8D5` mask includes defined AF. For null input,
the duplicate's XOR result uses mask `8C5`, excluding undefined AF. Nonnull
volatile ECX/EDX residual values are not prescribed.

The review checks every byte of all five 96-byte receiver snapshots, the full
48-byte guarded borrowed allocation, and all nine copied bytes
`BA 47 D9 2E F5 31 8A 6C 00`, including NUL. Payload bytes are also recovered
from actual main immediates. The receiver's 56-byte body retains all unwritten
poison bytes and its surrounding guards. Input and receiver allocations are
disjoint, each nonnull child is observed before its own free, and sequential
allocations may legitimately reuse child addresses.

## Provider evidence and limits

For `malloc`, `free`, `_callnewh`, and `memcpy`, the peer independently compares
the packed provider record, recorded IAT/export addresses, PE32/I386 export
RVA, module base, MEM_IMAGE/I386 checks in the whole verifier, physical NT and
mapped paths, file identity, file size, whole-file SHA-256, and all 32 prefix
bytes after the recorded ASLR base relocations. Recorded before/after values
agree. Frozen UCRT and VCRUNTIME DLLs are read as historical pinned data.
This corroborates Root's recorded provider evidence offline; it is not a new
live provider attestation.

The pass is limited to the sealed qualified successful/null raw domain and
its complete checked helper graph. It does not establish a complete class,
general string ownership, the game's private heap, exception compatibility,
live game execution, or gameplay. No build or test is added by this metadata
review; its focused checks are the new independent parsers and exact before /
after preservation audit. The final peer seal includes the failed accounting
attempt as well as both successful analyses and every helper/log artifact.
