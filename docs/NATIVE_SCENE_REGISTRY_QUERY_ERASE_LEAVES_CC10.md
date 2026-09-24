# Native scene registry query and single erase, CC10

This packet reconstructs three complete normal bodies over actual scene
registry storage. It does not use `SceneNodeRegistry`, `SceneResource`, host
companion keys, or node ownership operations. The existing logical path is
unchanged. Baseline main: `62e67ad27`.

| Native range, exclusive end | Bytes | Source operation | Coverage |
| --- | ---: | --- | --- |
| `B82650..B827B9` | 361 | `equal_range_native_scene_registry_00b82650` | Complete normal valid domain |
| `B827C0..B828F4` | 308 | `erase_native_scene_registry_entry_00b827c0` | Complete normal valid domain |
| `B820B0..B820F6` | 70 | `count_native_scene_registry_range_00b820b0` | Complete normal valid domain |

The first two use native ECX registry with stacked arguments, returning an
output iterator address and popping 8/12 bytes. Counting is cdecl with stacked
iterator pairs, counter address and unused tag. The source APIs expose borrowed
argument cells and caller scratch; they are not replacements for the native
entry ABI. Final instructions are respectively `B827B6 RET8` (3B),
`B828F1 RET12` (3B), and `B820F5 RET` (1B).

## Actual storage and valid domain

The registry is 28h bytes: embedded list at +4, sentinel +8, DWORD size +C,
boundary vector at +10 with begin/end/capacity at +14/+18/+1C, and mask/active
at +20/+24. List entries are actual Ch `{next,previous,borrowed key}`. Boundary
iterators are actual 8B `{embedded list owner,entry}`. None of these words is
a host metadata address. Existing allocation/free ownership remains with the
caller and `NativeSceneRegistryStorageBindings`.

All reached words and output buffers must be accessible. Iterators have valid
owners; dereferenced entries are not sentinels; boundary indices are in range;
walks terminate. There is no asynchronous mutation. Reached free bindings may
change live fields and caller argument cells, which must remain valid for the
remaining instructions. The native CRT invalid-parameter path is excluded.
Detected violations throw source `invalid_argument`; this is an explicit
contract rejection, not reconstruction of the CRT callback or its continuation.

Equal-range has borrowed 8B arguments and separate 12B scratch. Its two opaque
scratch words are untouched. The final word is written only when the second
scan begins, and reread before the found/absent decision. Erase borrows three
DWORD argument cells. Count borrows six words, leaving the unused tag intact.
Their preimages are supplied by the caller. Saved-register/private-stack
aliases and native FH3/SEH transport are not represented by these interfaces.

## Ordering and arithmetic

Both key operations use the actual signed division contract of read-only
`C03DBE`: EAX quotient and EDX remainder for `(key XOR DEADBEEF) / 1F31D`.
The reached positive divisor makes its negative-dividend/positive-remainder
repair unreachable. Source division/remainder plus unsigned DWORD products
preserve `remainder*41A7 - quotient*B14`, sign test, and negative +7FFFFFFF.
Mask/active are read after arithmetic. Key comparisons themselves are unsigned.
There is no custom CRT implementation or general division ABI claim.

Equal-range rereads the key pointer at each comparison. The lower scan reads
node key before caller key; the upper scan reads caller key before node key.
Boundary reads and sentinel tests retain native order. Found output stores
offsets C,0,8,4; absence loads current output before current sentinel and stores
4,0,8,C. It returns the captured output identity even when output stores alias
other caller-visible storage.

Single erase updates every equal boundary backwards before unlinking. It
reloads the caller node argument after each boundary owner store (`B8289F`).
The caller owner is captured before the node argument rewrite. Link operations
capture next, write previous-next, then freshly read node next/previous before
writing next-previous. The genuine free entry is read at its reached call site.
After free, it decrements CURRENT registry count, reads CURRENT output pointer,
then stores captured next before captured owner. No raw entry read follows free.
The previously repaired `B828DD..B828F3` continuation is included in the 308B.
No key destruction, retain/release, rollback or cleanup handler is added. A
throwing source free binding leaves the preceding writes visible.

Count captures first node, last node, first owner and counter in native order.
It rereads last owner each turn. Each iteration increments the current counter
with DWORD wrap before checking the sentinel and reading the next link; the
counter is not initialized by this function.

## Verification and boundary

`scripts/build.ps1` passed strict MSVC Win32 and both existing CTests. All 739
owned bytes plus 26 read-only division bytes match live Ghidra and installed PE.
The report lists all 25 direct call sites, including excluded CRT checks.
Workers made no Ghidra changes. No tracked test was added.

One ignored original/source comparison uses actual 28h registry, Ch entries
and 8B boundaries constructed with existing raw providers, then fixture-seeded
with three borrowed identities. It compares found, absent-in-bucket and
absent-after-bucket results, all 12 scratch bytes, signed-negative hashing,
counter wrap, repeated backward boundaries, and a real free callback. The
callback changes current count/output/owner/node argument cells. Both executions
retain captured owner/next and match normalized registry, boundary, surviving
entry, output, argument and callback observations. Freed storage is not read.

Original bodies and the real C03DBE bytes run from relocated executable copies.
Only direct free is rebound to the same genuine allocation-domain free plus
controlled mutation; unreachable CRT debug calls trap. Shared constructor,
allocator and final registry destruction providers are source bindings. The
probe uses `/MD /O2 /W4 /WX /fp:strict /link /MANIFEST:EMBED`, requires active
assertions with a compile-time `NDEBUG` rejection, and records the complete
command in the JSON report. The initial ignored fixture needed a larger key
arena to supply bucket2 addresses; its failed assertion log is retained.

Evidence is under `local/output/cc10_registry_query_erase*` and
`local/cc10_registry_query_erase_build.log`; hashes are frozen in the report.
Full insertion, boundary-vector growth, erase-range/key registration, raw scene
attachment and `AC59A0` remain separate work. This packet makes no whole graph,
native exception/CRT fault, binary ABI, or gameplay compatibility claim.
