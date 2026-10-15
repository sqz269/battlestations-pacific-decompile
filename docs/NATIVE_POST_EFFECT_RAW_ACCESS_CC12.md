# Native post-effect raw access (CC12)

The three complete functions are one unconditional DWORD load followed by a
plain `RET`. The Win32 implementations use one-argument naked `__fastcall`
leaves so the receiver stays in ECX and the unchanged result bits return in EAX.

| Original function | Entire body | Read |
| --- | --- | --- |
| `00B4CB90..00B4CB93` | `8b 41 08 c3` | receiver + `08h` |
| `00B4EC50..00B4EC53` | `8b 41 18 c3` | receiver + `18h` |
| `00B4FFE0..00B4FFE6` | `8b 81 3c 02 00 00 c3` | receiver + `23Ch` |

There are no stack arguments, calls, branches, null guards, lifetime operations,
profile checks or conversions. MOV/RET leave EDX, EBX, ESI, EDI, EBP and EFLAGS
unchanged; RET consumes the return address. An unreadable receiver retains the
original faulting-load behavior. A zero stored DWORD is returned unchanged.
The older semantic `pass_frame_targets_00b4cb90(pass, zero)` host interface is
not the original ABI: this implementation adds no second argument.

`pass` and `owner` are storage-role hypotheses, not recovered class names.
The public result is `uint32_t` to retain raw pointer/DWORD identity without
assigning an unproved pointee type. Existing scene-renderer call sites reach
these leaves after `00B16109`; luminance-owner call sites also reach B4CB90.
This packet does not reconstruct those caller bodies or activate D5E13C.

Evidence comes from the saved `C:/Users/sqz269/bsp.gpr` program
`/battlestationspacific.exe`, fresh live whole-function bytes, installed-PE spans,
and strict MSVC x86 COFF comparison. All 15 emitted bytes must equal the three
original bodies, with no relocations. The accompanying JSON records current
source/object pins, exact compiler/build results, and proposed ledger names for
the integrator to review and apply. The worker makes no Ghidra mutations.

Whole emitted-body/register-ABI equality is separate from runtime fixture,
caller integration, class-layout, lifetime, application or game validation.
No new permanent tests or target execution are introduced.
