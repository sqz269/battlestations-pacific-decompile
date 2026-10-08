# Header13 published constructor code-gate audit

The published Root family `Main/local/h13p1` omits **10 normal-path linked helpers** from its runtime gate: the stack helper and nine import thunks. It also omits **6 cold helpers**: three direct standard exception/EH thunks and the separate library deletion chain. This audit recommends Root review/reopening of the fixture's complete-code-gate qualification. **Audit Source credit0**; published credit and ledgers are unchanged. Later clear70/callback families require their own artifact review; no blanket propagation is inferred.

The complete native constructor `[004B7EC0,004B7ECD)` remains **13 bytes/6 instructions**, literal `8bc133c98908894804894808c3`, SHA256 `1e09d34a9de7a48bd061703ef73f341df8596595468b282c5f19c36b6bf0b98d`. Native frozen bytes, independent raw leaf COFF, unique full linked13 at `2E001000`, and the existing serialized gate all match, with zero CALLs/relocations. All **26 retained function symbols /24 distinct TU bodies** from the three fresh TUs, including full2454-byte main and seven distinct standard exception bodies, are whole-gated by existing25 spans (24 TU plus cookie14). There is no missing emitted probe/capture body. Missing normal coverage is outside those TU bodies, in actual linked stack/import helpers.

Two weak externs are physically shared: `??_Eexception`/`??_Gexception` and `??_Ebad_alloc`/`??_Gbad_alloc`. Raw auxiliary TagIndex/characteristic is **1=SEARCH_NOLIBRARY**, not3=SEARCH_ALIAS. Actual fallback map address, full original/linked body and logical relocation equality are independently verified. The parser performs **191 symbol relocation checks** for **185 distinct physical operand sites**; the six additional checks repeat the two shared deleting-destructor bodies. Sizes, actual operands and every nonrelocated byte are compared against the linked PE.

The following table classifies every recovered omitted function/helper. Bytes are physical body / map span; alignment is explicitly separate. The stack body is43 bytes with5 alignment bytes, while its whole48-byte mapped span was absent. Unsized delete is one5-byte tail JMP followed by11 alignment bytes; its mapped16-byte span is shown without treating padding as executed instructions.

| Symbol | Linked VA | Bytes body/map | Classification |
|---|---|---:|---|
| `__CxxThrowException@8` | `2E0035B8` | 6/6 | cold_std_EH_import_thunk |
| `___std_exception_copy` | `2E0035AC` | 6/6 | cold_std_EH_import_thunk |
| `___std_exception_destroy` | `2E0035B2` | 6/6 | cold_std_EH_import_thunk |
| `__chkstk` | `2E0028C0` | 43/48 | normal_stack |
| `_memset` | `2E0035BE` | 6/6 | normal_import_thunk |
| `_BCryptOpenAlgorithmProvider@16` | `2E00283C` | 6/6 | normal_import_thunk |
| `_BCryptGetProperty@24` | `2E002842` | 6/6 | normal_import_thunk |
| `_BCryptCreateHash@28` | `2E00284E` | 6/6 | normal_import_thunk |
| `_BCryptHashData@16` | `2E002854` | 6/6 | normal_import_thunk |
| `_BCryptFinishHash@16` | `2E00285A` | 6/6 | normal_import_thunk |
| `_BCryptDestroyHash@4` | `2E002860` | 6/6 | normal_import_thunk |
| `_BCryptCloseAlgorithmProvider@8` | `2E002848` | 6/6 | normal_import_thunk |
| `_K32GetMappedFileNameW@16` | `2E002866` | 6/6 | normal_import_thunk |
| `??3@YAXPAXI@Z` | `2E002870` | 16/16 | cold_std_library_delete |
| `??3@YAXPAX@Z` | `2E002BD0` | 5/16 | cold_std_library_delete |
| `_free` | `2E0035D6` | 6/6 | cold_std_library_free_import_thunk |

Full linked instruction/direct CALL/JMP edges are recovered. Normal evidence includes `main -> verify_module -> hash_bytes -> __chkstk/BCrypt` and `main/hash_bytes -> memset`; mapped-file-name thunk is called by `verify_module`. Successful saved provider/guard/hash checks support those observer paths. The raw caller's target is indirect; its actual Source/Original/ordinary selections remain qualified by the preserved call table and saved observations, not guessed direct edges. Standard allocation-failure/exception and probe-failure paths are separately marked cold even when syntactically reachable through conditional control flow. Local FF25 thunks are exactly6 bytes and their physical IAT operands match actual PE imports; DLL implementation bodies are outside this audit.

The cold deletion chain is actual sized delete16 at`2E002870` -> unsized delete5 at`2E002BD0` -> free import thunk6 at`2E0035D6`, reached from exception/bad_alloc deleting-destructor bodies. These functions were not executed in the successful current allocation domain. The complete cookie14 is already gated, including its genuine named GS-failure tail edge. Generic GS report/EH/DLL bodies remain unexpanded and unadmitted. Library names are retained; no class-lifetime or heap-provider substitution is proposed.

Saved observations were independently decoded from the entire **96-byte serialized CaptureBox** and each entire **44-byte allocation**. Three actual canonical bases contain fresh/unowned12-byte headers at offset16, with16+16 guards inside the requested buffers. All12 zero stores/32 guard bytes, prefix/suffix capture canaries, full EAX root/ECX0/EDX seeds, nonvolatiles/ESP/ES, stack canaries and defined XOR flags verify. Cases are raw Source DF1, unmodified Original DF0 and ordinary Source DF0; `8C5=44` excludes undefined AF. SourceDF1 is limited to the zero-CALL leaf and restored before CRT. Historical actual malloc/free/new-handler IAT/export/NT/fileID/fullSHA/normalized32 observations are preserved and decoded; no new provider query is performed. Three allocations/entries/base-frees and inallocation guards remain the complete domain. No retired memory, adjacent heap metadata, populated-header reset, owning parent/97-array, sentinel, iterator/EH, private heap, World or game extension is admitted.

The current published report supplies Root seal SHA `3a0957e45617408c9fce00c33892cb8ca55a1fc507104567e66b27ffaacd3f11`; exact133 artifacts+seal134 and older16385 pins are verified. Rootclear377, callbackworker424, current pure-text h5draft21 and all their original historical associations are unchanged. The deduplicated before/after union is **21343 pins**; historical external Root sealer is also checked separately. Mutable Main publication is copied at generation and treated as historical context, not a live metadata postpin.

No old recipe/helper/stage, compiler, Native/Ghidra/provider query or process was replayed. New independent parser/cold-boundary/classification helpers operate only on sealed artifacts and saved output. No Source/shared/config/ledger edits or new tests are made. Root alone decides credit correction and any new complementary family; accepted old artifacts stay immutable.

Audit family `J:/PROG/battlestations-pacific-decompile-cc12_parent_header_constructor_code_gate_audit/local/h13cov` contains exact**20 listed+2 seals=22 actual files**. Receipt SHA256 `53a4d1d0af4ee38ba9d8aa7117ad56c98a7faccd1445f947b97d692f51e41ab5`; manifest SHA256 `5b19953e215d42d32532be6c2ef1d28706adc59fc75bba0f8ab1dd9a21c3aeb8`. Only exact root receipt/manifest are excluded; all new helper/log/frozen files are listed, and seal console output stays external. The JSON report links the full per-symbol body/relocation graph, every omission and complete decoded observations.
