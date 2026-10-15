# Scene integer-set decrement

Addresses: 004679F0, 00468210, 004688F0.

| Native address | Source API | Executed span | Coverage |
|---|---|---|---|
| 004679F0 | decrement_native_scene_integer_set_iterator_004679f0 | 004679F0..00467A78, 137 bytes | complete |
| 00468210 | decrement_native_scene_integer_set_iterator_00468210 | 00468210..0046821B, 12 bytes | complete |
| 004688F0 | decrement_native_scene_integer_set_iterator_004688f0 | 004688F0..004688FB, 12 bytes | complete |

The core accepts actual eight-byte iterator backing through ECX: owner DWORD
at +0 and live node DWORD at +4. It has plain returns and two tail transfers to
the invalid-parameter entry. Incoming EDX is unused; EAX has no defined result.
Both wrappers capture the original ECX pointer, invoke the decrement and return
that same address in EAX with plain RET. Correct existing STL library names are
preserved; the Source names describe the observed body and layout.

All raw reads and ordered stores are retained. A zero owner first invokes the
concrete current-CRT provider, and node is read only after that handler returns.
A sentinel node reads its right link, publishes it to iterator.node, then tests
that captured right node's nil byte and reports if still nil. Otherwise a real
left subtree descends current right links until the next node is nil and then
publishes the last real node. With nil left, it ascends parents: each iteration
tests parent.nil, reads CURRENT iterator.node and current parent.left, publishes
the parent when equal, then reloads that parent's parent after publication.
The final validation tests CURRENT iterator.node.nil; a failing tail reports
and returns without publishing the captured parent. Success publishes parent.
There is no copied owner, normalization, cycle check or positive-only domain.

The actual node producer 004681B0 allocates 14h bytes, writes links at0/4/8,
the key DWORD at0Ch, color10h and nil11h. The full core assembly matches the
accepted record-map decrement schedule, using nil11h throughout instead of
nil19h. The separate integer-set Source does not call that record-map body.
The exact original137B includes both executed LEA ESP,[ESP] no-ops; the two
wrappers contribute24B. No interfunction alignment is counted.

All live xrefs were examined. The insertion driver 0046AC50 passes its actual
local iterator at ESP+10h, initially owner=EDI and node=selected ESI. At its
return it reloads CURRENT node from ESP+14h, then its current key DWORD at0Ch;
it ignores child EAX. Each wrapper calls the core with unchanged incoming ECX;
neither wrapper currently has a live incoming xref. Exact call/transfer rows,
containing functions, original bytes and ABI evidence are in the JSON report.

The only external dependency is accepted
invoke_native_invalid_parameter_00bf6713, whose actual selected SDK
_invalid_parameter_noinfo policy permits a returning handler. The core has one
CALL and two tail JMP sites to that native entry. Source preserves the handler
continuation and raw backing effects; it does not claim native encoded-global
handler selection, debugger/Watson delivery or FH3/private-frame equivalence.

Guarded live batches verify C:/Users/sqz269/bsp.gpr and program
/battlestationspacific.exe, x86 image base00400000. Annotation takes the shared
write lock, records old values, preserves existing names/comments, saves the
project and refreshes owned exports. The Source draft has full-body/static/call
and installed-PE byte evidence; Win32 build and existing checks are pending.
No new routine test is added. Compilation and existing checks do not establish
direct native differential execution, original binary ABI, application adoption
or game behavior. Valid backing remains required for every taken access.

Final worker validation: `scripts/build.ps1` Release Win32 passed at Source
draft `fb49c2d46`, and both enabled existing checks (`reconstructed_math`,
`tool_tests`) passed in 8.79 seconds. The native differential suite is
not enabled in this worker tree; no new entry was directly native-tested.
The log is `local/goal_oct15_scene_integer_set_iterator_build.log`; its hash is in the report.
The inherited duplicate `spawn_request_id_matches` LNK4006 warning remains.
JSON transfer claims, documentation address claims and staged whitespace
checks pass. Listing-gap queries do not expose typed flow/fallthrough
or no-return properties, and no live flow/prototype repair is claimed.
