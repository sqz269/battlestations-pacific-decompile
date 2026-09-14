# Raw unit-part destruction (AO)

Addresses: `00712C80`, `007112E0`, `00712FD0`, `007102A0`, `00710730`, `00711120`.

The three destructor bodies now operate on actual part, entry and effect storage through required borrowed service bindings. This completes their physical instruction paths; executable admission of populated raw unit parts remains open. Descriptive names are hypotheses, not recovered symbols.

| Body | Inclusive physical end | Bytes | Native ABI | Source |
| --- | --- | ---: | --- | --- |
| `00712C80` | `00712F18` | 665 | ECX=1ACh part, RET | `destroy_native_unit_part_00712c80` |
| `007112E0` | `0071136B` | 140 | ECX=10h entry, RET | `destroy_native_unit_part_entry_007112e0` |
| `00712FD0` | `00712FED` | 30 | ECX=part, stack flags, EAX=original pointer, RET4 | `delete_native_unit_part_00712fd0` |
| `007102A0` | `007102FE` | 95 | ECX=array, stack requested capacity, RET4 | canonical `00B1C500` reserve |
| `00710730` | `0071077F` | 80 | ECX=array, stack requested count, RET4 | canonical `00B1C770` resize |
| `00711120` | `00711136` | 23 | ECX=array, RET | canonical `00B1D1D0` destroy |

The new interfaces use EDX for a stable access record. They are not drop-in binary replacements. Array specializations have equal normalized instruction graphs after aligning their reserve/resize calls; reuse retains the canonical valid-storage and representable-allocation contract.

## Ownership and ordering

`712C80` writes CFD7B8, detaches through the actual spatial publication only when both owner+164 and byte+184 are nonzero, then clears +184. It captures selected+160 before clearing F8. The selected root+0C is tested and reread; a matching actual E188DC publication is cleared through the canonical renderer call. Selected release performs the actual atomic decrement and current slot-0 dispatch before clearing its cell; a second unconditional +160 store is retained.

The entry loop uses live signed +1A4 and reloads +1A0. Each captured nonnull entry is destroyed and freed, while stale pointer slots remain. An explicit resize0 precedes the member array's second resize0/free. Cleanup then destroys lists at +194, +188 and +178, group rows at +168, any republished selected set at +160, and the collision base. The base writes CE89E8 and frees current nonnull +FC while retaining that dangling value. The part allocation remains owned by the caller.

`7112E0` walks live signed entry+8. It tests and rereads each current effect pointer, calls the canonical effect stop with actual lock/lifetime services, sets captured effect+9 to one, then reloads the backing array before clearing that slot. It destroys the pointer array but retains both the entry and effect allocations. The scalar wrapper calls the complete part destructor before testing flags bit zero; only that bit frees the part, and EAX returns its original address.

## Exception evidence

The original part handler C84A8C refers to FuncInfo DB32A4 and seven-state map DB32C8. Reviewed funclets C84A30..C84A8B unwind, from state six downward: entry pointer array, shape list194, shape list188, list178, group rows168, selected cell160, collision base. Entry handler C848BB refers to FuncInfo DB3024 and map DB301C; its sole action C848B0 destroys entry+4. The source preserves those member cleanup states for C++ exceptions. Throwing cleanup during unwinding terminates.

The focused source case throws during effect child deactivation: the entry array and all seven part members are cleaned, the effect retirement store remains unreached, the actual effect lock is released, and the scalar wrapper retains the part allocation. Native FH3 exception dispatch and fault delivery were not executed or proved.

## Validation and analysis limits

Six paired original-byte cases pass: five part scenarios cover empty/null-owner, populated grid and loose detachment, listener match, selected count zero/nonzero, repeated/sparse effects, and scalar flags 81h/2; the sixth exercises array growth and destruction. Full normalized live images, ownership and actual CRT free order/preimages match. Freed storage is never read afterward. One additional source nested-unwind case passes. Repeat runs have identical result hashes. No repository tests were added; strict MSVC Win32 build and both existing CTests pass.

The reference calls canonical source dependencies for spatial removal, selected release, listener/renderer operations, effect stop, allocation/free and lock services. These are shared boundaries. The fixture selected-set terminal owns and frees its fixture allocation; it does not identify the game's unresolved terminal target. Renderer child+30 is null in these fixtures, so its deeper invalidation path is not newly exercised.

All six physical bodies (1,033 bytes) match live Ghidra and the unchanged original PE. Eighteen direct calls pass stored-function ownership checks; nine calls in the decoded part tail are recorded separately because Ghidra does not yet assign them to a function. Four indirect sites and the exception maps were inspected. All eight native seed checks pass. Compiled object review confirms reloads, signed loop tests, double resize, cleanup order and post-destructor scalar flag testing.

The locked flow repair decoded six gaps, preserving prior values in `reports/native_unit_part_destruction_flow_ao.json`. Stored bodies remain truncated at 712D7C, 711357 and 711131. The supported body-only script request was refused because Ghidra script execution is disabled; no body extension was applied. The full physical tails remain explicit evidence rather than being counted as successful stored-body checks. The original installation was not modified.

Evidence: `reports/native_unit_part_destruction_ao.json`; local replay inputs and receipts under `local/unit_part_destruction_ao/`. Actual selected-set terminal dispatch, native exception delivery, populated raw executable admission and gameplay parity remain open.
