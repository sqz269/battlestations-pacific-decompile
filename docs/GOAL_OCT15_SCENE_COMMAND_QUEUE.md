# Scene command queue, 00469610

The new ordinary C++ interface follows all 115 bytes / 34 instructions of
00469610 and the 11-byte / 5-instruction cleanup at 00C61B10. Names are role
hypotheses; existing Ghidra names and comments are preserved. This supersedes
the old host/projection description only for this explicit actual-storage API.

At original entry ESP=P, the record arguments occupy P+4..P+0F. The saved
allocation cell at P-10h first receives ECX through PUSH ECX, before allocation.
0046962B calls 00BF681B with size14h, then stores the returned pointer in that
same cell and arms state0. Incoming argument words have not yet been read.
On the nonnull branch, target is captured before command; the outgoing target
word is written before the fresh owner read. Command and owner are then pushed.
00469654 calls the concrete raw constructor 004690D0 using these actual outgoing
argument cells, not an eagerly copied host record.

The constructor parameter area starts at P-20h; its saved-record cell is P-34h.
The outgoing list argument at P-18h intentionally reuses the constructor target
word. After construction, that word receives the constructor's actual returned
record value. The null allocation branch supplies zero. The wrapper disarms
before 0046966C calls the concrete raw list insertion 00468350 at database+14Ch.
An insertion exception has no record rollback, matching the native state.
Neither the record nor argument strings are copied again by the wrapper.

Handler00C61B1B loads FuncInfo00D89180, whose maxState1 unwind map at00D89178
contains {-1,00C61B10}. The action reads CURRENT [EBP-10h] and 00C61B14 calls
00BF65AC. The Source guard similarly reloads the explicit saved-allocation word
if ordinary C++ construction throws. It is unarmed before allocation and
disarmed before insertion; it supplies no Original FH3/SEH private-frame entry.

The allocator and free are the existing genuine current CRT services in
singleton_lifetime.cpp: malloc/_callnewh/bad_alloc and free. Original heap,
encoded handler state, exception object identity, private argument/register
homes, asynchronous fault transport and binary ABI are unproved. Metadata
bindings are stable and disjoint from reached raw accesses; the explicit cell
ranges follow the original frame, including their intentional reuse. Reached
payload aliases remain admitted at the concrete providers' contracts. No new
callback, null recovery, list guard, exception policy or synthetic record is
introduced. The semantic result is void.

Evidence and validation state are in
reports/goal_oct15_scene_command_queue.json. The new entry is not executed by
the existing math differential fixtures, and no game validation is claimed.
