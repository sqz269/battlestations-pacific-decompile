# CC12 CRT dispatch first direct target: saved entry

The admitted physical call at `00C051C7` targets `00C05070`: immediate frozen caller bytes `E8 A4 FE FF FF` give `00C051CC + sign_extend(FFFFFEA4) = 00C05070`. Root previously accepted that caller body. This packet closes only the target's saved entry evidence; descriptive routing does not establish TLS or getptd behavior.

## Saved metadata and admission

Two exact typed-flow requests at `00C05070`, bracketed by three count checks, retained equal 2,158-byte raw bodies at epoch 98 / count 64,730. Their raw SHA-256 is `e5416201e9877bca3a12bac06f1f5177a0b06bf519332f77468cbb8f59d85695`. Each verifies `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, and x86:LE:32:default. All automatic target objects remain opaque; both entry responses contain zero such objects.

Saved owner: `LIBCRT_unmatched_00c05070`, non-thunk, no-return false, one complete body metadata range `00C05070..00C050A1` totaling 50 bytes. All four instruction/code-unit aliases identify one whole one-byte InstructionDB unit at the entry. Saved flow is FALL_THROUGH to `00C05071`, with NONE override and empty default/effective targets. This metadata does not admit reading the remaining body.

Root independently reviewed that boundary before admitting paired one-byte reads, one context-zero entry-text query and four prototype metadata prefix lines. Root's 1,164-byte gate receipt (`e1197886d764dad4e74f9c438d12b8ea240936227dc2de63b35c22d9b230f5b2`) and physical producer are sealed.

## Entry byte and limited contract

Both reads return exactly `56`. An independent opcode-family decode and actual detailed Capstone 5.0.7 at base `00C05070` agree on one-byte `PUSH ESI`; saved context-zero text and all listing lengths agree. Decoded-byte SHA-256: `de5a6f78116eca62d7fc5ce159d23ae6b889b365a1739ad2cf36f925a140d0cc`.

If this captured one-byte instruction executes normally in an appropriate 32-bit x86 environment: v := entry ESI; ESP := entry ESP-4 modulo 2^32; M32[SS:ESP] := v; ESI and flags unchanged. The store may alias other linear memory, including partially overlapping cells. The arithmetic fallthrough is 00C05071; its instruction and subsequent body behavior are unread. No frame, restoration, arguments, return type, original ABI, TLS or getptd semantics follow.

Only these four lines of the prototype capture were interpreted:

```text
Function: LIBCRT_unmatched_00c05070 at 00c05070
Signature: undefined LIBCRT_unmatched_00c05070(void)
Entry: 00c05070
Body: 00c05070 - 00c050a1
```

The `undefined` return and `(void)` parameter spelling are saved annotations, not an authenticated original type or calling convention. Every companion-suffix byte remains opaque; the report records only its length and digest. The saved no-return flag also does not establish actual runtime return behavior.

Nine successful Native CLI calls were retained: two typed captures, three counts, two exact one-byte reads, one text query and one prototype capture. No continuation, remaining body, data-cell or callee queries followed; no GPR mutations occurred. Closing work was entirely offline.

## Replay and exact delivery

The archive retains all Native captures and schedules, four physical helpers and their preimages, the Root gate and producer, actual detailed Capstone output, and twelve immediate positive inputs connected through eight actual Git object payloads to the frozen worker commit. Referenced predecessor archives were not opened. Source subtree objects carry revision identity only; they were not inventoried.

Each original/distinct-copy replay hashes every member before executing the pinned pure parser. Its audited process performs exactly one supplied-ZIP read and one pinned parser execution; post-preload imports, non-ZIP reads, writes and external effects are forbidden. The original and copied ZIP have distinct filesystem identities and equal digests, and produce equal replay results. Replay recomputes the manual opcode and metadata proof and checks the retained actual Capstone output; Capstone is not imported inside the guard.

Archive: `local/cc12_application_crt_getptd_first_direct_target_entry/evidence.zip` (507239 bytes), SHA-256 `6e0575ada3c432bd47202ca6c05e3276891ff6a77ac09faa8615fff02b9a3740`.

Frozen worker HEAD: `9e501691f109f8387d69014dfce06f9afe74f5d4`. Worker base/delivery subtree identities: `src=af8efebaa30365fa0f7f2f591c45861a55b4b1b2`, `include=09add3be594e63edcce6a16ec041ed965441e898`. No Source census or equality to current Root Source is asserted.

Only this document and its companion JSON are tracked outputs. External staged/final receipts pin exact working/index/commit bytes and blobs, the twelve unchanged immediate input objects, and unchanged worker Source tree identities. They remain outside the archive to avoid circular hashes. This packet supplies Native evidence only: no Source, CMake or ledger edit; no build, new test, original ABI, startup, runtime or gameplay credit.
