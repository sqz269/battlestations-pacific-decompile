# Raw-reader twelve-byte storage and listing recovery

The complete BF05D0 reserve and BF0700 resize algorithms are recoverable from physical bytes, including BF05D0's missing buffer/capacity publication. The exact current Ghidra override and no-return flags remain **inaccessible**: the supported read-only script endpoint is disabled, and the installed bridge exposes those getters only inside mutating endpoints otherwise. This packet preserves that response and proposes a conditional repair; it does not guess old property values or repair the listing.

Base: `27f97f574388ff576d57a25acd213560ec2ad434`. The [report](../reports/native_raw_reader_storage_br.json) retains all 473 BO discovery artifacts plus its document/report, and the independent EH discovery `d05b9060fa693c8be106ff9a7e58bf8becf8f4eb` with all 187 artifacts plus document/report. Original BO, BP attachment and EH worktrees remain untouched. Fresh BF05D0/BF0700 spans total 423 bytes and match both the installed PE and live Ghidra. PE SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`; every live batch verifies project `bsp` and `/battlestationspacific.exe` against the configuration naming `C:/Users/sqz269/bsp.gpr`.

## Actual layout and arithmetic

Both functions take ECX = actual twelve-byte triplet `(data+0, count+4, capacity+8)`, one signed DWORD request on the stack, and return with `RET 4`. Neither establishes a semantic EAX result. A record is twelve bytes: an existing raw string header `(uint32 length+0, data+4)` followed by an **opaque DWORD at +8**. That word is copied on reserve and left untouched on count growth; no hash, index, pointer or initialized-value interpretation is established. Future source must copy its raw bits, including untouched allocation bytes, rather than read it as a newly initialized typed field.

Comparisons of request, capacity and count are signed. Address products, loop counters, size subtraction and `length+1` use 32-bit machine arithmetic. BF05D0 clamps requests below one to one by rewriting its argument slot; it does not clamp BF0700's requested count. Allocation size is `12*request modulo 2^32`, with no overflow check. For example, `0x15555556` produces eight allocation bytes and `0x40000000` produces zero. Translating that expression as overflowing signed C++ multiplication or checked/64-bit allocation changes the operation. Negative/inconsistent counts can address outside valid storage; no new safe-vector policy follows from the native code.

## BF05D0 reserve: 298 bytes

`BF05D0..BF06F9` first clamps the signed capacity request and skips work when current signed capacity already covers it. Otherwise it allocates the wrapped byte count through the actual BF55BE thunk to BF681B. It saves the new allocation base but leaves the caller's buffer, count and capacity published throughout copying and old-record release.

For each index less than the **current** signed count, it computes a new record address, publishes that candidate in the EH local, and arms state zero. A zero computed address skips the record stores. Otherwise it reloads the current old buffer and computes the old record address. It zeroes both new string-header DWORDs before testing source/destination identity. If the addresses differ, it calls actual `41DD40(new_header, source_length, preserve=1)`. After that call, it rereads current source length as the copy gate; if nonzero, it captures current destination length, current source data and current destination data, then calls BF7680 with **destination length** as the byte count. It finally copies the old record's third DWORD to the new record. BF0684 increments the index; BF0687 compares it with current count while state zero is still armed. BF068A then disarms, BF0692 publishes the completed count, and BF0696 branches using the preserved BF0687 comparison flags.

The old-record pass again rereads current buffer/count. For each nonnull captured string data pointer it captures wrapping `length+1`, resolves the real argumentless `419CC0`, then calls `BD1510(pool,data,size,1)`; the third native stack argument is unused by BD1510. Fields of the released old headers are not cleared. After this pass it reloads the current old buffer and calls BF6989. Only normal return from that free reaches the hidden stores that publish the new buffer and requested capacity. Count is never assigned by BF05D0. Capacity is the clamped argument value, reloaded from the argument slot after either loop where ESI was reused.

## BF0700 resize: 125 bytes

`BF0700..BF077C` calls the full reserve operation only when requested signed count exceeds current signed capacity. It then compares current count with the request. Growth computes the initial wrapped byte offset and a fixed wrapped iteration count, reloads the buffer on **every** iteration, and zeroes only the first eight bytes of each nonzero computed record address. The third DWORD remains untouched. A zero address skips stores rather than establishing a valid null-buffer policy.

The shrink comparison rereads current count, including after growth. Each shrink iteration publishes `--count` **before** reading that record's data/length and before a potentially throwing pool getter. It captures nonnull data, then `length+1`, resolves actual 419CC0 and returns through BD1510. It clears neither record header nor third word. It rereads current count after the provider returns. Only normal completion publishes the final requested count. A throwing getter leaves the already-decremented count in place and does not perform the not-yet-called return.

## Exceptional ownership and current providers

The independently recovered CC7790 action uses parent EH EBP equal to entry ESP, not the physical EBP loop counter. Its locals are:

| EH parent slot | Published by BF05D0 | Meaning |
| --- | --- | --- |
| `-18h` | BF0623 | Newly allocated buffer base |
| `-14h` | BF0627 / BF0692 | Completed record count |
| `-10h` | BF063B | Current candidate destination |

CC7790 computes end = allocation + `12*completed`, pushes end and current candidate as begin, and calls **401130, a complete one-byte RET**. In the ordinary armed copy iteration those boundaries coincide. Irrespective of that equality, the actual helper performs no destruction, free or rollback. State zero covers the candidate copy; BF068A disarms before the old-string release pass and old-buffer free. Therefore no prefix guard, array free or catch-based recovery may be invented. Allocation failure occurs before state zero; copy failure leaves the allocation and completed/candidate string storage unpublished without cleanup supplied by this action. A later old-release getter failure can leave some old headers pointing to returned storage and the new copied buffer unpublished. Native asynchronous/FH3 eligibility remains separate from this concrete action contract.

Existing source providers are available, with their original limits:

- `resize_native_string_header_0041dd40(void*, NativeStringRawPoolContext&, uint32_t, bool)` in `native_string.cpp` preserves current fields and propagates getter exceptions. The `NativeStringStorage` overload and its `noexcept` release adapter have a different exception boundary and must not replace this overload.
- `native_string_pool_get_or_create_00419cc0` and `return_native_string_pool_00bd1510` operate on the actual publications `01090AA8/01090AA0` and live small-return gate `01090AA4`. The getter must still run for large releases and disabled small returns. BD1510 itself is `noexcept` in this established source domain.
- BF7680 is the existing CRT copy dependency with an overlap-aware backward path. The established raw-string source policy uses `memmove` for nonzero copies and omits zero-byte calls. Valid ranges and the explicit source call-boundary qualification remain necessary.
- BF55BE jumps to BF681B, whose retained listing shows malloc, new-handler retry and bad-allocation throw behavior. BF6989 jumps through BF65AC to BF9DC8 free. Existing `singleton_lifetime_allocate({object,native_bytes,host_bytes})` and `singleton_lifetime_free` are the shared **host CRT service boundary**; current native-path scratch allocation passes equal native/host byte sizes. They are not proof of the original CRT heap, new-handler globals or exception-object identity. Current source files and the relevant ledger/export evidence are retained. This discovery adds no allocator or vector substitute.

A minimum future source packet consists of both complete reserve and resize bodies, raw record operations, the actual raw string-pool context, and an explicitly accepted common allocation/free domain. The algorithms and no-op cleanup are established; original native heap/EH identity is not source-closed by the shared host service. Third-word meaning is unnecessary for a raw bit-preserving implementation and remains unknown. No implementation is delivered here.

## Listing diagnosis and proposed recovery

Live listing has 101 instructions in BF05D0; disk decoding has 106. The only missing region is `[BF06DD,BF06E7)`, ten PE/live-matched bytes `83c404893b5f8973085d` immediately after the listed direct call `BF06D8 -> BF6989`:

| Address | Physical instruction |
| --- | --- |
| BF06DD | `ADD ESP,4` |
| BF06E0 | `MOV [EBX],EDI` — publish new buffer |
| BF06E2 | `POP EDI` |
| BF06E3 | `MOV [EBX+8],ESI` — publish capacity |
| BF06E6 | `POP EBP` |

No call instruction is hidden in this gap. All six BF05D0 and three BF0700 direct calls are fully decoded and present in the live listing. Current BF05D0 pseudocode ends at free and omits the two publications. Its displayed body bounds are BF05D0..BF06F9; those minimum/maximum bounds alone do not prove membership of every missing byte.

The repository's existing flow-repair helper documents recurrent CALL_RETURN overrides after CRT-free calls, but this historical diagnosis is **not an observed current property value**. The new command `python tools/bsp.py ghidra flow-properties <addresses> --output local/result.json` verifies the target, uses fixed read-only Java through the supported bridge endpoint, preserves raw response/query/identities and exits unsuccessfully on errors or mismatched rows. It queries instruction override/default/effective flow/fallthrough, containing body and function no-return/thunk state. Installed Ghidra 12.0.4 API signatures were inspected. Actual Java execution remains unverified because the bridge returned `Script execution disabled`; the final exact response is `local/raw_reader_storage_br/flow_properties_disabled.json`. The earlier attempt is also retained. No settings, restart, mutating dry run, program write or save was attempted.

The reviewable future procedure is conditional:

1. With a supported read-only capability available, reverify target and full PE/live bytes; capture exact BF06D8 override/default/effective flow/default/current fallthrough and BF6989/BF65AC/BF9DC8 no-return/thunk values. Record current BF05D0 body membership and documentation. Those old values are currently unknown, not `NONE`, `false` or `CALL_RETURN`.
2. Only if the observed call-site override is CALL_RETURN and the recovered fallthrough is still absent, approve clearing **that one override to NONE** under the existing address lease and Ghidra write lock. Keep callee flags untouched. If another property explains the gap, stop and revise the proposal rather than apply an assumed fix.
3. The existing `ghidra_flow_repair.py 00bf05d0 --apply --record <owned-record.json>` provides the locked clear/disassemble operation; inspect its current code and preconditions before use. Recover exactly BF06DD..BF06E6, preserve existing instructions/comments/names, and record all before/after properties. No function deletion or recreation is called for.
4. Verify 106 listed instructions, complete bytes, all nine call rows, the two publication stores, continued epilogue and missing-byte body membership. If body membership still excludes the recovered instructions, handle that as a separately reviewed bounded repair. Refresh affected exports/comments only after review and save only in the authorized integration step.

The report includes the full proposed old/current/new-value table, not an applied mutation record. No C++ changes, build, tests, runtime/game probes or push occurred in this packet.
