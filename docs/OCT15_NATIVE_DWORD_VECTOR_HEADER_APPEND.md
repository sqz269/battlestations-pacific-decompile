# Actual checked DWORD-vector header Append

Address: `00698980`. Established library identity:
`LIBSTL_CheckedDwordVectorRows_Append_00698980` (preserved; not a recovered symbol).
Source: `src/native_dword_vector_header_append.cpp` and
`include/bsp/native_dword_vector_header_append.hpp`.

| Routine | Coverage | New Source interface |
| --- | --- | --- |
| `00698980` | complete: `00698980..00698A02`, inclusive; 131 bytes, 53 current starts | `void __fastcall(receiver, unused_edx, fill_header)` |

The complete wrapper composes the actual accepted counted-fill core, current
invalid-parameter service, and actual InsertOne. It does not replace the older
`append_input_checked_row_storage` projection in
`native_input_configuration_modifiers`, its internal row-copy interface, fixtures,
or ledger records. The configuration loader remains outside this packet.

## Native storage, inputs, and return

Native ECX is the raw receiver address; incoming EDX is overwritten by the initial
begin load. One physical stack word, entry `[ESP+4]`, is the fill-header address.
Both native exits use `RET 4`; ESI/EDI are preserved. The receiver directly exposes
begin/end/capacity DWORDs at `+4/+8/+C`. No opaque `+0` access occurs in this wrapper;
actual child/public aliases can still affect those bytes. The supplied fill points
to a real 16-byte inner-vector header consumed by the actual child constructor,
not a host row or container model.

Native EAX has no stable semantic result. The spare path retains incidental core
residue; the slow path retains a pointer into its private output scratch. The sole
observed caller immediately overwrites EAX after the call. Source returns void.
The new fastcall declaration retains the consumed physical input shape while
excluding original mangling, ambient registers, flags and private-frame identity.

## Ordered branches and ownership

1. Capture initial begin. If it is nonzero, read current end and compute the
   arithmetic-right-shift-by-four of wrapped `end - initial_begin`. Otherwise size
   is zero and that initial end read is skipped. Only a nonzero initial begin
   permits the current capacity read and its wrapped SAR4 distance. Compare the
   resulting DWORD distances unsigned; size below capacity selects the spare path.
2. Spare capacity: capture current end separately. Call actual counted fill with
   output equal to that captured end, count one, the fill header, receiver owner
   bits, duplicate fill bits, and a canonical unused Source tag zero. After normal
   return, store captured end plus `10h` to the current receiver's end field.
   Neither a current-end reload nor child EAX supplies this publication. If the
   captured output is null, still call the actual core: its null-output iteration
   skips construction/source access but advances; Append then stores `10h`.
3. Slow path: capture current end before comparing captured initial begin against
   it. A greater initial begin invokes the actual diagnostic, which may return.
   Preserve the captured end across that call. Native loads the fill argument
   afterward, then calls actual InsertOne with receiver, unused EDX zero in Source,
   an uninitialized private eight-byte output pair, receiver owner, captured end,
   and fill. Ignore its result and output words; they carry no wrapper ownership.

All public raw loads and stores use volatile DWORD operations. Addresses,
subtractions and additions wrap as uint32 values; SAR4 is expressed with unsigned
bit operations. No typed-array subtraction, endpoint guard, header snapshot,
allocator, callback, count bridge, catch, rollback, or noexcept promise is added.
A child failure propagates without the spare end store, retaining actual child
progress and cleanup. A throwing diagnostic prevents InsertOne; a returning one
retains the captured position despite public-header mutation. Append has no local
EH registration, action, owning temporary or cleanup obligation.

## Physical child arguments and the partial native word

Native pushes four counted-fill stack words in source/receiver/source/tag order;
ECX supplies captured output and EDX supplies one. Its tag DWORD is loaded from
private scratch after only the low byte was set to zero. The high 24 bits are
unspecified, not observed zeros. Complete core, catch and action listings never
read the third or fourth stack word. The accepted Source core leaves those
parameters unused. Source passes canonical tag zero without evaluating an
uninitialized C++ word or inventing native register/stack meaning.

The owner word is forwarded only by a nonempty completed-prefix catch; actual
Tidy ignores that context. This wrapper nevertheless passes the observed receiver
bits. Counted-fill construction and prefix cleanup retain their accepted actual
ordering and ownership; the failing element is handled by the actual constructor.
The child inert action's private-frame/literal-RET proof remains qualified by its
own accepted contract, rather than exposed as an EBP helper or dummy destructor.

## Native transfers and producer evidence

| Containing function | Site | Actual target | Binding |
| --- | --- | --- | --- |
| `00698980` | `006989C9` | `00698040` | actual counted-fill core; four stacked words, callee `RET 10h` |
| `00698980` | `006989E3` | `00BF6713` | current invalid-parameter service; may return |
| `00698980` | `006989F6` | `006988F0` | actual InsertOne; four stacked words, callee `RET 10h` |

```text
006989C9 CALL 00698040
006989E3 CALL 00BF6713
006989F6 CALL 006988F0
```

The candidate's 131 PE/live bytes and all 53 exact current instruction starts
match. Separately verified support is 164 counted-fill normal/catch/action bytes
with 64 starts, one literal RET byte/start, and 96 raw child-EH bytes. The bounded
external producer window `00698C2E..00698C45` has 24 bytes/six starts; it pushes a
private fill header, sets ECX to the receiver, and overwrites EAX immediately after
Append. Total refreshed code evidence is 320 bytes/124 starts; only 131/53 belongs
to this packet. No whole-caller reconstruction follows from that producer window.

Ghidra project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, was
verified before the live batches. The ignored evidence receipt records all bytes,
typed ranges, exact current starts, rel32 targets and provider hashes. Published
Main `39297a02b0630334186fce5c800f9dcfaaf1f0de` contains the accepted InsertOne;
this packet synchronizes its accepted successor, recorded in the report. All 30
provider Source/header fingerprints match the frozen readiness inputs.

## Verification and limits

Default ./scripts/build.ps1 completed with exit 0. Existing suites passed 3/3
(reconstructed_math, native_math_differential, tool_tests; 14.75 seconds).
No test case was added; these suites do not execute the new wrapper. The existing
unrelated duplicate-symbol LNK4006 warning remains in the build log.

The actual CMake I386 object was parsed and all six code sections reviewed:
203 bytes/94 instructions, consisting of the 141-byte/61-instruction wrapper
and 62 bytes/33 instructions in five raw helpers. All three REL32 relocations
bind the actual children; both exits use RET 4. Fourteen focused emitted-code
checks pass. No native execution, original ABI or game validation was performed.
The accompanying report records final three-row native and Markdown transfer
verification and address-owned ledger provenance.

The current SDK diagnostic is not original CRT encoded-handler/debugger/Watson
policy. Child heap, constructor, destruction and exception domains retain their
accepted CurrentCpp qualifications. Source private fill arguments and scratch do
not recreate native private-stack aliases or the unspecified tag bits. All reached
accesses must be valid and child loops finite; concurrent mutation is outside this
contract. Original heap/CRT/FH3/RTTI/SEH/fault handling, nested cleanup failures,
register/flag identity, binary ABI and game behavior remain unproved. Build and
emitted-object inspection establish Source properties, not native/game parity.
