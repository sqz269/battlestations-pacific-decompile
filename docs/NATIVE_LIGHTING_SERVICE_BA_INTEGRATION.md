# BA raw ambient, registry and texture storage

Addresses: 00b7c290, 00b3ce70, 00b3ce80, 00b14500, 00b14590, 004324a0, 00432050, 00b82390, 00b823b0, 00b82570, 00b51fc0, 00b52040, 00b52170, 00b521e0, 00b523c0, 00b523e0

Five modules implement 15 new complete native entries and extend one existing
ambient constructor with an actual raw-storage interface. The 16 entries total
1219 native bytes and include one five-byte forwarding thunk. Existing ledger
identities and prior evidence remain; one analysis-only EH definition and flow
repairs add no reconstructed entry count.

Reviewed source `15210f771f9e162fb081e0ca4dd293c564e6258b` was merged with main.
Exact combined source `b768339463e7017192a26a08d4d7d689321982b3` passed the strict Win32 build, both
existing CTests, four probes linked only to the three current libraries, and
complete emitted-byte checks for both saved-dimension leaves. All 16 original
analysis signatures and complete stored ranges were saved and verified;
all 50 numeric call rows pass. Registry member conventions remain inferred.

- ambient: One original-byte fixture: 948 comparisons over 61,390,848 arena bytes, covering 8 current-constant patterns, 3 MXCSR modes, 39 layouts and 12 write/read frontiers. The 393 original parent bytes are unchanged. The aggregate log does not serialize 948 individual arena snapshots.
- registry: 9 original/source pairs, 932 compared bytes and two allocation CALL relocations. Pair-word/source alias order, allocation initialization and current key reads use the rebuilt CRT allocator; original allocator code and allocation-failure exception identity are not independently replayed.
- service: 10 original/source pairs, 36,819 normalized state bytes and 10 event DWORDs; 195 original bytes with seven declared operand relocations. Actual nonempty record/string cleanup, registration-return mutation of current begin/end, fresh-pointer free and throwing validation before free/clear are covered. Private CRT free observation forwards to the actual free import and restores it.
- texture: 20 original-byte pairs, 464 compared payload bytes; six full bodies, 502 original bytes, six external CALL operand relocations. Real CRT new-handler failure, header/record aliases, signed resize, wrapped allocation sizes and retained pointer/capacity after destruction are covered. Original and source parents share the rebuilt CRT allocator/free.
- saved_dimensions: No runtime fixture added. Two complete four-byte functions in the exact combined library member match 8B4134C3 and 8B4138C3. Fresh producer and profile evidence selects saved owner fields +34/+38, separately from descriptor dimensions +28/+2C.

Full hashes, immutable worker captures and validation evidence are in
`reports/native_lighting_service_ba_validation.json`.

- Actual borrowed storage, allocator, current constant, profile and pool domains remain preconditions; added source interfaces are not drop-in original binary entries.
- Registry allocator conventions are inferred thiscall with an unused ECX receiver from all observed callers; exact original source declarations remain uncertain.
- Original parent byte probes share established current-source descendants. They do not independently certify every original allocator, pool, destructor or exception type.
- Ambient private native spill/argument aliases and unrestricted native SEH/FH3 behavior remain unproved; masked MXCSR fixture coverage is bounded.
- Saved-dimension byte comparison covers the two leaves and their demonstrated 2D profile, not arbitrary texture profiles or complete texture ownership.
- Full service texture/camera construction, original FH3 compatibility, unrestricted concurrency, rendering and gameplay remain open. Next-wave BC work is excluded from this batch.
