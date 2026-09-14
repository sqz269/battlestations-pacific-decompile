# Native CRT SBH free BW

Discovery only, from `e9c7792df0bc8a213b6b13c88c206575f2b1eb4d`.
The preserved library name is `___sbh_free_block` at `00C11D68` (Visual Studio
2005 Release identification). This packet owns only that function and its
new evidence files. It adds no source, frame policy, heap owner, or annotations.

The complete `00C11D68..00C1207B` body is **788 bytes / 259 instructions**.
Fresh saved-program bytes equal the installed PE, and the complete live listing
has exactly the raw decoder's instruction-start sequence. There are no omitted
instructions or external tail jumps. The sole direct call and three indirect
calls are accounted for separately below. Ghidra project/program identity is
verified by the supported CLI before each live batch.

Corrected allocator discovery `40d4693c` is retained by its report SHA256
`e9e30237c1ee8c72e8240fab16afb5e50f907b29f747d50dec50f232eedcf5e7`.
Its complete 1,259-artifact path set and both hashes were verified, including an
independent chunked second pass. Its `_free`/classifier contracts are reused
with provenance; the allocator worker's new C12968/C1207C/C1212C work remains
external. The pinned report remains unchanged.

## Original ABI and memory domain

This is cdecl with descriptor `D` at `[entry ESP+4]` and user pointer `p` at
`[entry ESP+8]`, plain `RET`, and caller cleanup. The frame uses EBP, 16 local
bytes, saves ESI/EDI, and conditionally saves EBX only after the allocated-bit
gate. All four nonvolatile registers are restored on ordinary return. There
is no meaningful EAX result contract: EAX and flags depend on the path.

Both incoming argument slots are writable scratch. `[EBP+C]` successively
holds neighbor/start pointers, and byte `[EBP+F]` holds an old class counter.
Descriptor compaction can subtract `14h` from `[EBP+8]`. A future source ABI
must qualify this stack behavior, as well as actual native pointer identity;
this is not an ordinary opaque-buffer deallocation contract.

The entry captures `R=[D+10h]`, derives unsigned group index
`g=(p-[D+Ch])>>15`, and retains `G=R+144h+g*204h`. It loads the header at
`F=p-4`, subtracts one with x86 wrapping, then tests bit 0. If the result is
odd, it returns without list/counter changes. Descriptor/base/header reads
have already occurred; this is not null validation or a general double-free
safety guarantee.

| Native location | Contract established by this body |
|---|---|
| `D+0`, `D+4` | Aggregate nonempty-class masks, classes 0..31 and 32..63, MSB first |
| `D+8` | Group availability/decommitted mask, MSB first; all ones permits region release |
| `D+C` | Region reservation base; retained classifier bounds it to a 1 MiB domain |
| `D+10` | Actual metadata allocation R; freed through the owning CRT heap |
| `R+4..43h` | 64 byte counts of groups whose corresponding class list is nonempty |
| `R+44h+4*g`, `R+C4h+4*g` | This group's low/high class masks |
| `G=R+144h+204h*g`, `G+0` | Group metadata and allocated-block count |
| `S=G+8*k`; `S+4`, `S+8` | Class k sentinel next/previous links; these heads have an 8-byte stride, not independent 12-byte structures |
| Free block `F+0/+4/+8`, `F+size-4` | Size, next, previous, matching footer |

Class selection is the exact x86 signed arithmetic shift of the size by four,
minus one, followed by an **unsigned** clamp to `3Fh`. Shift masks follow x86
CL semantics. Sizes, pointer arithmetic, DWORD counts, and byte counts wrap;
no additional range/invariant checks occur here. Descriptive field names are
interpretations backed by the recorded accesses, not recovered C++ types.

## Coalescing and publication order

1. Capture successor header at `F+size` and predecessor footer at `F-4`.
   If the successor is free, compute its class. When its next and previous
   links are equal, clear its group-class bit, decrement the class byte count,
   and clear the descriptor aggregate bit only if that byte becomes zero.
   Then unlink it in the native order `previous.next=next`, reload the block
   links, `next.previous=previous`, and add its size.
2. If the predecessor is free, move F backward by the captured footer size
   and add that size. If its previous and resulting classes match, retain its
   current list position. Otherwise perform the same singleton-mask/count
   updates and ordered unlink. Forward coalescing has already happened.
3. If no predecessor was reused in the same class, insert F at the resulting
   class head: write F.previous, F.next, sentinel.next, reload F.next, and
   write next.previous. If the inserted list was empty, increment its byte
   count first, set the descriptor aggregate bit only if the old count was
   zero, then set the group mask. The original byte scratch at `[EBP+F]`
   preserves that old-count test.
4. Write the resulting header, then footer. Decrement `G.count` last. A
   nonzero count returns; zero enters the empty-group cache path.

Links and globals are repeatedly reloaded between writes. A reconstruction
must preserve this schedule under its declared alias contract; a generic list
helper with cached neighbors is not automatically equivalent.

## Empty-group eviction and region removal

`0109E310` is the descriptor of a previously cached empty group and
`0109ED78` its group index. When the current group becomes empty, an absent
old descriptor skips directly to publishing the current pair. Otherwise:

1. Load ESI once from actual IAT `00CE2088` (`KERNEL32.dll!VirtualFree`).
   At `00C11FAF`, call it with the old descriptor's region plus old-group
   index shifted by 15, size `8000h`, type `4000h` (`MEM_DECOMMIT`).
2. Ignore the BOOL. Reload the cache globals, OR the old-group bit into
   `oldD+8`, reload metadata, clear `R+C4h+4*oldg`, then decrement byte
   `R+43h`. Reload the descriptor/metadata again; if that byte is zero,
   clear bit 0 of `oldD+4`. The native path clears only the high group mask
   and class-63 count here; it relies on the actual empty-group invariant.
3. If the currently reloaded `oldD+8` equals `FFFFFFFFh`, at `00C1200A`
   call the same captured ESI with region base, size 0, `8000h`
   (`MEM_RELEASE`). Ignore the BOOL. Reload oldD, then at `00C1201C`
   call actual IAT `00CE20FC` (`KERNEL32.dll!HeapFree`) with the current
   `0109E1BC` heap, flags 0, and oldD's metadata pointer. Ignore that BOOL.
4. Load the current count (`0109ED64`), old descriptor, and array base
   (`0109ED68`). At `00C12042`, call original `_memmove` at `00BF87E0`
   cdecl with `(oldD, oldD+14h, count*14h-oldD+base-14h)`, using native
   wrapping arithmetic. The destination/source can overlap; the returned
   pointer is ignored. After its return and caller cleanup, decrement the
   global count. If the current descriptor argument is unsigned-greater
   than the reloaded old cached descriptor, subtract `14h` from that
   incoming descriptor slot. Reset `0109ED70` to the current array base.
5. Publish `0109E310=current adjusted descriptor` at `00C1206C`, then
   `0109ED78=captured g` at `00C12071`, and restore registers/return.

It evicts the **previously cached** group, not the just-freed group. The
metadata transition proceeds even if either VirtualFree or HeapFree reports
failure. There is no errno/GetLastError call, rollback, catch, local lock,
FS linkage, or exception frame in this body. A fault or nonlocal unwind can
leave earlier stores/API actions committed. Actual API ABI preservation is
required for the captured ESI and other nonvolatile values.

## Source availability and next owning boundary

The baseline already contains complete, qualified C11CF5 source in
`src/native_crt_small_block_heap_init.cpp`: real `HeapAlloc(actual heap,0,140h)`,
publication of `0109ED68` even on null, success-only clears of `0109E310`
and `0109ED64`, and ordered publication of `0109ED70`, threshold, capacity.
Its header borrows actual owning words. It creates no replacement heap and
does not implement this free routine. The lookup for C119BF names original
startup/selection calls but has no reconstructed source record at this base.

Indexed C11D68/BF87E0 lookups contain no reconstructed-source record. A
bounded source-tree search found only C11CF5 references to this cache/SBH
state. BF87E0 is named `_memmove` and has indexed dependency C0C82B
`__VEC_memcpy`; neither provider body is expanded or claimed here. A host
`memmove`, callback, or invented memcpy provider would not establish original
CRT identity. Real Win32 API names/argument declarations and flag constants
are retained from the installed SDK, but no runtime API experiment was run.

The smallest useful next discovery is the complete original BF87E0 provider
boundary, coordinated with the allocator worker's descriptor relocation and
region/group initialization contracts. A future complete C11D68 source packet
also needs explicit borrowed bindings to the same real heap, descriptor array,
count, search cursor, and empty-group cache, plus valid boundary tags/lists
and the caller's synchronization domain. It must preserve the ignored API
failures and alias-sensitive ordering. No partial free-list helper or private
heap state is recommended as closure.

The retained `_free` caller holds native lock 4 around classification and
this body, with its own SEH4 cleanup obligations. C11D68 does not establish
that lock domain. Live xrefs corroborate shared writes by initialization,
allocation, region creation, and this free body; data-only xrefs are not
promoted into ownership claims. The allocation worker's exact C12968/
C1207C/C1212C evidence is external and will need primary review before use.

All local evidence, scripts, selected current source, native bytes, listing,
SDK snippets, and provenance are inventoried in the report with SHA256 and
SHA512, then independently checked in a second pass. No C++, build, tests,
native execution, Ghidra mutation, settings changes, or push occurred. This
is complete bounded discovery, not source readiness or gameplay parity.
