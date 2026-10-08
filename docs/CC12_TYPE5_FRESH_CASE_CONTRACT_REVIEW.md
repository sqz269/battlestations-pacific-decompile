# Type5 fresh-case contract worksheet

This Source/TEXT worksheet gives expectations only, with Source credit0. It does not inspect any active/accepted fixture directory or claim process observations. The exact selected probe SHA is `859e8d19d2c5f98ab23f18eadd191b3812f9b7517acde2187b36b0e5195e9b2d`; recipe SHA is `b09092f24d5c7ac0bc53319df6c7d588c58e1133efdd84682781174f97e27cc6`. Six current Source/header files and those two TEXT files are frozen unchanged.

The current constructor is `construct_native_scene_property_record_type5_storage_008ef2b0`. Its padded fastcall interface supplies fresh/unowned writable56 root inECX, incomingEDX unused, then TEXT atT+4, opaque WORD18 atT+8, opaque WORD08 atT+C. RET12 returns fullEAXroot. Word names indicate storage offsets; they are not counts, ownership flags or floating-point claims. Root/text-through-firstNUL/activeframe must be disjoint and ranges/length+1 nonwrapping; require DF0 and successful currentprovider calls without failure/reentry/unwind. A general nullTEXT branch exists in Source but is not a selected case or extra target entry.

| Selected raw target | Full input8 | Expected child6 includingNUL | Later input bytes preserved, not copied | WORD18 | WORD08 | Root poison |
| --- | --- | --- | --- | --- | --- | --- |
| Source | `D7329A61E400BC58` | `D7329A61E400` | `BC58` | `3D92C7A4` | `A5E1703C` | `A6` |
| bound Original | `4BC625F89300E17A` | `4BC625F89300` | `E17A` | `C26D385B` | `5A1E8FC3` | `59` |

Both inputs have their firstNUL at offset5. The raw `duplicate_native_string_00438e40` scans that boundary, allocates length+1=6 and copies all6 bytes, includingNUL, through the private CDECL size bridge and genuine canonical malloc/new-handler service. The original TEXT remains borrowed; the newly returned childbase is published at+1C. The trailing two bytes do not become child bytes and both complete input8 spans stay unchanged. Heap ownership follows successful actual current allocation, not marker2C, opaque words or poisonowner. The private adapter has no standalone Native credit; this worksheet neither grants nor changes standalone duplicate admission. Its ECX/noargs/plainRET entry is distinct from the older CDECL API.

All roots start as56 poison bytes. Writes33 bytes occupy half-open `[00,0C),[18,28),[2C,2D),[34,38)`, preserving23 in `[0C,18),[28,2C),[2D,34)`. PhaseDWORD00 is literalCE89D4, tag04 is5, WORD08 goes to08, WORD18 to18, actualchild to1C, DWORD20/24/34 are0 and byte2C is1. OwnerDWORD30 staysA6A6A6A6 or59595959; uppermarkerbytes2D..2F staypoison. These are partial rawstorage writes, not completeclass/owner initialization. WORD08 is read before savedESI changesESP; WORD18 is read after save, and bothwords/phase/tag are stored before the duplicate call. Child1C stores fullEAX, then finalXOREAX provides zeros, marker/return/epilogue. No phase dispatch or automatic destructor is supplied. The JSON contains every56-offset mask and complete symbolic-pointer root template.

Each actual local `GuardedText` is40 bytes: pre fourDWORDs0..15, input8 at16..23, post fourDWORDs24..39. Source preguards areE3911100..E3911103 and post7C621100..7C621103; Original preE3912200..E3912203 and post7C622200..7C622203. Both full40 objects, total80 bytes, remain unchanged after each target. Guards belong to these actual localobjects; no adjacentheap redzone/metadata or inside-root guards are invented. Both actual56 roots are checked in full whilelive.

`Capture` is132 bytes: pre16, `Registers`100 at16..115, post16 at116..131. All25 register fields are little-endianDWORDs. Offsets are decimal:

| Register field | Capture offset | Registers offset |
| --- | --- | --- |
| esp_before | 16 | 0 |
| esp_after | 20 | 4 |
| eax_after | 24 | 8 |
| ecx_after | 28 | 12 |
| edx_after | 32 | 16 |
| ebx_before | 36 | 20 |
| esi_before | 40 | 24 |
| edi_before | 44 | 28 |
| ebp_before | 48 | 32 |
| ebx_after | 52 | 36 |
| esi_after | 56 | 40 |
| edi_after | 60 | 44 |
| ebp_after | 64 | 48 |
| flags_before | 68 | 52 |
| flags_after | 72 | 56 |
| es_before | 76 | 60 |
| es_after | 80 | 64 |
| stack_guard0 | 84 | 68 |
| stack_guard1 | 88 | 72 |
| text_slot | 92 | 76 |
| text_word | 96 | 80 |
| word18_slot | 100 | 84 |
| word18_word | 104 | 88 |
| word08_slot | 108 | 92 |
| word08_word | 112 | 96 |

Capture fill isB8. Source tag100 hex gives preE3910100..03/post7C620100..03; Original tag200 gives preE3910200..03/post7C620200..03. Other live captures are compared in full. The rawtext seedsEBX57C29AE1, EDIB48630DF, incomingEDX8AD5F263; ESI is the actualcaptureaddress and EBP the actualrawframeP. `machine_ok` compares nonvolatiles before/after but does not separately assert seed constants. EAX equals the fullactualrootaddress; selected nonnull ECX/EDX provider residuals are recorded without guesses. Stack canaries are4E97C3A6 thenF261B80D atR andR+4.

R is capturedESP before targetarguments and equalsP-20 from rawtext. PUSH WORD08,WORD18,TEXT derivesQ=R-12 and actualCALL derivesT=R-16. Capture132 has no Q/T fields. AfterRET12, ESP returnsR; dead argument slots areR-12/-8/-4 (T+4/+8/+C). FullTEXT pointer and bothopaqueDWORDs are read beforePUSHFD can overwrite the lastdeadslot. Actual fresh raw/ordinary/Main machinecode must establish that transport; no serialized entryT or fixedaddress is invented.

FinalconstructorXOR defines CF/OF/SF0, ZF/PF1, leavesAFundefined and overwrites provider arithmetic residuals. The selected predicate is **EFLAGS_after &8C5 ==44**, excludingAF; Type8's8D5 CMP/ADD expectation andT-based ADDformula do not apply. RawCLD establishes entryDF0; afterDF0 is a selected actualcurrentprovider predicate, not a blanket DF/ES/FPU/MXCSR preservation claim. ES before/after is recorded, without a `machine_ok` equality branch; a separately reviewed recordeddecoder may compareES, but is uninspected here.

Future successful schedule: gate actualcode/providers, allocate two actual56 roots, initialize both input40/capture132 objects and poisons, callSource then boundOriginal once each. Each call creates one actual6 child. AfterSource, compare root0/child0, root1poison, bothfull40 inputs and othercapture; afterOriginal compare bothcomplete roots/children/inputs and othercapture. Save bothcapture132/root56/input40/child6 artifacts whilelive. Free Sourcechild0, Originalchild1, actualroot0base, actualroot1base once each: four explicit canonical allocations/frees. Borrowed TEXT is neverchild-freed. Original63 VirtualAlloc/RX/VirtualFree is separate OS storage, not part of that4. Afterfree onlysavedaddress bits/captures and stilllivecode/module evidence may be read. No retiredobject read or foreveruniqueaddress assumption follows. Bothordinarywrappers are retained code-only; no dynamicordinarytarget entry is added.

No selected case, NULcopy, opaque-word or root-mask disagreement was found. Review limits are concrete: ES is recorded but not a probe equality assertion; AF must remain excluded even in an independent decoder; nonvolatile equality is not a seedvalue assertion; Q/T are derived, not captured. `machine_ok` additionally requires TEXT>=EBP+32 and root disjointR-16..EBP+32. That numericplacement exceeds Source's general disjointness predicate and needs actual Main/frame addresses. Recipeordinary checks oneCALL/plainRET and selectedopcode exclusions; alone those do not prove ECXreceiver and TEXT/WORD18/WORD08 transport. Root must review full freshlyemitted ordinary/raw/Main bytes and any independent recordeddecoder predicates.

The selected future recipe uses `manual_static_review.json` with key`passed`, plus exact EXE/gate/coverage/external-helper/build/object and recipe/helper/probe bindings. These remain TEXT, neverimported/executed here. Root authors actualfinalgeneration/review/receipts and validates completefour freshTUs/helper/import code, Source63 and its soleCALLoperand35..39, boundOriginal63 to the same rawduplicate57, allprovider/caller/adapter bodies before0roots/targets and afterfourfrees. ActualmappedI386 malloc/free/_callnewh/memcpy IAT/export/MEM_IMAGE/NTphysicalfileidentity/fullSHA/ASLRnormalized32byte-prefix checks remain unperformed. Coldlibrarycode gating is not EH execution; privateOriginalCRT/class/refcount/clone/poolpublication/World/game remain separate. Any change to generated cases requires Rootreview, not silentworksheet reuse.

Baseline `6bc5aa26a6205ef603ef94b2e6965966cfa64704`; Main generation `6bc5aa26a6205ef603ef94b2e6965966cfa64704`. The matching JSON records eight exact pins, all fields/guards/predicates and Source/TEXT line references. `local/t5cases/receipt.json` binds its exactrecursive `artifact_manifest.json`; only those two exactroot files are excluded. Every actualutility/log/frozencopy/stop/commit record is included, with postconsoles outside. No Source/test/sharedledger edits, Native/emitted-control-flow/Ghidra/provider/compiler/target operation, accepted/activefixture access, oldhelper/reader import/execution or new admission occurred.
