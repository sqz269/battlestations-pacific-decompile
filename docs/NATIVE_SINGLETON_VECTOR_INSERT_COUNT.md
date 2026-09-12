# Raw singleton vector count insertion

`insert_count_native_singleton_slots_00bd0700` reconstructs the complete original **00BD0700..00BD08B3, 436 bytes**. It covers arbitrary unsigned counts, growth, and both in-place branches. This is a raw four-byte-slot vector operation with fixed completed providers. The previous count-one typed-manager specialization is not used.

## Original ABI and representation

The source is an MSVC Win32 naked fastcall entry. ECX contains the actual raw owner; incoming EDX is unconsumed. Stack arguments are `(iterator_owner, position, count, value_slot)` and every normal path uses `RET10h`. Iterator owner is unconsumed. There is no specified return-register result. EBX, ESI, EDI and EBP are preserved on normal return. Owner fields are untouched DWORD+0, begin+4, end+8 and capacity-end+0C; slot width is four bytes. No manager vptr, typed projection or private allocation domain is introduced.

The original instruction schedule is retained, including raw modulo32 subtraction and `SAR2`. This avoids introducing C++ unrelated-pointer subtraction or typed alias assumptions. Descriptive names are reconstruction hypotheses, not recovered symbols.

## Complete behavior and aliases

The first load dereferences `value_slot`, then stores that DWORD into this invocation's value-slot argument word, **before the count-zero test**. Capacity is captured from current begin/capacity-end before that test too. Count zero therefore is not a blanket no-access operation. The input iterator-owner word is never loaded.

For nonzero count, subtraction from `3FFFFFFFh` checks the captured-begin/current-end size against count. The size calculation uses signed `SAR2` and raw unsigned comparison. A failure invokes the actual complete BD0590 throw provider. Capacity comparison retains the captured capacity. Growth computes `capacity + (capacity >> 1)`, replacing that candidate by zero when it exceeds the native limit; the candidate is then raised to current size plus count. No generic `std::vector` arithmetic replaces these operations.

The growth branch calls BCFEB0 with the candidate in ECX and zero EDX, retains returned storage, copies prefix from current begin to original position, fills count copies from the captured argument word, then copies the tail from the reloaded position argument to current end. After those services, it reloads current begin/end, captures the then-current size, adds count, and frees the then-current nonnull begin. **BD0801's `ADD ESP,4` and the stores after it are part of the complete body.** The replacement begin, capacity-end and end are published in that order. Neither pre-copy size nor original value/position memory is silently substituted for the native reload schedule.

The two in-place branches remain separate:

- With unsigned tail length less than count, copy the retained entire tail to position plus count times four. Reload current end, derive the excess count from that end and retained position, then fill starting at that current end. Reload saved byte count, add it to current owner end, reload the new end, and assign from retained position to that end minus byte count. A returning memmove invalid handler can therefore affect subsequent end loads; these captures and reloads are not coalesced.
- With tail length at least count, copy the retained last count slots to retained old end, publish the returned end, shift the remaining middle range backward using retained position/tail-start/old-end, and assign the insertion range. Both cdecl calls retain their arguments until the combined final `ADD ESP,18h` exactly as native. The saved count word becomes byte count; the fill/assign argument points to the captured value word. Aliases to invocation storage retain that schedule.

## Providers and exception ownership

The twelve native direct calls map to seven fixed completed source symbols: BCFEB0 allocation, BD0590 length throw, BD0500 copy, BD0560 fill, BD0160 assign, BD0180 backward copy, and existing `singleton_lifetime_free`. Incoming ECX at the stdcall leaf calls remains the raw owner even though those helpers do not consume it. No service callbacks or unresolved placeholder calls are introduced.

BCFEB0 uses actual shared `singleton_lifetime_allocate`; the current source CRT owns its malloc/new-handler/free domain. BD0590 owns an actual 28h legacy length-error payload through completed 408720, 411700, 411940, 411780 and 4072D0 behavior and throws `NativeSingletonVectorLengthError`, with new source catch type/RTTI/FH3 transport. It is not a host `std::length_error` replacement. Allocation overflow/failure uses source `std::bad_alloc`. Original static-CRT heap, encoded handlers and throw-object/RTTI identities remain explicit boundaries.

The copy leaves call the actual SDK `memmove_s`, which reaches current source errno and invalid-parameter services. Returning handlers and ignored status are retained, but original BF66EF/109DD64 handler ownership and service-internal volatile registers/flags are not reproduced. Source allocations must belong to the paired current CRT.

The original BD0700 has no FS exception registration, FuncInfo, unwind map or replacement-allocation cleanup. The naked reconstruction likewise emits no local EH frame. Provider exceptions propagate; no compensation is added after replacement allocation when a later service throws. Source handler/catch effects, hardware fault addresses and arbitrary native SEH identity are not claimed.

## Evidence and validation

Primary ingested the reviewed source draft after the worker stopped; no worker build or seal is claimed. Fresh guarded queries verified the existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and all four owned entries against the installed PE SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. BD0700 contains exactly436 bytes/181 instructions. The emitted body matches every original byte except the twelve named four-byte CALL operands; each resolves to the actual completed source provider. It is the only owned CODE section. The four-byte weak AVX2 BSS symbol comes from MSVC headers and is not manager storage.

The strict Win32 main build passed both existing CTests and all eight native reference seeds. Primary froze29 unchanged prebuild source/header/build inputs,7 exact archived objects,7 actual compiler commands and195 compiler read dependencies. All133 complete provider CODE sections and associated data/EH sections match the previously accepted main build. The worker's completed insertion static checker was replayed unchanged on the frozen main archive; its unfinished build/seal templates were not used.

Ghidra's BD07FC CALL_RETURN override was cleared with the old value recorded, restoring BD0801 ADD ESP,4. Names/comments are saved and exports refreshed. The [audit](../reports/native_singleton_vector_insert_count_audit.json) and immutable `local/vector_insertion_primary/` evidence distinguish native bytes, source compilation and static archive proof. No new test or probe was added. Existing tests do not execute this entry, and game integration, original CRT/EH identity and complete manager destruction/registered-owner dispatch remain unproven.
