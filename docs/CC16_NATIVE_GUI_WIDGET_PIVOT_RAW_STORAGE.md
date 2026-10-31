# Raw ActualWidget pivot storage entry — A9E0B0

The packet appends one raw four-argument Source entry to the existing transform pair. It executes the four Native pivot memory operations in order, then calls the existing `recompose_native_gui_widget_transform_00aa7220` on the same widget, scratch and bindings. It supplies no Glyph current30 wiring, owner projection, new callee/provider body or Native permission.

The immutable baseline is `7a06afff214abcb5bca15609dfee47b9b64e115a` (published Main). Both complete Source preimages equal baseline Git blobs, the raw worktree bytes and ignored frozen copies. Their reviewed preimage hashes are H `7ce3ccf33392eeff3e603aa43edd941a94d5cdb23d655d70a722bdb60773354d` and CPP `8f2bbaa84a9b44d90ce4ae89ccdc616118b7a358fb3be90220573ac3ef7eafad`. Source changes consist only of P1, a declaration/contract insertion, and P2, an appended implementation insertion, immediately before the existing namespace closes. The report embeds both complete preimages, exact insertion bytes/offsets, all five ordered forward/reverse state hashes and a minimal independent static reader. No old line is changed. The complete old AA7220 body, typed pivot facade and listed providers remain exact.

## Recovered Native entry and operand order

The read-only batch first verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` through `bsp.py ghidra proto`, then read the complete A9E0B0 listing with `bsp.py show --asm`. The live body is A9E0B0..A9E0C6: seven instructions, 23 bytes, one direct callee.

```text
00a9e0b0: MOV EAX,dword ptr [ESP + 0x4]
00a9e0b4: FLD float ptr [EAX]
00a9e0b6: FSTP float ptr [ECX + 0x18]
00a9e0b9: FLD float ptr [EAX + 0x4]
00a9e0bc: FSTP float ptr [ECX + 0x1c]
00a9e0bf: CALL 0x00aa7220
00a9e0c4: RET 0x4
```

The installed PE interval has SHA256 `5c65b9395eff65c752e12759b4aa684f0e1a60b9ec182578799a404fe14cb877`. Its D5C6F8 cell (table D5C6C8 +30h) contains A9E0B0, independently recorded as four PE bytes; that is table evidence, not a live object's current profile or publication-generation authentication. The PE and complete saved assemblies are pinned separately from Source bytes. Historical exports/comments are not rewritten.

ECX is the actual widget and the incoming stack word is the actual pair address. The second FLD follows the first FSTP. For the concrete overlap `pair == widget+14h`, the first store replaces `pair[1]` before the second load, so eager pair capture, MOVSS, memcpy, memmove or C++ float assignments would change the addressed operations. The new implementation sets the two Source operand addresses in inline assembly, keeps exactly `FLD [pair] / FSTP [widget+18h] / FLD [pair+4] / FSTP [widget+1Ch]`, then invokes the same existing raw AA7220 once. It adds no address/profile/equality/null/finite-value guard. Inputs and outputs need only satisfy the accesses actually reached; pair/widget overlap is not repaired. Caller-private scratch and metadata extents remain disjoint from those live operands and genuine provider extents.

## Actual existing callee and suppliers

AA7220 is the complete existing raw 583-byte normal body with explicit caller-retained `NativeGuiWidgetTransformScratch` and `NativeGuiWidgetTransformBindings`. It uses actual widget fields rather than a `GuiWidgetOwner`, Raw122, Factory record or CurrentModel projection. The Source scratch is the initialized stable 1DCh-byte preimage corresponding to the Native local region; holes C..F, D0..D3 and D8..DB remain unwritten. This wrapper neither creates nor clears scratch or a frame. Preserve its address, bytes, disjoint extents and the reached actual storage/provider graph through return or failure; do not borrow one private DEFAULT factory attempt's scratch for a different Glyph invocation.

The genuine bindings borrow actual held RO D7A24C and D7A208 float cells and the SAME process `GameNativeGuiTextState::widget_y_scale_00e12fc4()` cell. E12FC4 is distinct from Glyph/Text E12FD4. The refs are live volatile reads, not early snapshots. Existing resolved-setter bindings demonstrate the actual data/state preparation route; this wrapper adds no startup service lookup or authentication service. Public raw bindings remain a caller precondition, not proof that arbitrary supplied refs are genuine.

AA7220 captures widget+4C at AA7255 and its vtable at AA72C2; its existing x87/SSE schedule calls raw B64780 at AA7423 and complete raw 413920 three times at AA7442/49/50. After those calls it reloads CURRENT widget+4C at AA7455, reads slot38 from the EARLIER captured vtable at AA7459, and dispatches that exact target/current receiver/matrix at AA745C. No current-vtable substitute, captured-node substitute, equality rule or extra receiver read is introduced. The full original Source body is recovered by deleting only P2.

The existing `NativeGuiFrameBoxConstructedTransformDispatch` is a real finite provider: it admits captured target B6DB10 and calls the existing whole raw B6DB10 body. That body uses actual storage, raw x87 4134F0 copying, raw descendant invalidation B6DA30 and current bounds notification B6DBC0. The provider's reached current A0/current3C domain remains the admitted actual Node/Model/Group table aliases with B6DBC0. Unsupported captured slot38 targets or current notification targets remain unsupported after any prior effects; this packet does not widen them or invent a provider. Exact current Source creator, C++ subobject family, publication generation, extent and custody remain actual caller obligations. The existence of a raw transform body alone establishes none of those facts.

## Source ABI, escape and acceptance limits

The public signature is:

```cpp
void set_native_gui_widget_pivot_00a9e0b0(
    void* actual_widget, const volatile void* actual_pair2,
    NativeGuiWidgetTransformScratch&, NativeGuiWidgetTransformBindings&);
```

This is a new four-argument Source ABI. It is not an Original ECX/stack/RET4 replacement, a recovered semantic EAX result or proof of Original physical stack layout, private aliasing, fault timing, unmasked-FP environment or SEH/FH3 behavior. The four ordered x87 memory operations retain their Source-level addressed overlap and ambient arithmetic contract, but no compiled/runtime equivalence is claimed here. The existing typed pivot facade and all old entry signatures remain byte-exact.

No `noexcept`, local exception handler, destructor cleanup, retry, reset, rollback, null fallback, count repair, owner/frame creation, public capability or Native issuer is added. If a reached store or genuine provider escapes, the caller retains the actual mutated storage, scratch and outstanding provider obligations under the existing contracts. This function does not turn that debt into normal-return, reference, disposal, Native-origin or activation credit. Existing provider Host rejection remains a Source qualification, not an inferred Native input filter.

Current Glyph Context/Acquired do not supply this transform scratch/bindings route, and the baseline Source inventory has no genuine implementation of `NativeGuiTextGlyphQuadStorageCalls`. A later direct-known-current30 packet must supply real distinct per-invocation retained borrows and preserve exact already captured target/profile coupling plus OTHER-target transport. This packet leaves Glyph, its mandatory pool borrow, GPA tags, constructor transport, home epochs, header states and existing counters untouched. Full Glyph producer/Raw122/replacement transport/CString/Origin/runtime gates stay separate and false.

## Mechanical receipt

P1 inserts 790 bytes at baseline H byte 1669; P2 inserts 532 bytes at baseline CPP byte 5271. Whole postimages are H `8dad778892b62bc27de08907eff18db3badd061a3a0d67711694aeb7df783642` and CPP `39c87d3fd8136d2802ea4c6def1fba8f0df853c8c4f82d778ab512bb65a45d40`. All 18 complete old facade/provider/context/Glyph witness files equal baseline Git and raw bytes; their exact whole-file pins and separate native assembly/PE pins are in the report. Whole-file labels never imply a bounded region hash or vice versa.

Run the report's read-only embedded Python reader from the repository root, passing the report path as argv[1] and optionally the sealed packet revision as argv[2]. It reconstructs both full postimages, reverses both seams, checks baseline Git/raw/frozen equality where local frozen files exist, and checks all unchanged whole-file witnesses. It executes no reconstructed C++ or Game code. Ignored frozen material is under `local/output/cc16_native_gui_widget_pivot_raw_storage_20261030`. Root will independently review, compile and integrate; none of those later actions is claimed as a worker result.

Only the two Source files and these two new evidence files belong to the packet. No worker compiler/build/tests/probe/Game process, CMake/ledger edit, Ghidra mutation or original-game write occurred. The packet claims zero additional Native/vendor/duplicate-helper/ABI/runtime/activation credit. Commit identity and final evidence hashes are returned separately after sealing, avoiding a self-referential report hash.
