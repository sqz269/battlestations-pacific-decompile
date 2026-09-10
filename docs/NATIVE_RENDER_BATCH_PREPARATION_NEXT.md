# Next raw batch-preparation and sorting work

The actual pointer-slot sort and raw key readers are ready for bounded
implementation. Full `00B51DF0` still requires the real queue singleton:
`004C11F0` runs before the enable-byte test and can construct/register the queue.
The newly integrated `read_native_render_batch_sort_configuration_00b1cb30`
closes its field-copy dependency, not that acquisition/lifetime path.

This is a read-only review at main `eb6b5a6`. Seventeen complete function spans
(2,248 bytes) and the 16-byte `00D5E5AC` batch vtable were freshly compared with
live Ghidra and the installed PE. All matched SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Current saved prototypes/comments and relevant source hashes are captured in
`reports/native_render_batch_preparation_next.json`. No C++, annotations,
shared ledgers, tests or game files changed; no build or native test was run.

## Actual storage and preparation order

The existing `NativeRenderBatchStorage` is the actual 18h owner: vtable+00,
reference count+04, mode+08, entry-pointer array+0C, signed count+10 and signed
capacity+14. Preparation does not read mode or capacity, retain entries, allocate
slots, or change ownership. `D5E5AC+0C` is `B51DF0`; the adjacent execution slot
`+08=B55550` is not called by preparation.

The array contains four-byte entry pointers. Sort receives addresses of those
pointer **slots**, not a linked-list sentinel or entry-object range. Its
partition result is two slot addresses delimiting the equal band. The queue's
command array at+14 and 20-byte (14h) rows at+24 are different containers.

Although the raw batch declaration currently spells the element type
`InstanceRenderEntry*`, the separately declared `InstanceRenderEntry` and
`InstanceRenderQueue` are explicitly semantic interfaces. The latter owns a
`std::vector`; generated sections expose projected material values. Neither
establishes native entry/section/material/effect object layout. A raw port must
read and write the actual pointer cells and byte-addressed pointees; it cannot
cast native objects to these semantic classes or sort a detached vector.

Original `B51DF0` uses ECX=batch, stack original index, RET4 and no semantic
return. It saves the original index in EDI, initializes a local enable byte to
zero, and passes its stack index word as the getter's DWORD output. `4C11F0`
returns the actual queue, then `B1CB30` reads:

1. Byte `queue+4+index*8`, followed by its output store.
2. DWORD `queue+8+index*8`, followed by its output store.

The getter has no index bounds check; its EAX is the copied DWORD with AL set
to1. Preparation ignores that return and copied DWORD. Zero enable skips all
batch array/count/entry access. Any nonzero enable sorts; the saved original
index alone selects index-zero key generation versus the other comparator.

Index zero initially compares signed count+10 with zero. For each iteration:

| Step | Actual access and order |
| --- | --- |
| 1 | Reload batch+0C; read entry pointer at array+4*index. |
| 2 | Read `entry+04 -> section`, then `section+20 -> material`. |
| 3 | Compare signed WORD `material+34` with zero; capture `material+7C -> effect`; read DWORD `effect+B0`. |
| 4 | Only for positive signed count, read pointer `material+10`; only if nonnull read DWORD `texture+20`. Otherwise use zero. |
| 5 | Compute the first prefix multiplication; then read unsigned BYTE `effect+C0`. |
| 6 | After the second multiplication, load binary32 `entry+14` into x87 ST0 and call `BF7456`. |
| 7 | Discard helper EDX, add zero-extended EAX, store key low DWORD at entry+20 **before** high DWORD at entry+24. |
| 8 | Restore the actual batch pointer, increment the index and reread signed count+10 before continuing. |

The signed count condition survives the intervening MOV/CDQ instructions until
`B51E5F:JLE`. Nonpositive material count must not even read slot zero. Effect
and B0 are nevertheless required for every processed entry. There are no null
guards for entry, section, material or effect, and no material callback/error
channel. Duplicate entry pointers produce repeated current reads and writes.
Do not hoist depth, count, array or field reads across the observed operations.
The touched extents establish only minimum readable storage: entry28h,
section24h, material80h, effectC1h and conditional texture24h. They do not
establish complete object sizes or ownership-bearing class definitions.

After key generation, index zero reloads array and count to form bounds. Other
indices read count, then array to form last, then reread array for first.
Bounds use native 32-bit `last=first+count*4`, and ideal is
`SAR32(last-first,2)`. Nonpositive count skips key generation but still reaches
that sort call. The sort itself returns without slot access for signed
distance<=1. Replacing these operations with a vector's unsigned size and a
new rejection path changes the native contract.

## Key and comparison closure

All arithmetic in this formula is unsigned:

```text
texture = material_count34 > 0 && slot0 != null ? texture20 : 0
prefix = (((effectB0 & 0x3f) * 0x1000 + (texture & 0xfff)) * 0x100)
         + uint8(effectC0)
key = (uint64(prefix) << 37) + uint64(depth_helper_EAX)
```

The two native `__allmul` calls at `BF7DF0` compute low64 products. The second
constant is high DWORD20h/low DWORD0, hence `2^37`. Native key bit63 is zero;
bits32..36 are zero; depth contributes only bits0..31. In particular, a finite
depth of -1.75 contributes `0xffffffff`, without extending its sign upward.

`B51B00` compares unsigned key high DWORDs at+24 and reads low DWORDs at+20
only on a high-word tie. `B51AB0` first traverses the left section/material/
effect path, then the right path; it reads **right B0 before left B0**. Unequal
values compare as signed32. Only on equality does it load right depth, then
left depth, perform `FCOMIP left,right`, pop the remaining x87 value, and return
true only for left>right. Equal and unordered depths return false when x87
exceptions are masked. All sort callers consume AL; the unequal-material path
retains the right B0's upper24 bits in EAX.

`BF7456` is a complete 117-byte leaf: input ST0, output EDX:EAX, pop one x87
value, RET. It duplicates/stores the sign, performs FISTP signed64 under the
current rounding mode, reloads that integer, computes a residual and adjusts
toward zero. Its special zero/integer-indefinite path pops both remaining
values. It leaves the control word unchanged but has x87 status/exception
effects. With binary32 input and masked exceptions, EAX is low32 of signed64
truncation; NaN, infinity and signed64 overflow contribute zero.

Existing `render_batch_depth_word_00bf7456` is a useful numerical oracle for
that masked binary32 domain. It does not reproduce status flags, unmasked traps
or general extended-precision ST0 behavior. Likewise the current value-based
material predicate preserves the comparison result, not x87 effects. A raw
port claiming the complete observed floating-point behavior must retain those
native operations. Reusing the numerical helpers requires an explicit limited
contract and cannot be silently labeled full native conversion.

## Complete pointer-slot sort closure

All ten functions are available as named, exported, byte-verified native code.
Their existing C++ implementations are internal algorithms over a semantic
vector; none is an actual-storage public entry. There is no allocation,
refcounting, virtual material execution or native SEH registration in this
closure. The native comparator is an ordinary supplied function pointer called
with ECX=left entry, EDX=right entry; AL is tested. That existing native boundary
supports a concrete host callable over borrowed actual addresses.

| Address | Role | Original arguments; cleanup | Bytes |
| --- | --- | --- | ---: |
| B1DCE0 | Introsort | ECX first, EDX last; stack signed ideal, comparator; RET8 | 235 |
| B1D420 | Insertion | ECX first, EDX last; stack comparator; RET4 | 150 |
| B1C210 | Rotate slots | ECX first, EDX middle; stack last, two unread words; RET0C | 145 |
| B1D280 | Partition equal band | ECX output pair, EDX first; stack last, comparator; RET8 | 382 |
| B1CF80 | Choose median | ECX first, EDX middle; stack final slot, comparator; RET8 | 156 |
| B1C8C0 | Order three samples | ECX first, EDX middle; stack final slot, comparator; RET8 | 77 |
| B1D020 | Make heap | ECX first, EDX last; stack comparator, two unread words; RET0C | 62 |
| B1D6A0 | Sort heap | ECX first, EDX last; stack comparator; RET4 | 70 |
| B1C910 | Adjust heap | ECX first, EDX signed hole; stack signed length, saved entry, comparator; RET0C | 110 |
| B1C1B0 | Push heap | ECX first, EDX signed hole; stack signed top, saved entry, comparator; RET0C | 92 |

Preserve insertion cutoff32; ninther starts at42 entries; GCD rotation cycles
start at first+gcd and descend; equal-band slides retain their two-swap order.
Positive ideal becomes `ideal/2+(ideal/2)/2`; signed ideal<=0 selects heap when
more than32 entries remain. The smaller unequal side recurses, with right
recursion on ties. Heap child ties select right, and parent ties stop upward
movement. Large-sort equal-entry order is not generally stable. Native code
uses signed pointer-distance arithmetic and unsigned address comparisons for
several partition bounds; no host `std::sort` substitution is justified.

The current public category sort adds nonnull/finite-depth/count preflight.
The current batch projection adds checked configuration lookup, a material
reader callback and explicit partial-failure results. Those are useful host
interfaces, not native validation discovered in these functions. Existing
tracked reports describe prior numerical/permutation fixtures; this review
refreshes their code evidence and does not claim to have rerun those fixtures.

## Ready packets and remaining owner work

1. **Raw pointer-slot sort plus unsigned-key predicate.** Own the ten addresses
   above and B51B00. Implement directly over caller-supplied current pointer
   storage, signed ideal and the native comparator boundary. No queue getter,
   material owner, allocation or entry constructor is needed. Preserve exact
   mutations and existing borrowed ownership; expose a new C++ ABI explicitly.
2. **Raw key-generation slice and material predicate.** Own the index-zero
   B51DF0 slice, B51AB0 and conversion support BF7456. Read actual byte-addressed
   objects and current signed batch fields, write low/high keys in native order,
   and either implement the complete x87 operations or declare numerical-only
   scope. Material/effect constructors and virtual execution are not required
   for a borrowed-storage leaf, but valid actual pointees and their lifetimes are.
3. **Borrowed-queue preparation dispatch, after 1 and 2.** Use the existing raw
   B1CB30 getter with an explicitly supplied already-published actual queue.
   Preserve original-index selection and disabled access order. This is a
   bounded slice with an acquisition precondition, not complete B51DF0 and not
   a stubbed successful singleton getter.

Full B51DF0 remains dependent on actual 4C11F0 acquisition. The verified getter
fast path reads global F8D440, while its slow path obtains the manager lock,
allocates34h, calls B1F280, publishes and registers the result, then unlocks.
It has native SEH registration at C64F13. No queue-owner implementation exists
in this baseline. Concurrent queue-owner review reports B1F280 can act on the
existing control+20 preimage and reach renderer stop B28A90; destruction runs
B1EBE0 and live command virtual execution B1D950. These are not empty-queue
operations that can be omitted. That independent packet owns the detailed
row/lifecycle evidence. The actual B1CB30 field getter does not close them.
That review is committed as `9a517e0`, with details in
`docs/NATIVE_RENDER_QUEUE_OWNER_NEXT.md` and
`reports/native_render_queue_owner_next.json`; it adds evidence, not an owner
implementation. Its report hash is pinned in this packet's audit.

All recorded Ghidra prototypes still show `undefined name(void)` despite the
register/stack contracts above. Existing comments include older hypotheses and
later updates; they were captured without editing. Original ABI descriptions
here come from complete assembly, not those incomplete saved signatures.
