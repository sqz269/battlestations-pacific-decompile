# Native property-record type-4 raw storage, CC12

The whole leaf at `008EF230` partially initializes supplied 56-byte storage
with two opaque DWORDs. The new Win32 Source body exactly matches Original:
56 bytes, 16 instructions, no CALLs and no relocations. Its descriptive name
and the interpretation of the first word as declaration identity are hypotheses.

The worker implementation, strict standalone build and bounded fixture passed.
Source admission remains pending Root's independent complementary family,
full main Win32 build/checks and publication. This is a raw storage operation;
native enum lookup, parsing, declaration ownership, phase dispatch, clone,
allocator/EH, class lifetime and gameplay are outside its scope.

## Exact physical contract

Whole range: `[008EF230,008EF268)`. Original, fresh Source COFF and unique
linked body SHA-256:
`b6b40a5cbb0a523d345de24f97ac879a4e5a56c8d9a1f121f636fd33ce4dfd98`.

```cpp
void* __fastcall construct_native_scene_property_record_type4_storage_008ef230(
    void* actual_record_ecx, void* unused_edx,
    std::uint32_t declaration_identity_bits, std::uint32_t value_bits) noexcept;
```

ECX is the actual fresh/unowned writable root, EDX is unused on entry,
`[ESP+4]` is the first word, and `[ESP+8]` is the second. `RET8` consumes both
words. EAX returns the root, ECX becomes zero, and EDX contains the exact second
word. EBX, ESI, EDI, EBP and DF are preserved. Defined XOR flags are
CF=OF=SF=0 and ZF=PF=1; AF is undefined and excluded from validation.

| Offset | Exact effect |
| --- | --- |
| `+00` | DWORD opaque native phase identity `00CE89D4` |
| `+04` | DWORD tag 4 |
| `+28` | Exact first raw DWORD |
| `+0C` | Exact second raw DWORD |
| `+18,+1C,+20,+24,+30,+34` | Zero DWORDs |
| `+2C` | Byte 1 |
| `[08,0C),[10,18),[2D,30)` | All 15 bytes preserved |

Exactly 41 bytes are written. The Source retains every original store width
and its order: the first stack word is read before any store; the second is
read after the `+28` store. Neither word is converted or dereferenced. There
is no target SSE, x87, MXCSR or other numeric operation. The phase identity
is not mapped or dispatched as a Source vtable.

The caller retains allocation ownership. Storage must be disjoint from the
active call frame; overwriting a live owning property object is excluded.
The separately audited real clone arm allocates `38h`, pushes source `+0C`
then `+28`, calls this leaf, and subsequently copies `+34`. Those caller
operations and their provider dependencies are not reconstructed here; see
[the readiness contract](CC12_PROPERTY_RECORD_TYPE4_STORAGE_READINESS.md).

## Worker validation

Ignored family `local/t4a/` was prepared, compiled, statically gated, executed
and sealed once, with no failed attempt or phase replay. Its three fresh TUs
are the new constructor, unchanged current `singleton_lifetime.cpp`, and a
new probe. It links no BSP archive or old object and adds no tracked test.
MSVC 14.51.36231 Win32 with SDK 10.0.26100.0 used
`/O2 /MD /W4 /WX /fp:strict /EHsc /Gy /GL-`; the executable embeds `asInvoker`.
All 184 consumed headers, including extensionless headers, seven libraries,
tools, Source, recipe and native PE were pinned. Short frozen filenames were
checked for duplicate-path consistency before compilation.

Nine complete code spans were gated: constructor 56 bytes, current allocator
90, current free 6, ordinary Source caller 22, ordinary indirect caller 69,
raw capture caller 173, bad_alloc constructor 24, probe main 2934, and the
actual security-cookie helper 14. Fresh TU sections were compared with linked
bytes after checking every relocation. Both ordinary UInt32-pair callers were
inspected statically for complete words, push order and ECX/EDX provenance.

The cookie helper is the actual mapped `MSVCRT:secchk.obj` function at
`23002D80..23002D8E`: CMP, local conditional branch, successful RET and failure
tail JMP. The linker map's next function at `23002D90` bounds its own CFG.
The separately named `___report_gsfailure` target at `230030E0` stops traversal;
its body and alignment are not admitted. All 14 helper bytes are included in
the pre-entry and post-free runtime code checks.

Actual mapped I386 malloc/free/`_callnewh` gates ran before creating either
root and after both matching frees. IAT targets matched actual module exports,
RVA, held physical-file identity, NT path, full SHA-256 and normalized 32-byte
code prefixes. The provider was `Windows/SysWOW64/ucrtbase.dll`, SHA-256
`60c5a497b52de80a3a0677564270dbea7e486086637debd567b4dffb28584c1b`.

The sole new process used two distinct actual canonical 56-byte malloc roots:

| Entry | First word | Second word | DF | Root poison |
| --- | --- | --- | --- | --- |
| Source, once | `D19A42C7` | `80000000` | 0 | `A6` |
| Unchanged RX Original, once | `2EB56C38` | `7FFFFFFF` | 1 | `59` |

Both complete 56-byte results, 15 untouched bytes, the other live root,
external capture guards, stack canaries, returned registers, nonvolatiles,
DF, defined flags and RET8 cleanup passed. The raw caller saved both old
argument words at `[ESP-8]` and `[ESP-4]` before its after-state PUSHFD could
overwrite a slot. After-state capture uses MOV/LEA before PUSHFD and clears DF
only after saving the target flags. No FP or numeric experiment was needed.

Both roots were freed once and the RX copy was released. Original PE/live
bytes, nine declared code spans, CRT provider and input bookends matched.
All 1,996 prior artifact pins remained unchanged, including historical frozen
Source associations and Root's corrected float-family inventory. Six metadata
copies are included in the complete new family inventory. Post/seal stdout
was captured outside the family; no receipt correction was needed.

## Receipts and remaining work

The [machine-readable report](../reports/native_scene_property_record_type4_storage_cc12.json)
contains exact paths, hashes and observed state. The family seal inventories
336 files plus itself; all hashes and the actual 337-file set were independently
checked after sealing.

- `static_gate.json`: `215649c635f188c792c5bc91cf1f7f9e1d9617fbbc932d00f8dbc6aa742e4676`
- `runtime.json`: `db59a01005cb5cf14f0c7353421ecfcaa3366ce267bb80229fdcf6b61076e3f5`
- `post.json`: `ebd07afca6384d372b1da85fc013f0a3a86fcdebdd8bbc29200ce9a797d82460`
- `seal.json`: `423d4099036e20b3a318a2a960ea66cc5c94c1682fc7f8f96ce2a41090391534`

The worker changed only the four owned files. Root owns the independent
complementary validation, full main build/checks, shared CMake/ledger/config,
locked Ghidra annotation/save/export/index and final admission. No native
owning declaration/class or startup/gameplay validation is claimed.

## Primary integration

Whole raw type4 storage[008EF230,008EF268),56B16,zeroCALL/relocations. Complete Source COFF/unique linked/runtime body literally equals live/installed Native SHA b6b40a5cbb0a523d345de24f97ac879a4e5a56c8d9a1f121f636fd33ce4dfd98. Physical ECX supplied fresh writable unowned56-byte current canonical heap storage disjoint from active frame; unused EDX explicit padding; two opaqueDWORDs at entryESP+4(identity bits)/+8(value bits), RET8. Firstword is read beforestores, stored+28; secondwordread occursafter+28store and isstored+C. No pointer dereference/numericconversion/FPoperation. Literal phase00CE89D4/tag4; sixzeroDWORD18/1C/20/24/30/34; byte1at2C.41written15preserved([8,C),[10,18),[2D,30)). EAXroot,ECX0,EDXvalue; nonvolatiles/DF/ES and definedXOR flags8C5 pass, AFundefinedexcluded. Phaseword is identityonly and cannotbe dispatchedas Source vtable. Worker fresh3TU/two56alloc/twofree and independent complementary current-main fresh3TU/two56alloc/twofree fixture each executeone Source/one unchangedRX Original. Independent Source13579BDF/FFFFFFFF/DF1/B6 and OriginalECA86420/00000000/DF0/C7, EDXD5A193C7, different stack/capture/nonvolatileguards pass. Root independentlydecodes124-bytecaptures/full56storage; bothdeadactualargumentwords capturedbeforePUSHFD, RET8 ESP/noRoot/capture/inputcanary damage. All9 complete code spans include whole173B64 rawcaller,22B6 ordinarycompiler caller and actualmap-owned14B4 cookie CMP/JNE/RET/failureJMP (distinct external failurebody excluded). Actual mapped I386 malloc/free/_callnewh IAT/export/module/physicalfile/fullSHA/prefix gates precede both fixture allocations and matchafterfrees.11843 priorpins and341sealedartifacts/exact342actualfiles unchanged. Prior primary t4p1 retains two unexecuted launch failures(reviewbasename and externallychanged mutable dispatchledger); zero process/targetentries there. New t4p2 freezesdispatch as historicalcontext, neverrepins oldreceipt or replays earlier successfulphase/process. Root type4 integrated Win32/all3 checks and current combined merged-main Win32/all3 checks pass. Rawcurrentcanonical storage only; identitysemanticlabelhypothesis, native enum/declaration/clone/owningclass/phase/privateCRT/EH/World/game unadmitted.

Independent family `local/t4p2`, seal `217505ab9fc25fcef8da9fa674a57e76f2d1409da0aeabb5f0cafe753012ca75`. Current combined build source revision `d0146717ee8426ff21bde233837957f551fc0e69`, all3 existing checks passed.
