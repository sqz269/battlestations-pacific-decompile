# Actual MPAK file open and search

Addresses: `00BB4A60`, `00BB4B20`, `00BB5BB0`.

The three complete bodies operate on the original provider and string storage.
`BB4A60` is a custom MPAK file-name search; its old `stl_probable` bookmark is
not supported by its provider-specific loop. The called checked subscript
`BB4140` remains the existing `STL_inst_00bb4140` library contract.
All descriptive names are hypotheses, not recovered symbols.

| Routine | Inclusive body | Original ABI | Coverage |
|---|---|---|---|
| `find_native_mpak_file_index_00bb4a60` | BB4A60..BB4B15, 182 bytes | ECX provider; stack name8h; RET4; EAX signed index/-1 | complete |
| `contains_native_mpak_file_00bb4b20` | BB4B20..BB4B35, 22 bytes | ECX provider; stack name8h; RET4; AL Boolean | complete |
| `open_native_mpak_file_00bb5bb0` | BB5BB0..BB5CCB, 284 bytes | ECX provider; stack name8h/flags; RET8; EAX stream | complete |

The initial live program had no function definitions for BB4B20 and BB5BB0.
The primary integrator defined these exact ranges under the Ghidra write lock;
this worker verified their live ranges and refreshed exported pseudocode. The
worker made no Ghidra mutations. `tools/bsp.py ghidra` verifies the configured
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` before every live batch.
The report records fresh disk/live byte comparisons for all 488 owned bytes.

## Storage and ordering

The producer is the already reconstructed `BB8240` constructor and `BB7C50`
directory parser in `native_mpak_provider.*`. The actual provider is 44h bytes,
its file vector header starts at +1Ch, and begin/end/capacity are +20h/+24h/+28h.
Its 24h-byte file records start with the existing eight-byte length/data string
header. `BB5590` initializes the device at +10h to -1; `BB8240` initializes the
cached file index at +40h to zero; `BB5080` writes that index on materialization.
No new object, record layout, map, copied file catalog, or pooled-string owner is
substituted for these bytes.

`BB4A60` starts at index and record byte offset zero. It reloads begin/end on
each pass, computes the signed DWORD byte difference divided by 24h, and tests
the resulting count with an unsigned comparison. A second current-bounds
check invokes `BF6713` if invalid; the callback can return. The record address
uses a fresh begin read after that callback. Equal recorded lengths are
required; two zero lengths match without reading data, and equal nonzero
lengths call the current CRT `_stricmp`. The first match wins. Reaching the end
returns FFFFFFFF. The Boolean wrapper compares the result as signed, not as an
unsigned index.

`BB5BB0` rejects flags bit 0 before reading either provider or name. Otherwise
it captures provider+10h, then searches non-null name data for the literal
`_0000`. A match qualifies only if its wrapping difference from the CURRENT
name data is not FFFFFFFF and the captured device is -1. It copy-constructs a
temporary actual header through `00426060` and calls `BDD850` with the current
manager and the same temporary pointer for both stack arguments. The second
argument is unused in the observed full BDD850 body. A throwing constructor has
no cleanup owner; once construction returns, a throwing route cleans the
current temporary header. On normal return the local cleanup state is disarmed
before the observed `419CC0`/`BD1510` release pair.
The original FH3 data confirms this boundary: handler CC4458 selects FuncInfo
DFDD84; its single unwind entry at DFDD7C maps state 0 to -1 through CC4450.
That funclet computes `[EBP-14h]` and jumps to the existing `0041DD20` string
destructor. This corroborates the source catch's current-header cleanup.

After temporary destruction the function reloads the manager publication and
writes the selected device to its +18h. It reloads original name data for the
trace arguments (`>PAK open %s deviceid:%d`, empty fallback at 010904F0), whose
installed `004254B0` body is one RET. The source's empty trace body therefore
implements an observed leaf, not a successful substitute for an unknown call.

Only a **signed cached index greater than zero** takes the fast path. The
checked STL subscript sees provider+1Ch and the captured index. `00435C40`
compares that record's native name with the original request. On success the
function reloads provider+40h; on failure it performs the actual `BB4A60`
search. Either result, including -1, is passed unchanged to `BB5080` with the
same actual provider. The optional temporary route does not replace the name
used in either file lookup.

## Call boundaries

| Site / containing routine | Native callee | Source treatment and evidence |
|---|---|---|
| BB4AB6 / BB4A60 | BF6713 | Returning original invalid-parameter contract through existing `SingletonLifetimeCallbacks`; callee calls BF66EF with five zero arguments. |
| BB4AE7 / BB4A60 | BF7FBF | Current CRT `_stricmp`; two pointer arguments, ADD ESP,8. |
| BB4B25 / BB4B20 | BB4A60 | Actual local search source, then signed result >= 0; RET4. |
| BB5BFC / BB5BB0 | BF9440 | Current CRT `strstr`; `_0000` at D641E8; ADD ESP,8. |
| BB5C1A / BB5BB0 | 426060 | Existing actual-header copy constructor, RET4. |
| BB5C35 / BB5BB0 | BDD850 | Explicit device-route boundary; ECX captured manager, temporary/temporary; RET8. |
| BB5C57 / BB5BB0 | 419CC0 | Existing actual pool storage reacquires the current pool for each release. |
| BB5C5E / BB5BB0 | BD1510 | Same storage returns captured block/current length+1/unused1; RET0Ch. |
| BB5C7F / BB5BB0 | 4254B0 | Exact installed RET leaf; three observed arguments, ADD ESP,0Ch. |
| BB5C93 / BB5BB0 | BB4140 | Original STL checked vector subscript boundary; RET4. Name was already pushed for the subsequent comparison. |
| BB5C9A / BB5BB0 | 435C40 | Existing actual-header case-insensitive equality; RET4 consumes that name. |
| BB5CAB / BB5BB0 | BB4A60 | Actual local search source, RET4. |
| BB5CB3 / BB5BB0 | BB5080 | Explicit materialization boundary to sibling actual source; RET4. |

The `NativeMpakOpenLibrary` and `NativeMpakOpenDispatch` interfaces have no
fallback implementations. The primary integration supplies the actual BDD850
and BB5080 sources and the existing STL contract; all contexts borrow the same
actual provider, manager publication and native string pool.

## Validation and limits

The packet adds no tests. `scripts/build.ps1` passed with strict MSVC Win32
Release and `MSBUILDDISABLENODEREUSE=1`; both existing CTests
(`reconstructed_math`, `native_math_differential`) passed after all eight native
seeds matched the installed image. The final report records the result and
exact source hashes. The call-site checker passed all thirteen direct call
sites against live Ghidra instructions and containing bodies, with zero failures.

This packet alone does not wire the runtime provider vtable or establish an
installed-archive or gameplay result. There is no packet-specific native/source
fixture claim. Original library internals are contracts rather than ports.
Host `_stricmp`/`strstr`, the current pool's noexcept release, C++ exception
handling versus original x86 FH3/SEH, and concurrent writes to native storage
remain explicit limits; source implementation and compilation are not proof of
binary replacement compatibility.
