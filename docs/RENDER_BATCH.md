# Retained command to material dispatch

Command vtable 00d5e5e0 slot 0 points to newly recovered 00b1d950. It first checks
renderer inhibit/lost through 00b20240, establishes context state, then checks
renderer frame activity through virtual +2Ch. That slot is now recovered as
00b1fe20, returning whether renderer+1998h is nonzero.

Each command owns two batch pointers at +Ch and +10h. Preparation uses virtual
+Ch (00b51df0) serially unless optional renderer synchronization is disabled and
both batch counts exceed 50. In that case both preparation jobs are dispatched
through the job service. Execution then invokes virtual +8h on batch 0 and 1 in
order, passing batch index and context.

Batch vtable 00d5e5ac slot +8h is 00b55550. Its RET8 consumes those two arguments,
but the function does not read them. When a frame is active it zeroes global
0108fbf4, walks entry pointers at batch+Ch/count+10h, obtains an implementation
through entry+4 -> object+20h -> material-related object+7Ch, and invokes its
virtual +14h with the entry pointer. The actual material/render implementation
must be resolved before this can be connected to the ported D3D drawing code.

Preparation 00b51df0 consults queue configuration through 00b1cb30. If enabled,
batch 0 derives a 64-bit key at entry+20h before sorting with comparator00b51b00.
The other batch selects comparator00b51ab0. Exact sort-key and comparator behavior
remain unported; these batches are not yet named opaque/transparent passes.

Initializer 00b1edc0 allocates an 18h context, stores it at command+28h, retains
supplied references, records a back-pointer and acquires two pooled batches.
This establishes the context whose lifetime the queue executor retains during
execution. Full allocator, pool and teardown behavior remains unresolved.

Three missing functions were created and five descriptive names/comments saved
in Ghidra. All five ranges were checked against the original binary; see
`reports/render_batch_evidence.json`. This is analysis progress; no C++ or tests
changed. Next resolve the entry material implementation's virtual +14h and its
constructor, then follow it into bindings and draws.
