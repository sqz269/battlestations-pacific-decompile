# Actual classification pointer lists

Addresses: 00714120,00714180,007141E0,00716A50,00716AA0,00716AF0,
00716A70,00716AC0,00716B10,00718350,00718380,007183B0,007188C0,
007188F0,00718920,0071A5A0,0071A760,0071A920,0071B230,0071B2C0,
0071B350,0071B8D0,0071B940,0071B9B0.

All three eight-body families are instruction-identical after substituting
their audited family call targets. The shared source implements their complete
ordinary control flow, including generic middle/multiple insertion. Each
actual16h header has an untouched word0, begin4, end8 and capacity-endC. Elements
are four-byte raw values. No pointee access, retain, release or deduplication is
performed.

Append compares unsigned size and capacity derived from wrapping pointer
differences followed by SAR2. Its fast path captures the current end, loads the
pointed value, stores it and publishes that captured end+4. Its slow path retains
the captured end across a returning invalid-parameter callback, then invokes
insert-one. Insert-one captures the original begin/offset, preserves returning
iterator checks, and writes the resulting iterator's position before its owner.
The output can alias caller storage; no owning iterator wrapper is introduced.

Insert-copies reads the value through its address before testing count0 and
captures the begin/capacity. Required size is bounded by3FFFFFFFh with the
existing length-error semantics. Growth uses capacity+floor(capacity/2), falling
back to required size if the candidate exceeds the maximum. It allocates,
copies prefix, fills the captured value and copies suffix. It then captures the
current old begin/size, frees that backing storage, and publishes captured new
begin, capacity end, then end. There is no local allocation/element rollback.

Without growth, a suffix shorter than count is copied past the inserted area;
the remaining values are filled and the current end updated before the final
range fill. Otherwise, the last count values are copied to the old end, the new
end is published, the remaining suffix moved backward, and the gap filled.
Current-field rereads are preserved around the copy/fill services.

Copy-range and backward-copy use CRT memmove_s, ignoring its return status.
Range copy calls when signed word count is nonzero; backward copy calls only
when it is positive, while still returning the computed destination for other
counts. Fill helpers reload the value at every store, preserving readable alias
behavior. Allocation checks count*4 before the existing BF681B allocation domain.
Source overflow and length failures use current C++ bad_alloc/length_error;
native exception object construction and throw machinery are not ported.

Three missing ADD ESP,4 instructions after returning BF65AC calls were restored
to their original insertion function membership. Original bytes, names and
comments were preserved; callee no-return flags were not changed.

The fixture relocates all24 original list bodies and72 direct-call sites,
binding known allocation/free/memmove services to the same source process.
Twenty-four paired scenarios span all three families: empty/fast append,
growth with an old-array value alias, both in-place insertion branches, count0,
and insert-one's returned iterator. It compares live contents, counts, capacity,
untouched header word, iterator owner/offset and surviving spare storage. Newly
allocated unused capacity is intentionally not compared because it is
uninitialized. Original error/throw paths are guarded out of this oracle;
source length and allocation-overflow throws are checked separately.

This does not prove native CRT invalid-parameter handler ownership, errno,
native exception ABI, hardware faults, arbitrary corrupt ranges, private stack
aliases or gameplay. The full C++ body claim describes the recovered schedule
over valid actual storage and documented library boundaries. See the BX list
report and integration manifest for exact evidence and artifact hashes.
