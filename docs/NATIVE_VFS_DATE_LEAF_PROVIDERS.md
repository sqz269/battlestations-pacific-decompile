# Native VFS date leaf providers

This packet implements six complete native bodies over actual Win32 storage:
FileStore date BE5C80, find BE5A50, lower bound BE54D0, actual-header comparator
443D00, and the package zero-date leaves BB9D50/BBB640. The source lives in
`src/native_vfs_date_leaf_providers.cpp`; the public contract is in
`include/bsp/native_vfs_date_leaf_providers.hpp`.

These are raw readers. They do not construct a `FileStore`, copy a tree into
`std::map`, normalize names, open streams, or retain/release a provider.
The comparator is reusable by the physical date reader. Existing typed
`vfs_file_date.cpp` and `file_store.cpp` remain separate projections.

## Complete bodies and original ABI

| Original span | Bytes | Native interface |
| --- | --- | --- |
| 00443D00..00443D50 | 81 | Two native-header pointers on stack; RET8; AL less-than boolean |
| 00BE54D0..00BE5522 | 83 | ECX actual primary tree; name stack; RET4; EAX lower-bound node |
| 00BE5A50..00BE5AB5 | 102 | ECX actual tree; hidden iterator output/name stack; RET8; EAX output |
| 00BE5C80..00BE5CEC | 109 | ECX actual FileStore; hidden date output/name stack; RET8; EAX output |
| 00BB9D50..00BB9D66 | 23 | ECX unused; hidden output/name-unused stack; RET8; EAX output |
| 00BBB640..00BBB656 | 23 | ECX unused; hidden output/name-unused stack; RET8; EAX output |

The six complete bodies total 421 bytes. Fresh guarded Ghidra reads match
the installed executable. At worker handoff BBB640 had no Ghidra function record; its full 23 bytes
were recovered before the primary defined and annotated the function.
Names are descriptive hypotheses. These C++ interfaces do not reproduce
the original binary calling convention or normalize incidental upper EAX
bits when the native result is only AL.

## Actual storage and current-field behavior

Native names are eight-byte headers: DWORD recorded length at +0 and data
pointer at +4. A zero recorded length is empty even with a nonnull or stale
data pointer. Two nonempty headers use the current CRT `_stricmp`, with
right data loaded before left data. There is no stored-length tie-break,
length-bounded byte comparison, ASCII-only replacement or embedded-NUL guard.

FileStore's primary tree is at provider+14, with its head at provider+18.
Tree+4 is the head; head+4 is the root. Nodes have left/parent/right at
+0/+4/+8, native key header+C/+10, stored stream pointer+14 and sentinel
byte+19. The date reader does not consume the stream pointer, tree count,
secondary tree or device ID. `NativeFileStoreNameIterator` is the actual
eight-byte `{owner, node}` result layout; the functions accept raw output
storage so aliases are preserved.

BE54D0 starts with the current head/root. Each visited non-sentinel node
compares its current key to the supplied header, then follows the current
left/right link. The selected lower-bound node remains captured. There is
no allocation, normalization, snapshot, iterator validation or ownership
operation in this body.

BE5A50 calls that lower bound before its null-tree check. It compares the
selected node with the current head. If the node is the head or the supplied
name is less than the selected key, it **rereads** the head on the fallback
path; the code does not cache the head across a CRT comparison. It captures
both result words before writing output owner then output node, including
when output aliases the tree header, a node key or the query header.

BE5C80 captures provider+18 **before** writing five zeros into caller output
in descending offset order: +10, +C, +8, +4, +0. It then searches the current
tree at provider+14 and validates the returned iterator owner. A node that
differs from the earlier captured sentinel writes FFFFFFFF to all five
output words, again descending. It returns the supplied output pointer.
Output aliasing the input name or provider bytes is not rejected or hidden
by a temporary result. Volatile native-width accesses preserve these reads
and stores in the MSVC Win32 implementation.

## Provider profiles and package behavior

FileStore constructor BE7FA0 installs D689E8; its current +20 entry is
BE5C80. MPKG constructor BB9CB0 installs D64390, whose +20 entry is BB9D50.
MSAR constructor BBB5B0 installs D643C4, whose +20 entry is BBB640. The two
package leaves do only five descending zero stores and return the output
pointer. Their name pointers are not read, and no provider argument is
needed by the C++ leaf interface because native ECX is unused.

MSAR is a concrete native profile, not evidence that startup reaches it.
The discovery traced its factory registration through 73BC40/736D00, but
fresh references to 73BC40 remain empty. This packet does not expand the
typed startup factory manager or synthesize archive dates. All three date
providers ignore the physical provider's global-manager+78 disable byte.

## Exceptions and remaining boundaries

None of the six bodies installs native EH cleanup. The two iterator-owner
checks call BF6713, whose current CRT handler may return. The source uses
the established `SingletonLifetimeCallbacks::invalid_parameter` binding
and continues after a returned call. It adds no unconditional throw, early
return, repaired iterator, retry or missing-provider callback. The callable
binding and every reached raw memory address are caller obligations.

The host CRT `_stricmp` and invalid-parameter binding remain explicit
boundaries; this packet does not reconstruct the original CRT, Watson or
SEH implementation. Exceptions from a reached host callback can propagate;
the tree, comparator and FileStore functions add no `noexcept` termination
or rollback. The package store-only leaves are `noexcept`.

The functions are ready for the BDD340-created date visitor to dispatch to
them after matching current concrete profiles. They do not implement that
visitor, BDD0A0 mount traversal, physical BF3A80/BF39C0, VFS construction,
FileStore population or stream destruction. The raw comparator can be
shared by the separately owned physical-tree reader.

## Validation

Validation results and artifact hashes are recorded in
`reports/native_vfs_date_leaf_providers_audit.json`. The ignored CMake
project hook adds this source to `bsp_core` and links one ignored probe
against that library without changing shared build registration.

The strict MSVC Win32 Release build passed. Seed verification matched all
eight existing reference spans, and both existing CTests passed. The one
focused original/actual-library probe passed all 42 scenarios.

The probe executes the six unchanged original byte bodies with one common
address translation in its own process and compares them with the actual
library on identical raw arena addresses. Their relative internal calls
remain native; only the external
CRT `_stricmp` and invalid-parameter entry are routed to explicit host
boundaries. It checks return values and the complete arena after each
invocation, including empty/nonempty trees, hits/misses, case differences,
mismatched recorded lengths, ignored zero-length data and output aliases.
Package calls receive an unreadable name pointer to verify it is unused.
The returning-invalid-parameter continuation is source/assembly reviewed;
the focused valid-storage probe does not claim to exercise those checks.

No committed test suite, Ghidra mutation, source-name ledger change or
shared CMake change is part of this worker commit. Original ABI compatibility
and gameplay remain unvalidated.


## Primary main-library validation

All six functions are registered in the main build. The strict MSVC Win32
build and both existing CTests passed, and eight fresh native seeds matched.
The primary verified all 16 worker pins and nine unchanged current files,
then refreshed 12 guarded live/PE spans totaling 1,011 bytes.

The unchanged fixture passed all 42 original/main pairs, comparing 43,008
full same-address arena bytes and return values. It linked the frozen actual
main library `74d8f91d3e4c526a60efb9d61d45f5b2b9d46803b411766b6c9688d1d36c9895`;
the exact archive object matches every worker code/directive section and
relocation. All six complete COFF symbols, 707 bytes and six relocations,
were independently checked against linked bytes. No source provider was
recompiled in the fixture. This probe does not record runtime code postimages
or compare the full native register/SEH ABI; invalid-parameter calls and
original CRT locale semantics remain outside its dynamic evidence.

BBB640 was defined from its complete reviewed bytes. The two unnamed
FileStore lookup functions and MSAR leaf received descriptive names; existing
names and comments were preserved. Reviewed evidence was appended and saved,
and all six exports and complete function records were refreshed.
The read-only bundle at `local/vfs_leaves_primary/` has manifest SHA256
`a017521181e7bcc5ed5867436aed43b115c4b1e69aafa7f18c94216636730cd0`.
Mount traversal, the physical provider and game validation remain separate.
