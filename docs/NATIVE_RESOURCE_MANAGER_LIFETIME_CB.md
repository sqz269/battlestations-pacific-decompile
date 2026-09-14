# Raw resource manager construction and lifetime

Packet `orch4_native_resource_manager_lifetime_cb` reconstructs 16 complete ordinary bodies (2254 physical bytes), with 785 executable ordinary instructions and33 support instructions individually owned in Ghidra. Three unreachable alignment spans (five bytes) are explicitly excluded from owner counts. The new source operates on actual28h manager and1Ch tree-node storage. It is not the earlier typed `GameResourceManager` projection. Names remain descriptive hypotheses.

| Address | Bytes | Behavior |
|---|---:|---|
| 00b7ca20 | 28 | `maximum_native_resource_parser_node_00b7ca20` |
| 00b7ca40 | 27 | `minimum_native_resource_parser_node_00b7ca40` |
| 00b7cf80 | 99 | `increment_native_resource_parser_iterator_00b7cf80` |
| 00b7f730 | 82 | `destroy_native_resource_parser_subtree_00b7f730` |
| 00b7ff20 | 82 | `destroy_native_resource_cache_subtree_00b7ff20` |
| 00b7f790 | 716 | `erase_native_resource_parser_iterator_00b7f790` |
| 00b80500 | 201 | `erase_native_resource_parser_range_00b80500` |
| 00b805d0 | 201 | `erase_native_resource_cache_range_00b805d0` |
| 00b80e90 | 52 | `destroy_native_resource_parser_tree_00b80e90` |
| 00b80ed0 | 52 | `destroy_native_resource_cache_tree_00b80ed0` |
| 00b7d290 | 31 | `delete_native_default_resource_factory_00b7d290` |
| 00b7d2c0 | 17 | `destroy_native_resource_manager_base_00b7d2c0` |
| 00b81040 | 260 | `construct_native_resource_manager_00b81040` |
| 00b80f10 | 187 | `destroy_native_resource_manager_00b80f10` |
| 00b81150 | 30 | `delete_native_resource_manager_00b81150` |
| 004c1400 | 189 | `get_native_resource_manager_004c1400` |

## Construction, publication and failure

B81040 writes primaryD63128, allocates a4h default factory and stampsD63060, then constructs parser and cache sentinels at manager+8 and+14h. Each head is allocated, published, marked nil, then receives root/left/right self links through current head reads; count becomes zero. Debug words at+8/+14h and working factory/resource fields+20/+24h remain untouched. Registration order is Mesh, SkinedMesh, SkinedMeshAnimation, MatrixIndexedMesh, Camera, GroupParams, using the actual CA singleton getters and BZ raw B80A50 name registration. Its return byte is ignored.

Constructor states0/1/2 correspond to base, completed parser tree, completed cache tree. FuncInfoDFB4BC/mapDFB4A4 calls CC2230/38/43, which reach B7D2C0/B80E90/B80ED0. A later constructor exception destroys completed trees and clears010901C4/stampsCE3818, but does not destroy the separately allocated default factory or unregister already published parsers. Source retains that native ownership, including the factory allocation on failure.

4C1400 captures its first resource-manager publication for the fast return. Its slow path captures the first lifetime manager's section, enters/increments, rechecks, allocates28h and constructs, publishes the result, then calls the lifetime getter again BEFORE reading the current resource publication for registration. This read order differs from the parser singleton getters. The original captured section is decremented/released, then the current publication returns. FuncInfoD8D58C/mapD8D57C has guard cleanup C64F40 and captured allocation free C64F48. Constructor failure frees that captured manager allocation after constructor cleanup; registration failure retains it and its publication. Source transports C++ exceptions, not native FH3/SEH identity or arbitrary stack aliases.

## Tree and manager destruction

Nine normalized code-family comparisons and a complete44-byte parser/cache erase EH comparison justify reusing actual cache algorithms for parser extrema, increment, single erase, subtree, range and tree destruction. Raw nodes own their keys and borrow the mapped pointer at+14h. No resource or parser AddRef/release is added. Full range erasure destroys the right subtree recursively, captures current key data and left link, returns that captured key storage through the current pool, frees the captured node, and continues left. It resets current sentinel links and count. Partial range advances the local first iterator before erasing its previously captured pair, preserving validation captures and returning-handler behavior. Parser erase uses the existing owning invalid-iterator exception with the matched CC20F0/DFB314 cleanup map.

B80E90/B80ED0 erase the captured [begin,end), free the CURRENT sentinel and zero head/count. Their previously truncated free tails were restored. B80F10 stampsD63128, invokes the current default factory's captured slot0 with flags1, clears/frees cache then parser trees, clears010901C4 and stampsCE3818. The dangling default-factory field+4 and working fields+20/+24h remain untouched. State2/1/0 disarm each member before its normal destruction; FuncInfoDFB480/mapDFB468 unwinds remaining cache/parser/base through CC2213/08/00. A second exception during cleanup terminates. B81150 runs the entire destructor before testing flags bit0 to free the manager allocation. B7D290 tests its bit0 before stampingCFD7DC and optionally freeing the4h default factory.

The existing raw singleton dispatcher now accepts manager profileD63128 and the six CA parser-secondary profiles through a borrowed `NativeResourceManagerContext`. It dispatches the captured popped profile, passes the popped owner and preserves the actual publication bindings through drain. Parser secondary entries adjust-4. This finite source binding does not replace original arbitrary virtual dispatch. Nondefault factory deletion targets require a separate recovered binding.

Five returning-free bodies were recreated without changing instruction bytes or callee no-return flags: B7FF20, B80E90, B80ED0, B81150 and C64F48. The default factory scalar and four missing dispatch handlers were defined. Prior comments/compiler names are retained. Alignment atB7CA2D (LEA ECX,[ECX]), B8057F and B8064F (NOP) follows unconditional transfers and is outside successful instruction-owner counts; reference paddingB7CB7D is not mutated.

## Validation and remaining work

Strict MSVC Win32 build and both existing CTests pass. One controlled-child fixture executes16 copied original bodies,68 direct relocations and13 absolute relocations. Shared canonical dependencies supply CA parser getters, BZ registration, allocation, pool, rotations/cache erase and lifetime/lock services. A callable factory table is installed immediately before original manager destruction, dispatching to copiedB7D290. Original exception/error paths are guarded out.

Paired checks cover fast returns without a lifetime manager, complete raw manager construction/getter, six names/seven registered owners, repeated getters, tracked lock depth returning to zero, complete normalized parser/cache tree links/colors/keys/values, partial parser erasure and populated scalar flags2 destruction. Explicitly seeded constructor storage retains all four untouched words. Keys/nodes are destroyed while borrowed cache values and unrelated manager fields remain unchanged. A separate complete source canonical drain pops the manager and then its six parser secondaries, clearing all seven publications without fixture removal or direct parser deletion. A third-name-call source exception cleans completed maps/base while retaining the default factory and two registered parsers, as the native constructor map specifies.

These checks use preconstructed raw string pools. Lazy pool recreation during drain, allocation failure and handler mutation are not newly exercised. The source interfaces remain distinct from the original ABI/FH3/SEH and hardware-fault cleanup. Concrete parse-slot8 bodies, game-specific parser registration, nondefault factory terminals, executable admission and gameplay remain open. Exact revision and artifact provenance are in `reports/native_resource_manager_lifetime_cb.json` and its integration receipt.

## Integrated revision

Tested source `f0e1c95726dd4d8b088b1b2c924c733292a27b5c` passes the strict Win32 build, both existing CTests and the lifecycle fixture. Source-stable merge `a419a94de558e88ec45e8c0b4e2e63727b28af53` adds only reviewed documentation and a listing helper; all 2669 production source/build input hashes remain identical. Exact successful-run artifacts and libraries are frozen, so no redundant rerun is claimed. The incoming stack walker is a listing-order heuristic with first-RET and stale-spill limitations, not a CFG/register proof. The incoming aircraft clamp remains unresolved, and copy-size bounds do not prove absence of class+50 writers without destination provenance. These peer analysis claims do not expand CB native ABI or gameplay coverage.
