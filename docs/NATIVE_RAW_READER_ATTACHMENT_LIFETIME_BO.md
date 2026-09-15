# Raw-reader attachment and lifetime frontier

The next smallest complete source packet is **`BF0430`, retained stream assignment, in the two established raw-reader provider domains**. It can use the existing retained-memory and physical-stream source providers directly. The larger reader destructors and twelve-byte record storage retain explicit dependencies; this discovery does not turn them into partial initializers or fabricate their exceptional cleanup.

This read-only packet starts at `d3b8261c98cb18b60d7ca08a2c1ac725ae225576`. Its [report](../reports/native_raw_reader_attachment_lifetime_bo.json) records complete PE/live spans, every direct and indirect call instruction in the captured function bodies, native ABI, ownership, metadata, source-provider hashes, listing gaps, and a double-hashed local inventory. Eight helper function bodies were inspected beyond the four requested roots; the additional three ten-byte fragments only forward EH metadata.

The BM discovery `f57a2fba9bef3751f78c8f7ed062c1680f26c573` and all 291 of its local artifacts were retained and independently checked with SHA-256 and SHA-512, including the nested BL evidence. The accepted BN constructor source/doc/report at `a1426c0d8b55f2009c4755c1bf2ef376dbe8542b` is pinned separately. Both original worktrees remain unchanged. The installed PE SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`; live queries verify project `bsp` and `/battlestationspacific.exe` against the configuration naming `C:/Users/sqz269/bsp.gpr`.

## Complete attachment operation

`BF0430..BF0468` is 57 bytes. Its original ABI is ECX = actual reader, one stack stream argument, and `RET 4`. EAX is incidental pointer/atomic/terminal state; there is no stable reader-pointer or Boolean return contract. The proposed source interface returns `void`.

The exact sequence is:

1. Capture the incoming stream value, then capture `reader+0` as the old stream.
2. If the pointers are identical, return without writing, counting references, reading profiles, or touching providers.
3. Publish the new stream to `reader+0` at `BF043D`.
4. If new is nonnull, increment its real intrusive DWORD at `+4` through `InterlockedIncrement` at `BF0445`.
5. If captured old is nonnull, decrement its real intrusive DWORD at `+4` at `BF0453`. Only a zero result selects the old stream's current profile and slot zero at `BF045D/BF045F`, then calls it with ECX = captured old at `BF0463`.
6. Return without another reader store.

The PE import table independently identifies `CE221C` as `InterlockedIncrement` and `CE2220` as `InterlockedDecrement`. No extra decrement, positive-count check, stream seek, header reset, or buffer cleanup belongs here. The function issues no direct assignments to reader fields at `+4..+6F`; effects through aliased reference-count locations or the actual providers remain observable. Assignment retains the incoming stream; it does not adopt or consume the caller's existing reference. Null detaches.

There is no exception frame or rollback in this function. If the old zero-reference terminal throws, the new pointer is already published and its reference already incremented. That state remains. No provider/profile validation should be moved ahead of the identity test, publication, or native zero-reference branch. A terminal may affect the reader; assignment makes no later store that would undo that effect.

## Real zero-reference providers

The existing `NativeRawScalarReaderContext` already carries pointers to the concrete `NativeRetainedMemoryOwnerContext` and `NativePhysicalStreamOpenContext`. Their source files and headers are retained at the current checkout revision. A selected context is required when its terminal dispatch is needed; same-pointer and nonzero-reference paths do not need provider access.

| Current old-stream profile | Current slot zero | Terminal behavior |
| --- | --- | --- |
| `D642C0`, retained memory stream | `BD30E0` | Reload current profile/slot four; call `BB8F90` with flags **1** |
| `D691B0`, physical stream | `BF55A0` | Reload current profile/slot four; call `BF5090` with flags **0**, then obtain the actual pool and append the dead address |

Both eight-byte profile prefixes match PE/live bytes. Complete native bodies of `BD30E0`, `BB8F90`, `BEF9C0`, `BF55A0`, and `BF5090` were also matched and inspected.

For memory, use `delete_native_memory_stream_00bb8f90(old, 1, *context.memory)` after preserving the current slot-zero and then reloaded current slot-four reads. The existing source destroys retained backing through its actual intrusive count, actual owner counters/profile tables and shared allocation service; scalar free occurs only after successful destruction. `NativeRetainedMemoryOwnerContext` is declared in `include/bsp/native_retained_memory_owners.hpp` and implemented by `src/native_retained_memory_owners.cpp`. Its private zero-reference dispatch already demonstrates the two profile reads. The existing `assign_native_retained_memory_slot_00b23640` has the same capture/publish/increment/decrement ordering and can express a memory-only attachment using the address of the captured incoming value. It is not a physical-stream recycling provider.

For physical streams, call `recycle_native_physical_stream_00bf55a0(old, *context.physical)` after the current slot-zero selection. The existing implementation checks the current slot four, calls `delete_native_physical_stream_00bf5090(old, 0)`, gets the actual current physical pool through `native_physical_stream_pool_00bf42a0`, and appends through `append_native_physical_stream_slot_00bf5190`. The destructor closes the current handle, stamps the base profiles, and leaves allocation ownership for recycling. A flags-one free or a second reference decrement would be wrong. These functions are in `native_physical_stream_open.hpp/.cpp`; the current numeric `D691B0` table must remain readable as required by that existing provider domain.

This closes attachment across the two supported raw-reader stream families without introducing a generic zero-reference callback. Arbitrary profiles, replaced/hooked or concurrently mutating providers, invalid reference storage, and native CRT/FH3/asynchronous-fault identity remain outside the source contract.

## Destruction: known sequence and remaining dependencies

`BE9F10..BE9F87` is a complete 120-byte structured-reader destructor, ECX = reader, plain RET. It arms state one and releases the separate `+68/+6C` name if its data is nonnull, using the real argumentless `419CC0` getter followed by `BD1510`. It leaves those header fields unchanged. It arms state zero before destroying ten path headers in reverse order through `BF7C6E`, then disarms before `BF09B0`. It does not free caller-owned `70h` storage.

`BF09B0..BF0A27` is a complete 120-byte base destructor, ECX = reader, plain RET. It captures the attached stream and arms state zero before decrement/current-slot-zero dispatch. On normal terminal return it writes the **current** `reader+0` to zero; this differs from attachment's lack of a later store. It disarms before calling `BF0700(reader+4,0)`, then frees the current buffer pointer through the real `BF6989` CRT thunk. Additional cleanup after a failure in that already-disarmed phase would need evidence.

`BF7C6E` computes the end cursor, decrements its remaining count, moves back one stride, and calls the destructor with ECX = current element. Its parent-EBP finally `BF7CB9` calls the retained BM `BF7C10 __ArrayUnwind` on the not-yet-visited earlier prefix after a callback fails. The failing element is not destroyed twice. Actual `BE9F10` supplies `41DD20`; its raw-pool source overload preserves a potentially throwing getter. The `noexcept` host-storage overload is a different contract. Native ArrayUnwind's `E06D7363` filter/terminate path remains as recovered in BM.

The matched outer EH metadata is:

| Function | FuncInfo / unwind map | State actions |
| --- | --- | --- |
| `BE9F10` | `E01AC4 / E01AB4` | State 1 → 0 via `CC70C8`; state 0 → -1 via `CC70C0` |
| `BF09B0` | `E02464 / E0245C` | State 0 → -1 via `CC7810` |
| `BF05D0` | `E023E0 / E023D8` | State 0 → -1 via `CC7790` |

All three FuncInfos carry magic `19930522` and EHFlags one. The three corresponding handler fragments forward metadata to the existing conflicting-FID `BF6B43`; this does not establish a new FH3 provider identity. The four action-target bodies remain an explicit follow-up within this bounded packet. `BF7C6E`'s separate SEH4 table `E02D30` directly identifies finally `BF7CB9` with parent state `-2`. General reader destruction is not declared source-closed merely from its normal sequence or these metadata pointers.

## Twelve-byte storage and the physical listing gap

`BF0700..BF077C` is a complete 125-byte resize body: ECX = raw `(data,count,capacity)` triplet, signed requested count on the stack, `RET 4`. If requested exceeds capacity it calls `BF05D0`. Count growth zeroes only the first eight bytes of each new twelve-byte record; its third DWORD remains untouched. Shrink publishes a decremented count before reading/releasing that record's string and rereads count after the real pool provider returns. The final requested count is published only after normal completion.

`BF05D0..BF06F9` is 298 PE/live-matched bytes. It clamps capacity requests to at least one, skips requests already covered, allocates `12*requested` via actual `BF55BE`, copies existing string headers using `41DD40` and `BF7680`, copies each third DWORD, releases the old strings, and calls `BF6989` on the old buffer.

The live and saved listing omit ten bytes at **`BF06DD..BF06E6`** immediately after that free call. Disk decoding recovers `ADD ESP,4`; publish new buffer to `[EBX]`; `POP EDI`; publish requested capacity to `[EBX+8]`; `POP EBP`. Thus buffer/capacity publication occurs after freeing the old buffer. The saved pseudocode ends at free and misses these essential writes. The flow report identifies the gap; no flow property was changed, no exact override value was presumed, and no repair was applied.

`BF05D0` and `BF0700` remain named-but-incomplete source dependencies. Their complete exceptional ownership, `CC7790` action, actual allocation/free domain, and third-DWORD meaning are not resolved by replacing them with a safe host vector or adding prefix cleanup. They are outside the next attachment packet.

## Next source packet and evidence limits

Use `src/native_raw_reader_attachment.cpp` and `include/bsp/native_raw_reader_attachment.hpp`, anchored only at `BF0430`. A suitable new interface is `void assign_native_raw_reader_stream_00bf0430(void* actual_reader, void* actual_stream, NativeRawScalarReaderContext&)`. Implement all assignment branches and use the existing concrete provider names and terminal flags above. Preserve both current-profile reads on the memory zero path and the physical provider's own slot-four lookup. No root allocation, stream attachment side operation, reader destruction, or storage initializer fragment belongs in this packet.

The BN constructor's valid, fresh, nonfaulting `70h` storage contract is unchanged. This discovery ran no build, tests, probes, native execution, or game checks. It modified no source, ledger, Ghidra name/comment/listing, or game installation. Two complete SHA-256/SHA-512 scans of the frozen local artifact set are retained; the report's final inventory includes those manifests as well.
