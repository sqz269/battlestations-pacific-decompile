# Loading tree and FileBlock owners integration BQ

Addresses: 00B7E000, 00B7E050, 00B7DF40, 00B7E740, 00B7CEF0, 00BE0A30, 00BDCB30, 00BDEBE0, 00504790, 00CC6740, 00CC6748, 00CC6753, 00CC6210, 00CC6218, 00CC6223, 00C68F90, 00C68F9B, 00C68FB4

This batch adds nine complete ordinary bodies across two source files: actual parser/cache head allocation and parser lookup/predecessor, plus FileBlock construction/destruction/scalar deletion and loading-job preparation. The validated source is `fa879ae60afb839ac011dbc11c29c4830ea1cd81` with 2618 recorded build inputs. Detailed contracts are in NATIVE_RESOURCE_MANAGER_TREES_BQ.md and NATIVE_FILEBLOCK_OWNER_BQ.md; exact receipts are in reports/native_loading_owners_integration_bq.json.

The native audit verifies 945 source bytes, all 328 source instruction starts and 28 compiler-support instruction starts. Thirty-three direct transfers pass the whole-report verifier. Both source files pass strict MSVC Win32 compilation, three focused source fixture modes pass, and the combined build and both existing tests pass. No new permanent test cases were added.

The tree source preserves actual storage and borrowed parser pointers. Head allocation does not initialize key/value fields or complete sentinel setup. FileBlock preparation allocates before substring extraction, publishes the owner before returning the temporary, and destruction reloads current VFS publication. The normal FileBlock fixture exercises raw string-pool/gate/list behavior with observation disabled. A deliberately throwing observer tests retained partial source state only.

Retained FileBlock frames do not execute the original outer FH3 cleanup. The packet documents the native name/base/temp/allocation actions and the difference explicitly. NativeStringStorage and BP/BO dependency-internal lifetime limits remain. Production loading-queue binding, actual resource-manager construction/registration, original ABI and gameplay remain incomplete.

Ghidra has nine reviewed source annotations, three defined FH3 dispatch handlers and six preserved compiler unwind names/comments. Returning-free epilogues in C68F90 and BDEBE0 were repaired with individual instruction ownership verified. The project was saved and exports refreshed. Zero-gap summaries alone did not prove those epilogues belonged to their functions.

The stopped tree worker's two source drafts were recovered and completed; the stopped FileBlock packet had analysis only and was implemented locally. Original worker worktrees remain intact. Incoming main `01e5f4f9cfb05f8887ba12ea185c293dd75fdf0c` was reviewed for integration overlap and leaves 18 borrowed source/header receipts unchanged. Its gameplay and numbering runtime claims remain other-owner evidence. The continuing reconstruction goal remains active.

The final clean merge includes main `01e5f4f9cfb05f8887ba12ea185c293dd75fdf0c`. The combined build and both tests passed again, and the namespace-aware header check found no duplicate definitions. The final source manifest and artifact hashes supersede the first build while retaining its receipt.
