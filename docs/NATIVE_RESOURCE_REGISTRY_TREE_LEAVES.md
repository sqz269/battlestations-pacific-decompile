# Actual resource registry tree leaves

This packet reconstructs six complete native entries, 396 bytes, against
actual caller-supplied tree, node and iterator storage. It uses the completed
actual string-pool and returning CRT services. Primary reviewed the full
source/header before verification; the approved source is unchanged.

The strict Win32 build, both existing CTests and all eight seeds pass. The
focused original/current-library sequence passes across 391 trace words.

| Entry | Complete range, exclusive end | Bytes | Original ABI |
|---|---|---:|---|
| B1A260 | B1A260–B1A2B2 | 82 | ECX tree; stack node; RET4; no semantic result |
| B19890 | B19890–B198F3 | 99 | ECX iterator; RET or CRT tail; no semantic result |
| B19640 | B19640–B19692 | 82 | ECX tree; stack node; RET4; EAX replacement |
| B19830 | B19830–B1987E | 78 | ECX tree; stack node; RET4; EAX replacement |
| B196D0 | B196D0–B196EC | 28 | ECX node; EAX maximum node; RET |
| B196F0 | B196F0–B1970B | 27 | ECX node; EAX minimum node; RET |

The new functions are in `src/native_resource_registry_tree_leaves.cpp` and
the new interface is in `include/bsp/native_resource_registry_tree_leaves.hpp`.
Rotations forward the native replacement result even though known callers
ignore it. The API is a source interface, not a binary ABI replacement.

## Storage and exact ordering

The tree's +4 word points to its actual head. Head +0/+4/+8 are minimum,
root and maximum. Native 1Ch nodes contain left/parent/right +0/+4/+8,
pooled key length/data +C/+10, factory value +14, and color/nil bytes +18/+19.
The source reads links and lengths as volatile DWORDs and nil as one byte.
The iterator is exactly two pointer words, owner followed by node. None of
these APIs constructs or populates the tree or introduces factory ownership.

Subtree disposal first checks node nil. Each non-nil iteration recursively
disposes the **current right child**, captures current key data, then
captures the **current left child before pool release**. For nonzero data it
captures current length plus one with DWORD wrap, then invokes the existing
actual pool's getter/return composition. Zero data skips length and pool
reads. The captured node is freed through the existing free boundary. Only
after that returning free does the source test the captured left child's
current nil byte, assign that child as current node and continue. No second
loop-header nil read is introduced. No key-header clearing, factory-value
read/disposal or head/count reset occurs.

`ActualNativeStringPoolStorage::release` calls full `419CC0` for every
release before full `BD1510`: even a large block or disabled small return
does not bypass the getter. This uses the application's actual `01090AA8`
publication, `01090AA4` gate and canonical lifetime domain, with the raw
8AD4A0h pool and embedded ring. There is no second allocator, cached pool or
old `SizedStoragePool` projection. The existing release API is `noexcept`;
failure while lazily recreating the pool terminates at that inherited source
boundary. Native recreation-failure EH/SEH is not claimed or replaced with
an invented cleanup path.

Iterator increment invokes the returning invalid-parameter service when
owner is null, then reads the **current** node. A nil node invokes the same
service and returns without advancing even if the handler repairs the node,
matching the native tail call. On a normal right subtree it selects its
minimum. On ascent it rereads iterator+4, publishes every intermediate parent
before following that parent's parent, then stores the final selected node.
It does not add an owner/destination equality check.

Each rotation captures its replacement, writes the moved child link,
**rereads the replacement child** for the nil/parent update, copies the
current original parent and reads the current head/root. A non-root update
reads the current parent again before selecting its side. Final replacement
and original-parent links follow native order. Min/max read the current
child and its nil byte, retaining the candidate before a nil child. No
color, payload, count or key access is added to these four helpers. The
hardware-layout tree has different color/nil offsets and is not used.

## Original evidence and analysis repair

Eleven fresh guarded spans, 917 bytes, match the original PE SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Every live batch verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The six full bodies are accompanied by complete
direct pool/CRT provider spans; these do not become new source claims here.

The saved B1A260 function already spans all 82 bytes, but its `B1A29C`
free call suppresses the saved `B1A2A1..B1A2AB` returning backedge. The exact
missing bytes are `83c404807e19008bfe74c5`. Full native execution in this
packet includes that backedge. Primary will restore only the call-site flow
and missing decode; the correct global `_free` name and library metadata
must remain. The other five saved extents are already complete. No worker
Ghidra, naming, shared ledger or shared CMake edit is made.

## Focused verification and limits

An ignored CMake hook adds only this new source to the actual Win32 library.
The strict build uses `/W4 /WX /fp:strict`; the two existing CTests and eight
original seed comparisons are retained. No permanent test or broad suite is
added. The audit pins the actual built archive and every selected complete
member object, COFF code section and relocation, plus its linked subset.

Four exact archive objects contain 114 nonempty COFF code sections, 8,008
bytes and 260 relocations. The linked subset has 52 sections and 4,971 bytes;
all six owned exports are linked separately and executed. Full static object
coverage does not mean every retained provider or exception-support branch
was executed.

One ignored fixture composes the six original bodies with the complete
current pool/getter/free/CRT providers, and compares them with this source
from the actual library. The original code runs in a separate allocation;
complete native preimages are checked against the installed PE. Only
existing provider entry bridges are patched. Owned instructions, recursion,
relative branches, nil-tail jump and returns remain original. Pool and CRT
original helper bodies are not re-executed; their current complete source
providers are explicit boundaries on both sides.

The focused sequence checks two subtree disposals, both extrema, paired root
and non-root rotations, and iterator continuation/tail/ascent. The first
subtree returns a real large key while the actual pool publication is empty.
Actual lazy pool allocation changes the original node's key header and left
link through a fixture allocation observer. Correct disposal still uses the
captured key/size and left child, proving the capture precedes the getter
and that large return did not bypass it. The second subtree returns two
actual small-pool keys; an observer changes the captured left child's nil
byte after the real current-node free, testing the returning backedge read.

The observers delegate to real CRT allocation/free. Their mutations are
fixture instrumentation, not production allocator callbacks or a claim that
the native CRT invokes these observers. Caller headers and factory values
are checked untouched; actual ring return state is observed. Fixture-created
raw nodes are caller inputs, not a reconstructed tree-population route. The
actual pool uses the existing concrete lifetime binding and is shut down
separately after observations.

Both returning iterator mutations invoke the actual CRT invalid-parameter
handler: one repairs null owner and changes the current node before its
read; the other changes a nil node but must return immediately from the
native tail. Ascent and all root/left-parent/right-parent rotation paths
retain their native reads and intermediate stores. Static linked-code
inspection complements final link and payload observations; the fixture
does not claim that every intermediate write is independently observed.

The immutable bundle preserves the unmodified final fixture and original
trace for primary relinking against a new main library. It separates full
static object evidence from executed source/provider paths. Native exception
runtime, invalid-memory faults, pool recreation failure, general erase
`B19F90`, range `B1A2F0`, registry destruction, factory lifetime and game
behavior remain outside this packet.


## Primary integration

Primary registered the full source entries, passed the strict Win32 build, both existing CTests and eight fresh original seeds, and used the same frozen main library `aa4a9f060ade446e7638fed51f8977610df500a5c070984af473e5d8cc5eb3ae` for all three packets. All 81 sealed worker files, 57 report pins, 16 unchanged current source/header inputs and 11 fresh spans covering 917 bytes passed. The unchanged fixture linked only the main archive and matched all 391 trace words across the six original/source entries. Four exact archive objects retain 114 COFF code sections, 8,008 bytes and 260 relocations; 52 linked sections covering 4,971 bytes pass nonrelocation-byte checks. Original pool/CRT helpers remain explicit complete-source bridges. The B1A260 returning-free backedge is now decoded and saved while preserving previous comments, neighbors and global `_free` metadata. The actual pool release noexcept and native recreation-failure limits remain. Reviewed names and evidence comments are saved with prior values retained; correct CRT library names remain. All affected exports were forcibly refreshed. Immutable primary evidence: `local/tree_leaves_primary/`. New source interfaces are not original caller ABI or gameplay validation.
