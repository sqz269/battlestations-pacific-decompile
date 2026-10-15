# Complete POD8 storage and iterator entries

Addresses: `00418700`, `00420AC0`, `00418710`.

| Native range, inclusive | Bytes | Coverage | Current Native definition | Source entry |
|---|---:|---|---|---|
| `00418700..0041870D` | 14 | complete bounded raw body | primary-created reviewed14B function | `allocate_native_pod8_storage_00418700` |
| `00420AC0..00420B06` | 71 | complete bounded raw body | primary-created reviewed71B function | `initialize_native_pod8_storage_00420ac0` |
| `00418710..00418734` | 37 | complete | existing `STL_inst_00418710` | `difference_native_pod8_iterator_00418710` |

The raw entries were recovered from the original executable's complete instruction
sequences, returns and surrounding INT3 padding, then matched against live memory.
They were not created, renamed or exported as new Ghidra functions by this worker.
After reviewing the complete bodies and byte/callee evidence, the primary created
both exact functions under the Ghidra write lock, saved physical prototypes and
preserved the earlier absence metadata. Their three transfer rows now receive full
live caller/callee validation. This establishes analysis boundaries, not Original
runtime use; the zero-reference census remains an explicit reachability limit.
Their descriptive storage-role names are provisional hypotheses, not recovered
symbols or evidence of a current caller. The existing iterator library identity,
category and prior records are preserved. No full vector/iterator class is invented.

## Allocation adapter and capacity initialization

The 14-byte adapter consumes no receiver: one uint32 count in the actual stack cell,
RET4 and EAX storage. MOV ECX,[ESP+4]; XOR EDX,EDX; call actual13390; RET4.
Source uses naked stdcall and preserves count0's real allocation(0) provider path.
The child consumes ECX count, no stacked input, returns EAX with plain RET.

The 71-byte initializer consumes ECX raw10h header plus one stacked uint32 count,
RET4, preserving ESI/EDI. Source fastcall adds an ignored EDX placeholder. Capture
count into EDI BEFORE header writes; CMP count,zero occurs before the three MOV
zero stores at +4/+8/+Ch, whose flags feed JNE. Proxy+0 is untouched. Count0
returns AL0/EAX0 without allocation. Existing pointer words are simply overwritten;
there is no old-storage free, element cleanup, owner replacement or rollback.

For nonzero count, compare UNSIGNED against1FFFFFFFh. Above it call the genuine
203A0 length-error entry. Then zero EDX, put captured count in ECX and call actual
13390. Publish returned pointer to first+4 and last+8; compute end with literal
LEA pointer+captured_count*8 moduloDWORD and store to+Ch. MOV AL,1 follows the
end store. Its return flag is in AL, not a normalized DWORD: upper EAX bits still
come from the computed endpoint. The Source bool interface consumes only AL.
No null-return guard or normalization is added. Any throwing provider sees the
already-zeroed header; no local EH/cleanup frame is invented.

## Iterator validation and difference

Existing 37-byte body: ECX actual left8B iterator, one stacked actual right8B
iterator pointer, RET4/EAX signed difference. ESI captures left address; read
left.owner+0, then capture right address into EDI before the validation branch.
If left.owner is zero, invoke invalid policy without reading right.owner. If
nonnull, compare right.owner and invoke invalid policy on mismatch. No null
guard for the actual iterator addresses is added.

If policy returns, reread CURRENT left.position+4 and right.position+4 through
the captured addresses; subtract moduloDWORD, restore EDI, SAR EAX,3, restore
ESI, RET4. No owner recheck or right-pointer argument-cell reload occurs after
the call. The real handler can change either position and the reloads observe
it. Source uses a signed32 result and an unused EDX binding placeholder. No
private iterator object, replacement state or injected success callback exists.

## Concrete calls, references and boundaries

| Raw/defined host | Transfer site | Native child | Source binding |
|---|---|---|---|
| raw14B `00418700` | `00418706` | `00413390` | accepted `allocate_native_pod8_records_00413390` |
| raw71B `00420AC0` | `00420AE6` | `004203A0` | accepted `throw_native_pod8_vector_length_error_004203a0` |
| raw71B `00420AC0` | `00420AEF` | `00413390` | accepted actual allocator above |
| existing `00418710` | `00418722` | `00BF6713` | concrete `invoke_native_invalid_parameter_00bf6713` |

The defined-function transfer is:

```text
00418722 CALL 00BF6713
```

The raw three CALL targets are proven directly by their original E8 rel32 bytes;
the worker initially labeled containment as uncreated. After primary function
creation, all four report rows are checked against live function ownership and
the physical CALL instructions. Historical raw metadata retains inclusive ends.
All live xrefs are empty, and a physical E8/E9 plus .text/.rdata/.data literal
address census finds ZERO current references to each of the three entries. No
consumer, indirect-call coverage or live entry use is inferred from that census.

The accepted allocator retains current malloc/new-handler/retry behavior and its
owning direct-multiplication-overflow Source transport. The length entry composes
the actual legacy message/28h owner and `NativeSingletonVectorLengthError` service.
These current catch/RTTI/CRT/frame identities are qualified, not original FH3 or
static-CRT exception identity. The invalid entry calls actual SDK
`_invalid_parameter_noinfo` with the accepted physical padding schedule. It may
return according to current CRT policy; Native encoded-handler/debugger/Watson
identity is not reconstructed here. No child is missing or substituted with a stub.

## Verification

Project/program checked: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
base00400000. Full disk bounds, live absence/existing prototype, all three bodies,
zero-reference census and actual child bodies/Source headers were inspected.
Strict MSVC Win32 `/MD /std:c++17 /EHsc /W4 /WX /permissive- /O2 /fp:strict /FAcs /c`
passes. `local/goal15h_pod8_storage_init/compare_bytes.py [COFF-object] [live-directory] [PE-path]`
compares all14+71+37B with fresh live and original disk bytes. Only FOUR verified
provider CALL operands are masked, after checking exact REL32 relocation symbols,
E8 opcodes and decoded original targets. Every other byte matches, including RET4;
raw14B/71B suffix padding is also checked. The script accepts the registered object.

One ignored actual-provider fixture at
`local/goal15h_pod8_storage_init/pod8_storage_probe.cpp` passes count0/nonzero AL,
genuine allocation/free, the owning length failure through the initializer with
header already zeroed, and direct adapter allocation. Through the real SDK API,
it temporarily configures/restores the actual current thread-local invalid handler
to change actual iterator position cells and return. The helper observes result-5
while owner remains invalid; a matched-owner call yields6. This is provider-domain
configuration in the fixture, not an injectable Source provider or callback API.

Run `local/goal15h_pod8_storage_init/run_probe.cmd [archive] [archive-only]`.
Default overlays this module on the primary existing Release archive; archive-only
uses the registered archive. Probe manifest is embedded. Primary owns full build,
registration, any new Ghidra functions/annotations/exports, and integration.
No new tracked test target, installed replacement, original class/template type,
Native differential, original SEH/hardware-fault or gameplay validation is claimed.
