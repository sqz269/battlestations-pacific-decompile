Primary compiled review: the registered Source initializer is45 bytes/17 instructions in one complete COFF function. Root read every emitted instruction and the physical indexed relocation graph. The sole child relocation resolves to the admitted actual Source55 allocator. Head publication at offset11, allocation BYTE31h at13, three fresh head reads at17/22/26 with ordered pointer stores at19/24/28, count zero at34 and opaque header return at40 preserve the selected schedule. There is no new EH payload, guard, cleanup or consumer.

Normal MSVC Win32 build 2026-10-09T18:56:24.196838+00:00 to 2026-10-09T18:56:40.997368+00:00 passed all three existing checks. The shared evidence pins107 Source/build inputs and four artifacts, replays all28 prior objects byte-identically, captures this new object (29 total), and resolves33 selected positive Core roots. The new public root is absent from the application map; no forced retention was added. Original ABI, Native allocator/CRT policy, production lifetime, startup and gameplay are unproved. The candidate section below is an immutable worker-time snapshot; its Source105 build receipts are now historical.

# Qualified Lua header initializer Source candidate

`006EEAF0..006EEB1A` allocates a link record, publishes it through the actual
header head cell, marks the allocation, and initializes three self-links
using three separate reads of the current head. This candidate calls the
admitted `allocate_native_lua_variant_link_record_006edea0` provider and
preserves that read/store order. It introduces no complete header or node type.

The packet owns only the header and implementation named
`native_lua_variant_header_initializer`, this document, and
`reports/cc12_native_lua_variant_header_initializer_source.json`. Root owns
registration, the normal Win32 build, emitted-code review and Core admission.

## Full body gate and closed dependency

Root read the complete 43-byte, 15-operation Native body, compared saved and
raw instruction starts, and replayed live bytes against the installed PE.
Its body SHA-256 is
`7ce75a0835bee066890295059ce0366d7f72b8c36b47e319df4cd56c5d3a5bff`.
The report pins the primary worktree's
`local/cc12_lua_container_initializer_primary/capture.json` and
`Root_Astra_gate_approval.json`. This worker replayed the PE bytes and all
15 instruction starts independently without reopening a Native child body.

The original gate recorded Source55 as incomplete. That statement remains
historically accurate in the immutable gate artifact. The dependency is now
closed by the Source55 admission in `42212ad35`, published through main
`07bb58abdc19981d07acd4c849bfed2a99efbce4`, and recorded in
`reports/cc12_native_lua_variant_link_record_allocation_primary_review.json`.
The actual provider source, accepted receipt, current build artifacts and its
selected Core member/public-body byte ranges were replayed before this work.
No missing dependency is replaced with a callback or service abstraction.

## Source interface and order

```cpp
void* initialize_native_lua_variant_header_006eeaf0(
    void* actual_header,
    void* volatile& actual_head_04,
    volatile std::uint32_t& actual_count_08);
```

The opaque pointer supplies the identity returned to the caller. The two
references must bind to the actual pointer cell at that header's +4 and the
actual DWORD count at +8. Detached copies do not satisfy the contract. An
access ending at +0Bh is not proof of a complete header size or layout.

The Native body saves ESI, retains incoming ECX as the receiver, and calls
`006EDEA0`. The new Source entry is plain C++ with explicit cell references;
its interface is not a replacement for the original register/stack ABI.

| Order | Operation | Native site |
| --- | --- | --- |
| 1 | Call the actual admitted allocation helper once | `006EEAF3` |
| 2 | Publish its pointer through the actual head cell | `006EEAF8` |
| 3 | Set BYTE+31h of the allocation result to 1 | `006EEAFB` |
| 4 | Reload head; store that pointer at its own DWORD+4 | `006EEAFF..006EEB02` |
| 5 | Reload head again; store that pointer at its own DWORD+0 | `006EEB05..006EEB08` |
| 6 | Reload head again; store that pointer at its own DWORD+8 | `006EEB0A..006EEB0D` |
| 7 | Zero the actual count | `006EEB10` |
| 8 | Return the original opaque header identity | `006EEB17..006EEB1A` |

Each of the three self-link stores uses one freshly read pointer for both
its target and value. They are volatile pointer-sized DWORD stores on Win32;
the head publication, byte store and count store are also volatile. No read
is shared across the three pairs. In particular, the byte mark uses the
allocation result before the first head reload.

The body directly accesses only the two supplied header cells. It does not
directly read or write header+0. Beyond the allocation provider's established
initialization, its node writes are only BYTE+31h and the three specified
pointer slots. Keeping other locations untouched requires that those exact
targets not alias those other locations. There is no deep copy, full-object
clearing, owner, destructor, cleanup, recovery path or consumer.

Caller-supplied cells and every reloaded head need live, aligned,
type-compatible backing for the specified accesses. Compatible changes or
aliases retain the separately sequenced volatile reads; arbitrary raw aliases,
concurrent synchronization and fault timing are not established. No runtime
guard, lock or promise of cross-thread synchronization is introduced.

## Allocator and exception boundary

The actual Source55 provider requests 52/52 bytes through
`singleton_lifetime_allocate` and returns usable, aligned, nonnull raw storage
or throws under the current host CRT policy. It clears DWORDs+0/+4/+8 and
writes BYTE+30h=1/BYTE+31h=0, leaving the other 38 bytes untouched. This caller
inherits that qualified allocation domain and then performs its own schedule.

An allocation failure occurs before this caller's owned header writes.
There is no `noexcept`, catch, translation, rollback or cleanup. Possible
effects inside the allocator or its host new handler are not suppressed.
Native null/wrapped-interior paths, `00BF681B` binding, handler/CRT and unwind
identity, registers, flags, stack layout, faults, raw aliases, original caller
ABI, production lifetime, startup and gameplay remain unproved.

## Current baseline and checks

The three current Source105 receipts are the primary reviews for the Lua
link-record allocator, observer-member initializer and squadron launch-task
scalar deletion. Their shared build ran from
`2026-10-09T18:27:44.966886+00:00` to
`2026-10-09T18:28:02.224133+00:00`, passed three existing checks, and captured
or replayed 28 whole objects with 32 unique positive Core definitions.

All 105 input hashes match after the receipts' canonical-LF normalization;
103 also match raw bytes. The same two historical report line-ending
differences are named in the candidate report. The four current artifacts
match exactly. Source55's 1,430-byte selected Core member and 91-byte public
body also match their accepted receipt hashes at the recorded offsets.
This is byte replay of the accepted Core evidence, not new symbol resolution
or a new ABI determination. Source97 and Source88 remain historical baselines.

The report pins actual Source55 files, its source and compiled receipts, the
underlying real allocator files and accepted receipt, the current shared
receipts, gate artifacts, and this candidate's three non-report files. It
does not duplicate the 105-input manifest or prior object graphs.

No CMake, ledger, Ghidra, test, probe or consumer was changed. This new source
has not been built or executed. The prior build proves the current provider
baseline; Root's next build and emitted-code review must assess this candidate.
