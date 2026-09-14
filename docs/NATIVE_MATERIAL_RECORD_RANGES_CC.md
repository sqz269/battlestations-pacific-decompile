# Native record construction and destruction ranges

These four complete functions provide the 300-byte record range operations
needed by material diagnostics vector insertion. They compose the actual raw
copy constructor `00B13070` and the existing destructor `00B10740`. They do not
allocate a replacement container, validate native iterator ranges, or implement
the parent insertion algorithm `00B145E0`.

| Native body | Bytes | Original interface |
| --- | ---: | --- |
| `00B13B00..00B13B94` | 149 | ECX output, EDX unsigned count; source and three ignored stack words; RET10h |
| `00B13920..00B139BE` | 159 | ECX first, EDX last; output and three ignored stack words; EAX final output; RET10h |
| `00B14360..00B14398` | 57 | ECX ignored owner/tag; output/count/source stack arguments; EAX output+300*count; RET0Ch |
| `00B143A0..00B143C5` | 38 | stdcall first/last; RET8 |

The fill loop rereads its original source-argument slot before each non-null
output construction. After success it decrements the unsigned count, advances
output by 300 with DWORD wrap, then publishes the completed output position.
The copy loop captures input first/last and initial output, constructs forward
until first equals last, and advances output before input. It writes the new
output position to its own actual output-argument slot on every iteration.
Both loops skip construction when the current output is zero while retaining
their native position/count updates. They add no negative-count or ordering
checks.

On a C++ construction exception, both native routines catch all, destroy the
completed output prefix in ascending order, and rethrow with
`00BF6885(nullptr,nullptr)`. The copy catch captures its cleanup end from the
current output-argument slot; the fill catch uses the completed position.
The failed current record is not included in that prefix. A destructor failure
escapes this catch body; the source does not retry or add a terminate wrapper.

The source's naked entry wrappers retain the actual first stack-argument slot
for these reads and writes. EDX remains native count/last, so the new raw string
context occupies the second stack slot; the three ignored iterator/tag words
are omitted, producing RET8 for both loop interfaces. The fill-tail wrapper
uses new stdcall output/count/source/context and returns the wrapped arithmetic
result after the real fill call. The destructor uses a third stdcall context
argument and stops on equality, advancing only after each destructor returns.

The saved EH tables explain cleanup that the pseudocode omitted. Copy uses
handler `00CBC301`, FuncInfo `00DF4444`, try map `00DF4418`, unwind map
`00DF442C`, and catch `00B13980`. Fill uses `00CBC341`, `00DF44D0`,
`00DF44A4`, `00DF44B8`, and catch `00B13B5E`. Each try covers states 0..1,
has catch-high state 2, and one catch-all handler with adjective `40h`. Both
FuncInfo records have three unwind states and EH flags 1. State 1 goes to 0
through placement-failure funclet `00CBC2F0` or `00CBC330`; states 0 and 2
go to -1 with no action. Each placement funclet passes the saved two placement
arguments to `00401130`, whose complete installed/live body is one RET byte.
Omitting that verified no-op in the source adds no replacement allocator or
unresolved-call stub. No separate funclet or RET-leaf credit is claimed.

The report pins native/live spans, source and provider hashes, original call
sites, saved Ghidra signatures and comments, generated-code review, and the
exact committed MSVC Win32 build. Names are descriptive hypotheses. The source
interfaces omit private native frame/tag storage and do not claim native FH3
identity, hardware-SEH behavior, binary substitution, a new native execution
fixture, completed vector insertion/diagnostics, rendering, or gameplay.
