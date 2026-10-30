# Native Lua variant retirement contract design review

The bounded design in immutable `83a63b4df1aa5962208bfb232d00da331ced9c45` **passes** independent contract review. The admitted implementation scope can be the three normal Source bodies `00886920`, `00884AF0`, `006EE890` and the qualified whole-range library operation reached through `006EE960`. No hidden normal-flow provider or unjustified input subset is required by that cycle. This is design approval only: no mutable candidate was inspected and no Source implementation, emission, build, ABI, runtime or application admission is granted here.

Review baseline is published Main `03ef58daf98d9dff844e8a4039be830868d429b7`. The complete 213-line readiness document and its report were read from the immutable commit. All 26 retained relevant current inputs still match its LF hashes. All 19 recorded native code/data blocks (`1,236B`) match the preserved installed PE; selected code blocks decode completely. No live Ghidra or target execution was used.

## The reached range contract follows the actual call

In `00886920`, tag5 captures nonnull map `M`. At `0088693F` it reads `H=M.head`; at `00886942` it reads `H.left`. It passes `first={M,H.left}`, `last={M,H}` and a separate private output iterator to `006EE960` at `0088694F`. The argument-building interval contains no call before that range operation.

For genuine live map12/node52 backing and no ambient concurrent mutation during this prefix, the actual validations necessarily select the whole-range branch:

| Native check | Actual argument proof |
| --- | --- |
| `006EE96B/976`, invalid handler `97A` | First owner is captured nonnull `M`, equal to the receiver. |
| `006EE972/983` | First node equals the same live head's left pointer. |
| `006EE98B/992`, invalid handler `996` | Last owner is the same nonnull `M`, equal to the receiver. |
| `006EE98D/99B` | Last node equals the current same head. |
| `006EE9A1..9AA` | Reload current head, read its current parent/root and call real `006EE890(M,root)`. This is the first external call on the valid path. |

The two invalid-parameter calls are not run on these actual valid arguments. No generic checked-iterator increment, single-node erase, `006EE5D0`, copy or arbitrary invalid-handler provider is reached. This proof applies to empty and nonempty actual maps and does not depend on count, key order, payload tags, a zero proxy word or a fabricated container. Header `M+0` is not read or changed by the selected operation. Invalid backing, foreign/mismatched iterators and concurrent races are not converted into success; general `006EE960` behavior remains outside this qualified contract and existing general Host APIs remain unchanged.

**Implementation barrier:** no journal allocation/vector growth, logging, virtual adapter, validation callback or other external call may be inserted between endpoint capture and branch/root selection. Such a call could change `M.head`, invalidating the proof. Pure independent metadata stores are acceptable. Fallible metadata preparation must retain correct native capture epochs outside this interval. The immutable Source review must check this actual implementation detail.

After subtree normal return, the library operation must preserve these ordered effects, even for an empty tree:

1. Reload `H1=M.head`; write `H1.parent=H1`.
2. Reload `H2=M.head`; write `M.count=0`, then `H2.left=H2`.
3. Reload `H3=M.head`; write `H3.right=H3`.
4. Reload `H4=M.head`, capture its current left; write output owner `M`, then output node; return that output.

There are four distinct head reloads. An early cached sentinel, early output, unconditional pre-clear, or manufactured empty-map result would change the contract. After range return, variant clear again reloads CURRENT `M.head` for sentinel free, clears captured `M+4/+8`, frees captured `M`, then clears retained variant `V+8` and sets tag=-1. Captured `M` and retained `V` must not be replaced by a reload of `V.payload` after callbacks.

The private output iterator does not alias the separately owned map/node allocation in this actual caller. This is an established caller shape, not a new restriction on arbitrary users of the full general range API. Native stack-home/alias identity and binary ABI are not supplied by a new Source interface. Padding NOP `006EE9DF` is unreachable on this ordinary path. The selected 201-byte range span is evidence, **not a whole Source `006EE960` implementation or vendor body credit**.

## Concrete services close this cycle

The raw `NativeStringRawPoolContext` is three borrowed actual references: pool `01090AA8`, manager `01090AA0`, and small-return gate `01090AA4`. It creates no alternate owner or callback. Existing `destroy_native_string_header_0041DD20(raw context)` and `006EE020` use the real raw-manager pool getter before every nonnull buffer return, including large buffers and disabled small returns; then they invoke the real `00BD1510` return with the live gate. Capture buffer first, skip length/getter for null, use wrapped DWORD length+1, and leave headers untouched.

Descriptor8/map12/node52 storage uses the matching `singleton_lifetime_allocate/free` Source heap. The existing free is qualified `std::free`, not C++ delete or numeric PE execution. Existing `006EDEA0` allocates real52-byte backing; `006EEAF0` publishes/reloads the head and sets nil/link/count fields without initializing header+0. These leaves establish available allocation/field services, not automatic pointer authentication or full producer adoption. The same raw pool/manager/gate and actual deletion domain must survive all normal, failed and eventual diagnostic uses.

No Lua interpreter callback, arbitrary current variant vtable, projected `std::map`, semantic string object or broad Host implementation is needed inside the three-body SCC. Numeric `D0E6F4` remains a stored identity; this direct cycle does not turn it into a callable Source table. Resize's current scalar-slot transport and all queue/payload/host adoption remain separate.

## Factoring the existing kernels

The existing Left55 and Seg33b bodies are real raw normal bodies, not placeholders. A shared kernel/narrow concrete composition is viable, but factoring must preserve the existing public Host behavior:

| Existing boundary | Preservation requirement |
| --- | --- |
| Left55 `variant_clear_00886920` | Keep tag1 descriptor capture/release/free and tag5 captured-map range call/current-head free/field resets/outer frees in order. Broad entry must still call the supplied `map_erase_range`, pool-return and free methods. |
| Left55 `variant_destruct_00884AF0` | Stamp identity, call clear, then release CURRENT name. Preserve supplied Host callbacks and their C++ escape behavior. |
| Seg33b `lua_map_erase_subtree_006EE890` | Keep direct right recursion, post-return CURRENT left capture, supplied value destructor, post-value CURRENT key-buffer/late length release, captured node free and continuation with captured left. |

The new concrete entry may select the actual narrow services directly. It must not instantiate a broad `Left55Host`, `Seg33bHost` or `LuaVariantHooks` with unused defaults, throwing stubs or fake success. It must not silently replace legacy Host callbacks with the canonical raw pool/free, or add mandatory operation-destructor termination to an existing Host exception path. Shared implementation changes earn no duplicate recovery credit.

## Alias, failure and retained custody

Native behavior is broader than an empty or frozen-header fixture. Any nonzero nil BYTE31h terminates subtree traversal; unknown/nonselected variant tags still reach tag=-1. Null selected tag1/tag5 payloads do not add a payload clear. No maximum depth, count-equals-node check, nil==1 rule, profile whitelist or old-owner registry is justified here.

Callbacks may change live fields. Preserve the current/captured distinction: right child is selected before recursion, left only afterward; key buffer only after value destruction; name buffer only after clear; each conditional length is read at its native epoch. Pool getter/return uses captured block/size even if the header changes. Free captured descriptor/map/node after normal child return; do not free a newly substituted variant payload. Do not ban a valid replacement or authenticate it using a numeric stamp, an old address, phase, or progress record. Matching live backing/heap/pool custody is a caller precondition, not automatic authentication.

Failure may leave freed descendants, stale public root/count, a still-live node/left/key/value, returned buffers with stale headers, an unfreed descriptor/map/sentinel, or overwritten payload aliases. Independent retained operation/context and entered/completed action metadata must survive the throw; zero/reset public fields are not completion or safe replay evidence. Do not infer allocation lifetime from a stale pointer and do not start catch-time rollback or retry an entered free.

Native destructor state0 actions are pinned: `C970D0 ->0041DD20(V+0C)` and `C830A0 ->006EE020(K)`. Normal demotion precedes the optional final string return. Their action-frame/FH3 execution is not established by this Source plan. Preserve existing legacy C++ behavior when factoring and state the new concrete Source failure contract explicitly; retained state is not Native EH restoration or permission to abandon active providers. Metadata allocation/failure/reentry and all irreversible-call capture details require review on the eventual immutable candidate.

## Review disposition

The design is sufficient to proceed with only the approved three bodies plus the reached whole-range service. It does not admit a general range function, broad host, current-slot scalar dispatcher, resize/copy/assignment, queue/payload parent or full Game/application graph. Eventual Source admission still requires independent immutable source review, relevant compilation/emission/common-build evidence, preserved guards and separate runtime/ABI gates. No candidate source or build artifacts were inspected in this design packet.

Only this document and `reports/cc16_native_lua_variant_retirement_contract_review.json` change. No C++/shared/Ghidra/build/test/executable/DLL/Lua-state/game action occurred. New Source/native/vendor/ABI/runtime/activation credit is zero.
