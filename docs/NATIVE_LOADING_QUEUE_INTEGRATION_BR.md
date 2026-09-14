# Loading queue submission and retirement integration (BR)

Addresses: `004FDC00`, `004FE700`, `00501720`, `005048F0`, `00504D20`,
`00506BF0`; correction at `00BE0A30`. EH support: `00C688E0`, `00C688E8`,
`00C68FF0`, `00C6900C`, `00C69080`, `00C69088`.

This batch adds six complete ordinary bodies (722 bytes): work-record
destruction, by-value construction, copy construction, vector append, queue
submission and front retirement. The existing 171-byte FileBlock constructor's
direct copy now permits the native BF7680 backward-overlap behavior. The new
sources compose completed BL/BK/BP/BQ dependencies over actual storage and
publication cells. They do not create alternate ownership domains.

The submission ABI correction matters: 4FE700 consumes three by-value stack
words, not a pointer to a name. The retirement ordering matters: FileBlock and
FileStore removal use the initially selected job; storage destruction selects
the current front again afterward. Capacity and the trailing vacated slot stay
unchanged. Packet details and limits are in
`NATIVE_LOADING_QUEUE_SUBMISSION_BR.md`,
`NATIVE_LOADING_QUEUE_RETIREMENT_BR.md` and `NATIVE_FILEBLOCK_COPY_BR.md`.

Primary validation has passed: strict MSVC Win32 compilation; one actual-service
queue fixture; the existing BQ FileBlock normal and deliberate observer-failure
modes rebuilt in a separate BR directory; the full Win32 build and both CTests.
The fixture's FileStore node retains a concrete memory stream and retirement
returns its object/byte counters to zero. No new repository test cases were added.

The initial three fixture executions link copied, hash-verified BQ libraries
from `fa879ae60afb839ac011dbc11c29c4830ea1cd81` plus the three BR object files.
They do not overwrite the BQ receipts. The report retains their exact libraries,
compiler commands, source, objects, executables and logs.

The PE/live-Ghidra byte audit covers all 301 ordinary instruction starts
(243 new, 58 in the corrected existing body), complete EH support spans and
all 31 direct-transfer report rows. The instruction ownership repair at 506BF0
and three handler definitions are saved; all 13 annotated functions retain
their previous comments. Five anonymous work routines received descriptive
hypothesis names, and correct existing compiler/library names remain intact.
The explicit 4FE700 ABI view reflects the by-value stack storage.

Production `GameFrameHost::update_loading_queue` remains unimplemented. The
509190 updater, 501620 starter, 501510 worker, 5092E0 drain, 509FD0/50ACE0
destruction, actual resource-manager owner and B80720 miss path remain on the
critical path. No placeholder worker or inferred stop/wake/join/CloseHandle
sequence is introduced. The queue fixture never starts a worker. New source
interfaces and source C++ failure boundaries are not native FH3/SEH/CRT ABI or
gameplay proof. Retirement inherits BQ's explicit retained FileBlock outer
failure model. Overall game reconstruction is still incomplete.
