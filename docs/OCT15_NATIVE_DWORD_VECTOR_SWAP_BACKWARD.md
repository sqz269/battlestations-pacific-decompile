# Native DWORD-vector header backward swap

Addresses: `006981C0`, `00698380`
Coverage: complete actual normal bodies; new MSVC Win32 Source interfaces.

| Routine | Coverage | Original physical ABI | Source |
| --- | --- | --- | --- |
| `006981C0` | complete,67B/26starts | cdecl six producer words; three consumed; plainRET; advanced EAX | `swap_native_dword_vector_headers_backward_006981c0` |
| `00698380` | complete,64B/29starts | cdecl first/last/output; plainRET; independent EAX | `swap_native_dword_vector_headers_backward_dispatch_00698380` |

The core captures last and output before first/equality, including empty.
Each iteration loads source[last-0Ch] then output[out-0Ch] before decrementing
both raw cursors10h. It compares decremented source with captured first, stores
destination begin then source begin, and separately loads/stores the+8 pair
and then+C pair in that source/destination order. Opaque+0 is not directly
accessed. These are staged ownership-word swaps across public aliases, with no
constructor, assignment, allocation, destructor, snapshot or rollback.
Return is the actual backward output.

The dispatcher captures its three inputs and calls the actual new core with SIX
physical words: first,last,output,duplicate outputbits,duplicate outputbits,tag.
The native producer removes18h; the complete core consumes only the first three.
Native incomingECX supplies a private tag whose low byte is cleared; high24bits
are dead. Source supplies canonical private zero. This qualifies a Source
private choice, not an owner/context or native ambient-register/frame identity.
Aliases into original native argument/local stack are outside the new Source
interface; dead-slot nonconsumption refers to direct input-slot reads.
Core EAX is discarded. Return is independently
`output-((SAR4(wrapped last-first))<<4)` with DWORD wrap and actual x86SAR,
without pointer subtraction or a count bridge.

Original library name `STL_SwapBackward_WordVector16_006981c0` stays intact.
The dispatcher is currently `FUN_00698380`. New descriptive names are hypotheses;
the historical WordVector16 identity does not introduce a public container type.

These transfers are proven against complete owned bytes and a bounded unowned
producer. The latter currently belongs to00698400, rather than the old snapshot
catch label. It pushes first,last,saved current end, then ignores dispatcher EAX
during the following accepted fill-call setup.

```text
006983A7 CALL 006981C0
00698639 CALL 00698380
00698649 CALL 00698010
```

Owned coverage is131B/55current starts and oneCALL. Separate caller window
`00698633..00698653` is33B/14starts and twoCALLs, not whole insertion coverage.
Every selected byte equals the PE/current program. Typed complete ranges,
exact current starts and default/effective targets are retained in ignored
machine evidence. Old graph extra6982B0/BF6713 edges are not actual dispatcher
calls; no Ghidra repair/mutation occurred.

FIFO claim succeeded before mutation. Accepted Main19e564ef4 was synced,
one Source line appended, then the registration lease released before build.
Warm scripts/build.ps1 passed; all three existing suites passed in8.90s.
Report and Markdown each passed3 transfers with zero failures/unchecked claims.
Focused emitted Source COFF inspection passed10 checks: core313B/110instructions,
dispatcher57B/27instructions, six pushed words/ADD ESP18h, no reads of deadslots,
exact staged public accesses and independent SAR4/SHL4 return. Primary precommit
review passed11 checks with no findings. Source/header hashes stayed frozen.
No new tests or native/game execution were added. Existing suites do not directly
prove this new routine's aliases, native ABI, faults or game behavior.

Equality must terminate and all reached accesses must be valid. No endpoint/null
validation or selfguard is added. Raw x86 DWORD loads/stores retain unaligned
public alias order. Source private argument/local storage, ambient registers/
flags, original faults/SEH/FH3/CRT/heap/mangling/native binaryABI and gameplay are
unproved. No public noexcept promise is added. Noinline only preserves an
auditable six-word child boundary.

Existing actual narrower storage, Hook APIs, reports and fixtures remain
separately qualified. This packet does not promote either EH core, general
insertion, placement or later loader work.
