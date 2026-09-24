# Raw resource factory registration and insertion

The six complete normal bodies total 1,627 bytes. They insert actual named nodes
and register borrowed factory values in the existing resource registry. This
closes a source dependency for procedural registration; it does not activate
B107F0, publish F8D420/F8D41C, construct BBC resource globals, or admit their
terminal graph into the application.

| Entry | Exclusive end | Bytes | Native interface |
|---|---|---:|---|
| B1B3A0 | B1B3F4 | 84 | ECX registry; name/factory; RET8 |
| B1B2B0 | B1B39E | 238 | ECX tree; name; RET4/EAX actual value cell |
| B1B0B0 | B1B2AC | 508 | ECX tree; output/hint owner/hint node/pair; RET10 |
| B1ADA0 | B1AF8C | 492 | ECX tree; output/left word/parent/pair; RET10 |
| B1AF90 | B1B0A4 | 276 | ECX tree; output12B/pair; RET8 |
| B19DD0 | B19DED | 29 | ECX current raw string header; RET |

The current Ghidra label `STL_xlen_throw_00b1ada0` is preserved, but is misleading:
the complete function includes node allocation at B1AE24, current count increase
at B1AE33, linking, rebalance and output publication. Only its size-limit branch
throws. Naming remains the primary integrator's later operation.

The actual 10h registry is profile0, preserved4, head8, countC. Its tree subobject
at +4 is opaque0/head4/count8. Actual 1Ch nodes contain left/parent/right0/4/8,
name length/data C/10, borrowed factory14, color18, nil19 and padding1A/1B.
There is no host map, owning factory wrapper, metadata pointer or extra credit.
The existing genuine registry construction/disposal, node allocation, rotations,
lookup, raw string pool and CRT providers all use these fields.

Frames are immutable views over genuinely live caller DWORD or typed iterator
objects. The source never casts a frame aggregate onto reused scratch. View
metadata and fresh persistent Acquired records are disjoint from native cells.
Caller-supplied preimages are retained wherever native code does not write.
With S denoting original entry ESP, including the return word:

| Frame | Native live argument/local sites |
|---|---|
| Link | output S+4, left S+8, parent S+C, pair S+10; actual SBO S-50..S-35 |
| Unique | output S+4, pair S+8; direction DWORD S-C, cursor owner/node S-8/S-4 |
| Hint | output S+4, **incoming** hint owner/node S+8/S+C, pair S+10; local result12B S-C..S-1 |
| Value | name S+4; result owner/node S-20/S-1C; temp length/data/value S-18/S-14/S-10 |
| Registration | name S+4, factory S+8; result owner/node S-8/S-4 |

The hint's local result first eight bytes retain one iterator lifetime throughout
their end-iterator and insertion-result roles. The incoming hint is the SAME
live object for decrement/increment, including the native current ESP+24/+28
sites. It is not copied into a disjoint cursor. Direction writes change only
the low byte of its live DWORD, and B1B058 later pushes that full current DWORD.
Result inserted+8 is likewise one byte; the upper three bytes remain preimage.
Nested provider frames represent their own call sites and persist for failure
disposition. Cross-provider private stack-address coincidence, saved registers,
native exception objects and FH3 frame identity are outside this interface.

B1ADA0 performs the unsigned count limit before reading its normal arguments.
It captures pair, head and parent, seeds the real allocation frame in native
push order, then calls B1AD10. Afterwards it captures current head, increments
current count, and uses the current left LOW byte if parent is not the head.
Current links are updated in native order. Only the existing pure repair block
was extracted into `detail::rebalance_linked_tree_node`; its text is identical,
and all other source in its prior caller is identical. The mirrored inline
rotation retains its distinct load/store order. The final current root-black
store precedes fetching the current output pointer. Output node precedes owner.

B1AF90 captures the pair identity while comparisons read current headers.
Its inlined comparison has the same left length/right length/right data/left
data schedule as the genuine 443D00 provider. The private cursor's node is
written before its owner. The predecessor uses the real raw iterator provider.
Both successful outputs store node, inserted byte, owner; duplicate insertion
keeps the existing node/value and stores inserted byte zero.

B1B0B0 retains captured hint owner/node while the returning validation and
comparison boundaries can change current cells. Predecessor and successor
operations overwrite the original incoming cursor. Successor equality consumes
only B19530's AL; the remaining EAX bits are not a bool. Direct linking returns
the captured outer output identity. Fallback reads returned owner, fetches the
current outer output, stores owner, then reads returned node and stores it.
This ordering is preserved even when output aliases caller-visible data.

B1B2B0 captures the name, performs lower-bound/current-head validation, and
creates a raw temporary only on a miss. It compares self-header identity before
zeroing length/data. After resize it tests current source length and captures
current temporary data into the EBP-equivalent local before branching. The copy
reads current temporary length/source data. Temp value is zeroed; state0 is armed
immediately before hinted insertion. On normal return it captures owner/node
and disarms state before cleanup. Normal cleanup uses **captured data**, but
current length+1 captured before the real pool getter. A replaced current temp
data pointer does not change that normal return identity. A later getter failure
does not undo successful tree insertion or trigger a second cleanup.

Exceptional temporary cleanup instead consumes state0 and reads the CURRENT
header through B19DD0. All 12 normalized instructions of its 29 bytes match
41DD20; only two CALL relative operands differ. The reused raw overload captures
current data then wrapping length+1 before the genuine getter and pool return,
without clearing the header. A cleanup exception during source unwinding
terminates; native double-exception/FH3 behavior is not claimed.

B1B3A0 captures the name and actual registry+4. It captures result owner and
current head before owner validation, then reads current result node. An
existing key keeps its borrowed factory. On a miss the genuine value insertion
finishes before the CURRENT incoming factory word is read and stored. No factory
callback or retain/release is added.

Compiler evidence totals 36 bytes: CBC6F0[8], CBC6F8[10], CBC710[8], CBC718[10].
DF4A20 has maxState1, map DF4A18: state0 -> -1/CBC6F0, no try map, flags1.
Its cleanup targets actual SBO EBP-50 and tail-jumps to 4072D0. The completed
message is armed only after real 408720 returns; existing owning 28h length-error
transport composes 411700/copy/destruction and host C++ throw. No original private
exception ABI is claimed. DF4A4C has maxState1, map DF4A44: state0 -> -1/CBC710,
no try map, flags1; its EBP-18 cleanup tail targets B19DD0. Each dispatcher loads
its descriptor and tail-jumps to BF6B43. Fresh queries show CBC6F8 and CBC718 are
undefined; exact inclusive ends and last five-byte JMPs are recorded for later
primary definitions. No Ghidra mutation was performed.

The required providers are raw B1AD10/B1ACA0/B19530/B1AB40 node leaves;
B19640/B19830/B19890 tree leaves; B19B90/B19D60 cache lookup; 443D00/BF7FBF host
CRT comparison; raw 41DD40 with overlap-compatible BF7680 memmove effect;
419CC0/BD1510 using the same actual pool/publication/manager/gate; and existing
SBO/exception ownership. BF6713 must be a callable genuine returning validation
binding. There is no unknown provider default. Native null/fault behavior,
concurrent mutation and inherited provider failure limitations remain outside
the valid storage/binding domain. Completed allocations and residual credits
remain represented in persistent diagnostics; no rollback is invented.

`./scripts/build.ps1` passed strict MSVC Win32 and both configured worker CTests.
One ignored standalone fixture includes the unchanged source body with three
observed genuine provider boundaries: allocation, iterator decrement/increment.
All six native normal bodies are copied from live/PE-equal bytes; internal calls
remain original copies, external calls are relocated to the genuine providers.
Original overflow/EH paths are not executed. It uses actual registry construction,
node/name allocation and disposal, an isolated genuine string pool, and borrowed
factory words. No application or original executable is launched.

The normal comparison inserts an ordered mix with both repair orientations,
checks case-insensitive duplicate registration preserving value/cardinality,
observes the same incoming hint cells and one aliasing result, checks pair/result
aliasing and preserved result padding, and changes current factory/left/output
cells at the real allocation return boundary. It compares normalized key-named
left/parent/right topology, head relationships, cardinality, keys, factory words
and colors. Separately allocated node/name/head addresses are normalized by key
identity, not compared byte-for-byte; padding/unwritten allocation bytes and
all private stack/register state are excluded. Selected raw output/preimage
and exact identity relationships are asserted separately. A direct 29-byte
cleanup pair verifies unchanged header words. Source-only cases check owning
length-error cleanup and state0 temporary cleanup after a provider throws before
allocation. They are not native exception comparisons.

Probe01 failed compilation because two inline-ASM thunks had multiple instructions
on one line; that source/log is retained. Probe02 compiled and passed after only
the fixture syntax and pre-run relocation offsets were corrected. No failed
runtime comparison was suppressed. The exact command uses /MD /EHsc /std:c++20
/O2 /Gy /W4 /WX /fp:strict and /link /OPT:REF /MANIFEST:EMBED. NDEBUG is not defined;
the fixture rejects it at compile time. No tracked test or production callback
seam was introduced.

Evidence is under ignored `local/output/cc10_resource_registry_insertion/`, with
fresh live bytes, complete listings, proto queries, descriptor data, source
equivalence proof, command/logs and final manifest/identity receipt. The frozen
readiness ZIP SHA-256 is
`4b1f30029557df82e450ca171b8fd5e1d29a847e86ae4d18c4b7a5edf19e889c`.
The report pins full ranges and every direct/compiler transfer. Build/source/
focused composition evidence does not establish drop-in ABI, native EH, later
startup admission, global factory lifetime or game behavior.
