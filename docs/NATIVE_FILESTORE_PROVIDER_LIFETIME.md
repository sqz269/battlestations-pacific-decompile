# Native FileStore provider lifetime

Addresses: `00BE7FA0`, `00BE80B0`, `00BE8120`, `00BE7BF0`, `00BE8090`.

Five complete source bodies (596 original bytes) operate on actual Win32 storage. Names are descriptive hypotheses. Explicit C++ services preserve the actual application string pool and raw lifetime domain; this is not a binary replacement or game-validation claim.

| Entry | Bytes | Coverage | Original ABI |
|---|---:|---|---|
| `00be7fa0` | 238 | complete in stated source domain | ECX raw2Ch owner; EAX original owner; RET0 |
| `00be80b0` | 105 | complete in stated source domain | ECX factory0Ch; no stacked arguments; EAX captured cached/new provider; RET0 |
| `00be8120` | 43 | complete in stated source domain | ECX factory; stacked actual8h system/virtual headers; EAX provider/null; RET8 |
| `00be7bf0` | 180 | complete in stated source domain | ECX actual2Ch provider; no stacked arguments; RET0; no semantic result |
| `00be8090` | 30 | complete in stated source domain | ECX owner; stacked DWORDflags; EAX original owner; RET4 |

The 2Ch provider contains the BB5590 base through+13, resident tree at+14 and pending tree at+20. Both tree+0 words remain untouched. Construction uses an actual empty8-byte name header, forms two1Ch sentinels in order, and publishes D689E8. The factory getter reads cache+8 once for a fast return; slow construction publishes only after success, without locking, acquiring a reference, or registering the provider.

`BE8120` rejects zero system-name length, then consumes AL from the actual425850 case-insensitive comparison against CFF1FC `filestore`. The virtual-name header is never inspected. Nonzero recorded lengths are not compared against the literal length.

Normal destruction installs D689E8, clears resident entries, erases/frees/resets the pending tree, then the resident tree, then destroys the provider base. The raw tail BE7C4A..BE7CA3 was initially missing from the saved function body; after verified local flow repair, supported locked recreation restored the complete body and preserved its previous plate comment. See `reports/native_av_function_definitions.json` and `reports/native_av_flow_repair.json`.

D689E8 slot0 is BD30E0, which invokes current slot4 with flags1 and does not decrement the reference count. Slot4 is BE8090: destroy first, optionally free using flags bit0, return the original address. Neither destruction entry clears the factory cache.

## Exception ownership

Constructor FuncInfo E017B8/mapE01798 arms state0 for the temporary header, jumps to state2 after the base succeeds, and arms state3 before pending allocation. State3 calls BE7BB0 on resident+14, then BB5380 on the provider; state2 cleans only the base. The metadata state1 row exists but is never assigned by the observed normal body. Getter mapE017DC separately frees the captured2Ch allocation if construction fails. Direct construction leaves that allocation to its caller.

Destructor FuncInfo E01738/mapE01720 chains state2 BE7A70(pending), state1 BE7BB0(resident), state0 BB5380(base). The state is lowered before starting each member cleanup. A failing current member is excluded from the remaining unwind chain; detached nodes are not helpfully reclaimed. Both distinct tree destructor entries were verified separately.

Cleanup guards implement the C++ domain only. Original FH3/SEH transport, arbitrary spill aliases and a second exception during cleanup are not proven. Existing noexcept string release excludes throwing lazy pool recreation.

## Validation

Strict Win32 build and both existing CTests passed. `local/native-av-provider/attempt03` passed 19 original/source states and 1357 checks, plus five source-only allocation-failure cases. All 558 frozen inputs remained unchanged; 127 linked objects matched the actual archive members. The original five bodies were copied unchanged with a uniform address delta; external calls use exact-ABI adapters to established source bodies and genuine CRT services. Native FH3 handlers were not executed.

The focused cases cover empty/populated destruction, direct and lazy construction, selector case/length behavior, cache reuse and stale cache retention, flags0/1/2/101, untouched words, sentinel/free order and actual small-string returns in one raw lifetime domain. The retained stream uses ref2 and ends atref1; zero-reference stream deletion is not exercised. Constructor state0 and destructor exception transitions are statically reviewed only.

Failed attempt01 is immutable: its driver incorrectly selected vtable slot0. Exact8-byte profile recapture and the corrected slot4 call fixed the fixture; production source was unchanged. Later source/header refinements require a fresh seal, never a rewritten attempt.

## Follow-up packets

- `native_filestore_runtime_dispatch_aw`: add finite D688B4 Create and D689E8 zero-reference/deleting routes to existing native runtime bindings, borrowing the same actual contexts.
- `native_vfs_factory_selection_aw`: reconstruct BDB040..BDB094, including BDB078 factory slot4(system,virtual). Native BE1890 mount, BE1740 insertion, MPKG BB9D90 and MPAK BB83A0 remain separate named but incomplete dependencies.

Detailed call sites, raw bounds, hashes, native ABI, exception maps, preserved attempts and limitations are in `reports/native_filestore_provider_lifetime.json`.
