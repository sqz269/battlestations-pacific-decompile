# Header13 r03 actual linked peer review

The selected `Main/local/h13p3` artifact subset passes this independent static peer review. Source credit remains **0**. No target was launched, no launch is admitted, and the Root family has separately been terminally sealed after a native-query dependency changed before launch.

## Exact scope and evidence

The review freezes and bookends exactly **26 selected Root files**, comprising the three objects, linked PE/map, packed gate, Native13 bytes, probe/leaf/canonical sources, provenance/static receipts, frozen provider file/evidence, and six independent-reader outputs. Root receipts are context, not substitutes for the peer parse. This does not claim whole Root-family immutability.

The standalone evidence family is `J:\PROG\battlestations-pacific-decompile-cc12_header13_r03_actual_linked_peer_review\local\h13actualpeer`. Its seal is `family_seal.json`, SHA256 `0345f4af49b3e93d79b31c4bdda6da1e99a9d3b224b01ad9022a95b2e7b08327`, covering 437 other files (438 including the seal). It retains every own parser version, full raw outputs, exact copies, and the stopped startup audit. The machine-readable report contains all 26 before/after pins and detailed frame/caller evidence.

## Independent code result

Raw COFF reconstruction yields **26 logical TU symbols / 24 physical bodies** and **195 logical / 189 physical relocation operands**. Both actual weak records use mode 1 and resolve through their raw TagIndex to defined mapped function fallbacks, with six alias relocation rechecks. No actual mode 3 behavior is inferred. Every relocated byte matches the PE.

The independent closure contains **41 complete spans, 6,697 bytes and 2,045 instructions**. Main is 2,615/719, raw capture 173/63 and ordinary caller 6/2 bytes/instructions. All local branches land on decoded instruction boundaries, all five register-indirect import calls have an all-predecessor IAT origin, and no ordinary local transfer remains unexpanded. The named GS failure frontier at `4B002DD0` remains deliberately unexpanded.

The 17 support bodies include nine normal import thunks, three cold standard-library thunks, cookie14, chkstk43, sized delete16, unsized delete5 and free6. Complete support extents come from raw thunks or a CFG-derived end inside the next mapped function boundary. Only the maximal five unreachable trailing CC bytes are excluded from chkstk; internal labels do not truncate it.

The packed 7,377-byte gate has exact EOF and exactly the independently derived span addresses, lengths and bytes. Its three provider records bind the PE IAT slots to direct EAT exports in the selected frozen ucrtbase file, including machine/timestamp/image size, hash and recorded physical identity. This is frozen-file evidence; no provider disk API or runtime module observation occurred. The Native13 and linked Source bytes agree exactly: `8bc133c98908894804894808c3`.

## Whole caller and capture review

All 719 Main instructions were reviewed. Four independently computed CFG dominance groups support the manually reviewed loop/dataflow proof: gate/live-code/provider/identity/Native13/RW-to-RX guards precede allocation and capture; three successful 44-byte allocation requests remain simultaneously live and distinct; all three target lanes use `root+16`; every post-call guard precedes continuation; all 132 buffer bytes are rechecked before exactly three success-path frees; provider and live-code final bookends precede success.

The lanes are **raw Source DF1, raw Native DF0, ordinary Source DF0**. The full 96-byte CaptureBox encloses its 88-byte payload at offset 4, with canaries at 0 and 92. Main verifies all before/after GPR slots, both stack canaries, ES equality, EAX=root, ECX=0, EDX preservation, DF and defined XOR flags `8C5=44`; AF is excluded. The machine contains the intended three poison bytes and three distinct EDX values. Main serializes all 96 box bytes.

Let R be raw-wrapper entry ESP. EBP becomes R-4; Q=R-28 is ESP before the indirect call after the nonvolatile/flag saves and two canaries. Direct target entry T=Q-4. The ordinary wrapper adds one return slot, so inner Source entry is T-4 and its `[ESP+4]` is the ordinary-entry return slot. After target return, the wrapper records Q using `LEA [ESP+8]` after saving flags/EAX, then restores its saved frame and clears DF before returning. Capture equality tests and disjoint canaries are preserved in the actual machine code.

## Provenance stop and limits

The first full dependency audit stopped because Python site startup imported `pywin32_bootstrap.py` without an earlier pin. Its STOP and partial outputs remain sealed; they are not the accepted result. The accepted `python -B -E -S .../peer_isolated_parser.py` replay independently repeated every raw/code/gate/dominance check with isolated startup, the exact pinned Capstone package, 98 decoder/interpreter file copies and 238 actual imported source/cache/binary file copies matched to the earlier inventory and final bookends.

Root reported h13p3 terminal seal SHA256 `4dff864a4cfe259f2920f03fcbe2a27c46a7ca7179297b1fc35135b6173d22f8` after `tools/ghidra_flow_repair.py` drift. That hold is contextual and remains in force; this peer packet neither repins it nor approves a later family. No later family was inspected. No Root helper, compiler/linker, Native query, provider disk API, Ghidra mutation or target process was run by this packet.

This proves bounded artifact agreement and static caller structure. External DLL internals, exceptional execution, private Native lifetime/class equivalence, startup and gameplay remain unvalidated. The allocator retains its OOM retry path; three success-path allocation requests do not assert only three malloc attempts under failure. There are no production changes or new tests.
