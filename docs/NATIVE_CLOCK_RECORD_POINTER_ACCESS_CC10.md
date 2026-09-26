# Native clock record pointer access CC10

This packet changes exactly three accesses to live `NativeRenderResourceRecord` pointer members: the last sink read in4DDA40, and the name/sentinel stores in4DC410. They now use typed volatile fields instead of `uint32_t` lvalues. Public headers, ABI shapes, CMake, tests and all other source operations remain unchanged. No object lifetime, count, ownership, default sink or failure cleanup is added.

The source baseline is `b1eeaa2780538f1697d588b2ced02d15be8c613d`. The exact reviewed diff and the frozen read-only proposal are retained under `local/clock_record_pointer_access_cc10/reviewed_readiness/`. Git's first attempt to apply that generated diff failed before source mutation; the original preparation script is preserved. The successful application made only the three reviewed byte replacements while retaining source line endings.

| Access | Native provenance | Source behavior |
| --- | --- | --- |
| Last record `resource_28` read |4DDA59, after count4DDA50/data4DDA53 | Compute the captured last actual record using the existing DWORD address arithmetic; read its live `void*` member once. |
| Record `name_data_04=nullptr` |4DC463 | Typed volatile `char*` store after name length zero, before name cleanup is armed and the genuine sentinel allocation. |
| Record `sentinel_0c` publication |4DC475 | Typed volatile node-pointer store immediately after4C3020, before alias count zero/current requested-word reload. |

The backing must already contain live, correctly aligned0x2C records. Existing reserve/append/growth paths default-placement-construct that actual type. This patch does not adopt arbitrary bytes or make a growth-created record into an initialized sink record: unknown08 and resource28 remain untouched during growth.

4DDA40 still reads current count, current data and the last sink before current profile/+10 slot. The current decrement callable executes once; only zero reaches the SAME canonical companion. After release returns, current count and conditional current data are reread, the current last record is destroyed, then the existing single-memory count decrement occurs. No captured pre-release record is reused for destruction, reference added, field reset or current profile cached.

4DC410 still reserves before its fresh count read and uses current data per iteration. Name length/pointer initialization precedes the armed sentinel allocation. Sentinel publication, alias count zero, current public requested-word reload and payload stores24/20/1C/18/14 retain their order. Its source catch still cleans the current name, rereads current data, retains the placement-address write/read and rethrows. Existing incomplete construction/copy/backing obligations and second cleanup failure termination remain unchanged.

The original entry ABIs remain ECX secondary/RET0 for4DDA40 and ECX vector, stack signed count/RET4 for4DC410. The reconstructed interfaces add explicit source context. Typed source access, strict compilation and emitted order are not original native EH/SEH or binary-entry compatibility proof.

## Compiler evidence

One strict MSVC Win32 Release build (`/W4 /WX /fp:strict`) and all three existing CTests passed. No new test, probe, controller or runtime ran. Build-time source/config/tool pins precede the build; raw build log, initial exec/wait receipts, exact output library/objects, selected AR members and link map are frozen.

The old current library members were copied before rebuilding and verified byte-for-byte against their then-current object files. They are historical09ed fixture-era artifacts; the new build is qualified against its own34 pinned inputs. The comparison spans the previously published canonical-header/access changes as well as this three-access patch. It does not attribute all compiler differences to this patch or require old/new byte identity.

| Evidence | Result |
| --- | --- |
| Old/new full COFF |29 old,30 new BSP function sections, including helper, catch/unwind and EH handler code; complete old/new raw members and section listings/relocations retained. |
| Matched symbols |28 matched;25 code arrays equal,3 different;26 relocation lists equal. Old raw pointer helper disappears and two typed current-data helpers appear. |
| Changed code arrays |4DE290 container307→306B; resize body317→315B; private resize EH handler29→29B with different code bytes and equal relocation list. |
| Actually linked code |18 unique full sections match the current application link outside relocation fields;39 resolved relocations match exact map targets, no unresolved target. |
| Link limits |Standalone4DDA40 is195B complete COFF with no unique link match. Private29B EH handlers have ambiguous identical linked matches and remain COFF-only. The inlined sink sequence in the linked306B container is inspected separately. |
| Order checks |28 byte/relocation checks cover sink capture/current reads, name/sentinel/payload stores, source catch/rethrow, normal guard disarm/free and failure cleanup/termination. |

The full rebuilt COFF bodies all decode; the historical raw objects and full sections are retained independently. Raw objects preserve compiler EH metadata. Code/relocation inspection does not prove native FH3 semantics or execution of those failure paths. The actual18 linked sections/39 target checks are distinct from the broader COFF comparison.

A separate incremental comparison uses the accepted nine-header-access packet's immutable new members, archive SHA256 `82549f7bd2f2d4d0b86fbffcfa944c88bca15fd96fe37dabbc2de447e9d90b72` and manifest SHA256 `08a8571b48caf375de140106583e0a649fd0d9e7856b460214ab227979d0f087`. Its two build-input source files match this packet's reviewed pre-change sources after newline normalization. All30 complete code arrays match the rebuilt members exactly;19 relocation lists match exactly, and all30 match after pairing only the cross-worktree MSVC anonymous-namespace discriminators. Original names, offsets, relocation types/targets, raw members and full section listings remain available. No code byte or address is normalized. The initial exact-private-name pairing refusal is preserved separately. This comparison isolates the three source access corrections: they produce the same Win32 code as the accepted current-source baseline. The historical09ed307→306/317→315/privateEH differences are not their incremental compiler delta.

Six original bodies totaling557B match installed PE bytes:4D45A0[120],4DDA40[84],4DDAA0[23],4DE290[95],4DDB40[31],4DC410[204]. Their saved read-only Ghidra queries verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`; installed bytes were rechecked during this packet. These are reused existing bodies, with zero newly credited native bytes and no Ghidra writes.

## Remaining composition boundary

Actual record copy/destruction, constructed alias-node fields and generic string-header byte-copy helpers keep their existing live-type contracts. Name/alias buffers must use the SAME actual pool publication/manager/gate and allocation sizes; providers, canonical registry, companions and synchronization must outlive manager drain. Canonical zero dispatch borrows the exact live aligned+4 atomic and genuine terminal/profile, with no second decrement or post-terminal access. The actual Win32 decrement callable is implementation-specific source ABI evidence, not the original numericIAT identity.

Cache-operation record/string pointer accesses remain a separate caller packet, including `native_sampler_cache_operation.cpp` resource/sentinel reads/stores. Genuine positive-child producer admission, initialized resource28, exact counts/ownership, current profiles and full nonempty composition remain open. No fake sink, count, source0/W, platformMSG or default behavior is supplied.

All1,684 historical09ed launch pins, the reviewed68-row readiness archive and the preserved prior preparation/runtime/provider archives remain unchanged. Historical empty-clock execution is not new-build or nonempty validation. Exact source/compiler evidence, the report, artifact index and external archive hash receipt reside under `local/clock_record_pointer_access_cc10/`.
