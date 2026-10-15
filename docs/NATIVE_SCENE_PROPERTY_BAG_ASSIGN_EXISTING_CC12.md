# Whole ordinary AssignExistingFrom (CC12)

The complete Original body is `008F23E0..008F253E`: 351 bytes and 118
instructions. The new name is a hypothesis. Its ordinary C++ interface borrows
the existing concrete insertion context and actual raw bag storage.

The loop allocates a real twelve-byte heap iterator even for an empty source.
For each key it constructs an actual eight-byte pooled query, performs actual
map lookup, captures a hit's mapped word, and retires the query. It then reads
the destination tag before any branch-specific source record load. Type6
recurses on the existing children; other hits call complete record assignment.
A miss reads no source mapped record and performs no clone or insertion.

The context retains the same canonical key/property pools and distinct
property/query empty roles. No new fallback cell, semantic merge host, model,
provider wrapper or callback is introduced. The old miss-path dead stack-word
read is not expressed as an indeterminate C++ pointer read: only a hit uses a
captured mapped pointer. No Original raw stack/register equivalence is claimed.

Inputs must satisfy the header's finite disjoint owning-graph, matching-hit-tag,
valid Type6 child, current ASCII/C-locale and complete assignment contracts,
including positive byte spans, an existing array owning flag and accepted FP
environment. This is no self-assignment safety shortcut. Counts, heads, keys,
nodes, bag counter/owner and record metadata retain their destination roles;
only admitted payloads and existing descendants are assigned.

Normal retirement finishes before pool teardown. The new void/context ABI does
not supply Original ECX/RET4, residual flags/registers, FS/SEH/EH/OOM, private
CRT, class dispatch, application retention or game qualification. Strict
compilation, complete emitted-body review, the required existing repository
build and any later runtime admission are recorded separately in the report.

The first strict MSVC Win32 compile passed. All eight emitted functions were
decoded completely: 503 code bytes and 174 instructions, including the complete
432-byte/138-instruction entry. Raw dumpbin bytes, all relocations and CFG edges
matched; the destination-tag comparison precedes both conditional source-load
paths. No FP instruction is emitted in this TU. The original required repository
build finished naturally with 2/2 existing tests passing. This TU is not yet
registered by Root, and AssignExistingFrom runtime admission remains unclaimed.
