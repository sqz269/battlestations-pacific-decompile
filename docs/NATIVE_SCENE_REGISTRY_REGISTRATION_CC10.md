# Actual scene-registry registration, CC10

Addresses: `00B83700`, `00B83E50`, `00B83D90`, `00B83BE0`.

These routines consume an actual **28h registry**. The existing actual 3Ch
scene-resource constructor embeds this registry at **resource+14h**; passing
the resource base, a host `SceneResource`, or a `SceneNodeRegistry` is invalid.
The list at registry+4 has opaque word0, sentinel+4, count+8. Its actual 0Ch
nodes hold next0, previous4, borrowed identity8. The boundary vector at +10h
has opaque0/begin4/end8/capacityC and actual 8-byte {registry+4,node} iterators.
Mask is +20h and active buckets +24h. No owner counts, pointed-object retains,
canonical metadata or B6/B7 attachment/virtual50 behavior is added.

| Native body | Exact bytes | Original ABI | Coverage |
|---|---|---|---|
| B83700 insert | [B83700,B83BDF),1247B | ECX registry; output/key-source stack words; EAX output; RET8 at B83BDC | complete raw normal body and own returning guards |
| B83E50 erase key | [B83E50,B83EBA),106B | ECX registry; key-source stack word; EAX current counter; RET4 at B83EB7 | complete raw normal body in nested leaf domain |
| B83D90 erase range | [B83D90,B83E42),178B | ECX registry; output/first pair/last pair; EAX output; RET14h at B83E3F | complete raw normal body and own returning guards |
| B83BE0 clear/reset | [B83BE0,B83C42),98B | ECX registry; RET at B83C41 | complete raw body, existing returning-free repair verified |

**Nested domain restriction:** existing B82650 equal-range, B827C0 entry erase
and B820B0 range count require valid accessible iterators/ranges and valid
boundary storage. Their native invalid-parameter continuation paths remain
excluded; they throw source `invalid_argument` on detected violations. This
packet does not turn that into universal invalid/corrupt-state compatibility.
The new routines' own reached BF6713 guards invoke the required genuine current
returning service. A continuing callback must leave every later reached word
and finite range valid. No asynchronous mutation is admitted.

## Direct closure and library boundaries

| Provider | Contract used |
|---|---|
| B823B0 | actual 0Ch allocation, current allocator; next/previous/key-source stack arguments read at their native stages after allocation |
| B83490 | genuine current raw resize frame at vector+10h; preserves capacity and mutable nested vector cells |
| B83560 | genuine raw assign frame, used by clear for nine sentinel pairs |
| B82D30 | captured current list count plus increment1; real owning source length error on overflow, borrowed CE38F8 message |
| B82650/B827C0/B820B0 | genuine query/entry erase/count in the explicit valid-iterator domain above |
| BF681B/BF65AC | existing borrowed current allocation/free domain, no heap substitute |
| BF6713 | required current genuine returning invalid-parameter service, no no-op/default/forced noreturn |
| C03DBE | signed CDQ/IDIV: EAX quotient and EDX remainder; divisor1F31D here |

C03DBE is a 26-byte external library arithmetic boundary, independently read
as assembly. With the positive constant divisor its negative-dividend/positive-
remainder adjustment is unreachable. Source signed quotient/remainder and
wrapping DWORD multiply/subtract reproduce the two callers' hash:
`key XOR DEADBEEF`, remainder*41A7 minus quotient*B14, add7FFFFFFF if sign set.
The current mask/active reads remain in the caller schedule. No general CRT
or STL implementation is introduced.

These four bodies have no native local EH registration/cleanup. In particular,
B83B21 allocates a node, then B83B2E checks/increments list size. If that second
call throws, the node allocation survives unlinked, and earlier bucket writes
survive. `NativeSceneRegistryRegistrationAcquired` is fresh, disjoint caller
diagnostics, recording the raw allocation immediately after return and before
growth. It adds no ownership credit and no rollback/destructor. On failure the
caller retains it for explicit disposition; on success actual list storage owns
the allocation. The nested size/vector providers' source C++ exception classes
and transport are not native FH3/SEH or original CRT RTTI identity.

## Insertion order and caller scratch

The caller supplies initialized stable `RegistrationInsertFrame` storage.
Its first 30h bytes correspond to the native 30h local allocation (native ESP
after allocation and four saved-register pushes is the following table's base).
Native saved registers/return gaps are deliberately not represented.

| Source local index | Native ESP offset | Native writes and later purpose |
|---|---|---|
| 0 | +10 | B83714 actual registry, later current reloads including B83A01/B83B58 |
| 1 | +14 | B8379A split index; B83A77 actual list owner, reread for node allocation/count growth |
| 2 | +18 | current list-owner scratch; B837C2/B8394D/B839F8/B83A86 writes and conditional post-handler reloads |
| 4 | +20 | B83866 captured owner for split repair; current comparisons/reloads later |
| 5 | +24 | B8386A current node, B8387E next; B83B1D successor, B83B4D inserted node; current final output |
| 7 | +2C | B8386E captured sentinel, later comparison |
| 9 | +34 | B83941 captured current head for upper-boundary pair |
| 11 | +3C | B83983 current head for backward boundary comparisons |
| 3/6/8/10 | +1C/+28/+30/+38 | untouched opaque preimages |

The next two source words are current output/key arguments, corresponding to
native ESP+44/+48. Nested resize and checked-size scratch are explicit source
frames with their own initialized preimages. Native transient B823B0 argument
slots use the existing genuine raw provider, whose three values are passed at
the native load points. Arbitrary aliases into transient helper frames,
saved-register gaps, return addresses and native EH private frames are outside
this source interface. Metadata/context storage is disjoint; nested adapter
frames retain their documented separation from vector/backing storage.

The split test captures count before active buckets and checks active <= count/4
**before duplicate lookup**. Growth may first write mask and resize the boundary
vector; resize receives inline pair stores zero0, captured head4, owner0, then
current mask+2 as count. A split redistributes the actual linked nodes and repairs
boundaries with exact component-store and current-pointer read order. Split
scratch is not collapsed to persistent C++ snapshots across callbacks.

The physical B838FE increment0 call is present in the 45-row call evidence but
is unreachable in normal entry flow: B838F3 `CMP EBX,EBX`, B838F5 `MOV EDI,[EBX+4]`
(flags unchanged), B838F8 `JZ B83903`. Source records this fact instead of adding
a list-count store during split. The alignment-only bytes B8389D..9F decode as
LEA ECX,[ECX], skipped by B8389B JMP B838A0; they are not a free continuation.

Lookup hashes the CURRENT key pointer/value after any split, then reloads key
pointer/value on each ordered comparison. Duplicate writes current output
owner, node, and ONE inserted byte0. Successful allocation captures successor,
previous and key-source pointer in native order, then reads the current raw key
through B823B0. Size growth follows allocation. The has-owner comparison is
captured BEFORE link stores; inserted node is then reread from current successor
previous. Boundary updates use current storage and current inserted-node scratch.
Final output pointer is current, stores owner/node/ONE byte1; output+9 padding
is untouched. No semantic owner wrapper or key copy is substituted.

## Erase and clear order

Clear resets captured head.next, reloads current head for head.previous, compares
captured first against current head, then stores count0. Each iteration captures
next BEFORE current free and compares next against CURRENT head AFTER free.
It preserves the sentinel. After frees it captures current head into caller pair
word1, initializes assign pair-pointer/count9, then stores list owner into pair0.
Assign retains actual vector capacity; mask1 then active1 are written only after
assign succeeds. A free callback's later count mutation is not silently erased.

Range erase captures initial first-node and first-owner before its own validation,
then reads current first-node argument. Whole-list detection captures sentinel
before validating last owner and reads current last node afterward. The clear
receiver is the current saved-registry cell; output after clear still uses the
captured list identity and its current head/next. Other ranges keep first owner
captured, validate CURRENT last-owner/node each loop, capture next BEFORE entry
erase, and point the nested erase output at the SAME first-owner/node argument
cells. Nested entry free/output mutations do not replace the captured loop next.
Final output pointer is current; owner is stored before node.

Erase-key uses caller-owned 16-byte iterator-pair output and reuses its original
key argument as counter. After query it captures range words3,2,1,0, THEN zeros
the counter. Count uses its actual address and current zero value as unused tag.
After count it rereads range word0, while the other three remain captured; range
erase output points back to the same range array. Final counter is read AFTER
all erase/clear/free callbacks. No forced 0/1 result or logical size projection.

## Evidence and validation

All 1629 owned bytes plus the 26-byte external arithmetic body match live Ghidra
and the original PE. All 45 direct call rows pass the report verifier, including
the physically present unreachable call. B83BE0 has 37 instructions and no gaps;
its B83C07 free continuation is already correctly listed. Worker performed no
Ghidra mutation, and this packet requires no new handler definitions.

Strict MSVC Win32 Release build and all three existing CTests pass. One ignored
composition probe copies nine complete original bodies (the four here, three
query/erase/count leaves, node allocator, arithmetic), relocates external calls
to genuine raw adapters and real allocator/free/CRT services, and compares with
the source on actual registry storage. It uses 36 stable borrowed identity
addresses to reach both split movement and boundary growth9->17, including a
duplicate that splits before lookup. It exercises allocator reentry into a
duplicate lookup, current output/key-argument mutation, a real returning CRT
guard repairing current begin, key/range erase and clear, current free-time
count/counter mutations, padding and all 30h outer scratch preimages, and retained
vector capacity. Nested adapter private stacks are not claimed as compared.

A source-only checked-size failure uses the genuine owning length exception
after completed raw node allocation, verifies the unlinked allocation and prior
count mutation survive, then explicitly disposes it. Native FH3/fault handling,
source/native exception identity, invalid nested query continuation, complete
scene attachment and gameplay are not claimed. No tracked test or app path was
added; all earlier frozen archives remain unchanged.
