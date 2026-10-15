# Avoid-zone owner cleanup and optional intersection point

Addresses: `0041C490`, `0041A4E0`. Packet `oct15_avoid_owner_and_intersection`.
Both complete bodies are reconstructed using existing concrete APIs. Names are
provisional descriptions, not recovered symbols. No Ghidra mutation was made.

| Address / proposed name | Body inclusive | Native ABI | Coverage |
| --- | --- | --- | --- |
| `0041C490` / `BSP_AvoidZoneOwner_DestroyAndNull` | `0041C490..0041C4C7` | ECX owner-cell address; no stack arguments; `RET` at `0041C4C7` | complete |
| `0041A4E0` / `BSP_Segment2f_IntersectionOptionalPoint` | `0041A4E0..0041A532` | fastcall ECX output, EDX a0; stack a1,b0,b1; EAX output; `RET 0Ch` at `0041A525` / `0041A530` | complete |

Live queries checked project `bsp`, configured at `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, language and image base. Both pseudocode
and complete assembly were read; the 22/28-instruction listings have no gaps
and reach their explicit returns after all returning calls. Exports exist;
the initial worker index was stale and was rebuilt. Main source/include had no
implementation of either address. There are no known live callers. Old names
are `FUN_0041c490` and `FUN_0041a4e0`; prior plate comments were saved under
`local/oct15_avoid_owner_and_intersection_previous_comments.json`.

`0041C490` captures the object from the owner cell at `0041C494`. Null input
skips all calls and every write. Nonnull cleanup has three distinct lifetimes:
counted corner records, their pointer buffer, and the zone object itself.
The native `004167C0` plus buffer-free/null sequence is identical to the already
complete `avoid_zone_destroy_owned_storage_0041a1e0`, so it is reused rather
than duplicated. The zone object is then freed through actual `00BF65AC`, and
only after that returns does `0041C4BF` null the captured owner cell. The
existing callback named `free_record_00bf65ac` is an untyped real CRT-free
endpoint; here its role is object deallocation. No `delete`, `unique_ptr`,
reference counting, extra reset or ownership policy is added.

The existing producer-backed `AvoidZoneNativeStorage` layout and allocation
access are reused. The preceding packet read `0041CCD0`'s concrete header,
bounds and 24h corner-record producer stores; the exact destructor and
returning CRT-free contracts were also read in full. Capacity and trailing
metadata are preserved until object deallocation. Every record/buffer/object
must meet its actual matching free contract. A nonempty opaque-prefix copy
does not establish those ownership identities. The owner cell must remain
writable through object free and final null, outside all freed allocations;
no concurrent mutation or callback invalidation is supported.

The `0041A4E0` pseudocode's thiscall form omits a live register argument.
At entry, `0041A4E4` saves output from ECX, and `0041A4E6` saves a0 from EDX.
After the local/saved-register frame, `0041A4E8` reads stacked b1. Following
the local-result push, `0041A4F1` reads stacked b0; after pushing b1,
`0041A4F6` reads stacked a1. The final call therefore supplies ECX=a0, EDX=a1,
stack b0,b1,local-result to existing exact `native_segment_crossing_004f3730`.
The installed `.text` bytes were independently disassembled through the call
at `0041A4FD`, confirming this register/stack provenance. Both callee and
wrapper remove three stack pointers with `RET 0Ch`. The apparently unused
first stack slot in pseudocode is a1, not an unused argument.

The established `Cc13AvoidZoneOptionalVector` storage is reused as an optional
POINT: x at +0h, z at +4h, valid byte +8h, three untouched bytes at +9h..+Bh.
Existing producer `0041B290` was read completely and confirms those coordinate
and validity stores; the existing 0Ch size and new local offset assertions
reconcile the declaration. This packet introduces no alternate overlay.

The intersection callee receives a separate local float2 result. On success,
the wrapper copies the two result representations with native MOVSS semantics,
then writes valid=1. On failure it writes only valid=0. Coordinates on a miss
and padding on either path retain their caller-owned bytes. It returns the
same output identity on both branches. Raw endpoint-pair pointers allow the
native alias behavior: endpoints may overlap each other or output, since every
input read finishes inside the callee before this wrapper writes output.
Readable stable endpoint words and writable output are required. Existing
crossing, x87 arithmetic, degeneracy and unordered rejection are inherited;
no replacement intersection test or extra float conversion is introduced.

| Function | Call site | Callee | Contract / source binding |
| --- | --- | --- | --- |
| `0041C490` | `0041C49C` | `004167C0` | Complete existing clear; composed through exact `0041A1E0`. |
| `0041C490` | `0041C4A8` | `00BF6989` | Pointer-buffer free, caller `ADD ESP,4`; then buffer pointer zero, composed through `0041A1E0`. |
| `0041C490` | `0041C4B7` | `00BF65AC` | Distinct object free, caller `ADD ESP,4`; then owner cell zero. |
| `0041A4E0` | `0041A4FD` | `004F3730` | Complete exact native crossing API; four endpoint pairs plus local result; callee `RET 0Ch`. |

Validation: the full warm-cache `./scripts/build.ps1` passed all three existing
suites (`reconstructed_math`, `native_math_differential`, `tool_tests`), 3/3,
in 7.11 test seconds. Log: `local/oct15_avoid_owner_and_intersection_build.log`.
The call report passed 4/4 rows; diff checks passed. No new routine tests were
added. Existing seed tests do not
establish packet-native differential coverage. New semantic C++ interfaces do
not prove drop-in register/exception ABI, live native ownership, original
runtime integration or gameplay equivalence.
