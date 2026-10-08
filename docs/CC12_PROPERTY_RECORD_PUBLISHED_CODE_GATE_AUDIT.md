# Published property-record runtime-code gate audit

Source 0; this is an artifact-only audit with no credit correction or new native execution. All four published Root fixtures omit emitted normal-path validation helpers. Their checked production Source constructors/providers are covered, but the surrounding probe/gate/capture/serialization coverage is incomplete. Root alone decides credit corrections and authorization for new complementary families.

| Family | Retained symbols / distinct bodies | Runtime spans | Covered TU bodies | Missing normal bodies | Missing cold std bodies | Other cold omissions |
|---|---:|---:|---:|---:|---:|---:|
| Type2 | 33 / 31 | 11 | 8 | 16 | 6 | 1 |
| Type4 | 31 / 29 | 9 | 8 | 13 | 6 | 2 |
| Type5 | 37 / 35 | 14 | 10 | 18 | 6 | 1 |
| Type7 | 35 / 33 | 9 | 8 | 17 | 6 | 2 |

All missing normal bodies are reachable through actual emitted direct CALL/JMP edges from `_main` after excluding `fail`, stdexception and compiler EH paths. Existing probe text distinguishes successful gate, input/capture verification, artifact output and serialization from error returns. Type2 `_main` is wholly absent from its serialized gate: VA `2C002230`, 3543 bytes and 986 gapless instructions. The other three complete `_main` bodies are covered.

Each family retains nine canonical standard exception symbols at seven unique bodies. The default `bad_alloc` constructor is covered; the six other bodies are missing. Actual weak `_Ebad_alloc`/`_Eexception` names share `_G` bodies and add no separate code. Those class/failure/EH paths were not exercised and receive no admission. The other cold omissions are `fail` in all four families and the `ordinary_original_call` EH handler in Type4/Type7. Standard library names remain unchanged.

Independent parsers read the raw I386 COFF symbol/section/relocation tables, exact linker function ownership and linked PE bytes. Whole emitted function sections decode without gaps; only actual four-byte COFF relocations are excluded from raw-vs-linked literal comparisons, with emitted target/address evidence recorded. All existing serialized gate spans (11/9/14/9) match their static JSON and linked image in full. No accepted parser, recipe, compiler, static stage, process, live Native/Ghidra query or rejected address was replayed. This audits retained TU functions; broad external library/class/EH closure stays outside scope.

Type2 exact diagnostics (20 rows, capped at 20; remaining cold details are in the sealed complete comparison):

| Exact symbol | VA | Bytes | COFF owner | Category |
|---|---|---:|---|---|
| `___local_stdio_printf_options` | `2C002220` | 6 | `probe.obj` | normal_helper_or_main |
| `_main` | `2C002230` | 3543 | `probe.obj` | normal_helper_or_main |
| `_printf` | `2C003030` | 47 | `probe.obj` | normal_helper_or_main |
| `?add_flags@@YAIII@Z` | `2C001240` | 113 | `probe.obj` | normal_helper_or_main |
| `?at_rva@@YAPBEABUModuleEvidence@@II@Z` | `2C0012C0` | 278 | `probe.obj` | normal_helper_or_main |
| `?disjoint@@YA_NIIII@Z` | `2C0013E0` | 58 | `probe.obj` | normal_helper_or_main |
| `?export_prefix_equal@@YA_NAAUModuleEvidence@@@Z` | `2C001420` | 620 | `probe.obj` | normal_helper_or_main |
| `?hash_bytes@@YA_NPBEKQAE@Z` | `2C0016C0` | 337 | `probe.obj` | normal_helper_or_main |
| `?live_code_equal@@YA_NABUGate@@@Z` | `2C001820` | 150 | `probe.obj` | normal_helper_or_main |
| `?nt_headers@@YAPBU_IMAGE_NT_HEADERS@@ABUModuleEvidence@@@Z` | `2C0018C0` | 81 | `probe.obj` | normal_helper_or_main |
| `?observe@@YA_NABUBorrowedText@@0ABUReceiver@@PBDPAXAAUObservation@@@Z` | `2C001920` | 488 | `probe.obj` | normal_helper_or_main |
| `?physical_export@@YAIABUModuleEvidence@@PBD@Z` | `2C001B10` | 389 | `probe.obj` | normal_helper_or_main |
| `?read_bytes@@YA_NPAX0K@Z` | `2C001CA0` | 56 | `probe.obj` | normal_helper_or_main |
| `?read_gate@@YA_NAAUGate@@@Z` | `2C001CE0` | 496 | `probe.obj` | normal_helper_or_main |
| `?verify_module@@YA_NABUCrtSpec@@AAUModuleEvidence@@@Z` | `2C001ED0` | 652 | `probe.obj` | normal_helper_or_main |
| `?wide_json@@YAXPB_W@Z` | `2C002160` | 181 | `probe.obj` | normal_helper_or_main |
| `??0bad_alloc@std@@QAE@ABV01@@Z` | `2C0010C0` | 48 | `canonical.obj` | cold_canonical_exception |
| `??0exception@std@@QAE@ABV01@@Z` | `2C001110` | 42 | `canonical.obj` | cold_canonical_exception |
| `??1bad_alloc@std@@UAE@XZ` | `2C001140` | 17 | `canonical.obj` | cold_canonical_exception |
| `??_Ebad_alloc@std@@UAEPAXI@Z` | `2C001160` | 45 | `canonical.obj` | cold_canonical_exception |

Type4 exact diagnostics (19 rows, capped at 20; remaining cold details are in the sealed complete comparison):

| Exact symbol | VA | Bytes | COFF owner | Category |
|---|---|---:|---|---|
| `___local_stdio_printf_options` | `23002060` | 6 | `probe.obj` | normal_helper_or_main |
| `_printf` | `23002C50` | 47 | `probe.obj` | normal_helper_or_main |
| `?at_rva@@YAPBEABUModuleEvidence@@II@Z` | `230011C0` | 278 | `probe.obj` | normal_helper_or_main |
| `?expected@@YA?AV?$array@E$0DI@@std@@EII@Z` | `230012E0` | 208 | `probe.obj` | normal_helper_or_main |
| `?export_prefix_equal@@YA_NAAUModuleEvidence@@@Z` | `230013B0` | 620 | `probe.obj` | normal_helper_or_main |
| `?hash_bytes@@YA_NPBEKQAE@Z` | `23001650` | 337 | `probe.obj` | normal_helper_or_main |
| `?live_code_equal@@YA_NABUGate@@@Z` | `230017B0` | 150 | `probe.obj` | normal_helper_or_main |
| `?machine_ok@@YA_NABUCapture@@PAXIIII@Z` | `23001850` | 244 | `probe.obj` | normal_helper_or_main |
| `?nt_headers@@YAPBU_IMAGE_NT_HEADERS@@ABUModuleEvidence@@@Z` | `23001950` | 81 | `probe.obj` | normal_helper_or_main |
| `?prepare_capture@@YAXAAUCapture@@I@Z` | `230019B0` | 101 | `probe.obj` | normal_helper_or_main |
| `?read_gate@@YA_NAAUGate@@@Z` | `23001A20` | 496 | `probe.obj` | normal_helper_or_main |
| `?save_blob@@YA_NPB_WPBXK@Z` | `23001C10` | 117 | `probe.obj` | normal_helper_or_main |
| `?verify_module@@YA_NABUCrtSpec@@AAUModuleEvidence@@@Z` | `23001C90` | 972 | `probe.obj` | normal_helper_or_main |
| `??0bad_alloc@std@@QAE@ABV01@@Z` | `23001040` | 48 | `canonical.obj` | cold_canonical_exception |
| `??0exception@std@@QAE@ABV01@@Z` | `23001090` | 42 | `canonical.obj` | cold_canonical_exception |
| `??1bad_alloc@std@@UAE@XZ` | `230010C0` | 17 | `canonical.obj` | cold_canonical_exception |
| `??_Ebad_alloc@std@@UAEPAXI@Z` | `230010E0` | 45 | `canonical.obj` | cold_canonical_exception |
| `??_Eexception@std@@UAEPAXI@Z` | `23001110` | 45 | `canonical.obj` | cold_canonical_exception |
| `?what@exception@std@@UBEPBDXZ` | `230011B0` | 14 | `canonical.obj` | cold_canonical_exception |

Type5 exact diagnostics (20 rows, capped at 20; remaining cold details are in the sealed complete comparison):

| Exact symbol | VA | Bytes | COFF owner | Category |
|---|---|---:|---|---|
| `___local_stdio_printf_options` | `2D002300` | 6 | `probe.obj` | normal_helper_or_main |
| `_printf` | `2D0033B0` | 47 | `probe.obj` | normal_helper_or_main |
| `?at_rva@@YAPBEABUModuleEvidence@@II@Z` | `2D001240` | 278 | `probe.obj` | normal_helper_or_main |
| `?close_module@@YAXAAUModuleEvidence@@@Z` | `2D001360` | 54 | `probe.obj` | normal_helper_or_main |
| `?disjoint@@YA_NIIII@Z` | `2D0013A0` | 54 | `probe.obj` | normal_helper_or_main |
| `?expected@@YA?AV?$array@E$0DI@@std@@EIII@Z` | `2D0013E0` | 177 | `probe.obj` | normal_helper_or_main |
| `?export_prefix_equal@@YA_NAAUModuleEvidence@@@Z` | `2D0014A0` | 620 | `probe.obj` | normal_helper_or_main |
| `?hash_bytes@@YA_NPBEKQAE@Z` | `2D001740` | 337 | `probe.obj` | normal_helper_or_main |
| `?live_code_equal@@YA_NABUGate@@@Z` | `2D0018A0` | 150 | `probe.obj` | normal_helper_or_main |
| `?machine_ok@@YA_NABUCapture@@PAXPBDIII@Z` | `2D001940` | 274 | `probe.obj` | normal_helper_or_main |
| `?module_unchanged@@YA_NABUCrtSpec@@AAUModuleEvidence@@@Z` | `2D001A60` | 244 | `probe.obj` | normal_helper_or_main |
| `?nt_headers@@YAPBU_IMAGE_NT_HEADERS@@ABUModuleEvidence@@@Z` | `2D001B60` | 81 | `probe.obj` | normal_helper_or_main |
| `?prepare_capture@@YAXAAUCapture@@I@Z` | `2D001BC0` | 104 | `probe.obj` | normal_helper_or_main |
| `?read_bytes@@YA_NPAX0K@Z` | `2D001C30` | 56 | `probe.obj` | normal_helper_or_main |
| `?read_gate@@YA_NAAUGate@@@Z` | `2D001C70` | 496 | `probe.obj` | normal_helper_or_main |
| `?save_blob@@YA_NPB_WPBXK@Z` | `2D001E60` | 117 | `probe.obj` | normal_helper_or_main |
| `?verify_module@@YA_NABUCrtSpec@@AAUModuleEvidence@@@Z` | `2D001EE0` | 972 | `probe.obj` | normal_helper_or_main |
| `?wide_json@@YAXPB_W@Z` | `2D0022B0` | 73 | `probe.obj` | normal_helper_or_main |
| `??0bad_alloc@std@@QAE@ABV01@@Z` | `2D0010C0` | 48 | `canonical.obj` | cold_canonical_exception |
| `??0exception@std@@QAE@ABV01@@Z` | `2D001110` | 42 | `canonical.obj` | cold_canonical_exception |

Type7 exact diagnostics (20 rows, capped at 20; remaining cold details are in the sealed complete comparison):

| Exact symbol | VA | Bytes | COFF owner | Category |
|---|---|---:|---|---|
| `___local_stdio_printf_options` | `29002240` | 6 | `probe.obj` | normal_helper_or_main |
| `_printf` | `29002FF0` | 47 | `probe.obj` | normal_helper_or_main |
| `?at_rva@@YAPBEABUModuleEvidence@@II@Z` | `290011C0` | 278 | `probe.obj` | normal_helper_or_main |
| `?close_module@@YAXAAUModuleEvidence@@@Z` | `290012E0` | 54 | `probe.obj` | normal_helper_or_main |
| `?disjoint@@YA_NIIII@Z` | `29001320` | 54 | `probe.obj` | normal_helper_or_main |
| `?expected@@YA?AV?$array@E$0DI@@std@@EPBI@Z` | `29001360` | 170 | `probe.obj` | normal_helper_or_main |
| `?export_prefix_equal@@YA_NAAUModuleEvidence@@@Z` | `29001410` | 620 | `probe.obj` | normal_helper_or_main |
| `?hash_bytes@@YA_NPBEKQAE@Z` | `290016B0` | 337 | `probe.obj` | normal_helper_or_main |
| `?live_code_equal@@YA_NABUGate@@@Z` | `29001810` | 150 | `probe.obj` | normal_helper_or_main |
| `?machine_ok@@YA_NABUCapture@@PAXPBIII@Z` | `290018B0` | 295 | `probe.obj` | normal_helper_or_main |
| `?module_unchanged@@YA_NABUCrtSpec@@AAUModuleEvidence@@@Z` | `290019E0` | 244 | `probe.obj` | normal_helper_or_main |
| `?nt_headers@@YAPBU_IMAGE_NT_HEADERS@@ABUModuleEvidence@@@Z` | `29001AE0` | 81 | `probe.obj` | normal_helper_or_main |
| `?prepare_capture@@YAXAAUCapture@@I@Z` | `29001B40` | 101 | `probe.obj` | normal_helper_or_main |
| `?read_gate@@YA_NAAUGate@@@Z` | `29001BB0` | 496 | `probe.obj` | normal_helper_or_main |
| `?save_blob@@YA_NPB_WPBXK@Z` | `29001DA0` | 117 | `probe.obj` | normal_helper_or_main |
| `?verify_module@@YA_NABUCrtSpec@@AAUModuleEvidence@@@Z` | `29001E20` | 972 | `probe.obj` | normal_helper_or_main |
| `?wide_json@@YAXPB_W@Z` | `290021F0` | 73 | `probe.obj` | normal_helper_or_main |
| `??0bad_alloc@std@@QAE@ABV01@@Z` | `29001040` | 48 | `canonical.obj` | cold_canonical_exception |
| `??0exception@std@@QAE@ABV01@@Z` | `29001090` | 42 | `canonical.obj` | cold_canonical_exception |
| `??1bad_alloc@std@@UAE@XZ` | `290010C0` | 17 | `canonical.obj` | cold_canonical_exception |

For any new authorized family, enumerate every retained map `f` body from every fresh TU, resolve complete COFF sections and weak aliases, deduplicate identical VA/size/bytes, then require coverage-set equality before launch. Include every normal/failure/EH/standard-exception body and actual map-owned cookie/stack/import helpers needed by that family. Retained byte coverage does not establish failure-path/class/EH execution. Keep the qualified raw storage domain and use new complementary cases only after Root authorization; preserve every old recipe, receipt and historical hash association.

Accepted exact inventories remain unchanged: Type2 101, Type4 342, Type5 362, Type7 349, plus hc70cov 46. Their prior books deduplicate to 15,677 checked immutable pins. Generation metadata is historical frozen context, not a live-main assertion. Five stopped read-only discovery/postprocessing attempts are preserved with original helpers and logs; recovery used new helpers without accepted-stage replay.

Immutable family `J:\PROG\battlestations-pacific-decompile-cc12_property_record_published_code_gate_audit\local\prcov`; exact inventory 106 listed + two seals = 108 actual. Receipt SHA256 `24c5c16d0552e4b57cf2828675ea8fd15cf855753b8fedef1acd6d3c766a0e82`; manifest SHA256 `748f6721d6dfabd0fd4d5aa40247636d825700a27a9a37e9dd10ac06fca160bd`; classified full-coverage SHA256 `b37495985db4f0e6e35d9c6e331b7edc9d191be1d7cb433d78e3f08bdf655dc3`. Only this document and its companion report are tracked.
