# Native CRT SEH4 frame boundary

The complete `__SEH_prolog4` (`00C07C00`, 69 bytes) and `__SEH_epilog4`
(`00C07C45`, 20 bytes) are custom compiler-frame operations. The prolog installs
the actual `__except_handler4` at `00C07C90`; that complete 406-byte handler is
also retained here to identify the real dependency boundary. No source,
Ghidra annotation/listing, build, test or runtime change is included.

Base: `27f97f574388ff576d57a25acd213560ec2ad434`. Complete fresh PE/live spans
agree with the installed executable, SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Each live CLI batch verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Correct CRT library names are preserved.

`reports/native_crt_seh4_frame_br.json` retains exact spans, call checks,
source availability, borrowed-state requirements and the whole-local two-pass
SHA256/SHA512 inventory. The complete 197-byte `00C17653` caller and scope
table are reused from discovery `01fc5c40538266cd29f1ab33e479401c70603be1`;
all 80 prior artifacts were verified before reuse. The complete 15-byte
`00BFE120` cookie checker is reused from Watson discovery
`ed185cb2dfdf1ba045dbf30ebfd47f083eeece80`, after verifying all 135 artifacts.
Both prior worktrees are unchanged. Reused spans were compared with the
current installed PE; they are not described as newly captured live spans.

## Prolog ABI and publication

Let `S` be ESP on entry to `00C07C00`, `N` the DWORD at `[S+8]`, `T` the raw
scope-table pointer at `[S+4]`, `R` the helper return address at `[S]`, `C` the
actual DWORD at `00E15590`, and `F=S+8`. The caller must have pushed the local
size, then scope pointer, then called this helper. Its native enclosing-function
return address is at `[F+4]`. This is not an ordinary cdecl helper that returns
with its argument area intact.

The exact ordered operations are:

1. Push the actual handler address `00C07C90`, then the previous `FS:[0]` link.
2. Read `N`, overwrite its old slot with incoming EBP, set EBP to `F`, and
   subtract `N` from ESP. No stack probe, size validation or local zero-fill
   occurs in this body.
3. Push EBX, ESI and EDI in that order. Read actual `00E15590` once into EAX.
   XOR the original scope-pointer slot `[F-4]` with that cookie, then XOR EAX
   with `F` and push `C XOR F`.
4. Store the resulting ESP, `P=F-N-20h`, into `[F-18h]`. Push a copy of `R`
   from `[F-8]`. Move the encoded scope pointer from `[F-4]` into EAX, store
   `-2` to `[F-4]`, then publish the encoded pointer into `[F-8]`.
5. Set EAX to `F-10h`, publish that address to `FS:[0]`, and RET through the
   copied `R`. The continuation sees EBP=`F`, ESP=`P` and EAX=`F-10h`.

| Location after prolog | Contents |
| --- | --- |
| `F+4` | Enclosing function's original return address |
| `F` | Incoming EBP |
| `F-4` | Try level `FFFFFFFE` |
| `F-8` | Scope pointer `T XOR C` |
| `F-Ch` | Actual handler pointer `00C07C90` |
| `F-10h` | Previous `FS:[0]`; start of published registration |
| `F-14h` | Exception-information slot; not initialized by the prolog |
| `F-18h` | Saved ESP `P` |
| `F-N-14h` | Saved EBX |
| `F-N-18h` | Saved ESI |
| `F-N-1Ch` | Saved EDI |
| `F-N-20h = P` | Saved EH cookie `C XOR F` |

This nonoverlapping map describes a valid compiler frame with room for the
fixed bookkeeping (`N >= 8`); the observed caller uses `N=14h`. The native
body does not enforce that condition. If slots alias, its ordered writes
remain authoritative rather than the abstract independent-field map.

Arithmetic flags on return remain those from `XOR EAX,EBP`: CF/OF zero,
ZF/SF/PF from `C XOR F`, AF undefined. Later MOV/LEA/PUSH/RET instructions do
not change them. ECX/EDX and incoming EBX/ESI/EDI values remain unchanged;
EBP, ESP, EAX and the FS chain intentionally change. The complement word
`00E15594` is not read. There is no cookie comparison in this helper.
FS publication occurs only after these stack writes, including the copied
continuation; no substitute handler or repair path is present.

## Epilog ABI and unlink order

At a normal call to `00C07C45`, EBP must still be `F` and the caller's ESP must
be `P`; the helper enters with ESP=`P-4`, holding its own return address. It
first loads `[F-10h]` into ECX and writes that old link to `FS:[0]`. Only then
does it pop its return address into ECX.

The first `POP EDI` discards the saved cookie at `P`; the second restores the
actual saved EDI. `POP ESI` and `POP EBX` restore the other saved registers.
`MOV ESP,EBP; POP EBP; PUSH ECX; RET` restores incoming EBP and transfers to
the helper continuation with ESP=`F+4`, pointing at the enclosing function's
return address. The caller then performs its own RET.

EAX/result and arithmetic flags are unchanged. ECX contains the helper
continuation. No stack cookie is checked, no decoder/global service is called,
and no exception policy is introduced. The early FS unlink and the transient
first EDI value must not be replaced by a generic destructor or reordered
cleanup. Faults during later restoration occur after the registration has
already been unlinked.

For the retained `00C17653` caller, `N=14h`, `T=00E037B8`, so `P=F-34h`;
saved EBX/ESI/EDI are at `F-28h/-2Ch/-30h`. The scope header is
`{-2,0,-34h,0}`: no GS-cookie check, and an EH-cookie check that reconstructs
`(F+0) XOR [F-34h]`. This equals the same actual `C`. The scope entry is
`{-2,00C176D9,00C176F0}`. The existing handler writes its exception-pointer
pair to `[registration-4]`, exactly `[F-14h]`, while the caller's omitted
handler starts by restoring ESP from `[F-18h]`. These offsets tie the caller,
prolog, scope table, cookie checker and dispatcher to one concrete frame.

## Actual installed handler and unresolved providers

`00C07C90..00C07E25` is fully retained. At native entry, stack DWORDs are
exception record at ESP+4, registration at ESP+8, and context at ESP+Ch;
the body does not read the usual fourth dispatcher argument. Its normal
return is plain RET with EAX disposition and restored EBX/EBP/ESI/EDI.
Its positive-filter path transfers through a direct JMP instead of returning
through that normal epilog.

It decodes `[registration+8] XOR [00E15590]`, takes `F=registration+10h`, and
validates scope cookie pairs through the actual ECX-input checker
`00BFE120`. The first pair is skipped only when its offset is `-2`; the
second pair is always checked. Offset additions and XORs are native 32-bit
operations. It then branches on `exception_record.flags & 66h`.

On dispatch it publishes a local `{exception_record, context}` pair into
`[registration-4]`, follows the scope chain from `[registration+Ch]`, and
passes filter address in ECX and `F` in EDX to `00C0DCB6`. A negative filter
result selects disposition zero; zero continues the scope chain; positive
selects unwinding and handler transfer. Appropriate exit/transfer paths
recheck the same cookie pairs; the retained body records the exact branches.

For a positive filter and exception code `E06D7363`, it tests actual pointer
word `00D6A618`, calls `00C16DD0` on that word's address, and conditionally
reloads the word for a cdecl indirect call `(exception_record,1)`. Current
PE/live bytes identify `___DestructExceptionObject` at `00C06AC8`; the PE
section is not writable. The real guard remains an obligation and is not
replaced by this static observation or an arbitrary callback.

| Actual provider | Observed boundary | Current source status |
| --- | --- | --- |
| `00BFE120 __security_check_cookie` | ECX cookie; compare actual `00E15590`; equality `F3 C3`, mismatch tail JMP `00C185A4` | Complete retained discovery; no exact source/owning failure-state provider |
| `00C0DCB6 _EH4_CallFilterFunc` | ECX filter, EDX `F`; EAX signed result | Named, no exact source; 23-byte body identified, not expanded here |
| `00C0DCCD _EH4_TransferToHandler` | ECX handler, EDX `F`; reached by JMP at `00C07E00` with dispatcher stack still present | Named, no exact source; 25-byte body identified, native continuation/NLG dependency remains |
| `00C0DCE6 _EH4_GlobalUnwind` | ECX registration | Named, no exact source; 26-byte body identified, actual RtlUnwind boundary remains |
| `00C0DD00 _EH4_LocalUnwind` | ECX registration, EDX target level; stacked `F`, actual cookie-word address; caller expects 8-byte cleanup | Named, no exact source; 23-byte body identified, `00C0DBC4` remains |
| `00C16DD0 __IsNonwritableInCurrentImage` | cdecl address of actual `00D6A618` word; caller pops 4 bytes | Named, no exact source; native image-validation/section providers remain |
| `00C06AC8 ___DestructExceptionObject` | Actual guarded indirect target; cdecl exception record and DWORD 1 | Named, no exact source; own destructor/termination/native-frame obligations remain |

After global unwind, the dispatcher conditionally performs local unwind if
the current try level differs from the selected level. It then stores the
selected scope's enclosing level, rechecks cookies, and jumps to the actual
handler-transfer provider. On the unwind-flag branch, a non-`-2` level invokes
local unwind with target `-2`, followed by cookie rechecks. No lock, heap or
PTD implementation is supplied by these frame instructions.

The repository already has complete qualified source for `00C1815E` in
`native_crt_cookie_initialization.cpp/.hpp`. Its context borrows the actual
cookie/complement words and invokes the real Win32 initialization services;
it explicitly does not provide canonical process storage or the checker and
failure owner. This is useful existing source, not an initialized native
cookie domain. Bounded source/ledger inspection found no exact implementation
for the three owned bodies or the listed handler providers. Dataref
lists are supporting evidence only: they do not enumerate every recovered
write or prove unique ownership.

## Source readiness and next bounded work

The 20-byte epilog is a complete instruction-level source candidate with no
direct/indirect callees or absolute cookie/handler reference. Any source
packet must keep its custom native frame/stack/FS preconditions explicit; it
cannot make ordinary C++ callers or an uninitialized lock domain usable.
The 69-byte prolog likewise requires deliberate compiler-frame entry and
continuation control, the same actual cookie domain, a real complete handler,
and scope-table storage whose offsets describe that exact frame. Passing a
host handler, host cookie or invented callback would not satisfy those needs.

The next useful bounded dependency discovery is the four named EH4 helpers
`00C0DCB6/00C0DCCD/00C0DCE6/00C0DD00` (97 physical bytes total), with new leases
and exact register, continuation and unwind contracts. Their current
prototypes/sizes identify the boundary but do not prove source closure. The
full dispatcher additionally needs real cookie-failure ownership and native
image/destructor providers. This packet does not recommend substituting a
modern host CRT or expanding directly into a full CRT implementation.

All 157 instructions across the three new function spans occur in the
retained live listings; no new omitted instruction was found. Twelve direct
CALL/tail-JMP rows from the dispatcher, the reused cookie-check tail, and the
two reused caller frame-helper calls pass `verify_report_calls.py` (15 rows).
The one dispatcher indirect site is qualified separately. Earlier omitted
`00C1768E` Watson continuation and `00C176D9..00C17707` filter/handler bytes,
including indirect `SetLastError`, remain retained without listing mutation.
The cookie check's `F3 C3` bytes are preserved despite the listing's short
`RET` spelling. This is discovery evidence, not build, native execution,
exception/unwind, binary drop-in or game validation.
