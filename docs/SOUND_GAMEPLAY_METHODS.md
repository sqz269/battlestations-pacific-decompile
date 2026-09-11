# Sound gameplay methods

Eighteen routines complete the handle, audibility, progress, configuration and pause
methods for the D5ABF8 bank channel, D5B510 spatial channel and D5B4C8 spatial event
projections. `SoundChannelRuntime` dispatches slots18/1C/20/3C/40 by profile. Names are
hypotheses, not recovered symbols; these C++ interfaces are not native ABI replacements.

| Address | Behavior | Original ABI | Coverage |
|---|---|---|---|
| 00A8A700 | Spatial channel configuration | ECX self; RET | Complete; tail separate |
| 0093FD00 | Assign16h descriptors | ECX actual12h header; source; RET4; EAX self | Complete |
| 0093FDF0 | Copy48h options | ECX destination; source; RET4; EAX self | Complete |
| 0093FDB0 | Destroy options | ECX self; RET | Complete; tail separate |
| 00A88B90 / 00A79A30 | Event / channel handle | ECX self; RET; EAX borrowed handle | Complete |
| 00A89C60 | Event progress constant | RET; ST0 float | Complete |
| 00A7A6B0 | Channel PCM progress | ECX self; RET; ST0 float | Complete |
| 00A7A710 | Channel audibility | ECX self; RET; ST0 float | Complete |
| 00A89250 | Recursive maximum audibility | ECX object; group; RET4; ST0 float | Complete |
| 00A89D00 | Event audibility under lock | ECX self; RET; ST0 float | Complete |
| 00A7BE90 | Empty configure method | RET | Complete literal no-op |
| 00A891F0 / 00A799A0 | Event / channel pause | ECX self; byte in stack word; RET4 | Complete |
| 00A89460 | Query-lock constructor | ECX self; RET; EAX self | Complete |
| 00A89390 | Query-lock base destructor | ECX self; RET | Complete |
| 00A89A40 | Query-lock singleton getter | No inputs; RET; EAX pointer | Complete |
| 00A89B40 | Query-lock deleting destructor | ECX self; flags; RET4; EAX old pointer | Complete |

## Recovered behavior

00A8A700 copies the sample's actual48h options, including owned16h descriptors. It
submits min/max distances at options0C/10, gets channel mode into an initially unwritten
output, and switches bits100000/200000 from options20 and bits100/200 from options14.
Each branch clears the opposite bit and preserves unrelated bits. Assembly masks override
the older pseudocode's misleading enum names and overlapping locals. Each FMOD call
reloads channel54. Result2B samples FMOD memory statistics; other results are ignored.

Options copy follows native field order and FLD/FSTP conversions, including field2C.
Padding17,23,39..3B remains untouched. Descriptor assignment resizes the destination to0
before reading the source count, reserves at least1, captures each source record before
growth, copies four words, and increments count. Later iterations reload source pointer
and count. Self-assignment clears the array; descriptor name pointers remain borrowed.
Destruction resizes to0 and frees the buffer, leaving dead pointer/capacity values. The
copy constructor has no local unwind cleanup; configuration enables cleanup only after
copy construction succeeds.

Event slot20 literally loads float bits3EAA7EFA from00D5B4C4. It does not query FMOD.
The matching bank slot divides PCM position by sample PCM length: an unsigned numerator
uses the native x87 correction and double spill, but the denominator uses signed FILD
without unsigned correction. The quotient spills to float; there is no clamp or zero guard.
The progress name is inferred from the matching bank slot.

Channel audibility initializes1 and calls FMOD even for null. Event audibility returns0
for null without touching the singleton; otherwise it captures/enters the query lock,
increments tracked depth, reloads event54 and gets its group into a null output. The
recursive helper starts at0 and traverses child groups, then channels, in reverse index
order. An unordered comparison replaces the prior maximum. Channel values initialize0;
one initially unwritten pointer slot is reused across both loops. FMOD errors are ignored.
Recursive results spill to float. Lock release uses the captured section on return/unwind.

The singleton at00F8BBDC uses the existing lifetime domain and tracked Win32 critical
sections. Its getter captures the first manager's lock, rechecks the global under it,
allocates/constructs, publishes, then gets the current manager again for registration.
Its registered deleting destructor destroys the section, clears the global, installs
the base vtable and optionally frees. It does not unregister itself. DEC4A4/DEC4D8/DEC538
establish base cleanup, failed-allocation cleanup and captured-lock release. Callers must
share the domain and dispatch registered cleanup through the reconstructed destructor.

Bank pause stores byte1C for the next update. Event pause immediately calls FMOD for a
nonnull handle and never writes1C; before event creation it has no effect. Handle getters
borrow without retaining. Event/ordinary slot3C is literally RET, not an unresolved stub.

## Validation and limits

Win32 Release and both existing tests passed. No permanent tests were added. The installed
fixture checks descriptor copy, untouched padding and self-assignment. Independent FMOD
getters verify channel min/max and both rolloff branches, handles, initial PCM progress
and both pause behaviors. Event audibility is checked paused and playing: the playing
value0.5 matched independent FMOD traversal. Repeated queries reuse one registration,
depth returns to0, and lifetime shutdown destroys the singleton and clears its global.
The existing bank/event parameter, spatial, completion, retirement and cleanup checks pass.

The no-sound master group exposes channels whose audibility query returns24h and leaves
its initialized0 unchanged. The fixture records that native error behavior. NaN ordering,
high-bit PCM lengths and exceptional allocations are assembly-grounded, not broadly
fixture-tested. Unwritten mode/pointer outputs use optionals and reject use before a value
exists; the concrete C adapter reserves all-ones as its marker, excluded from valid FMOD
modes/pointers. Valid nonnegative, nonoverflowing array storage and initialized scalar
bytes are required. Native allocator/SEH identity remains open.

Eight absent functions were defined and eighteen prototypes verified. Callsite fallthrough
was decoded without changing callee no-return flags. Stored bodies still end at00A8A806
and0093FDC4; separately verified tails end at00A8A815 and0093FDC9. The report distinguishes
these tails from stored-body calls. No native ABI, audible-output or gameplay claim is made.

## Follow-up packets

- Compose application sound initialization, shared lifetime/globals, clock/CRT, VFS sound
  directories and voice ownership. `game_hosts.cpp` still marks phase5 initialization
  00A88770 unimplemented at this packet's source revision.
- Bind gameplay/voice callers to the full factory and profile methods. Runtime dispatch
  support does not establish that the game startup owns or exercises these objects.
- Extend the recorded short Ghidra bodies only through a supported locked operation.
