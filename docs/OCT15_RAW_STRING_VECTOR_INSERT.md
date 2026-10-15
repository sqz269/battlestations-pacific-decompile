# Actual pooled-string vector general count insertion

Addresses: `0044F410`, `0044F5C8`, `0044F5E5`, `0044F653`, `0044F6B5`.

The complete normal body, both catches and temporary ownership are reconstructed
in `src/native_string_vector_insert.cpp`, over accepted actual raw-pool services.
The public function is `insert_native_string_header_vector_count_0044f410`.
The established `STL_CheckedNativeStringVector_InsertCount_0044f410` library name
is retained. Earlier append-only `native_checked_string_storage` source, its
storage boundary, native fixture results and provenance remain unchanged.
Those earlier fixtures do not validate this new general-insertion interface.

| Address | Native meaning and ABI | Coverage |
| --- | --- | --- |
| `0044F410` | fastcall: `ECX` actual10h header, `EDX` unused; stack unused owner/position/unsigned count/value header; `RET10h`; no semantic result | complete actual Source; adds explicit `NativeStringRawPoolContext` |
| `0044F5C8` | growth catch; original `EBP` supplies fresh backing, completed parent prefix, receiver | complete Source catch scope; no standalone native-frame API |
| `0044F5E5` | zero/zero rethrow within growth catch | complete Source rethrow fragment; no function starts here |
| `0044F653` | short-tail fill catch; parent receiver/count/position and CURRENT vector end | complete Source catch scope; no standalone native-frame API |
| `0044F6B5` | normal temporary cleanup and parent epilogue | complete semantic cleanup fragment inside `0044F410`; not a callee or independent function |

Header `+0` is opaque and unchanged; `+4/+8/+C` are begin/end/capacity end.
The constructor and Buy producers in preceding packets establish those offsets;
this body publishes capacity/end/begin at `0044F5BA/5BD/5C0`. Actual elements
are the canonical eight-byte pooled-string headers. The unused owner word is
never read. No owner/position/bounds/alignment/null/capacity-consistency check,
host iterator, parallel vector, generic allocator or callback is introduced.

An eight-byte temporary copies the value before vector reads and count checks.
The inline native schedule and accepted actual `0044BCB0` Source agree: compare
header identity before zero stores; actual `0041DD40` resize preserving; reread
CURRENT value length, temporary length, value data and temporary data for copy.
The accepted constructor's extra null-destination branch cannot be taken by the
private temporary. Its placement-delete unwind is a no-op. Temporary copy
failure therefore receives no synthesized string cleanup. The Source uses a
canonical `NativeString`, which has no implicit storage destructor, and accepted
raw-pool copy construction rather than its deleted C++ copy constructor.

The vector begin is captured after the temporary copy, before state0 is armed.
Capacity is then derived from that captured begin and CURRENT capacity end;
null begin gives zero without a capacity-end read. This happens even for count
zero. Zero count still constructs and destroys the temporary. For nonzero count,
each native size calculation rereads CURRENT end against the same captured
begin, using DWORD wrapping subtraction and arithmetic SAR3. The unsigned
`1FFFFFFFh-size < count` check uses the accepted owning length-error payload.
The capacity comparison uses another size read. Growth selects zero if
`1FFFFFFFh-(capacity>>1) < capacity`; otherwise capacity+capacity/2. It compares
that candidate against another size+count and rereads size again if selecting
size+count. Direct accepted allocation receives the selected count and EDX0.

After allocation, prefix begin is CURRENT header begin; position stays captured.
The new backing is not published. The growth try first copies prefix through
accepted actual uninitialized copy, then fills count copies of the temporary,
then copies the suffix from position to CURRENT old end. Parent completed end
starts at the new allocation, advances after prefix success, and advances after
fill success only after the old-end read. The final copy result is ignored.
Child copy/fill catches clean only their own completed prefixes, excluding the
failed element. Growth failure destroys the parent's completed prefix ascending
through actual DestroyDispatch, frees captured new backing, then rethrows.

After suffix success state0 resumes before old cleanup. The native body captures
CURRENT old begin, derives old size from CURRENT end and captures count+old size.
On nonnull old begin it destroys that captured begin through a fresh CURRENT
end, then frees CURRENT begin. It publishes new capacity end, new end, new begin
in that order, using captured new backing/capacity/size. Old destruction or free
failure does not enter the new-backing catch; completed new backing can remain
unpublished and unfreed. No rollback or broader allocation guard is added.

With spare capacity, old end is captured and tail count is SAR32(oldend-position,3).
Both general-insertion branches are complete:

- When tail<count, first uninitialized-copy the tail to position+count*8. This
  precedes the fill catch. Capture CURRENT end, derive count-SAR32(end-position,3),
  and arm state3 only before uninitialized fill at that captured end. On success,
  add count*8 to CURRENT header end, reload end and subtract count*8 to form the
  later assignment limit. State0 resumes before ascending assignment from
  position to that captured limit. On fill failure, the native catch destroys
  position+count*8 through CURRENT header end+count*8, then rethrows. No saved
  old-end substitute is used for this cleanup.
- When tail>=count, copy the last count headers to captured old end, publish the
  returned completed end immediately, backward-assign position..oldend-count*8
  into captured old end, then fill position..position+count*8. No local rollback
  catch surrounds these assignments. A later exception leaves the published end
  and partial assignments, then releases the private temporary.

The Source outer catch models temporary ownership after state0; the normal
temporary destruction is outside it, matching state -1 before normal pool
return. If any cleanup throws in the Source C++ domain, subsequent cleanup in
that scope stops and the newer exception escapes. This does not establish the
original FH3/SEH/double-exception personality or fault behavior.

The native helper stacks were checked through cleanup instructions. Both direct
uninitialized copies have ECX first, EDX last and four stack words consumed by
`RET10h`: destination, owner, count-as-unused-tag and another unused tag. The
first final tag has an uninitialized high24-bit private preimage with low byte0;
the suffix tag has original-position bits with low byte0. Accepted Source hides
these unconsumed private words rather than reading C++ indeterminate storage.
The fill and ordinary uninitialized-copy dispatchers have owner ECX plus three
stack words, `RET0Ch`. Backward and assigned-fill dispatchers are cdecl3ptr;
`0044F6B2 ADD ESP,18h` consumes both three-word argument blocks. The short assigned fill
has `0044F64E ADD ESP,0Ch`. DestroyDispatch consumes its two stack words with
`RET8`; raw range destruction has two unused owner/count words, also `RET8`.

FuncInfo `00D87404` was read directly: magic19930522h, maxState5, unwind map
`00D87450`, two try records at `00D87428`. State0 is `{-1,00C60300}`; all four
remaining unwind entries are `{0,0}`. The two try records are
`{1,1,2,1,00D873F4}` and `{3,3,4,1,00D873E4}`. Their handler records are
`{40h,0,0,0044F5C8}` and `{40h,0,0,0044F653}`. Entry handler `00C60308`
loads this FuncInfo then jumps to the native FH3 service. The read-only unwind
action `00C60300` is only `LEA ECX,[EBP-28h]; JMP0041DD20`; Source reuses that
already accepted actual temporary destructor. It is not a newly reconstructed
standalone leaf or a passed ordinary CALL row.

All148 bytes of the handler arrays, FuncInfo, try map and unwind map match
between disk and the four live byte queries. Their `[00D873E4,00D87478)` SHA256
is `232e5c22044733b6b143fc8d5d16f0fce15b425a2df8a6b7776ddfefc6e637d1`.
The JSON report carries their raw hex and the18-byte unwind-action/entry-handler
decode; `local/oct15_raw_string_insert_eh_bytes.json` is the ignored receipt.

Historical cached metadata listed a suspect callee at `0044F6B5`, and earlier
AT evidence qualified the rethrow as outside its saved catch body. Current typed
metadata at modification109 instead gives main ranges
`0044F410..0044F5C7`, `0044F5EA..0044F652`, `0044F679..0044F6EB`; the growth
catch is `0044F5C8..0044F5E9`, and the short catch is `0044F653..0044F678`.
No function starts at normal-cleanup label `0044F6B5` or rethrow `0044F5E5`.
The rethrow has CALL_RETURN, a terminal call to the actual no-return throw
service and no fallthrough. No speculative flow clearing or body repair is
justified. The three owned exports were force-refreshed; no Ghidra mutation,
script enabling, annotation or save was performed by this worker. The legacy
flow-properties script route returned scripts-disabled; typed-flow supplied
the exact read-only metadata. The read-only repair reporter found zero gaps.

The original disk span `[0044F410,0044F6EC)` is732 bytes, SHA256
`9faffefbc39b8eef20caeda65d3950e1966d9a8660ced3aaefa9f0c70b4136de`.
All281 decoded instruction starts match the refreshed assembly; both catches
and normal cleanup were also inspected directly from disk. Ignored receipts:
`local/oct15_raw_string_insert_disk.json`,
`local/oct15_raw_string_insert_typed_flow.json`, and
`local/oct15_raw_string_insert_flow_properties.json`. The two complete direct
caller bodies were inspected: InsertOne supplies count1 and an arbitrary
position/value, while Resize supplies its computed growth count and captured
end with an actual by-value fill header. The current body ownership supports
the following24 ordinary CALL claims, including the now-contained rethrow:

```text
0044F450 CALL 0041DD40
0044F465 CALL 00BF7680
0044F4AB CALL 00431F70
0044F51B CALL 0042BF40
0044F542 CALL 0044D0C0
0044F555 CALL 0044DB60
0044F570 CALL 0044D0C0
0044F5A0 CALL 00432050
0044F5A9 CALL 00BF65AC
0044F5D3 CALL 00432360
0044F5D9 CALL 00BF65AC
0044F5E5 CALL 00BF6885
0044F60C CALL 0044F170
0044F62C CALL 0044DB60
0044F649 CALL 0044C770
0044F66B CALL 00432360
0044F674 CALL 00BF6885
0044F68E CALL 0044F170
0044F69C CALL 0044D850
0044F6AD CALL 0044C770
0044F6CD CALL 00419CC0
0044F6D4 CALL 00BD1510
004501D0 CALL 0044F410
0049E0B3 CALL 0044F410
```

All allocation, actual string copy/resize/destruction, uninitialized copy/fill,
backward assignment and assigned fill dependencies are accepted at main
`c1615ecc7`. The MSVC Win32 build and all three existing suites passed. JSON and
Markdown verification each passed24 ordinary CALL claims, with no unchecked
claims or warnings. Ledger/index and diff checks passed: only owned addresses
changed, earlier name evidence was appended and all prior reconstruction records
remain unchanged. Neither the rethrow nor normal-cleanup label received a new
function-name entry. No new routine tests are added.
Existing math native differential seeds do not cover this function. Complete
Source coverage has the inherited valid taken backing, actual pool/manager and
current-CRT overlap/zero-byte-copy qualifications. Extra context, by-value
parameters and compiler-private storage do not reproduce original argument-slot
aliases, CRT exception identity, binary ABI, FH3/SEH or fault addresses. No new
native differential, production binding or gameplay validation is claimed.
