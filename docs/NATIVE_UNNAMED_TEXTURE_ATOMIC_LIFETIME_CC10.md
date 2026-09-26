# Unnamed logical-texture count lifetime: B34020

The unnamed base constructor now placement-constructs one
`std::atomic<std::int32_t>(1)` at the original receiver+4 count initialization
point. It preserves the native store sequence, count bits, original receiver
return and existing caller interface. This is a bounded producer correction,
not a complete owner lifetime, concurrent pool reuse or runtime admission proof.
The named B34120 constructor is a separate packet.

## Original contract and preserved fields

The complete original `[00B34020,00B34067)` window is71B/18 instructions:
ECX receiver, stack borrowed COM/flags, EAX same receiver, RET8. Its count MOV
at B34035 follows BOTH CEB130 and D5F1F4 profile stores. There are no calls,
branches or native EH frame in this leaf. Source/native order remains:

1. Profile CEB130, profile D5F1F4, actual+4 count1.
2. Zero+8, +0C and +14; borrowed COM+10; flags+1C.
3. Fresh shared serial read to owner+20; fresh second serial read, DWORD
   wrapping increment and serial write; final D5F280 profile; same owner return.

Base+18 stays untouched. Derived bytes and the raw slab+30 index are not
initialized by this leaf. No release/AddRef, parallel count, extra owner credit,
metadata, default, reset guard, compiler barrier, noinline or volatile workaround
was added. Profile and other raw DWORD/pointer field assumptions remain outside
this correction. Genuine shared serial0108D6E8 remains a separately live DWORD;
it cannot overlap the new atomic as an incompatible uint32_t subobject.

The source asserts Win32 pointer/atomic size4, atomic alignment4 and always
lock-free operation. Placement uses the existing count point and original raw
receiver; it establishes only the atomic object's lifetime. The installed
MSVC14.51 C++17 classification is target-specific: `_HAS_CXX20=0`, outer atomic
default constructor nontrivial, copy/move constructors nontrivial, nonaggregate,
trivial destructor, size/alignment4 and always-lock-free. The inner four-byte
storage's trivial default constructor does not make the outer atomic an
implicit-lifetime type. The old source had no explicit placement and its raw
pool/Entry borrow provided no separate outer atomic construction. Deleted
copying or a value constructor alone would not establish this conclusion for
other standard-library implementations. Frozen readiness wording is qualified
by the independently compiled installed-toolchain proof nested in this packet.

## Backing and retirement obligations

B3D650 is the sole recovered direct native caller, at B3D67A. It calls this
base once, stamps the real cube profile and retains its existing COM descriptor
and HRESULT behavior. B2A380 obtains the raw slot through B3F2C0/B3F170 before
calling B3D650 and publishing it. No caller is activated here.

The genuine cube slab uses malloc backing, 34h stride and +30 slot indices;
malloc alignment plus the stride preserves the four-byte count alignment.
Other caller backing must independently provide the required alignment. Size
or equal pointer addresses alone do not prove a C++ object lifetime.

Admission requires fresh or fully retired, exclusively available backing.
Every old companion/reference right, terminal callback, canonical binding,
Entry and caller postcallback access must have completed before reuse. Pool
return alone is insufficient: B3F410 reaches B3D940 before the canonical zero
provider returns to unbind/erase its SAME Entry. Registry metadata locking does
not serialize that complete ownership/reuse interval. The caller must retain
and synchronize the pool, renderer, serial, strings and other borrowed contexts.
Partial constructor failures also retain their existing COM/creator/raw-slot
obligations; no rollback or retry was added. This patch does not close concurrent
reuse, enclosing owner/raw-field lifetimes, original FH3/SEH or hardware faults.

## Validation and compiled limits

At baseline5413707f9e0ea56923453a5e0fcdd15891ec17d6, one strict MSVC Win32
Release build (`/WX`, `/fp:strict`) and all three existing CTests passed. There
were no new tests or runtime cases. The applied two-file source exactly matches
the frozen reviewed proposal. CMake, named constructor and previous evidence
remain unchanged. A preservation-helper comparison failure before build is
retained: it compared Git-format headers with a plain unified proposal; fixing
only that helper left the already-applied production bytes unchanged.
The final-pin helper initially compared the intentionally rebuilt library with
its prebuild identity; its failed receipt and helper are preserved. Correcting
that comparison to the exact preserved old library changed no production or
build artifact. All154 final source/tool/preservation/provider/archive pins pass.

Exact old/new full core libraries and their1783 member indices are preserved.
The complete base constructor code section changes216B to211B: one MOV
`[esi+4],1` occurs at section offset40, after the profile stores at18/34 and
before all later fields. It has no calls, branches, relocations or EH sections.
The three existing base helper sections remain byte/relocation-identical; five
additional atomic/placement COMDAT helper sections total70B and are unselected.
The complete base member changes4 code sections/261B to9/326B. This is semantic
store-schedule correspondence, not native byte or stack-ABI equivalence.

All28 physical cube-caller sections and relocations are identical, including
18 code sections/1753B and its existing exception paths. Its whole object hash
differs; section/relocation correspondence is the claim. Physical-section
collection records `PointerToRawData=0` storage as zero physical bytes, never
initialized .bss bytes. Full section bytes/relocations are in
`compiled_sections.json`; decoded complete sections and schedule are in
`compiled_base.json`, `compiled_caller.json` and `compiled_schedule.json`.

The actual application map selects the base member's B34090 unwind only.
B34020 and B3D650 are not selected. No linked constructor body, original ABI,
factory execution, nonempty source, platform/message, source0/W or game behavior
is credited. Nine original PE windows totaling1231B preserve the constructor,
caller, raw allocation/return and terminal context. These are existing native
providers, not new byte credit; ledger/Ghidra integration remains root-owned.

Evidence: `local/cc10_unnamed_texture_atomic_lifetime_implementation/` contains
pre-sync/source/tool/build seals, the unchanged prior readiness archive and
separate preserved clear-archive pins, actual raw build receipts, full old/new libraries/objects, compiler
and linker records, original PE proof, installed atomic classification and
final Git blobs. The companion report records the bounded validation and limits.
