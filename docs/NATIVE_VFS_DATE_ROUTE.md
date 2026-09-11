# Actual VFS date visitor route

`src/native_vfs_date_route.cpp` implements the six original bodies used by the
stack date visitor created in BDD340, with contracts in
`include/bsp/native_vfs_date_route.hpp`. It reads caller-supplied manager, mount,
provider, profile and pooled-string storage. No tree construction, mount sorting,
stream owner, projected VFS context or provider callback is introduced.

The traversal is explicitly qualified to the **D683B0 date visitor route**.
It does not claim all other visitors that can reach the original BDD0A0 body.
The current concrete provider date words supported at +20 are BE5C80 (FileStore),
BB9D50 (MPKG), BBB640 (MSAR), and BF3A80 (physical). Physical dispatch retains
its established D69168 slot domain. A different visitor identity/slot or provider
date word is an explicit `std::invalid_argument` **source boundary**, not a
recovered native failure mode. Base-provider purecall and other visitor domains
remain outside the implementation. MSAR's concrete zero leaf does not establish
normal-startup reachability of its registration routine.

## Complete original bodies

| Address | Bytes | Original ABI and result |
|---|---:|---|
| BDD340 | 249 | ECX captured manager; stack output/name; EAX output; RET8 |
| BDD0A0 | 672 | ECX manager; stack name/visitor; no specified result; RET8 |
| BD9E80 | 66 | ECX visitor; stack mount payload/name; no specified result; RET8 |
| BD9F00 | 39 | ECX visitor; EAX0/1; RET |
| BD90B0 | 7 | ECX visitor; write D68380 identity; RET |
| BD97E0 | 99 | ECX owner/node iterator; no specified result; RET |

These six full spans total **1,132 bytes**. The source interfaces are rebuilt
C++ functions, not original calling conventions or SEH replacements. Addresses
and descriptive names are evidence labels; names are not recovered symbols.

The report pins 33 fresh guarded spans totaling 4,101 bytes, including these
bodies, D683B0/D68380, all four provider profiles, both unwind maps and handlers,
and the current raw string, comparator, leaf and physical body dependencies.
Every pinned span matches the installed PE. All live reads used the guarded
`tools/bsp.py ghidra` interface against the existing bsp.gpr/program.

## Actual storage and profile binding

The manager's status DWORD is at `+18`; the mount tree owner is `manager+3C`,
with its current head pointer at `manager+40`. The head's left link starts the
in-order traversal. Mount node left/parent/right links are at `+0/+4/+8`, priority
at `+C`, prefix length/data at `+10/+14`, provider at `+18`, ownership byte at
`+1C`, and nil byte at `+21`. The visitor payload is the existing `node+10`.
Traversal reads no priority or ownership policy and does not AddRef a provider.
The caller supplies valid existing storage; neither startup insertion order nor
tree population is synthesized by this packet.

The date visitor is six DWORDs: original profile identity D683B0 followed by
five result words. The source retains that literal native identity. Its context
borrows the application's actual three-DWORD D683B0 profile storage so the host
can read the current +4/+8 words without installing a fake C++ vtable or calling
unrebuilt original code. This is an explicit address-binding boundary. Every
reached dispatch rereads the visitor identity and current slot in that storage.
No table copy or baked callback is retained. The +0 scalar destructor BD9F30 is
not invoked on the BDD340 stack route; only the direct unwind reset BD90B0 is
needed. D68380's +4/+8 purecall words are not date callbacks.

The context also borrows the completed `NativePhysicalFileDateContext`. Its
`ActualNativeStringPoolStorage` is the application's owning pool bridge, with
the actual publication, shutdown gate and canonical lifetime domain. Pool
operations do not fall back to a new CRT allocator or lifetime manager.

The manager passed to BDD340 is captured for the complete traversal. The physical
provider independently reads the **current** 0109CEEC publication and that
manager's +78 byte when BF3A80 executes. The two managers need not be the same.
BDD0A0 writes only its passed manager's status to FFFFFFFF before copying the name.

## Native operation and failure order

BDD340 initializes D683B0 and zeroes the five result words in descending order.
It copies the actual input header before arming copied-name cleanup, normalizes
through the completed BEE690 adapter, then invokes BDD0A0 on the captured manager.
Normalization lowercases ASCII, converts counted backslashes and trims space
bytes; it does not add the separate slash/dot canonicalizer or open-name aliases.
The result loads and ascending output stores preserve the native interleaving.
The copied name is released after publication. Normal return does **not** call
BD90B0; on C++ unwind the armed name is released and the visitor base identity
is reset, matching the recovered ownership states.

BDD0A0 copies its input before owning that local. It walks the actual iterator,
capturing the current head before each reached owner check. If the node is not
that captured end, it validates the node against the owner's current head before
reading its prefix. Empty prefixes match all names. A nonempty prefix must be
strictly shorter under unsigned length comparison, equal the copied prefix
substring through 435C40, and have a following `/` byte. Prefix length and main
data are reread after comparison. The temporary's returned pointer is passed to
the comparator; no returned-buffer identity is assumed.

The prefix-comparison temporary has a normal free flag but no additional native
unwind state. The source does not invent cleanup for that temporary if comparison
throws. On a match, the prefix is reread after its normal release. A nonempty
prefix produces a substring after `prefix.length+1`; an empty prefix selects the
full copied name. Both then receive a separate copied callback argument.

Substring ownership is armed only after it returns. A failure in the callback
argument's initial copy does not own or clean its partial output. After a
successful copy, native state3 owns that argument and the main name. The suffix
intermediate is disarmed/freed before visiting the provider. The pre-callback
check rereads actual iterator node and owner; the payload remains the originally
captured node+10 pointer through all intervening string work.

The current visitor +4 word selects BD9E80. That function reads payload+8, then
the provider's current +20 slot, and invokes the complete selected raw leaf or
physical adapter. It copies five words **from the returned pointer**, interleaving
each source read and destination store. It overwrites earlier results even for
an all-zero provider miss. No stream lookup or typed provider is substituted.

After provider work BDD0A0 rereads the current visitor and +8 slot. BD9F00 checks
the result DWORDs from +4 through +14 and stops at the first nonzero value.
The copied callback name is disarmed/released before stopping or advancing.
BD97E0 reads current links, preserves each iterator parent-climb write, and
returns through a tail-called invalid-parameter handler on nil-node advance.
Other reached CRT checks continue if the handler returns. The next iteration
reloads the current head; there is no initial-head snapshot or post-callback
retention/recovery policy.

The source reuses actual adapters for 425F40, 435C40, 443D00, 41DD40, 41DD20,
469840, 426060 and BEE690. Their existing zero-byte-copy and noexcept pool-release
boundaries remain explicit. C++ exception ownership is reconstructed, but native
hardware SEH, raw binary ABI, invalid backing memory, other visitor dispatch and
forced physical time-conversion failure are not dynamically certified here.

## Verification and integration

The audit records a strict MSVC Win32 build through an ignored CMake source
hook, both existing CTests, all eight seed checks and a focused ignored probe.
No permanent tests, shared CMake, ledger or saved Ghidra state were changed.
Current source, actual built archive, object and probe bytes are frozen by hash.

The probe executes thirteen unchanged original bodies, totaling 1,642 bytes:
the six owned route bodies plus the six leaf/comparator bodies and 435C40. The
original code uses one common relocation delta. A mapped D683B0 profile supplies
the translated native entries when executing original code and original selector
words when running the rebuilt route. These address-binding table changes are
fixture setup; the body bytes remain unpatched.

Original string resize, normalization and substring call sites are bridged to
the actual built library with the recovered register/stack ABI. Original pool
getter/return calls use the actual getter and pool storage functions, and the
physical date slot uses the complete built physical adapter. The same actual
owning pool, mutable mount arena, provider records and name inputs are used for
both runs. This is a native composition comparison, not an independent execution
of every original dependency. No provider behavior is replaced by a fixture
callback.

Eleven route comparisons cover empty traversal, ordered MPKG/MSAR misses before
FileStore success, strict prefix length/slash checks, normalization, aliased
input/output, early success before an unusable later provider, all-zero misses,
a returning CRT repair after an MPKG miss, and physical enabled/disabled dates.
The repair updates the head after the date callback and proves the next-iteration
head reread. It repairs deliberately invalid iterator input through the established
CRT boundary. It is not an original tree-population claim.

Each comparison checks the specified output pointer, all 2,048 arena bytes,
invalid-handler count, and the complete pool prefix through its arena/ring up to
the critical-section offset. The live OS critical section is preserved across
replays. A real fixture file has a fixed UTC last-write timestamp. Separate
original/rebuilt checks cover BD9F00's nonzero result and BD90B0's base reset.
No throwing native unwind or changed visitor-slot behavior is fixture-certified.

The primary integrator owns shared source registration and metadata. This packet
closes the actual BDD340 date route within the declared visitor/provider domains;
it does not close unrelated BDD0A0 callers, mount registration, stream ownership,
or the downstream renderer reload callbacks. ABI compatibility and game
validation remain false.


## Primary main-library integration

All six qualified entries are registered in main. Strict MSVC Win32 compilation,
both existing CTests and eight fresh native seeds passed. The primary checked
34 worker pins, twenty literal current files and 33 fresh guarded spans totaling
4,101 bytes. Three main objects matched every worker code/directive section
and relocation; object debug metadata and anonymous namespace hashes differ.

The unchanged probe linked actual main library
`90f6e12be9a855a4abcd0f5f7c04ba8518868fee030f3660bc25a25c10a9a265`.
Eleven original route comparisons again passed, covering thirteen unchanged
original bodies, all 22,528 arena bytes and 100,082,092 pool-prefix bytes across
the pairs, plus output pointers and returning CRT counts. Real fixed UTC file,
separate current physical manager, post-MPKG head repair and stop/reset checks
passed. The temporary file was removed.

Nine exact main archive members and 255 complete COFF sections were verified,
22,431 bytes and 922 relocations. Four separate owned entries are retained;
BD97E0 and BD9E80 are inlined into the retained traversal. Their full source
object is frozen, but separate linked entries are not claimed. No runtime code
postimages or additional throwing/native-SEH comparisons were introduced.

The read-only primary bundle is `local/vfs_date_route_primary/`, seal
`fe1e7be74acd0fa95d551c51ee4f9c456764b8b1f583f66de16bc22c67be027a`.
Existing saved names/comments were preserved. The reset and mount successor
received descriptive names, reviewed evidence was appended and saved, all six
exports refreshed and qualified complete source records registered. Other
visitor/provider domains and the limitations above remain outside this packet.
