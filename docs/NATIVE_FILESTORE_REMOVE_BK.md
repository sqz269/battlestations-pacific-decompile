# Native FileStore RemoveFile (BE7130)

`remove_native_file_store_file_00be7130` reconstructs the complete
`00BE7130..00BE7209` body (218 bytes) over actual FileStore storage. The original
uses ECX for the 2Ch FileStore and one stack argument for the actual eight-byte
name header, then returns with `RET 4`. EAX has no established semantic result.
The current decompiler's zero-argument prototype is incorrect: BE7148 loads
the entry stack argument, and BE7207 supplies the callee stack cleanup.

The source interface borrows the existing native string pool, stream-profile
dispatcher and returning invalid-parameter service. It does not introduce a
projected container, pending-request policy or Boolean result.

## Native schedule

1. BEE780 constructs and normalizes a local name copy. Its completed lifetime
   starts only after the constructor returns. The caller's header is unchanged.
   This preserves existing ASCII case, slash and space normalization, including
   the pooled header's allocation and failure behavior.
2. BE5E90 finds the normalized key in the resident tree at `store+14h`, using an
   actual eight-byte owner/node iterator. BE7176 captures its owner. BE717C
   captures the current tree head before checking whether the owner is null or
   differs from that tree. BF6713 can return; there is no invented early return.
3. BE718A reads the iterator node after that handler. Its comparison uses the
   previously captured head. A hit passes the captured owner, current node and
   the same iterator as output to BE6760. Source reads preserve this distinction
   even when a service changes current storage.
4. BE6760 advances its by-value iterator, removes and rebalances the actual
   resident node, destroys the original payload and frees the node, then updates
   the current count and output. The established payload destructor owns the
   stream decrement and zero-reference dispatch. RemoveFile adds no retain and
   does not touch the pending tree at `store+20h`.
5. Both hit and miss load the current normalized name and call 4254B0. Its entire
   live and PE body is one `RET`, so these format strings produce no diagnostic
   output or missing-key exception. Finally the current normalized buffer is
   released through 419CC0/BD1510 when nonnull, using current length plus one.

The existing BE6760 implementation, including two-child transplant and callback
storage reloads, remains the tree operation. This packet neither duplicates nor
changes its behavior. An invalid iterator can still reach the existing native
iterator error transport; no null-store or null-name safety policy was added.

## Unwind and evidence

The original registers handler CC6D88 and stores state 0 before the find call.
The handler is an unrecognized ten-byte code span, `CC6D88..CC6D91`, loading
FuncInfo E0168C and tail-jumping to BF6B43. E01684 contains the one-entry unwind
map, with previous state -1 and cleanup CC6D80. That eight-byte funclet computes
the completed local header and tail-jumps to 41DD20. Source RAII likewise arms
only after successful construction and releases the current completed header if
find or erase throws. There are no new cleanup guards inside the constructor.

Five fresh read-only spans match the installed PE: the complete main body,
CC6D80, CC6D88, 44 bytes of unwind metadata and the one-byte diagnostic leaf;
281 bytes in total. The executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The report records all seven main-body calls and the cleanup tail jump, with
exact body ranges, bytes and hashes. `verify_report_calls.py` checks the eight
rows against the live containing bodies and exact transfers. The unrecognized
handler is reported explicitly instead of treating it as a Ghidra function.
Each live batch used the verified existing `C:/Users/sqz269/bsp.gpr` project and
`/battlestationspacific.exe`; there were no Ghidra edits, annotations or saves.

## Validation

The TU passed MSVC Win32 `/std:c++17 /EHsc /MD /O2 /W4 /WX /fp:strict`.
One ignored source fixture, `local/remove_bk/remove_probe.cpp`, extends the
already established full request/physical completion fixture. It freezes the
complete removal TU, 41 transitive headers and three existing primary libraries
before building, rather than supplying unresolved source stubs. The executable
uses `/MANIFEST:EMBED`, `/BASE:0x30000000` and `/DYNAMICBASE:NO`. Exact input,
source, command-script, object, executable and output hashes are in the report.

The fixture passed the real 257-byte physical read through request deduplication,
the manager queue, current-factory completion/AddFile and resident insertion.
It then established that:

- Normalizing a removal name leaves its caller-owned header unchanged.
- An external stream reference survives cache erasure with identical data; a
  second removal of the missing key leaves that reference unchanged.
- Removing the sole cached reference destroys the actual stream and backing.
- A pending entry with the same normalized key survives resident removal and is
  subsequently cleared by the actual FileStore destructor.
- Actual stream/backing allocation counters return to zero after final cleanup.

The resolver and final callback are explicit controlled fixture services. All
request, queue, physical I/O, cache, tree and ownership operations use complete
existing source services. The fixture does not execute original RemoveFile
machine code and does not exercise returning-handler stack alias mutation or
throw injection. Those schedules are supported by the native listing and FH3
evidence, separately from successful source execution.

The explicit source interface is not a binary ABI or original FH3 replacement.
`NativeStringStorage::release` is noexcept, so a throwing native pool getter,
arbitrary EH-frame aliases, SEH, concurrent mutation and gameplay remain outside
the validated contract. Primary integration/CMake and game validation are
separate; this packet changed only its four leased source/evidence files.
