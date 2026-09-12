# Native gamepad force requests

Addresses: `008722D0`, `00872310`, `00872320`, `00872330`, `00872350`,
`008723A0`, `008723B0`, `008723D0`, `008723F0`, `00872460`, `00872470`,
`008724A0`, `008724C0`, `008724E0`, `00872500`, `00872520`, `00A93F90`,
`00A93FC0`, `00A94510`.

`native_gamepad_force_requests.hpp/.cpp` supplies the three actual request
allocations and all five request methods consumed by `NativeGamepadDispatch`.
Existing typed `GamepadForceRequest` implementations remain available. The new
objects have one canonical profile DWORD in their actual allocation; no C++
object, retained reference, map, SDK owner, or second publication shadows them.
The callable C++ interfaces and finite dispatch are source contracts, not a
drop-in replacement for the executable's vtables, calling convention, or SEH.
Descriptive names express recovered behavior; they are not original symbols.

## Native bodies and ABI

Every body below has complete reconstruction over valid caller-owned storage.
ECX supplies the actual request in every native entry. EDX is scratch; no
hidden input was recovered. All stack offsets are measured at function entry.

| Entry | Coverage | Native result / stack cleanup | Body end exclusive |
|---|---|---|---|
| 008722D0 constant constructor | complete | EAX=this; channel, amplitude, remaining at +4/+8/+C; RET0C | 008722FF |
| 00872310 constant channel | complete | EAX channel; RET | 00872314 |
| 00872320 constant value | complete | ST0 amplitude; RET | 00872324 |
| 00872330 constant expired | complete | EAX 0/1; RET | 00872342 |
| 00872350 fading constructor | complete | EAX=this; channel, amplitude, duration at +4/+8/+C; RET0C | 00872387 |
| 008723A0 fading channel | complete | EAX channel; RET | 008723A4 |
| 008723B0 fading value | complete | ST0 result after native float spill; RET | 008723CC |
| 008723D0 fading expired | complete | EAX 0/1; RET | 008723E5 |
| 008723F0 alternating constructor | complete | EAX=this; channel, selector, values[2], periods[2], duration; RET1C | 0087244F |
| 00872460 alternating channel | complete | EAX channel; RET | 00872464 |
| 00872470 alternating value | complete | ST0 selected value; RET | 00872493 |
| 008724A0 alternating expired | complete | EAX 0/1; RET | 008724B5 |
| 008724C0 base scalar deletion | complete | flags at +4; EAX=captured identity; RET4 | 008724DF |
| 008724E0 constant scalar deletion | complete | flags at +4; EAX=captured identity; RET4 | 008724FF |
| 00872500 fading scalar deletion | complete | flags at +4; EAX=captured identity; RET4 | 0087251F |
| 00872520 alternating scalar deletion | complete | flags at +4; EAX=captured identity; RET4 | 0087253F |
| 00A93F90 constant update | complete | seconds at +4; no semantic result; RET4 | 00A93FBF |
| 00A93FC0 fading update | complete | seconds at +4; no semantic result; RET4 | 00A93FCD |
| 00A94510 alternating update | complete, recognized CRT boundary | seconds at +4; no semantic result; RET4 | 00A94544 |

The initially missing `008724C0` is 31 bytes. Its final instruction is the
three-byte RET4 at `008724DC`. Primary defined, saved and exported that body
through `008724DE`; read-back shows 11 instructions and no gaps. The other
18 bodies also have no listing gaps. No worker modified Ghidra or no-return
metadata. The prototype still needs primary's address-specific annotation.

## Profiles, production, and untouched bytes

| Profile | Size | Kind +4 | Data after common channel DWORD +8 |
|---|---|---|---|
| D0DB78 constant | 14h | 0 | amplitude +C; remaining +10 |
| D0DB8C fading | 18h | 1 | amplitude +C; duration +10; elapsed +14 initialized to +0 |
| D0DBA0 alternating | 28h | 2 | selector BYTE +C; values +10/+14; periods +18/+1C; duration +20; phase +24 initialized to +0 |

The kind at +4 is not a reference count. Alternating constructor stores only
one unnormalized selector byte: +D..+F retain the allocation preimage. The
canonical storage types have no implicit initialization. Constructors preserve
the native field-store order.

Standalone constructors have no recovered direct callers. Their profiles also
occur in the inline event producers: `008734B2` allocates 14h, `00873696`
allocates 18h, and `008737B2` allocates 28h through `00BF681B`. Their following
definition reads and stores (`008734BE..008734E0`, `008736A2..008736D4`,
`008737BE..0087380C`) independently confirm the layout. These events read
definition fields after allocation; the new convenience allocation functions
explicitly accept already evaluated values and do not replace those producers.
Allocation and bit0 scalar deletion share `singleton_lifetime_allocate/free`,
the existing BF681B/BF65AC-compatible CRT domain.

The actual profile table at `D0DB64..D0DBB3` is:

| Profile | slot00(flags) | slot04 channel | slot08 value | slot0C expired | slot10(seconds) |
|---|---|---|---|---|---|
| D0DB64 base | 008724C0 | BF698E | BF698E | BF698E | BF698E |
| D0DB78 | 008724E0 | 00872310 | 00872320 | 00872330 | A93F90 |
| D0DB8C | 00872500 | 008723A0 | 008723B0 | 008723D0 | A93FC0 |
| D0DBA0 | 00872520 | 00872460 | 00872470 | 008724A0 | A94510 |

Slot00 directly takes flags. There is no refcount decrement or BD30E0 thunk.
Each scalar stamps D0DB64, frees only for flags bit0, and returns the captured
allocation address even after freeing it. Other payload bytes are untouched.
There are no member destructors or native EH maps in these standalone bodies.
The finite dispatcher reloads the current profile for each method invocation.
The base non-delete slots use the real CRT `_purecall`; its body at BF698E
invokes the installed purecall handler and the CRT fatal path. Unknown profiles
are explicit source binding errors.

## Floating-point behavior and services

Constant expiration is the native ordered `0 >= remaining` COMISS result;
NaNs are unexpired. Its update reloads the borrowed live D7A278 double, skips
subtraction only on ordered equality, and otherwise stores the x87 subtraction
as float. D7A278 initially contains `000000E0FFFFEF47` (little-endian bytes),
the double equal to FLT_MAX; the source never replaces the live read with that
literal.

Fading value preserves the original duration float spill, x87 evaluation of
`((duration-elapsed)/duration)*amplitude`, and final float spill before ST0
return. Elapsed increases with the original x87 add. Alternating value uses
the unnormalized selector byte and a strict ordered period > phase test.
Selector zero chooses the first value while first_period > phase; selector
nonzero chooses the second value while second_period > phase. Neither branch
clamps or normalizes values. Fading and alternating expiration use their native
x87 comparisons; alternating compares the wrapped phase against duration.

Alternating update rounds `seconds+phase` and `second_period+first_period` to
float before the recognized fmod library operation. `A94530` calls `BF857A`,
whose complete thunk sets EDX=E15500 and jumps to C07EE0; E15500 identifies
`fmod`. The source calls genuine `std::fmod` and rounds its result back to
float. Original CRT dispatch internals, error hooks, and floating-environment
binary identity are not claimed.

Application subclasses of `NativeGamepadForceRequestDispatch` must still bind
the two genuine device force/value methods inherited from `NativeGamepadDispatch`
and keep the borrowed D7A278 publication alive. Raw event submission, A95BF0
producer dispatch, A95680/A949A0 insertion, application attachment, and physical
controller delivery remain separate work. No provider stub is installed.

## Validation

MSVC Win32 Release built successfully with both existing CTests passing. All
eight seed ranges match the installed executable, and the final CALL audit
checked five rows with zero failures. The three instruction byte ranges used
by the focused fixture also match the installed executable.

Validation results and the five exact address/native CALL rows are recorded in
`reports/native_gamepad_force_requests.json`. The ignored focused fixture links
the built core archive. It checks copied original instruction bytes against the
source constructor/preimage, fading x87 value, and live sentinel update, then
feeds all three real request profiles through the existing raw18h tree pump and
clear using concrete raw XInput value/motor methods. It does not call a hardware
SDK or claim application force-event wiring. Scalar checks distinguish flags2
preservation from flags1 deletion and confirm the captured return identity.
