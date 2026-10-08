# Actual nested type-6 payload profile binding audit (CC12)

Addresses: 008F59E0 008F41F0 008EF780

The instructions on the successful native clone path establish that its nested
type-6 payload has profile `00D16504`, whose actual slot 0 is `008F59E0`. All three
owned routines remain **unready for whole raw current Source registration**.
The scalar's first missing lifetime provider is `008F5410`; the clone's first
missing raw direct provider after allocation is `00480690`; the type-6
constructor lacks a genuine current raw child-bag producer. No new
Source, native execution, tests, Ghidra mutation, or shared ledger/export write
was performed. Baseline is `d02cf8082`, which already contains the distinct
raw property-array release `008F03F0`; that leaf does not close type 6.

The integrator independently read all three complete live listings, rehashed
the installed PE spans, profile slot and supporting arm, and verified all 34
sealed worker artifacts. Ten serialized direct-call rows passed with zero
failures. This is static evidence; no new native execution was performed.

## Native evidence and coverage

Each live query used `bsp.py`, whose client verifies project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base before the query.
`config/target.json` pins `C:/Users/sqz269/bsp.gpr` and the installed PE at
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
The complete saved live pseudocode and listings have no gaps. Whole owned
body bytes and the actual profile slot matched that installed PE:

| Entry | Inclusive end | Exclusive end | Bytes / instructions | Coverage |
| --- | --- | --- | --- | --- |
| `008F59E0` | `008F59FD` | `008F59FE` | 30 / 11 | complete |
| `008F41F0` | `008F4289` | `008F428A` | 154 / 49 | complete |
| `008EF780` | `008EF7B9` | `008EF7BA` | 58 / 16 | complete |
| `00D16504` | `00D16507` | `00D16508` | 4 / data | actual slot 0 only |
| `008F50ED` | `008F5133` | `008F5134` | 71 / supporting arm | partial `008F4F60` type-6 arm only |

The JSON report and ignored byte receipt carry all five SHA-256 values, PE
file offsets for owned bodies/slot, and live/disk equality. No original body
or instruction-flow repair is required for the three owned routines.

## Precise producer admission

`008F41FC -> 00BF681B` allocates the actual `0x114` root. On success,
`008F420A` writes `D16504` at root `+0`, `008F4212` writes `D162C4` at `+4`,
and the constructor copy clears count `+8`, all 64 heads `+0C..+10B`, ordinal
`+10C`, and owner `+110`. It then iterates the **actual** source map at
source `+4`, clones each source node's `+8` record, and inserts the returned
record under node `+4` (or the exact fallback address `00F89450`). It returns
the actual new root in EAX. This is not an allocated semantic bag model.

The genuine `008F4F60` type-6 arm allocates a separate `0x38` record at
`008F50EF`, retains it in EDI, and loads source record `+C` into ECX at
`008F510D`. `008F5110 -> 008F41F0` returns the newly profiled child;
`008F5115 PUSH EAX`, `008F5116 MOV ECX,EDI`, and `008F5118 -> 008EF780`
pass that exact child to that exact newly allocated record. After the
constructor, `008F5121` copies the source ordinal to the returned record
`+34`. The body containing these sites is `008F4F60..008F52B5`; only its
71-byte type-6 arm is claimed here.

The complete `008EF780` body publishes record profile `CE89D4`, type `6`
at `+4`, and the supplied child at `+C`. At `008EF7A7`, it writes the actual
record back to that child `+110`. The record owner `+30` starts at zero;
subsequent keyed insertion is a separate missing raw provider.

The actual four-byte `D16504` slot is `e0 59 8f 00`. The consumer at
`008F06AD/AF` loads the nested child's actual profile and slot 0;
`008F06B1` pushes `1` and `008F06B3` calls EAX. Therefore the successful
clone/constructor admission above selects `008F59E0` with flags `1`.
The general consumer remains dynamic. Other profiles and producer families
are not admitted by this audit, and no universal type dispatcher is invented.

Allocation and ownership failures are not repaired by inference.
`008F41F0` retains a null-allocation branch and still enters source iteration;
a nonempty source with a null new root is not made safe. `008EF780` writes
child `+110` without a null check. Its admitted native successful producer
passes a fresh actual child, not an arbitrary aliased or cyclic tree.

## Whole contracts and remaining Source providers

`008F59E0` receives ECX = actual `0x114` bag and one stack flags argument.
It saves ESI = this, calls `008F5410` unconditionally, then tests the low
flag bit. If set, it passes the original root to genuine free `00BF65AC`,
performs `ADD ESP,4`, restores ESI and returns the original address in EAX
with `RET 4`. It returns that address even when storage was released.
The complete ordinary callee `008F5410..008F5467` receives ECX = actual bag,
has no stack arguments and plain RET; its normal x86 SEH linkage is restored.
It sets the outer profile, clears the actual embedded map at bag `+4`, sets
its inner `D162C4` profile, and clears again. It frees neither root nor an
interior pointer. Its owning-record `008F3F30` provider remains incomplete in
current raw Source, as do the actual record scalar/thunk/type lifetime chain.

`008F41F0` has ECX = actual source bag, no stack arguments, EAX = new root,
plain RET, and preserves ESI/EDI. The current genuine allocation service is
available. Its first missing raw Source callee is `00480690` at `008F4247`:
ECX is the actual three-DWORD iterator, `iterator+0` points to the actual
embedded map, there are no stack arguments, and its complete 65-byte listing
ends in plain RET and preserves EDI. This small supporting read only
establishes the ABI; it does not provide Source or admit manufactured map
nodes. `0047E480`, whole `008F4F60`, and `008F33F0` remain explicit missing
raw providers. Existing `ScenePropertyMergeHost` abstract methods are not
implementations of these bodies.

`008EF780` has ECX = supplied actual `0x38` record and a single stack child
pointer; it returns that record in EAX with `RET 4`. It modifies EAX/ECX/EDX
and leaves EBX/ESI/EDI/EBP untouched. Its **whole** body writes:

| Actual field | Written value |
| --- | --- |
| record `+00` / `+04` / `+0C` | `CE89D4` / `6` / supplied actual child |
| record `+08,+18,+1C,+20,+24,+30,+34` | zero |
| record `+2C` byte | `1` |
| child `+110` | supplied actual record |

It leaves record DWORDs `+10,+14,+28` and bytes `+2D..+2F` untouched.
It allocates and releases nothing, calls nothing, and performs no size,
null, prior-owner, alias or cycle validation. A complete field initializer is
not promoted to independently Source-ready status: the actual observed caller
needs the missing raw clone `008F41F0`. An alternative genuine raw empty bag
producer would need the missing whole constructor `008F41A0` first. Existing
value containers and copy specs do not supply that raw ownership admission.

Current `singleton_lifetime_allocate` is real `malloc` plus `_callnewh` retry
and `bad_alloc`; `singleton_lifetime_free` is real `free`. These heap services
remain usable boundaries, not replacements for raw profiles, record/map
producers, or recursive destruction. No opaque cleanup callback, copied
semantic bag, null-only destructor closure, class closure, historical CRT/EH
identity, binary replacement ABI, world, or game claim is made.

## Call rows and next bounded work

The report supplies ten exact direct call-site/target/containing-function
rows: two scalar calls, five clone calls, and three supporting type-6 arm
calls. `008F06B3 CALL EAX` is recorded separately without a static `native`
target. The existing `verify_report_calls.py` checked all ten direct rows with zero failures.
No C++ changed, so no build or new native/source test was run.

There is **no ready Source follow-up** from this packet. The exact lifetime
blocker is `008F5410`, then its genuine owning-record-map `008F3F30` and
record/type/nested chain. For advancing producer admission independently,
the next bounded read-only packet is
`cc12_property_bag_raw_constructor_readiness`, entry `008F41A0`, known
native body `008F41A0..008F41DC` (61 bytes). It must establish the whole raw
`0x114` constructor, exact original ABI/bytes, and actual current heap
allocation admission for its supplied storage. It must recover the root and
embedded profiles, count, 64 heads, ordinal and supplied owner; it must not
claim clone, raw populated-map production, recursive destruction or game
closure. Source registration readiness is not preassigned.

## Raw evidence

Ignored evidence is sealed under
`J:/PROG/battlestations-pacific-decompile-cc12_property_payload/local/cc12_property_payload_binding/`:
complete owned `*_decompiled.c`, `*_assembly.txt`, `*_bytes.txt` and `.bin`,
`byte_receipt.json`, `artifact_manifest.json`, `flow_report.txt`,
`direct_call_instructions.json`, `direct_call_body_ranges.txt`,
`type6_binding_listing.txt`, `type6_release_indirect_listing.txt`, the two
first-missing-callee listings, and `current_dependency_lookup.txt`.
The tracked JSON report pins the ignored artifact manifest's SHA-256.
These are static evidence and readiness receipts; installed byte equality
provides no runtime or game proof.
