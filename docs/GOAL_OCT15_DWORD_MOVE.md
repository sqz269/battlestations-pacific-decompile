# Raw DWORD range movement

| Routine | Source API | Coverage | Native ABI |
| --- | --- | --- | --- |
| `00467D90` | `move_native_scene_dword_range_00467d90` | complete | cdecl first,last,destination; plain RET; EAX captured result |

The whole 50-byte, 21-instruction body occupies `[00467D90,00467DC2)`.
No indexed or existing raw Source implementation was found before this packet.
The descriptive Source/Ghidra role is provisional; the native library callee
retains its correct `_memmove_s` name. No nominal pointee or collection class
is inferred from DWORD scaling or the pointer-vector caller's current label.

The native body reads last then first before wrapping subtraction, preserves
first in EDX, and applies SAR2 to the raw DWORD difference. TEST establishes
the signed condition before destination is read; destination, scaled bytes and
result are all captured before the conditional CRT call. PUSH ESI/PUSH EDI and
the intervening MOV/LEA instructions retain that TEST condition. Source expresses
SAR2 by unsigned shift and explicit sign-fill bits, with no implementation-defined
unsigned-to-signed conversion or signed shift. Scaling and result addition wrap
modulo 2^32. A shifted value of zero or a negative bit pattern skips the call
but still returns the captured wrapped/rounded destination+bytes. There is no
alignment, first/last ordering, nonnull or range normalization guard.

For signed-positive shifted distance only, the native pushes count, first,
destination-size, destination, invokes the real secure-move CRT and cleans
four words with ADD ESP,10h. Count and destination-size are the same scaled
DWORD; its errno result is ignored. A returning current SDK diagnostic cannot
change this helper's pre-call result. All native arguments are captured before
that call; the new Source interface does not recreate physical argument cells
or their private-stack coincidence with moved backing.

| Containing routine | Site | Native callee | Contract |
| --- | --- | --- | --- |
| `00467D90` | `00467DB5` | `00BF67A7` | cdecl(destination,bytes,first,bytes), ADD ESP,10h; ignored status; captured result survives. |
| `004F60E0` | `004F612D` | `00467D90` | Captured source begin/end, current destination begin; ADD ESP,0Ch. |
| `004F60E0` | `004F6196` | `00467D90` | Captured source begin/prefix-end, current destination begin; ADD ESP,0Ch. |

Both live incoming sites were read through their argument setup and cleanup.
They belong to the same pointer-vector assignment routine. No caller is newly
owned, annotated or reconstructed by this packet, and the helper's zero/negative
paths remain complete even though these observed callers normally supply ranges.

The live native `_memmove_s` body was inspected: zero count succeeds; null
destination/source reports EINVAL; count beyond destination-size reports ERANGE;
otherwise it invokes actual memmove. Source calls the concrete current SDK
`::memmove_s`, already used by accepted byte-range and scene-database Source,
without a callback, stub or new service. Original static-CRT errno storage and
encoded invalid-handler selection, fault/SEH/register/heap ABI, native binary
identity, candidate differential and game validation are not claimed.

Live batches verify `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
x86 language and image base. The report records full live/installed bytes,
typed transfers and prior annotations. Naming/comments are changed only under
the write lock, preserving old values before save and refreshed export.
Required `scripts/build.ps1` passed at Source revision `983c031c6`, using
MSVC Win32. Both enabled existing tests passed in 8.28 seconds: math and tool
tests. This worker configuration does not enable optional native differential
fixtures. The existing unrelated LNK4006 duplicate `spawn_request_id_matches`
warning remains. Three typed transfer rows and documentation verification
passed. No new tests were added.

The primary independently reviewed the complete 21-instruction native listing
against the unchanged Source/header at `983c031c6` and found no mismatch.
The report records the reviewed Source/header hashes.
