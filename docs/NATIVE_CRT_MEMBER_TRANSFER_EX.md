# Native CRT member transfer (EX)

Address: `00BF6AA6` (`_CallMemberFunction0`), complete seven-byte body through
`00BF6AAC`. Source: `src/native_crt_member_transfer.cpp`; interface:
`include/bsp/native_crt_member_transfer.hpp`. The correct Visual Studio library
name is retained. The C++ interface name is descriptive, not a recovered symbol.

EX implements the original two-word Win32 `__stdcall` adapter as a naked MSVC
entry. It takes the actual object pointer and actual executable zero-argument
member entry. The latter is an ordinary native address, not a C++ pointer-to-member
descriptor or a substituted callback. No object/frame/target/data owner is added.

## Exact ABI and instruction coverage

| Native instruction | Bytes | Effect with entry ESP = S |
| --- | --- | --- |
| `00BF6AA6 POP EAX` | `58` | EAX = continuation K; ESP = S+4 |
| `00BF6AA7 POP ECX` | `59` | ECX = object O; ESP = S+8 |
| `00BF6AA8 XCHG [ESP],EAX` | `87 04 24` | EAX = target T; [S+8] = K |
| `00BF6AAB JMP EAX` | `FF E0` | Enter actual T with ECX=O, [ESP]=K |

Entry stack is `[K, O, T]`. At transfer, ESP=S+8 and the target sees the real
continuation at its stack top. A balanced target `RET` resumes K at ESP=S+12:
both argument words were consumed, matching `__stdcall` cleanup of eight bytes.
The adapter does not issue a second call or build a synthetic return frame.
An exceptional or nonlocal target exit need not return to K.

Memory `XCHG` preserves x86 implicit atomic read-modify-write semantics without
an explicit LOCK byte. POP/XCHG/JMP preserve EFLAGS, including DF; the adapter
also leaves EDX, EBX, ESI, EDI, EBP and FP/SIMD state untouched. EAX/ECX and ESP
have the exact effects above. Target register/flag effects propagate. Valid
readable/writable native stack and executable-target domains remain caller
obligations. No checks are inserted for a null object, target validity, stack
access, faults or exception ownership.

Coverage is complete: four instructions, seven original bytes. There are no
direct callees, imports, native data accesses, source data definitions or body
relocations. The indirect target is borrowed from the caller and remains a real
native executable entry; source availability of that target is not supplied here.

## Current caller evidence

Fresh typed Ghidra queries verify project `bsp`, program
`/battlestationspacific.exe`, language `x86:LE:32:default` and base `00400000`
before each request; configured project file is `C:/Users/sqz269/bsp.gpr`.
Every owned byte and the complete physical caller span match the installed PE.

The only current direct xref is `00C06AF9` inside `___DestructExceptionObject`
at `00C06AC8`. Its saved Ghidra body is `00C06AC8..00C06B0A` (67 bytes).
The complete physical span `00C06AC8..00C06B1B` is 84 bytes, including its
nine-byte filter and eight-byte handler. This is read-only caller evidence;
EX does not alter, reconstruct or claim the parent body or handler.

The parent loads its exception-record pointer from `[EBP+8]`, the non-null
throw-information pointer from record+1Ch, and the non-null destructor entry
from throw-information+4. `00C06AF5 PUSH EAX` supplies that actual destructor;
`00C06AF6 PUSH [ECX+18h]` supplies the actual object, which may be null.
`00C06AF9 CALL 00BF6AA6` supplies K=`00C06AFE`. The next instruction resets
the try level, with no caller `ADD ESP,8`: the adapter/target return already
consumes those two words. This physical call is not attributed to the separate
`___AdjustPointer` at `00C06B1C`.

## Build and static artifact validation

The companion report records final results and exact paths. The local payload
`local/native_crt_member_transfer_ex/` retains executable verification methods,
full commands/stdout/stderr, installed/live spans, all decoded instructions,
compiler source/header/read/write logs, toolchain inputs, object/archive/MAP/PE
artifacts, source guards, annotation old values/readback, failures and seals.

`verify-seeds` runs before CMake configuration so both existing CTests are
registered. `scripts/build.ps1` performs the full strict MSVC Win32 Release build
and existing tests. EX adds no tests. Before/after guards cover current build
sources, configuration, toolchain and installed-image inputs. The actual CL
command is checked for `/W4 /WX /fp:strict /Oy- /O2 /MD /std:c++17`; complete
matching command/read/write groups and the resolved `/Fo` output are retained.

Static inspection compares all seven bytes in the current naked COFF body,
the unique actual archive member, every matching symbol definition across all
current build objects and archive members, and the actual forced-linked PE.
The forced link pulls the real archive member with `/INCLUDE:<actual symbol>`.
Its `static_link_probe.exe` filename is safe, `/MANIFEST:EMBED` is explicit, and
the embedded PE resource is parsed to verify the actual `asInvoker` manifest.
The driver only supplies an unused `main`; it never invokes the adapter.
The probe image is never executed. The ordinary game does not retain this leaf.

The correct `_CallMemberFunction0` name and original library comment are
preserved. EX prepares an exact evidence-comment plan and retains current old
values for the primary integrator to apply under the shared Ghidra write lock,
save, read back and export after review. Worker Ghidra access remains read-only:
no rename, comment, prototype, save, export or parent mutation occurs here.

## Evidence limits and handoff

Exact leaf artifact coverage and the original stack ABI are established by the
instruction/byte checks. Adapter execution, destructor execution, exceptional
paths, caller adoption, complete CRT exception closure and gameplay are not
validated by EX. Only the existing unrelated math/differential tests are run.
Remaining exception-record/scope/FS/termination/header-domain providers from
the EV readiness packet remain separate obligations.

The local payload has two full independent SHA256/SHA512 inventories of itself
and every declared external path spelling. Case aliases are opened and hashed
independently; no path or hash normalization substitutes for those reads.
Only the three explicitly named recursive seal sidecars are excluded. The
handoff records exact base, committed seven-file patch, worker head and manifest
pins. No merge, push, game launch, forced-image launch or installation occurs.
