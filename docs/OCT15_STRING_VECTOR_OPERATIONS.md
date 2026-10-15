# Native string range wrappers and vector-assignment readiness

Owned addresses: `00B6C670`, `00432360`, `00506C80`.

This packet completes two actual-storage wrappers and maps the remaining native
vector-assignment dependencies. **`00506C80` remains unimplemented and is not
recorded as reconstructed.** No placeholder assignment or generic container
bridge is supplied.

| Address/body | Source | Coverage/state | Original ABI |
| --- | --- | --- | --- |
| `00B6C670..00B6C6AC` | `equal_native_string_range_00b6c670` | complete | cdecl, three stacked pointers, plain RET, AL Boolean only |
| `00432360..00432375` | `destroy_native_string_range_dispatch_00432360` | complete | ECX unused owner, stacked first/last, RET8, no semantic result |
| `00506C80..00506DC1` | none | complete normal-body analysis; source-blocked | ECX destination owner, stacked source owner, RET4, EAX destination |

The existing names `STL_NativeStringRange_Equal_00b6c670`,
`STL_NativeStringRange_DestroyDispatch_00432360` and `BSP_StringVector_Assign`
are retained. Source/header: `native_string_vector_operations.cpp/.hpp` under
`src/` and `include/bsp/`, registered in `cmake/startup.cmake` for `bsp_core`.

## Completed wrappers

`00B6C670` captures last, supplies a private two-DWORD output to the existing
complete `00B6C480` mismatch source, and compares its first result to captured
last. The physical child words are output, first, last, second-first, unused
last, unused dispatch word. The native low dispatch byte is cleared while its
high bytes come from indeterminate private storage; source supplies zero.
Six pushes and `ADD ESP,18h` at `00B6C69C` establish the physical count. Only
the three incoming pointers and mismatch result affect its Boolean result.
It performs no range validation. No native callers are recorded for this wrapper.

`00432360` captures stacked last twice, puts last in EDX, pushes last and the
incoming owner as the child's two unused stack words, and puts stacked first
in ECX. The existing complete raw-header `00432050` source supplies the actual
`00419CC0`/`00BD1510` string-pool return path through an explicit
`NativeStringRawPoolContext`. The new source interface carries owner, first,
last and that borrowed context; it does not claim the original entry ABI.
The child iterates ascending eight-byte headers, preserving header fields and
reading data before length+1 for each nonnull release. No backing-buffer free,
vector-field store, fallback storage, callback model or ownership transfer is
introduced. Getter exceptions propagate without wrapper cleanup or rollback.

All three known destruction-dispatch caller setups were read: assignment and
the two separate catches at `0044F5C8`/`0044F653`. Both catches pass actual
first/last ranges before their own rethrow/free logic; that outer logic does
not belong to this forwarding leaf.

```text
00B6C697 CALL 00B6C480
0043236E CALL 00432050
0044F5D3 CALL 00432360
0044F66B CALL 00432360
```

## Assignment layout and normal paths

The native owner is 10h bytes: opaque +0, begin +4, live end +8, capacity end
+0Ch. Producer `00432090` clears the three pointers before allocating count*8
bytes and publishing begin/end/capacity. The native string producer `0041E870`
and canonical `NativeString` establish length+0/data+4 in each eight-byte
element. This differs from `NativeStringVectorStorage`'s 0Ch data/count/capacity
container. No new overlay or host vector is declared.

Pointer differences use wrapping DWORD subtraction followed by arithmetic
`SAR 3`; size/capacity comparisons in assignment use unsigned `JA`. Owner+0
is untouched. The complete assembly has no local EH registration or rollback.

| Path | Decision and exact operation order |
| --- | --- |
| Self (`00506C88`) | Equal owner addresses return destination without any source-field read. |
| Empty source (`00506C90..00506CB0`) | Null source begin or SAR3 span zero invokes `00498FF0` on destination; backing capacity is retained. |
| Reuse/shrink (`00506CB3..00506D17`) | Source count <= current destination size: assign captured source range to captured destination begin; destroy from returned copy end to the then-current destination end; reread source begin/end and destination begin before publishing the new end. |
| Reuse/grow (`00506D1A..00506D65`) | Source count exceeds size but fits capacity: recompute current destination size, capture a source split using initial source begin, assign prefix, then uninitialized-copy split..CURRENT source end into CURRENT destination end. Publish child result as end only after success. |
| Replacement (`00506D68..00506DC1`) | If old begin nonnull, destroy captured old range then free CURRENT destination begin. Recompute CURRENT source count, invoke Buy (which clears fields first); if AL nonzero, reload current destination begin and source endpoints, construct the new range, then publish returned end. Return destination even if Buy returns false. |

Assignment's normal direct calls and their stack-cleanup evidence:

```text
00506CA5 CALL 00498FF0
00506CCD CALL 0048F400
00506CE0 CALL 00432050
00506D44 CALL 0048F400
00506D57 CALL 00505AD0
00506D73 CALL 00432360
00506D7C CALL 00BF65AC
00506D9A CALL 00432090
00506DB1 CALL 00505AD0
```

`0048F400` is cdecl with three input words; `ADD ESP,0Ch` at `00506CD9` and
`00506D4F` accounts for them. `00432050` removes its two unused words with
RET8. `00432360` removes first/last with RET8; `_free` is one cdecl word
(`ADD ESP,4` at `00506D81`). Buy uses RET4. `00505AD0` consumes three stacked
words with RET0Ch while receiving destination owner in ECX. Assignment has
RET4 on every return and supplies EAX=destination, including its empty path.

## Exact source dependency map

| Address | Native responsibility | Current source readiness |
| --- | --- | --- |
| `00432050` | Ascending string-header destruction; no header clearing | Existing complete actual-header/raw-pool source reused. |
| `00432360` | Destruction dispatch, ECX owner and stacked range | Completed here over that actual child. |
| `0048F400` | Three-input assignment dispatch; six physical child words, then wrapping computed result | No exact source entry. Core `0048DA50` has a qualified `NativeStringStorage` fragment, not a complete unrestricted raw-pool wrapper. |
| `00498FF0` | Capture end, validate begin twice with returning reports, erase the captured range | No exact source entry. `004954F0` has a qualified storage fragment with structural-mutation restrictions. |
| `00505AD0` | Convert stacked first/end/destination plus ECX owner to custom-register uninitialized-copy invocation | No exact source entry. Core `0044D0C0` also has no source. |
| `00432090` | Clear pointer fields, check unsigned count<=1FFFFFFFh, allocate count*8 and publish fields | No exact source entry; owns allocator/length-error requirements below. |
| `00BF65AC` | CRT backing free | Existing `singleton_lifetime_free` current-CRT service; original allocator-domain/CRT identity remains qualified. |

`0048F400` forwards six words to `0048DA50`, only three consumed. Its returned
end is computed from captured addresses independently of the child's EAX.
The child skips identical element addresses, resizes with preserve=true, tests
CURRENT source length, then copies CURRENT destination length from CURRENT
source data to CURRENT destination data. A future complete raw-pool adapter
must preserve those reloads and the existing native overlap-copy policy.

The complete `00505AD0` listing establishes ECX=first,
EDX=end, and four stacked words: destination, owner, duplicate destination,
dispatch tag. The final two are unused by the uninitialized-copy body. That
body's RET10h plus the dispatcher's POP/RET0Ch prove both interfaces.

## Uninitialized-copy ownership and EH boundary

`0044D0C0` captures original destination at EBP-14h and a completed end in its
first argument slot. It constructs ascending elements through `0044BCB0`,
advancing that completed end only after a successful constructor. The existing
`construct_native_render_alias_string_0044bcb0` has a complete raw-pool overload;
it does not make its caller's range-level catch complete.

The handler at `00C60150` selects FuncInfo `00D87064` (magic19930522h,
maxState2). Both unwind-map actions at `00D87040` are zero. The one try block
at `00D87050` covers state0, with catchHigh1 and one handler. The handler record
at `00D87030` is adjectives40h/type0/disp0/handler`0044D10A`: catch-all.
These are live byte reads, not inferred source cleanup.

The separate catch body reads original destination and completed end, destroys
completed elements ascending through `00431F40`, then rethrows with
`__CxxThrowException(0,0)`. The failed current construction is not added to that
completed range. `00431F40` currently has a semantic `Seg2Host` wrapper; the
actual raw-pool header destructor exists separately at `0041DD20`. A future
range implementation must compose the actual domain and define its Source
cleanup-failure limit, rather than claiming FH3/double-exception identity.

```text
0048F42E CALL 0048DA50
00499000 CALL 00BF6713
0049900D CALL 00BF6713
0049901D CALL 004954F0
004320B6 CALL 00431F70
004320BF CALL 0042BF40
00505AEC CALL 0044D0C0
0044D0FA CALL 0044BCB0
0044D11A CALL 00431F40
0044D12A CALL 00BF6885
```

Ghidra containment confirms the last two sites belong to `Catch_All@0044D10A`,
not the enclosing normal body. Assignment itself owns no catch: on in-capacity
copy failure the changed prefix survives and end is not published; on replacement
failure the old backing is already freed and the new backing remains published
by Buy, with only completed new strings covered by the child catch. No strong
exception guarantee or replacement-backup restoration is present.

Buy's other unresolved subtree is explicitly bounded out of this packet:
`0042BF40` takes count in ECX, checks multiplication overflow and enters native
bad_alloc construction/throw or actual `operator_new`. `00431F70` constructs
"vector<T> too long" using `00408720`, constructs native legacy logic_error
via `00411700`, changes the length_error vtable and throws with metadata
`00D83F98`. Those two child string/exception constructors have existing Source
entries; the exact allocator/length-error wrappers and their native throw
policy remain missing. No generic `std::length_error` replacement is supplied.

## Evidence, readiness and limits

All normal assignment instructions, direct dispatch bodies and relevant
producer/child listings were read. The 21-function caller inventory is recorded
in the JSON report; the two Lua caller argument setups were inspected in the
preceding packet. This report does not claim all assignment call sites reviewed
or production cache binding. Both implemented wrappers' known callers were read.

Remaining direct source blockers are `0048F400`, `00498FF0`, `00505AD0`, and
`00432090`, including the named-but-incomplete cores and owning EH/allocation
subtrees above. Assignment remains source-blocked after removal of the real
`00432360` dependency. The next bounded core candidate is a full raw-pool
`0048DA50` adapter, followed by its `0048F400` dispatch; it needs its own lease.

Every live query verifies project `bsp` configured as
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language and
image base through `bsp.py`'s client. No Ghidra writes occurred. Checks and build
results are recorded in `reports/oct15_string_vector_operations.json`.
MSVC Win32 Release and all three existing CTest suites passed. The JSON and
document each pass 23 exact call-site checks. A previously recorded duplicate
spawn-symbol linker warning remains outside these owned files.
The two new functions have no added routine tests or native differential probe.
Existing math differential checks concern existing seeds. Current CRT/provider
policy, explicit Source context, private-stack/dead-byte differences, original
ABI/FH3/SEH, runtime binding and game/gameplay proof remain distinct limits.
