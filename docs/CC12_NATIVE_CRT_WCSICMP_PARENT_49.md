# Complete CRT wide comparison parent49

The complete Original `__wcsicmp` at `00C03A39` now has an ordinary Win32
Source implementation with an explicitly borrowed real locale-changed cell
and required callee services. The correct library name is preserved. This
packet supplies no default locale, CRT global, TLS/errno owner or GUI adapter.

## Whole Original body and order

The existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
was verified before the analysis/export batch. Full pseudocode was inspected
first, then all assembly and live bytes through the BSP wrappers. The body is
`[00C03A39,00C03ACA)`: 145 bytes, 56 instructions, final plain `RET` at
`00C03AC9`. Original ABI is cdecl with two stacked wchar pointers, EAX signed
result, and saved/restored ESI/EDI. The new Source context is a different API.

| Original site | Established behavior |
|---|---|
| `00C03A3C` | Read the actual **DWORD** at `0109DE1C` before reading/checking the two argument values. Any nonzero bit pattern takes the locale arm. |
| `00C03AB6..00C03AC4` | Push null locale, exact right, exact left; call three-argument `__wcsicmp_l` at `00C0392A`; caller pops12 bytes. Return that result without local validation or errno work. |
| `00C03A45..00C03A72` | Zero gate: check left first; only nonnull left reaches the right check. Either null reaches the same error arm. |
| `00C03A4D..00C03A65` | Call `00BFFB8B` for current owning CRT errno address; push five zeros, store literal **`0x16` = 22 decimal** through returned EAX at `00C03A57`; call `00BF66EF`; pop20 bytes; if it returns, return `0x7fffffff`. |
| `00C03A74..00C03A9E` | Read unsigned WORD left; fold only `0041..005A` by adding20h. Then read unsigned WORD right and apply the same fold. Advance each cursor2 via two INC instructions. |
| `00C03A9F..00C03AB4` | Test folded left NUL before equality. Equality with nonzero left loops; otherwise return zero-extended WORD left minus zero-extended WORD right. Difference range is `[-65535,65535]`, not a signed16-bit subtraction. |

The right unit is still read when the left unit is NUL. Case folding does not
map non-ASCII units or use a Unicode/SDK helper. There is no parent local
cleanup frame, allocation, stored owner or other data write outside errno and
effects performed by the required callees.

Existing `00BFFB8B` evidence returns per-thread errno at PTD+8 or its Original
fallback `00E159E8`; this packet borrows that service instead of substituting
the compiler CRT's `_errno`. `00BF66EF` assembly decodes its actual handler,
tail-jumps to a returning configured handler, or clears debugger-hook state
and tail-jumps to Watson. Therefore the parent must allow a handler return,
nonreturn or provider exception; there is no invented no-op handler. Its five
raw DWORD slots retain exactly the zero-delivery established by this caller.
These read-only callee inspections did not rename or mutate their bodies.

## Source interface and genuine integration prerequisites

`include/bsp/native_crt_wcsicmp.hpp:15` defines required borrowed callbacks:
owning errno-address, five-DWORD invalid-parameter delivery, and three-argument
locale comparison. It forward-declares the SAME `NativeCrtWcsicmpLocaleView`
from packet48; this parent only forwards null and never constructs or inspects
a duplicate view. `src/native_crt_wcsicmp.cpp:9` reads the separately borrowed
`const volatile int32_t& actual_locale_changed_0109de1c` first. The valid loop
uses ordered volatile unsigned16-bit reads and unsigned Win32 address
increments. The static width constraints require the existing Win32 target.

Before production adoption, the caller must provide the application's real
live changed-cell and owning CRT errno/handler/locale services, retain every
context throughout reached calls, and respect those services' own lifetime
and serialization contracts. No copied flag, image-zero assumption, fake
default locale/TLS/errno, unresolved-body stub or process-global provider was
introduced. Source-ready means the whole parent's ordinary control flow over
those required services is available; it does not close the CRT owner domain.
The existing two-argument GUI callback named `00C0392A` is a Source projection
and cannot be treated as the Original three-argument callee ABI.

## Validation and annotation

The complete live145-byte body matched the installed PE bytes, SHA256
`32666e0611c1f21b5d9ed707f5c2f01d92fa1a395d204d2cae4def038b2aaa61`.
All56 instructions, all three call targets, all internal branches and all
memory operands were audited. A focused relocated-parent fixture changed
only these four operand ranges:

| Operand offset from entry | Original instruction / target |
|---:|---|
| `5..8` | `00C03A3C` actual changed-cell address `0109DE1C` |
| `21..24` | `00C03A4D` errno service `00BFFB8B` |
| `37..40` | `00C03A5D` invalid-parameter service `00BF66EF` |
| `135..138` | `00C03ABF` locale delegate `00C0392A` |

All other bytes and internal branch displacements were preserved. Six bounded
cases passed: a folded/equal loop covering both fold and range branches; left
NUL/rightFFFF returning -65535; leftFFFF/rightNUL returning65535; null left;
null right; nonzero changed cell with null inputs delegating unchanged to null
locale. Controlled audited services checked exact argument identity, errno22
before handler, all five zero slots, unchanged callee return, later handler
errno effects, and a changed-cell mutation that must not re-dispatch the call.
The fixture proves whole relocated-parent behavior under these services, not
their real CRT bodies or private frame/argument-home aliases.

The new source compiled with MSVC Win32 `/MD /O2 /W4 /WX`; the focused native
fixture used `/MD /MANIFEST:EMBED` and passed. `scripts/build.ps1` on the merged
Main baseline exited0, including both existing enabled CTests. Root owns
CMake registration of packets46/48/49 and the subsequent combined build; this
baseline CMake build does not yet register the new49 translation unit. No
permanent tests were added. Ignored evidence is under
`local/cc12_native_crt_wcsicmp_parent_49/`.

Canonical `ghidra_annotate.py --apply --addresses 00c03a39` took the write lock,
preserved the existing `__wcsicmp` name and library plate comment, appended
the evidence, saved the program, and recorded the prior values in
`local/ghidra-annotations-20261016T010952Z.json`. The affected export was
refreshed. Only this entry's name/reconstruction records changed; no unrelated
body repair or tags were changed. Source-header/body hashes at close are
`937ca41fa72263ec61ca8e80a7726c67672dca23f25b26799e8aa19f58025f72` and
`ded240153f3a8d3c82c696a4422daed351f8c65b58e369541b42f47be253280f`.

No Original entry/register/fault/SEH ABI equivalence, full CRT owner domain,
native GUI comparison integration, native Text drawing, rendering or gameplay
is claimed. No game/audio/installation state changed.
