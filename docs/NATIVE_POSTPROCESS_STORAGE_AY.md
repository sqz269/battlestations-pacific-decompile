# Native postprocessor temporary storage

Addresses: 00b78a90, 00b78d70, 00b77790, 00b76830, 00b76c40, 00b76890, 00b8a2d0, 00b8a370, 00b79bc0

This batch reconstructs the temporary node-to-track arrays and track access
used by B79BC0. Five new complete source bodies cover 769 bytes; three
verified canonical array reuses cover 279 bytes. These are dependencies of
the full postprocessor, which remains unimplemented. Descriptive names are
hypotheses, not recovered symbols.

## Actual storage and ordering

The producer at B79F22..B79F85 initializes a 0Ch stack header, resizes it to
the current instance node count, and writes each node into a 10h record.
Each record contains a borrowed node at 0 and an owning pointer-buffer header
at 4/8/C. Its resource-item pointers are borrowed. The header is reused from
`NativeRenderPointerArrayStorage`; no second container or count is introduced.

| Entry | Complete behavior |
|---|---|
| B78A90, 395 bytes | Minimum-one outer reserve; deep-copy each current record's nested buffer; destroy old nested buffers ascending; free current outer backing; publish replacement then capacity. |
| B78D70, 251 bytes | Grow through reserve; clear only new nested headers, leaving node words unwritten; decrement count before descending destruction; reread outer backing/count after callbacks. |
| B77790, 106 bytes | Clear destination count through resize0; reserve current source count even if zero; capture each source cell address before possible destination growth; copy its current value afterward; return destination. Ordinary self-copy empties it. |
| B76830, 95 bytes | Same pointer reserve as canonical B1C500 after call relocation normalization. |
| B76C40, 80 bytes | Same pointer resize as canonical B1C770, calling the verified reserve specialization. |
| B76890, 104 bytes | Same eight-byte pair reserve as canonical B40C80/B40AC0; reuse its actual 0Ch header and algorithm. |
| B8A2D0, 4 bytes | Read the current signed count at item+C; ECX input, EAX result, RET. |
| B8A370, 13 bytes | Read current backing at item+8, then the raw DWORD-scaled stacked index; EAX pointer, RET4. No bounds handler. |

The three new sequence routines retain native ECX and stacked arguments using
unused EDX in their source fastcall interfaces. Reserve/resize return no
meaningful value; copy returns the same header. Arithmetic uses native DWORD
offsets. Valid readable/writable storage remains a caller requirement.

Nested cleanup uses canonical resize0 and free. Negative nested capacity
therefore takes the minimum-one reserve path; afterward data/capacity remain
stale. No node or item is retained or released. A callback may replace the
current outer backing/count, while a nested source header already captured by
the copy remains the source for that call. The fixture covers both forms.

The pair specialization is reused through the existing material-state array
implementation. Its established source boundary rejects corrupt extents and
allocation overflow; original out-of-bounds behavior is not claimed. No
render-owner implementation was copied or modified.

## Unwind evidence and saved analysis

B78A90's handler CC1CF7 selects FuncInfo DFAD18. Its single unwind-map entry
at DFAD10 transitions state0 to -1 and invokes CC1CE0. That funclet computes
the current placement address and calls 401130, whose complete body is RET.
It does not destroy already-copied nested arrays or free the new outer block.
The source therefore introduces no speculative rollback. A failed nested
allocation leaves the old outer header and earlier new allocations intact.
The second source-failure case verifies destination count was cleared before
a pointer-copy allocation failed. These are source exception-state checks;
native FH3 integration is not established.

Supported flow repair removed seven erroneous `_free` call-site overrides
and restored 109 bytes across B78A90/B78D70/B76830/B76890. All seven call gaps
are repaired in saved Ghidra. An unreachable three-byte alignment gap after
B78AEB is left alone. No global no-return setting, function recreation or
script-guard bypass was used.

## Verification and scope

Strict MSVC Win32 and both existing CTests pass. Eleven complete reference
bodies, totaling 1327 bytes, match live Ghidra and the untouched installed PE.
The three reuses have verified normalized instruction identity and direct
targets; both compiled access leaves are literally byte-identical. All 23
direct-call rows pass mechanical attribution. Unwind metadata, the handler
and placement-delete bytes are retained separately.

One ignored probe provides five paired storage cases: deep reserve and
descending shrink with untouched node words, self-copy and pair reserve;
source cell capture across allocation mutation; outer backing replacement
during nested copy; backing/count replacement during shrink; and negative
nested-capacity cleanup. It also checks the raw signed count and wrapped
index leaf. It compares allocation sizes, live buffers, free preimages/order,
all headers and EAX identities. Real CRT allocation/free is used on both
sides. Two separate source failures preserve their partial states. Repeated
processes produce identical result images.

Full B79BC0 still needs its camera target ownership sequence and complete
phase orchestration over actual virtual services. AU registry, AV skin
finalization, AW animator lifetime/finalizers, AX angle extraction, AT range
helpers and AS bone caller remain available. Full B891A0 graph creation and
admission, native exception integration and gameplay remain required. No
worker or repository test suite was added.
