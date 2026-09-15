# Native animation name registry and lifetime

Addresses: 00b79a80, 00b77710, 00b79740, 00b78310, 00b795f0, 00b925d0, 00b8a330, 00b75d20, 00b79b00, 00b79ba0, 00b790d0, 00b76bb0, 00b76910, 00b79590, 00b78f80, 00b79010, 008153e0, 00818030, 00b76970, 00b769c0, 00b798c0, 004cec60

This batch implements the name registry used by the compact-track and track phases of the resource postprocessor analyzed in [AT](NATIVE_RESOURCE_INSTANCE_POSTPROCESS_AT.md). It includes populated construction, name registration, destruction, the scalar deleting wrapper, both name producers and the float-array helpers. It reuses the existing native tree algorithms and actual string-pool services. It does not implement the full `00b79bc0` postprocessor or admit a populated resource graph into the running rebuild.

The source is in `include/bsp/native_animation_registry.hpp` and `src/native_animation_registry.cpp`; machine evidence and limits are in `reports/native_animation_registry_au.json`. Descriptive names are hypotheses, not recovered symbols. New C++ context parameters change the original ABI. Native FH3/SEH, binary replacement compatibility and gameplay remain unproved.

## Actual storage and services

| Object | Native offsets |
| --- | --- |
| Registry, 20h bytes | profile 0, reference count 4, tree opaque word 8, head C, count/next captured index 10, sample data 14, count 18, capacity 1C |
| Name tree node, 1Ch bytes | left 0, parent 4, right 8, name length C, name data 10, integer index 14, color 18, nil byte 19 |
| Float array, 0Ch bytes | backing 0, signed count 4, signed capacity 8 |
| Compact-track producer | current item+C points to 18h records, name at record+4, current count at item+10 |
| Track producer | current item+8 points to track pointers, current count at item+C, each track name starts at track+8 |

The context borrows `NativeStringStorage` and the existing invalid-parameter callback. Production requires `ActualNativeStringPoolStorage` over the shared native manager and publication cells. There is no shadow registry, replacement container or copied string manager. Allocation/free, case-insensitive comparison, checked iterator handling, reference-base teardown and native string ownership reuse the existing implementations. Registry opaque words and node padding remain untouched.

## Recovered order and scope

`B79A80` constructs the reference base, stamps D62EF4, allocates the 1Ch sentinel through `B77710`, publishes the head and writes its self-links, then clears count and sample words. The sentinel allocator preserves the original conditional placement writes and payload/padding. Original constructor ABI is ECX self, EAX self, RET; the private allocator consumes no register or stack input and returns EAX/RET.

`B79740` is ECX registry, one stack name-header pointer, EAX index, RET4. It finds an existing case-insensitive key and returns that node's stored index. For a missing key it captures the tree count **before** allocation and string copies, makes two temporary owning name pairs, inserts a node, releases the second then first temporary, and returns the captured index. It preserves the source header rereads, first copied length/data capture, and second data capture versus current length at cleanup. Source exceptions dispose completed temporaries; no native FH3 equivalence is inferred.

`B925D0` and `B8A330` are ECX item, one stack registry pointer, RET4 producers. They reread the current backing pointer and signed count on every iteration. `B75D20` is the complete ECX track/EAX track+8/RET getter. Fixture allocation callbacks replace item backing/count during registration to verify these rereads.

`B76910` and `B78310` have complete normalized-byte equivalence to canonical lower-bound `BE54D0` and find `BE5A50`. Both source paths reuse those canonical implementations. The unique insertion (`B795F0`), node link/rebalance (`B790D0`), iterator decrement (`B76BB0`) and left/right rotation (`B76970`/`B769C0`) reuse existing layout-parametric tree algorithms; right rotation mirrors left/right access over the same algorithm. This batch does not introduce another STL implementation. `B790D0`'s old generic `STL_xlen_throw` label covered its 492-byte linking/rebalancing body as well as the length branch; evidence corrects that classification. The limit is 15555554h and the source exception uses the existing owning legacy string/exception transport.

`B78F80` initializes actual node links, copies the name and then rereads the pair's index after string services. `B79010` allocates 1Ch and frees that allocation if source construction throws. `B79590` erases right first, captures string data then the left link, returns the current length+1 string, frees the node, then follows captured left. The generic extraction in `include/bsp/detail/native_tree_subtree_storage.hpp` also replaces the existing `4CEC60` body without changing its distinct color/nil offsets or ordering. A separate original-code VFS comparison covers that shared-source change.

The private `clear_name_tree` specializes **only the full-range branch of B798C0 reached by B79B00**: the destructor supplies its own current begin/end, with no intervening service call. Partial-range erase remains unreconstructed. No full-function ledger entry is added for B798C0.

`8153E0` and `818030` are ECX array header, signed stack capacity/count, RET4. Reserve clamps requested capacity to at least one, allocates with original DWORD size arithmetic, and copies each current float with x87 FLD/FSTP. This preserves signaling-NaN quieting and status effects rather than treating the values as raw words. It rereads current count/backing, frees current old storage, then publishes data/capacity. Resize reserves if required, preserves the original four-lane zero fill and signed wrapped comparisons, decrements the live count when shrinking and finally publishes the requested count. Resize does not free storage. BF55BE is a direct jump to the canonical BF681B allocator.

`B79B00` stamps D62EF4, resizes the sample array to zero, frees its current data, clears the full name range, frees its current head, clears head/count and invokes the reference-base teardown that stamps CEB130. Its dangling sample data/capacity are retained as native side effects. Original ECX self/RET, no established return value. `B79BA0` is ECX self/stack flags/EAX original self/RET4; bit 0 frees the registry after payload destruction.

## Ghidra evidence limitation

The verified project is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. Twenty-six reference spans total 3219 bytes and match both live Ghidra bytes and the unchanged installed PE. Source and original reference execution cover the complete physical `B79B00..B79B9B` body (156 bytes). Ghidra still stores only `B79B00..B79B41` (66 bytes).

The supported flow-repair tool decoded the 90-byte tail and saved the program, but did not extend the stored function body. Therefore `tools/verify_report_calls.py` reports **68 rows checked, 3 failed**: B79B5D to B798C0, B79B66 to BF65AC and B79B86 to BD30F0 are outside any stored Ghidra function. The other 65 rows pass. The report retains these failing rows and does not bypass or weaken the verifier. The physical listing and PE/live bytes establish their instruction targets separately from Ghidra function ownership.

Three other helper repairs (`B79BA0`, `B79590`, `8153E0`) restored 23 bytes after free calls and have no remaining call gaps. Receipts are in the two AU flow reports. No global no-return annotation, function deletion/recreation, re-import or disabled-script workaround was used. Extending B79B00's stored body remains a metadata follow-up.

## Validation

The strict MSVC Win32 build and both existing CTests pass. The focused ignored probe links compiled production objects with `/MD /EHsc /O2 /W4 /WX /fp:strict` and an embedded manifest. Reference bodies relocate recorded direct-call displacements to original reference helpers or established actual providers. Unexercised partial-erase/length-exception external targets abort if reached; they are not successful stubs.

Five repeat-stable paired records cover four registry lifetimes and one VFS subtree lifetime: ordered and duplicate/case-insensitive keys, empty and 193-byte names, rebalancing, signaling NaN and negative zero during sample growth, shrink/destruction, both producer backing/count mutations, and a heap registry deleted with flags=1. Comparisons include actual registry words, live allocation contents, tree topology/padding, allocation/free preimages/order, string ownership events, pool bump/counters/head-tail indices and initialized arena bytes, plus x87/SSE status. They do not serialize the entire string-pool image.

Two source-only failure cases cover sentinel allocation and name-node allocation after temporary copies. Each run constructs a real 8AD4A0h pool with canonical initialization and teardown. Publication begins with that pool present; this adds no proof of lazy recreation or manager registration. Native exception dispatch, the full animator/skin/camera postprocessor, graph admission and gameplay are still open. No workers were dispatched.
