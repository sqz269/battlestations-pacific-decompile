# Reference-counted pointer arrays used by vehicle classes

Addresses: `00546630`, `005471B0`.

## Reconstructed scope

| Entry | Ordinary bytes | Original ABI | Coverage |
| --- | ---: | --- | --- |
| `00546630` | 244 | ECX header, signed capacity on stack, RET4 | complete ordinary body; native EH metadata is external |
| `005471B0` | 126 | ECX header, signed count on stack, RET4 | complete ordinary body |

`native_vehicle_pointer_array` implements the actual three-DWORD header:
data at +0, signed count at +4, signed capacity at +8. Both routines borrow
the header and backing storage. Every nonnull element has an aligned reference
count at +4 and a current table at +0. The names are descriptive hypotheses.
The helper is shared by vehicle classes and gun/other consumers; the filename
does not establish a vehicle-exclusive native type.

Reserve clamps capacity to at least one, returns when the current signed
capacity suffices, and allocates the wrapped DWORD capacity times four.
It zero-initializes each reached destination, copies and atomically retains
each nonnull referent, then walks the old slots forwards and releases them.
It frees the current old backing before publishing the replacement and capacity.
The count is not assigned. Both loops reload the current count and data header;
the source does not replace these observations with snapshots.

Resize reserves only when requested count exceeds capacity. It zeroes newly
reached slots, then removes excess entries from the back. Each removal decrements
the published count before capturing the slot and referent. A nonnull referent
is atomically released; zero dispatches the current table's current slot0 with
ECX=referent and EDX=table, with no stack arguments. After dispatch returns,
the captured slot is cleared even when dispatch replaced its contents. A null
referent causes no slot write. The final count assignment uses the request bits.

The complete listings establish signed comparisons, wrapping address arithmetic,
and RET4 in both functions. All 25 current direct xref sites to resize were
inspected around argument setup: they include zero, dynamic counts and index+1,
so the interface retains a count parameter. Their complete caller bodies are
outside this packet. Reserve's only current direct caller is resize at `005471BE`.

## Corrected Ghidra flow

The call at `005466FA` to returning CRT free had a CALL_RETURN override.
The locked repair checked disk bytes, removed the override, disassembled
`005466FF..00546711`, saved the existing project and refreshed the export.
Those 19 bytes publish data and capacity and restore saved registers; omitting
them leaves a dangling old header. The repair record preserves the old override.
The original free function's no-return flag and the executable bytes were not
changed. The refreshed listing and decompiler include the publication tail.

## Interfaces and limits

The source uses the established `singleton_lifetime_allocate/free` boundary,
whose source CRT allocation/free are explicit shared dependencies. Required
`NativeVehiclePointerArrayCalls::zero_reference` dispatch receives the captured
target, owner and table. It has no successful default. Numeric game profiles
still need a reconstructed terminal implementation before executable admission.

These are new MSVC Win32 C++ interfaces. The original FS exception frame and
handler `00C6CD07`, original CRT internals, malformed/private-stack aliases,
hardware faults and throwing native callbacks are not implemented or validated.
The ordinary reserve listing keeps its EH state at -1. Source exceptions escape
without adding rollback; a throwing terminal skips the captured-slot clear and
later publication. No gameplay or drop-in binary compatibility is claimed.

## Validation

The strict MSVC Win32 Release build and all three existing CTests passed.
Validation results are recorded in `reports/native_vehicle_pointer_array_orch4.json`.
The focused ignored fixture executes original relocated reserve/resize bytes and
the source over the same five-stage lifecycle, with shared allocation/free and
Windows atomic operations. Its terminal records release order and changes the
captured slot, checking that the caller subsequently clears it. Original and
source outputs match in all five snapshots and are retained separately. All
370 original routine bytes matched the installed PE and verified Ghidra image.
This fixture is not an independent
oracle for the shared CRT or original exception machinery.
