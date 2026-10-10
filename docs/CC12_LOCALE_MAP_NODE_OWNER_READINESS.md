# CC12 locale map node owner readiness

`00737CE0` returns the node's 16-bit payload first, then reloads and returns its narrow-byte payload. Each non-null payload performs its own `00419CC0` getter followed by `00BD1510`; pointer and byte size are captured before that getter. The function's own instructions leave both headers and the node allocation unchanged. Its normal ownership order is established, while its state-0 exception cleanup remains outside this packet's admitted scope.

Only `00737CE0..00737D56` was expanded: **119 bytes, 38 instructions**, below the 512-byte limit. All bytes match the installed PE and the complete contiguous Ghidra body/listing. Exact typed flow records no no-return flag, entry override, or body gap. Every live batch verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`; modification number remained 55. No other Native provider, exception selector/map/action, or literal-data family was opened, and no Ghidra mutation occurred.

## Raw fields and normal return

| Node offset | Observed role |
| --- | --- |
| `+00h` | Narrow-byte length, read only when the current `+04h` pointer is non-null. |
| `+04h` | Narrow-byte pointer, reloaded after the wide stage finishes. |
| `+08h` | 16-bit code-unit length, read only when captured `+0Ch` is non-null. |
| `+0Ch` | 16-bit payload pointer, captured before the first getter. |
| `+10h` | Not read or written by this body; remains opaque. |
| `+14h` | Map caller's saved next link; not read or written by this body. |
| `+18h` | Map caller's later block-index read; not read or written by this body. |

The character-width interpretation is supported by the size expressions and existing actual-header Source contracts. No text encoding, key hash, or unused-field semantics is inferred. The `1Ch` node-slot interpretation and the map's ownership of node recycling remain separate raw-map dependencies.

The original receiver is `ECX`, captured in `ESI` at `00737CF7` and saved on the stack for exception handling. There are no stack arguments, `ESI` is preserved, and the epilogue restores the exception link and ends with plain `RET`. No meaningful `EAX` return contract is established.

1. `00737CFD` captures `node+0Ch`. `00737D02` arms state 0. A null pointer skips both the length read and the wide provider calls.
2. For non-null wide data, `00737D0C..16` captures current `node+08h`, computes `2*length+2` with DWORD wrapping, and pushes `(block, byte_size, unused=1)`. `00737D17` calls the no-argument getter `00419CC0`; `00737D1C` moves its returned owner into `ECX`; `00737D1E` calls `00BD1510`. The pending three-argument packet stays intact across the getter.
3. Only after that branch finishes does `00737D23` reload `node+04h`. `00737D28` writes state -1 before the narrow provider calls. A null narrow pointer skips its length read and both providers.
4. For non-null narrow data, `00737D32..3A` captures current `node+00h`, computes wrapping `length+1`, and pushes `(block, byte_size, unused=1)`. `00737D3B..42` performs another getter followed by return through that getter's owner.

There is no cached pool shared across stages, no length/overflow validation, no header zeroing, no alias-deduplication, and no node-slot return in this body. A non-null pointer always calls the getter before return, even if a lower-level return provider would ignore the pool for that size or shutdown gate. Getter/return internals remain existing provider dependencies; they were not reopened here.

## Mutation and failure boundary

The first pointer and byte size survive the first getter as captured arguments. Provider changes to the wide header cannot retarget that pending return. The narrow pointer and length are read afterward, so wide-stage provider changes to the narrow header can affect the second stage. Each narrow pointer/size pair is then captured before its own getter. The body performs no node-field stores, but this does not promise providers leave the node or other shared state unchanged.

The node body installs handler selector `00C86268`, saves its receiver, and visibly changes state `-1 -> 0 -> -1`. State 0 is armed for the wide getter/return. The wide-pointer load precedes that arming; the later narrow-pointer load precedes disarming. The narrow getter/return execute after state -1 is written. These are exact instruction boundaries, not a reconstruction of the selected unwind map or cleanup action. In particular, **this packet does not establish whether or how the narrow header is cleaned if the wide stage throws**. The selector, FuncInfo, map and actions were not inspected.

On normal return, control goes back to the raw map's separately retained order: its next node was saved before this call, and its current node `+18h`/pool-table reads and inline node recycling follow afterward. On an escaping failure, the map does not reach that node's recycling or its final bucket/count clears. Earlier node returns and payload/provider side effects are not rolled back. Repeating this call is not known safe because headers are left unchanged; aliasing payload pointers are not protected against repeated returns. Valid receiver, payload, saved-next, context-cell, and provider lifetimes remain required.

## Existing Source providers and canonical context

The current Source already has the exact eight-byte narrow header schedule in `destroy_native_string_header_0041dd20(void*, NativeStringRawPoolContext&)`. That overload captures pointer and `length+1`, uses the actual raw manager/string-pool getter, and lets getter exceptions propagate. Its complete 51-byte COFF section has the expected getter and return relocations. It is a suitable normal-path narrow-stage building block; it does not supply this node's state-0 cleanup.

`NativeStringRawPoolContext` borrows the real `01090AA8` pool-publication cell, `01090AA4` small-return gate and canonical `01090AA0` manager-publication cell. `GameNativeStringProcess` owns those retained process-lifetime cells and initializes both the raw context and storage bridge from the same cells. `GameVfsHost::raw_strings()` exposes its existing raw context. This requires borrowing the canonical cells rather than caching a pool pointer or creating a private registration domain.

The existing wide destructor has the right normal header schedule: capture non-null data, compute `2*length+2`, release, leave the header unchanged. Its public Source interface currently accepts only `NativeStringStorage&` and is `noexcept`. `NativeStringStorage::release` and `ActualNativeStringPoolStorage::release` are also `noexcept`; a failing lazy getter through that interface terminates instead of propagating to this node's native cleanup state. No raw-context wide overload was found in the current declaration. The accepted `00432050` range helper likewise uses the `noexcept` storage interface, walks eight-byte narrow headers forward, and does not free the range allocation. It is neither this node's wide-then-narrow owner nor an exception-compatible raw context by itself.

Seven complete current Source sections total 431 bytes: raw narrow destructor 51, storage-interface narrow destructor 28, wide destructor 34, storage bridge getter 35, range helper 41, canonical process factory 234, and VFS raw-context accessor 8. Full section bytes, instructions and relocations are retained. Existing semantic `LocaleTables::Node` strings/unique_ptr ownership is not substituted. Payload pool `01090AA8` is also distinct from the map's node-slot section/table/minimum at `00E17698/00E176B4/00E176C0`; proving the payload provider does not bind that separate node pool.

## Evidence qualification and next frontier

The immutable epoch remains `Source3352-particle-model-typed-views`. All 3,352 project inputs and four artifacts matched current Root bytes at capture. Actual Git blobs match all 3,352 worker inputs; 3,345 match the frozen compiler-input bytes exactly and seven differ only in line endings. All byte cohorts are retained separately. The full Core archive was reparsed (1,991 members, 1,988 ordinary COFF objects), together with 70 frozen `bsp_game` target objects. Root's separate 80-object current inventory is not borrowed. No exact address-named `00737CE0` symbol occurs in that frozen index scope; Source interfaces and ownership were inspected separately.

Whole constructor, profile, base and raw-map worker reports are hash-pinned dependencies; their original captures remain intact and their proof trees are not recursively copied. Constructor/profile are accepted. Base `90c7ba1a2` and raw map `cf42da5cd133bc1e13465e4275e083f97a510968` remain pending Root acceptance at capture. No fresh Native evidence for those previous bodies is claimed here.

After that immutable capture, Root accepted base at `260638e2c` and raw map at `d5f24c14e`, integrated/pushed on main `3129866ef`. A separate acceptance dependency archive retains the two complete review documents and two complete review reports from that exact Git commit, with verified Git blob identities. An independent Git batch confirms all 3,352 Source blobs at that main commit still equal the original node-capture Git cohort. Both original-location and physically relocated ZIP-only acceptance replays verify the retained whole-file hashes; they do not replay live Git or import Root's proof claims. The original pending-status captures and main evidence bundle remain unchanged.

The smallest missing Native frontier is the node's **directly referenced `00C86268` exception selector, exact referenced FuncInfo/map, then any required action under separate admission**. That must resolve state-0 cleanup and exceptional rereads before a complete node helper is implemented. The smallest later Source frontier is a raw-context wide return plus node composition using the existing raw narrow overload, with the recovered cleanup state and canonical context lifetime explicit. The map's node-pool binding and derived locale destructor exception/range contracts remain separate gates. **Complete node/raw-map implementation GO: false. Application activation GO: false.**

The adjacent JSON report pins both successful ZIP-only replays, including physical relocation and a deliberate external-read denial. Replay checks saved evidence, separately from actual Git/current-source qualification at capture. No C++ change, build, CTest, Original/fixture execution, ABI replacement, startup, gameplay, SDK/OS interaction, or Ghidra mutation is claimed.
