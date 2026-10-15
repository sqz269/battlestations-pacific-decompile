# Actual pooled-string range assignment and dispatch

Addresses: `0048DA50`, `0048F400`.

Both bodies now have complete actual-header Source implementations using the
existing concrete `NativeStringRawPoolContext` resize path. The earlier
`copy_native_string_range_storage` fragment in `native_checked_string_storage.cpp`
remains separate, with its original status and provenance. This packet does not
implement vector assignment `00506C80` or alter its previous readiness report.

| Native body | Source | Coverage | Original ABI |
| --- | --- | --- | --- |
| `0048DA50..0048DAA1` | `assign_native_string_header_range_0048da50` | complete, qualified providers | cdecl, six physical DWORDs/three consumed, plain RET, EAX advanced destination |
| `0048F400..0048F444` | `assign_native_string_range_dispatch_0048f400` | complete, qualified providers | cdecl, three stacked pointers, plain RET, EAX independently computed end |

Source/header: `src/native_string_range_assignment.cpp` and
`include/bsp/native_string_range_assignment.hpp`, registered for `bsp_core`.
The established `STL_NativeStringRange_Assign_0048da50` and
`STL_NativeStringRange_AssignDispatch_0048f400` names are preserved.

## Actual state and provider binding

Elements are the canonical eight-byte native string headers: DWORD length+0,
data pointer+4, established by constructor `0041E870` and the existing
`NativeString` declaration. No capacity member, overlay, `std::string`, staging
owner or generic vector is introduced. The range borrows the existing headers
and backing. Only the real resize/copy path changes their state or storage.

The complete Source `resize_native_string_header_0041dd40` raw-pool overload in
`src/native_string.cpp` was inspected against the entire native resize listing.
Its concrete `RawStringPoolAccess` resolves `00419CC0` using the borrowed actual
publication and manager cells before each allocation/return, then uses actual
`00BD1120`/`00BD1510` and the current small-return gate. The new API has no
`NativeStringStorage` interface or configurable allocator/copy callback.

The consumed resize contract includes the equal-length fast return before any
data-pointer read, zero-length return/clear path, allocation before old-buffer
release, current header reloads after allocation, preserve=true, current pool
resolution on release, and final pointer/length/terminator publication. Its
existing allocation-failure, raw-memory, zero-copy and current-CRT qualifications
are inherited, rather than replaced by a stronger allocation policy.

`00BF7680` retains its established `_memcpy` library name. Its native prefix
tests source/destination overlap and chooses the backward branch at `00BF7844`;
backward STD/REP MOVSD/CLD instructions were inspected. The accepted Source
service is current-CRT `memmove` for nonzero copies, as in the raw resize path.
This preserves overlapping byte-copy semantics, not native instruction/fault
order, vectorized dispatch, original CRT identity, or private frames.

## Exact core schedule

The core captures first, last and destination addresses and walks until pointer
equality, using wrapping DWORD increments of eight. It performs no null, range,
alignment or capacity validation and does not precompute an element count.

For each pair:

1. Identical header addresses skip **all** source/destination field reads,
   resizing and copying, then advance both cursors.
2. Otherwise capture the source length and resize the actual destination with
   preserve=true through the concrete raw-pool overload.
3. Recheck CURRENT source length. Zero skips the outer copy and its pointer
   reads, regardless of the captured requested length or current destination.
4. If nonzero, read CURRENT destination length, then CURRENT source data, then
   CURRENT destination data. Copy exactly that destination length with overlap
   allowed. Do not substitute the requested/source length or snapshot data across
   resizing. Source uses volatile DWORD reads to preserve this outer ordering.
5. Advance both captured cursors by eight; return the advanced destination.

No terminator is included in the outer copy. Resize writes one only when its
own native schedule does so. Equal recorded lengths do not repair a stale data
pointer or terminator; identical header addresses do not consult the pool.
Overlapping header ranges retain forward, sequential mutation. There is no
temporary vector, snapshot of future elements, or backward element assignment.

The accepted zero-byte service omission is explicit. When step4 reads a zero
current destination length, source still reads source data and destination data
in their original order, then omits only the zero-byte `memmove`. The inner raw
resize already omits its zero-byte preserve copy under the same established
Source policy. Invalid/null data with a nonzero count has no fallback.

## Physical arguments and dispatcher

| Entry ESP offset | Native core input | Consumed? |
| --- | --- | --- |
| +04h | first | yes |
| +08h | last | yes |
| +0Ch | destination | yes |
| +10h | caller word | no |
| +14h | dispatch word | no |
| +18h | dispatch word | no |

All four known core caller setups were inspected. The two dispatchers supply
destination as word4; the two erase callers supply their iterator-output pointer.
The core never reads that word or the two dispatch words. The new core API
retains all six words and adds the explicitly borrowed raw-pool context.

| Caller | Exact core site | Six-word cleanup | Role |
| --- | --- | --- | --- |
| `0048E780` | `0048E7AE` | ADD ESP,18h at `0048E7B3` | sibling three-input dispatch |
| `0048F400` | `0048F42E` | ADD ESP,18h at `0048F433` | owned three-input dispatch |
| `004954F0` | `00495535` | ADD ESP,18h at `00495543` | suffix assignment for range erase |
| `00589690` | `005896BD` | ADD ESP,18h at `005896C9` | suffix assignment for one-element erase |

The owned dispatcher captures all three addresses, supplies six child words
(including duplicate destination and two zeroed dead words), then ignores the
child EAX. It computes its own result as captured destination plus
`SAR32(captured_last-captured_first,3)*8`, with DWORD wrap. Its native private
dispatch words only have their low bytes initialized; source initializes both
whole words. Original private-stack aliasing and high bytes remain qualified.

The sole dispatcher caller is `00506C80`, with two setup sites. Both have three
cdecl words, confirmed by `ADD ESP,0Ch` at `00506CD9` and `00506D4F`. Neither the
core nor dispatcher installs a local native EH frame, cleanup or rollback.
Exceptions from the actual pool/resize path propagate; completed prefix changes
and the failing element's existing resize effects are retained. The new source
adds no catch, rollback, `noexcept`, `noreturn` or EH conversion.

```text
0048DA6E CALL 0041DD40
0048DA83 CALL 00BF7680
0048F42E CALL 0048DA50
0048E7AE CALL 0048DA50
00495535 CALL 0048DA50
005896BD CALL 0048DA50
00506CCD CALL 0048F400
00506D44 CALL 0048F400
```

Native resize uses RET8 for its two DWORD inputs. Outer-copy cleanup is
`ADD ESP,0Ch` at `0048DA88`, matching three cdecl words. Live containing-function
queries confirmed all eight call-site owners before report verification.

## Verification and remaining boundary

Every live query uses the `bsp.py` verified client for project `bsp` configured
as `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language and
image base. Ghidra was read only. Complete owned listings and all known caller
setups were reviewed; the actual resize Source/native body and accepted overlap
provider contract were inspected. Checks are recorded in the JSON report.
MSVC Win32 Release and the three existing CTest suites passed; report and
documentation each pass eight exact call-site checks.

This closes the actual range-assignment/dispatch prerequisite for future vector
assignment. `00506C80` still requires its other exact owning/empty-clear/Buy/EH
dependencies and is not marked reconstructed here. No production cache binding
or prior packet file is changed. The old qualified fragment is not superseded.

The new functions require valid taken raw backing and the actual pool domain.
Source adds a context argument; original ABI/private-stack/fault/CRT/SEH identity
and original-game/gameplay validation are not established. No new routine test
or native differential probe is added; existing math differential tests cover
their existing seeds, not these functions.
