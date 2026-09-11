# Libm error callback storage and source readiness

The missing writer is present in the executable: a coherent 41-byte registration
body at `C0F0BB..C0F0E4`, immediately before the full libm error service. Saved
Ghidra analysis has no function or references for this entry, so the earlier
direct-xref discovery missed its writes. A nonnull input calls actual pointer
encoder `C04F67`, publishes encoded `109ED80`, then publishes flag `109E1B8=1`.
A null input clears only the flag and leaves the encoded word stale.

This establishes a real storage producer, not a current registration or callback
lifetime. No direct caller or pointer-table reference to the writer was found
in the finite scans below. The full 637-byte service/default pair is ready for
a qualified source packet once the separately owned actual decoder is available
and its caller-owned callback/name identity domain is adopted. It must neither
force flag0 nor replace a true callback with the default. Encoder plus setter
form a separate 151-byte publication packet; they are not implemented here.

## Evidence and scope

Base `d7389974` uses existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Every live query is guarded through `bsp.py`.
Original PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Forty-eight disk-backed spans total2,523 bytes; four separately recorded saved
virtual spans total28 bytes. The latter are not on-disk or live-process values.
Only discovery addresses `C0F0E4`, `C28545`, then explicitly approved `C0F0BB`
and `C04F67` are leased. `C04FDE` and startup bodies are supporting reads only.
No source, Ghidra, shared metadata, build, test, runtime or game change occurred.

| Entry | Complete half-open extent | Bytes | Status |
|---|---|---:|---|
| `___libm_error_support` | `C0F0E4..C0F35E` | 634 | Existing saved full body,176 instructions |
| Default matherr target | `C28545..C28548` | 3 | `33 C0 C3`, no argument read or callback |
| Registration candidate | `C0F0BB..C0F0E4` | 41 | Fully decoded coherent entry, no saved function; library name remains provisional |
| Actual pointer encoder | `C04F67..C04FD5` | 110 | Existing complete body,36 instructions |

All176 libm-service instructions are reachable in its static CFG, using the
actual13-entry `C0F360[52]` table. This is not feasibility or runtime coverage.
Preceding bytes `C0F0B5..C0F0BB` decode `FPATAN; RET; FPTAN; RET`; the next
entry is the saved service atC0F0E4. The candidate is therefore separate from
those return-terminated leaves and ends before the next entry. Its lack of an
incoming saved reference prevents a claim of a previously established function
or library symbol. No analysis repair has been applied.

## Actual writer, stale storage and lifetime

The candidate is cdecl, one pointer-sized word at entryESP+4, plain RET.
AtC0F0BB it compares that slot with zero. Null executes
`AND DWORD[109E1B8],0; RET`: this is an actual read-modify-write, not a store
to both globals. The encoded pointer remains untouched and no encoder runs.
Nonnull rereads the current argument slot atC0F0CA, pushes it, callsC04F67,
pops the temporary, then writes `109ED80=EAX` atC0F0D4 and flag1 atC0F0D9.
There is no pre-clear, lock, atomic pair publication, result validation or
rollback. An encoder failure before return gains no publication by this body;
side effects inside the actual encoder/providers remain. Incidental EAX on
the null branch is not normalized. A semantic result is not established.

Both globals belong to writable `.data` (`C0000040`), sectionbaseE08000,
rawsize10000h, virtualsize297EDCh. Both are beyond raw backing. Saved Ghidra
bytes are four zeros each, consistent with PE virtual initialization. This
neither identifies live encoded-null bits nor proves that flag stays zero.
The service snapshots the flag comparison before capturing its argument
pointers, then reads the current encoded word only on the nonzero branch.
It retains that decoded callback for the current invocation. Registration
changes during callback/errno affect later invocations, not reselection here.
No reset, free, ownership transfer, retained reference or thread join occurs
in the writer. Code and borrowed callback data must outlive all possible calls;
clearing a flag alone does not retire an already captured callback safely.

## Finite alias/caller and startup checks

The offline PE scan examined every unaligned DWORD window of disk-backed
sections for values in `[109E180,109E1E0)` and `[109ED40,109EDC0)`. Its293
neighbor hits are saved as candidates, not all claimed as decoded accesses.
Exactly five hits refer to either requested word. All five match verified
instructions: writer flag clear, encoded publication and flag publication,
then service flag read and encoded read. None is a data-table pointer to either
word. Saved direct xrefs expose only the service's two reads.

A second finite scan examines every disk section for exact absolute pointers
toC0F0BB/C28545 and every executable-section byte position for E8/E9 rel32
targets at those entries. Setter has no hit. Default has its immediate
identity atC0F127 and direct calls atC13619 andC2755E. There is no PE export
directory. Computed pointers, short branches, external modules, dynamic
registrations and runtime writes are outside these negative observations.
No absence-of-caller or lifetime proof is inferred.

The named startup pointer families were inspected concretely:

| Full initializer/support | Actual writes and limit |
|---|---|
| `__init_pointers BFBDFB[76]` and all eight direct initialization targets | Eleven absolute destination words: E15580,109DE44,109E454,109DD64,109E16C,109E450,109E43C/440/444/448,109E170. None is either math callback word. |
| `__initp_misc_cfltcvt_tab C0680F[31]` | Ten DWORDs E15BC8..E15BEC, each encoded throughC04F67 and published after return. Captured disk table is tenC273EF pointers. It does not populate109ED80 or109E1B8. |
| `__cinit BFBC47[146]` | Calls that conversion initializer and walks existing initialization tables. Its optional D693D8 and109FED8 callbacks remain external startup edges; no invented registration conclusion. |
| `__mtinit C053DC[388]` | Publishes real TLS/FLS state and calls__init_pointers; does not register the math callback in its direct stores. |
| `C050F8[182]` | Populates actual PTD+1F8/+1FC with named EncodePointer/DecodePointer resolutions. Locale/lock/SEH dependencies remain incomplete. |
| `__mtterm C050BB[61]` | Frees current FLS/TLS slots and writesE15AFC/E15B00=-1 before lock teardown. It does not clear the math callback words. |

The absence of C0F0BB from the inspected initialization families is bounded.
It does not close every startup table target or another module's callback
registration. Those facts are not needed to replace the service's actual
callback with a synthetic one; such replacement would be unsupported.

## Encoder and decoder storage domain

EncoderC04F67 is the110-byte twin of separately owned decoderC04FDE: PTD slot
1F8 rather than1FC, literal EncodePointer rather than DecodePointer, and the
relocated call to the sameC04EFB gate. Every other instruction byte agrees.
The comparison is recorded bytewise, including differing CALL displacement.

It always reads currentE15B00 and calls imported TlsGetValue (CE20BC), retaining
that imported getter in ESI. A nonnull first result allows reading current
E15AFC; if it is notFFFFFFFF, capture that index, call the same imported getter
again using currentE15B00, then call its newly returned raw getter with the
captured index. Current PTD+1F8 supplies the encoder. A null encoder in an
existing PTD yields identity; it does not take the fallback.

Fallback captures GetModuleHandleA("KERNEL32.DLL"), then calls fullC04EFB and
conditionally GetProcAddress on that captured module for "EncodePointer".
A nonnull selected encoder is called stdcall with the current argument word;
returned EAX is stored back into the actual argument slot at saved-ESI ESP+8,
then reloaded. Identity paths leave the slot untouched. The selected provider
is actual OS/PTD storage, not an injected generic encode/decode callback.
The same storage contract, with slot1FC, applies to the decoder dependency.

Fresh__mtinit confirms E15B00 contains the real TLS index whose value is the
selected raw FlsGetValue/TlsGetValue, before encoding the four global function
words109DE34..40. Actual FLS/TLS PTD allocation is214h; C050F8 resolves both
encoder/decoder pointers into it. Encoder reaches a1FCh-byte prefix, decoder
a200h-byte prefix; these bounds do not establish a universal PTD size or its
construction. The callbacks/slots/module and PTD must remain valid for the
duration of each reached call. Neither source packet should silently construct
a private TLS/PTD owner or republish provider fields.

Named incomplete startup closure includes __mtinit, __mtterm, C050F8, the
errno/PTD getterC051B7, __calloc_crtC0485A, __mtinitlocksC11A93, __lockC11C21,
locale reference C0190B, cleanupC051AE and lock teardownC11ADC. Saved
__SEH_prolog4C07C00/__SEH_epilog4C07C45 names do not constitute complete
runtime reconstruction. Their boundaries are recorded; no startup source is
proposed here. C04EFB/BFBB61/C05920 prerequisites and thenC04FDE belong to
the other worker's separately reviewed sequence.

## Callback ABI, canonical names and mutable aliases

The actual admitted callback type is `int (__cdecl*)(CameraAxesCrtException*)`.
The pointed record is32bytes: type0, name4, argument1+8, argument2+10h,
result+18h. The service fills both argument qwords on every callback route,
including unary names. The callback receives the same mutable record whose
result is later loaded. It can change type/name/arguments/result and other
owned state; the service does not freeze or sanitize it. It must return with
balanced cdecl stack, preserved nonvolatile registers and the required net
x87 stack depth. Raw native code must remain callable; no decoded-null guard,
substitute callback, thunk with an argument envelope, or result-only adapter is
part of the original contract. There is no evidence of a current nondefault
registered callback body, so its ownership and arbitrary behavior remain
outside this packet. This is an actual CRT extension-point ABI, not a new
generic reconstruction service.

Name identity is observable, not just string contents. Each literal is in
read-only `.rdata` and is stored as an address, without a name read by the
service. These exact identities and full NUL bytes are captured:

| Name | Address | Name | Address |
|---|---|---|---|
| tan | D571A0 | sin | D571B4 |
| pow | D571C4 | modf | D571C8 |
| log | D571D0 | log10 | D571D4 |
| floor | D571F4 | exp | D571FC |
| cos | D57204 | ceil | D57210 |
| atan | D57218 | asin | D57228 |
| acos | D57230 | exp10 | D6A968 |

Thirteen same addresses (all except exp10) occur in the actual29-row mutable
__umatherr operation/name tableE165C8[232]. FullC135BA[158] loads its selected
name pointer from that table; it does not independently duplicate the strings.
Libm itself still uses immediate literal identities: it must not read its names
from that mutable table, whose later pointer changes belong to__umatherr only.
A future source should borrow actual canonical name identities, or use one
explicitly mapped rebuilt identity domain shared by all admitted callbacks
and consumers. Private per-function string copies do not establish compatible
pointer comparisons. Readonly bytes can be reconstructed only with that
identity qualification and full data/relocation evidence.

The service supports valid aliasing among first/second/output qword pointers.
Raw globals are mutable storage too: an aliased output can change a callback
word after the earlier selection; no recapture of the selected callback occurs.
Before decoding, first/second/output pointers are captured in the order second,
output, first. Selector is loaded from its actual caller slot only after the
decoder returns. A new raw-word interface must retain that current selector
read rather than pre-copy all four arguments. Eight byte writes create local
positive zero; the separate32-byte record starts uninitialized and receives
only the native writes. Aliases into a new implementation's private EH/frame
storage must be excluded or represented explicitly as caller-prepared scratch,
never claimed equivalent merely because record fields match.

## Full dispatch and errno order

Selection of defaultC28545 or current decoded callback occurs even for an
unknown selector or selector26. Unknown selectors perform no pointee read or
write, callback or errno operation after selection. Selector26 performs
`FLD1; FSTP output` and no callback. No native EH guard or FP control/status
reset is present.

| Selector | Name/type | Initial result | Errno if callback returns zero |
|---|---|---|---|
| 2/3 | log,2/1 | current output | 34/33 |
| 8/9 | log10,2/1 | current output | 34/33 |
| 14/15 | exp,3/4 | current output | 34/none |
| 24/25 | pow,3/4 | current output | 34/none |
| 27/28 | pow,2/1 | current output | 34/33 |
| 29 | pow,1 | first-to-output before argument reloads | 33 |
| 58/61 | acos/asin,1 | current output | 33 |
| 166 | exp10,3 | current output | 34 |
| 1000..1005 | log/log10/exp/atan/ceil/floor,1 | first-to-output before argument reloads | 33 |
| 1006 | pow,1 | current output | 33 |
| 1007 | modf,1 | first-to-output before argument reloads | 33 |
| 1008/1009 | acos/asin,1 | current output | 33 |
| 1010..1012 | sin/cos/tan,1 | first times local positive zero, retained ST0 product | 33 |

For early-copy routes, FLD first/FSTP output happens before current first and
second are reloaded into the record. For1010..1012, `FLD first; FMUL zero;
FST output` retains the extended product while argument1 andargument2 are
loaded/spilled, then FSTP stores that retained product into record.result.
All x87 conversions, rounding, NaN changes and hardware exception order are
material; no ordinary C++ double copies or host arithmetic are proposed.

Callback occurs at one ofC0F1A6/C0F1E2/C0F342. The route establishes errno
policy before the callback; changes to record.type do not reroute it. The
underflow path ignores callback return and writes no errno. Other routes test
EAX, call owning errno accessorBFFB8B only for zero, then store34 or33 before
the final current `FLD record.result; FSTP output`. Reentry/record mutation by
that accessor therefore precedes the final result read. No errno location is
cached and no cleanup or rollback is inserted when a provider fails.

## Existing source boundary and proposed next packets

Full current `CameraAxesCrtException` and `LegacyCrtMathRuntime` source/header
inputs are pinned. The runtime offers the owning `errno_location_00bffb8b`
service, and a distinct `matherr_bypass_00e16bd0` used by the existing87except
route. Its private atomic binding has no public getter. Libm should borrow
that actual runtime explicitly for errno; it must not substituteE16BD0 for
109E1B8 or call87except as a replacement. The existing default comment/fragment
is not an implemented public three-byte callback entry.

1. After the separateC04FDE source exists, propose four new
   `native_crt_libm_error_support` cpp/hpp/doc/audit files owning only full
   C0F0E4/C28545. Fixed context borrows actual109E1B8/109ED80, concrete decoder
   access, canonical name identities and the owning LegacyCrtMathRuntime. A
   raw16-byte argument-slot view avoids floating C++ conversions and preserves
   selector reload after decode. The full literal default entry and full x87
   service schedules remain required. Final interface/assembly bridges need
   primary review; no implementation is included in this discovery.
2. Separately propose fullC04F67[110]+C0F0BB[41] in four new
   `native_crt_libm_callback_registration` files, reusing the actual decoder's
   TLS/PTD/module-gate context with EncodePointer identity. Preserve the actual
   writable encoder argument slot and stale encoded pointer on disable. This
   would implement publication, not establish an absent current registration,
   source the arbitrary callback or reconstruct whole CRT startup lifetime.

Concrete remaining facts for use in a running host are the actual callback
code identity and lifetime, coherent initialized encoder/decoder state, and a
canonical name-address domain compatible with every admitted callback. These
must be explicit caller-owned preconditions until separately evidenced. The
finite negative caller scan is not a reason to force flag0. This read-only
packet makes no reconstructed/build/fixture/native-ABI/game-valid claim.
