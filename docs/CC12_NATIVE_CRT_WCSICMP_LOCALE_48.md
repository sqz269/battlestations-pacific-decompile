# Original `__wcsicmp_l` parent, packet 48

The complete Original `[00C0392A,00C03A39)` body is reconstructed in
`native_crt_wcsicmp_locale.hpp/.cpp`: 271 bytes, 105 instructions. The correct
Visual Studio 2005 library identity `__wcsicmp_l` is retained. This Source
function requires four explicit borrowed callee services; it creates no CRT
globals, locale, PTD, TLS, errno, handler or current-locale provider.

## ABI and GUI call chain

Original is **cdecl with three stacked arguments**: left UTF16 pointer, right
UTF16 pointer, `_locale_t`. It returns an `int` in EAX with a plain RET; the
caller removes 12 argument bytes. The Source entry adds a services reference,
and its callbacks add a context pointer. It is an ordinary C++ API, not an
Original entry-point or binary ABI replacement.

The actual GUI content caller `ABA8D0` calls **`__wcsicmp` at C03A39** from
`ABA945`, passing two pointers and removing eight bytes. That wrapper tests
live DWORD `0109DE1C` before its null checks. In its changed-locale arm,
`C03AB6..C03AC4` pushes ESI=0 as locale, right, left, calls C0392A and removes
12 bytes. Thus the existing two-argument
`GameNativeTextApplicationBindings.compare_current_locale_00c0392a` is a Source
projection of this dispatch. Its name does not establish the three-argument
library ABI or supply the actual current-locale service. No GUI binding changed.

## Exact parent behavior

`BF7E24 _LocaleUpdate` is called before either pointer is checked. The local
starts at EBP-10h and occupies 16 bytes: locinfo DWORD+0, mbcinfo DWORD+4,
PTD DWORD+8, acquired byte+Ch, untouched padding+Dh..Fh. Its thiscall constructor
receives ECX=local and one stacked requested locale, returning RET4. It writes
acquired=0 first. An explicit locale copies only the first two DWORDs, leaving
PTD and padding untouched. A null locale obtains the shared live PTD/locale
state; if PTD+70 bit1 was clear, it ORs the DWORD with 2 and sets acquired=1.

A null left pointer takes precedence over a null right pointer. Either null
path calls BFFB8B for the actual writable errno cell, writes DWORD **0x16 (22)**,
and calls BF66EF with five zero DWORD arguments. If that handler returns,
conditional cleanup runs and the result is `INT_MAX`. BF66EF's real handler
dispatch can transfer without returning; that does not reach parent cleanup.

For valid pointers, the parent reads DWORD `[local.locinfo+14h]` once. Zero
selects the C-locale arm, which reads unsigned WORDs left then right and folds
only A..Z. Both cursors advance by two, using Win32 address arithmetic. It
tests folded left zero before equality. Neither high-bit UTF16 units nor the
adjacent ASCII boundary characters receive another mapping.

The other arm reads left WORD, calls BFD39F with the zero-extended unit and
the local locale view, and keeps only AX. It then reads right WORD, advances
left by two, calls BFD39F with the **same local view**, keeps AX, and advances
right by two. Right is still read and lowercased when folded left is zero.
The next right read therefore observes changes made by the preceding left
callback. Stop on folded left zero or inequality; return the zero-extended
WORD difference, from -65535 through 65535, rather than merely a sign.

Every normal exit reloads acquired byte+Ch. If nonzero, it loads the actual
PTD and ANDs DWORD `[PTD+70h]` with FFFFFFFDh. Only bit1 is cleared; all other
bits, including intervening handler/lowercase changes, survive. No destructor,
catch block, RAII cleanup or invented exception policy was introduced.

## Explicit borrowed service boundary

The API borrows one live domain through a required services aggregate:

| Callee | Required contract |
| --- | --- |
| BF7E24 `_LocaleUpdate` | Actual locale/PTD update into the supplied local. Real shared E162B0/E160D0/E161C8, C0522E/C01A7F/C0CC02 and owner/synchronization contracts remain necessary. |
| BFFB8B | Actual errno address. Its seven-instruction body obtains C051B7 PTD and returns PTD+8 or fixed fallback E159E8. No separate errno cell is created here. |
| BF66EF | Actual invalid-parameter dispatch, with the five zero DWORD prefix. Its encoded handler/decoder/Watson domain remains external. |
| BFD39F `__towlower_l` | Zero-extended WORD input and the same borrowed local locale view; the parent consumes only the returned low WORD. Actual nested locale/ctype/LCMapStringW behavior remains external. |

All required callbacks and reached raw storage must be live and compatible.
There are no guards, stub fallbacks, host CRT locale substitutions, copies of
authoritative tables, or hidden owned cells. The Source view holds actual
pointer identities. It does not recreate the CRT runtime or make the normal
GUI/Text composition ready by itself.

The first-member locale view and update are asserted standard-layout, with
offset zero for that view, and the Win32 sizes/offsets are asserted separately.
Callbacks must respect this Source C++ object/alias domain. A compatible bridge
to an actual CRT callee remains necessary; matching physical DWORDs alone does
not establish cast-safety to unrelated CRT C++ types or private stack aliases.

## Validation and limits

Every live batch used the configured existing `bsp` project and
`/battlestationspacific.exe`, verified by the BSP wrapper. Saved parent bytes
match the installed executable. The complete byte, instruction, constant,
memory-operand, branch and call audit is embedded in the JSON report; all 271
bytes/105 instructions are accounted for. There are seven external rel32 call
operands, 17 internal branches, 23 memory operands, no absolute memory operands
and no unclassified operands. Native seed verification passed before execution.

A manifested `/MD /O2 /W4 /WX` MSVC Win32 probe compiled the owned Source and
compared nine distinct cases against the complete copied Original parent.
Only the seven named callee call operands (28 bytes) were relocated to the
same controlled services used by Source. **No constant instruction was
patched.** All other bytes and internal branches remained unchanged. The probe
checks normal results, errno/handler order, same local view identity, complete
PTD flags, low-AX truncation, ordered right reads after left callbacks, and
Original stack/EBX/ESI/EDI preservation on each case. Every case passed.

This is **whole-parent comparison with controlled callee services**, not
execution of the full actual CRT subtree. The isolated baseline
`scripts/build.ps1` exited zero and all three existing CTest suites passed:
`reconstructed_math`, `native_math_differential`, and `tool_tests`. Those suites
do not contain this new routine. This worker did not register
the new source in CMake; the primary integrator owns that registration and its
integrated build. The standalone probe did compile the owned source directly.

The established domain is valid reached raw storage and normally returning
callee services. Throwing callbacks, native faults/SEH/nonlocal transfer,
private argument-home/stack aliases, exact generated register/code identity,
and fault/unwind PC identity are outside this ordinary Source API's proof.
Cleanup on a nonreturning handler or throw is not claimed. No actual-current-
locale provider, complete CRT, GUI activation, Original entry-point ABI,
startup, game or visual validation is claimed. No installation changed.

Machine-readable evidence: `reports/cc12_native_crt_wcsicmp_locale_48.json`.
Private reproducibility artifacts remain under
`local/cc12_native_crt_wcsicmp_locale_48/`; no test framework was added.
