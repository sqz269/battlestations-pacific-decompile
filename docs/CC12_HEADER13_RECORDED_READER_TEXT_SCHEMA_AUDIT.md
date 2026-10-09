No concrete serialized-field contradiction was found between the chosen Header13 probe TEXT, selected Q recorded reader and final generator candidate. The schema names, field order, widths and input bindings agree. This is a Source-credit-0 TEXT audit, with no reader/module execution, machine ABI recovery, provider query, process observation or receipt acceptance.

Chosen Q is `../battlestations-pacific-decompile-cc12_header13_independent_reader_text_correction/local/h13readerfix20261008a`:37 exact files. Its actual3312-byte `selection.json` SHA256 is `1f750fba27fcf8f358c5fd36c0cd8ee33ec35d7a45cf32920f78ca0dd4237849`. Chosen D is `../battlestations-pacific-decompile-cc12_parent_header_initializer_complete_helper_text_draft/local/h13draft`:39 exact files, seal SHA256 `aa812320a924f44415274a3524c29880b645cf228d0f1a1434d4e120b3986f45`. Candidate G is `../battlestations-pacific-decompile-cc12_header13_final_generator_text_proposal/local/h13gen20261008a/header13_final_generator.r01.py.txt`, SHA256 `88b0939b5e0ec8397bf41f0d5ea10acb031b339af606c766e7472c489d9b64a8`. The local audit freezes11 limited TEXT/selection/seal inputs and verifies every Q/D inventory association before and after.

| Protocol or binding | Writer/reader agreement |
| --- | --- |
| Probe stdout | Exactly three CASE rows, three UCRT rows and PASS; no JSON runtime from the probe. CASE order0,1,2 is selected raw Source DF1, raw Original DF0, ordinary Source DF0. UCRT order is malloc/free/_callnewh. PASS counters are3 allocations,3 targets,3 frees. |
| CASE | Ordered fields CASE/BASE/ROOT/REQUESTED/HEADER/DF/EDX/BUFFER/CAPTURE. BASE,ROOT,EDX are8 hex digits. REQUESTED44 and HEADER12 are literal labels. BUFFER is44 bytes/88 hex digits; CAPTURE is96 bytes/192 hex digits. DF and EDX columns print chosen case values, which Q separately checks against captured data. |
| UCRT | Ordered UCRT/IAT/EXPORT/RVA/MACHINE/FILEID/NT/FILE_NT/SHA/NORMALIZED_EXPORT_PREFIX32. IAT is the bound pointer value, not the executable IAT-cell address. FILEID is volume8hex:high8hexlow8hex; machine is literal014c; SHA and prefix are32 bytes/64 hex digits each. Paths are plaintext wide-printf substrings, not JSON strings; Q's delimited capture admits spaces and backslashes. |
| JSON records | Candidate writes process_args.v1, process_exit.v1, runtime_evidence.v1 and frozen_provider_evidence.v1 with all Q-read keys. JSON uses UTF-8/LF and standard escaping. Saved stdout is UTF-8 encoded from the subprocess-decoded string; this audit asserts no raw console encoding. |
| Input configurations | Eleven base artifact keys plus static_evidence make12 code keys. Nine recorded additions make21. Runtime input_pins has13 keys, exactly Q's required set. Relative pins use relative/size/sha256; reader summary pins use path/size/sha256. Candidate identity and Q selection match, including source hashes and three object basenames. |
| Post/seal | Candidate records a process result before success checks. Q requires exit0 and complete success plaintext; diagnostic FAIL output is not a success schema. Q summary emits every field read by candidate seal. Actual external reader-execution records, optimization preflight and before/after reader/interpreter pins are required from Root's external workflow; they are not probe-written fields or evidence produced here. |

The96-byte record is CaptureBox, not a96-byte Capture and not44 bytes of pre-register data. Capture itself is88 bytes. Q's `<24I` decoding matches the selected declarations:

| CaptureBox field | Byte offset | Bytes |
| --- | ---: | ---: |
| pre canary |0|4|
| before[8] |4|32|
| after[8] |36|32|
| flags_before / flags_after |68 /72|4 each|
| stack0 / stack1 |76 /80|4 each|
| es_before / es_after |84 /88|4 each|
| post canary |92|4|

The separate44-byte caller allocation contains pre16/header12/post16, with ROOT=BASE+16 and the12 header bytes zero after its selected entry. Each CASE serializes only its current live buffer; selected probe code also checks the other live buffers, which are not additional serialized snapshots. PASS counts are counters, not an independent heap trace. No adjacent heap metadata or retired-buffer observation is implied.

Observed versus derived labels matter. ES values are recorded in the capture, and Q adds equality plus16-bit-range assertions for these three selected cases; that is not blanket segment ABI. Q's arithmetic flag mask is`8C5=44`, with AF observed but excluded, while DF equality and additional non-arithmetic flag assertions are selected recorded-case predicates. Frame names R/Q/T and the ordinary inner frame are decoder interpretations, not separately emitted target-entry pointers and not validated by this schema audit. The Source target label comes from the selected probe order and static/gate binding, not a per-CASE target-address field. HMODULE and MEM_IMAGE are not separate stdout fields: module base is derived as IAT−RVA, while MEM_IMAGE is a check in selected probe code. FileID values are compared with frozen evidence; neither reader nor this audit re-queries physical identity.

`NORMALIZED_EXPORT_PREFIX32` means physical export bytes adjusted to the recorded module load base, despite its name. The selected probe and Q raw-format reader use that same representation. It is not normalization back to the preferred image base. Actual providers, emitted code, pointer/frame validity, complete gate coverage, external execution records and final approval remain Root prerequisites.

Frozen evidence locations are `local/h13schema/schema_comparison.json` and `receipt_schema_supplement.json`. TEXT references: D probe22-27,267-298,326-386; Q input_contract complete, capture_provider48-75,106-152,180-230, raw_formats86-104; G67-89,262-267,1431-1435,1523-1525,1583-1619,1726-1817. All selected old bytes remain unchanged. The local seal includes utilities, preparation stops, console records, frozen inputs, metadata and commit snapshots. Existing qualified Source domains are unchanged; no new Source or reconstruction credit is requested.
