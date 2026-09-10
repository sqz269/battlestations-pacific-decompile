# Native command and checked alias integration

This batch integrates nine complete native functions: five render-command
ownership functions and four checked alias iterator/erase functions. A separate
host companion binds the actual batch reference count to its existing throwing
pool-return implementation; it adds no original function body.

The command now constructs and destroys actual `44h` storage with real contexts,
batches, populated groups, models, pointer arrays and pooled strings. Its
ownership rules allow retained models, contexts and batches to survive command
destruction. Alias operations preserve returning validation-handler repairs,
captured iterator inputs, native link-write order, the count loaded after free,
and output publication when storage overlaps.

The primary review rechecked 111 installed code/data spans and seven source or
build-registration hashes. Both worker implementations match their committed
source text. The combined strict MSVC Win32 build and existing CTest checks pass
(2/2). Primary fixture reruns pass:

- Full command constructors and populated teardown: 1,451 normalized values
  match the installed machine code, including changed loop bounds, retained
  owners, scalar flags 2, pooled batch reuse and shutdown.
- Checked iterator/erase sequence: all 225 trace words match, including returning
  handler repairs, release/free reentry, live count changes and output overlap.
- Batch companion: a real allocator failure during free-list growth propagates
  after entry destruction, releases the captured lock, preserves the unreturned
  dead slot, and retires the companion once. Successful recycling and companion
  self-deletion also pass. This is a compiled-host failure check; the command
  comparison separately executes the original successful batch paths.

Ghidra's false no-return flow at command scalar deletion and alias erase was
repaired with prior documentation preserved. Ten function annotations were
saved and their exports refreshed; the sharded ledger contains nine full-body
records plus the batch-companion fragment. The evidence and artifact hashes are
in `reports/native_command_alias_integration_audit.json`.

These are new C++ interfaces over native layouts, not drop-in binary replacements.
The command fixture does not execute its exception tables or scalar free-bit
branch. Full `00B1D950` command execution, queue/gather/preparation owners, legacy
string/error construction and the remaining renderer/world startup path are
still separate work. No complete renderer or gameplay-validated rebuild is
claimed by this batch.
