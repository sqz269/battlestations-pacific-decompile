# Native device-class registry array

Packet `orch4_device_array_c3` reconstructs the two complete raw array bodies
used by the device-class singleton. Names are descriptive hypotheses, not
recovered symbols.

| Routine | Bytes | Original ABI |
| --- | ---: | --- |
| `0043FA60..0043FABE` | 95 | ECX actual data/count/capacity header, stacked signed capacity, `RET 4` |
| `00440180..004401CF` | 80 | ECX actual header, stacked signed count, `RET 4` |

The source exposes new MSVC Win32 C++ interfaces over the actual twelve-byte
header. The registry producer at `00441780` owns a 10h-byte object with
`{vptr,data04,count08,capacity0C}`; every live factory call passes
`registry+4`. There is no host vector, parallel owner, or callback facade.
Allocation/free compose the existing `singleton_lifetime_allocate` and
`singleton_lifetime_free` boundary. The pointed-to DWORDs are borrowed class
pointers: these two bodies never retain, release, or destroy them.

## Reserve `0043FA60`

The request is clamped to one with a signed comparison. If the signed current
capacity is already sufficient, the function returns. Otherwise it computes
`DWORD(request * 4)`, allowing 32-bit wrap, and allocates that byte count.
For every signed index below the current count it tests the reached destination,
reloads the current data pointer and current count, and copies one DWORD. It
then frees the current data pointer and only afterwards publishes replacement
data and capacity. Count is unchanged. The repaired live listing includes the
previously hidden publication instructions at `0043FAB1..0043FAB9` and has no
remaining gaps.

## Resize `00440180`

Resize reserves exactly the requested count when signed `requested > capacity`.
Growth starts from the current count and writes zero to each reached nonnull
slot. Shrink repeatedly decrements the header count without clearing storage or
releasing its class pointer, then the final instruction writes the requested
count for both paths. Negative requests and arithmetic wrap are not rejected.

## Direct caller audit

Live Ghidra has seven calls from four functions. `004430C2`, `00443365`,
`00443449`, and `00443472` all pass the current `00441780` result plus four;
the first three are respectively initial lookup, fresh publication, and cached
retain, while the last reacquires the slot for return. `00442113` clears a
class slot during destruction and passes the same singleton `+4` header.
`0044217C` passes zero to resize that header. `0044138D` passes owner `+4` and
zero before freeing its current backing. At every id-based site the count check
and `id+1` use signed/DWORD instructions. A cached index edge naming caller
`00440C70` is absent from the live xref set and is not accepted as a call.

`0043FA60` has one live caller, the resize body. Its only external calls are
the operator-new wrapper `00BF55BE` and free `00BF6989`; the existing canonical
allocation layer models their shared new-handler/CRT domain. `00440180` calls
only reserve. No extra release appears in either listing.

## Evidence and limits

The complete repaired reserve listing and complete resize listing were read in
the verified `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
Source was independently compared against every instruction, including signed
branches, current-header reloads, wrapped address arithmetic, free/publication
order, zero-growth, and count-only shrink. The report retains all ten direct
call rows.

This closes the raw array dependency needed by the factory at `00443090`.
It does not implement `00441780`, the registry constructors/destructors, the
factory, a binary ABI bridge, or game/runtime validation. Host CRT exception
identity, allocation overflow behavior outside backed memory, and arbitrary
hardware faults remain outside the new source interface.
