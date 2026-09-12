# Retained sound source record

Addresses: `00A7C5F0`, `004C7FA0`, `004E7BB0`.

The actual 14h source record contains a retained sample pointer at +0, an
eight-byte native string at +4/+8, and floats at +C/+10. The new
`SoundRetainedSourceRecord` declaration describes this representation; operations
also accept actual byte storage so the retained channel variants can use +5C
directly. Names are descriptive hypotheses, not recovered symbols. No hidden
reference count, allocator, string capacity or implicit destructor is added.

| Offset | Representation | Producer evidence |
| --- | --- | --- |
| +0 | Sample pointer | `004ED910` acquires by the +4 name through `00A83FD0`, publishes/retains the result into the record, releases its temporary, and passes the record to `00A7F2F0`. |
| +4/+8 | Native string length/data | `00A7C5F0:00A7C63D..00A7C668` clears the header, resizes from the source length with preserve=1, and copies current destination length bytes from the current source buffer. |
| +C/+10 | Two float32 fields | `00A7C670/677` and `00A7C67C/67F` are sequential FLD/FSTP pairs. Semantic scalar names are left unresolved. |

`004ED910` advances its record pointer by 14h for three entries. This is a
source record, distinct from the actual 7Ch sample allocation established by
`00A85440`/`00A84D70` and the existing sample-cache reconstruction. All three
retained channel constructors (`00A7DA40`, `00A7DB80`, `00A7DCD0`) pass their
destination +5C and their source pointer to `00A7C5F0` at `00A7DAA9`,
`00A7DBE9`, `00A7DD39`. Their ordinary and scalar destructors consume
`004C7FA0` at `00A7DAF8/DB48/DC48/DC98/DD88/DDD8`.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| `00A7C5F0..00A7C692` copy constructor | ECX destination, stack source; EAX destination; RET4 at `00A7C690` | Complete normal body and sample unwind in the C++ exception domain. |
| `004C7FA0..004C801D` destructor | ECX record; RET at `004C801D` | Complete normal body; native pool-getter exception machinery is outside the explicit noexcept storage boundary. |
| `004E7BB0..004E7BF0` sample-slot assignment | ECX destination slot, stack source-slot address; EAX destination; RET4 at `004E7BEE` | Complete normal body; no local exception cleanup. |

The constructor clears destination +0 before reading source +0, publishes and
atomically retains a nonnull sample at its actual +4, then constructs the string.
The source header and its buffer are reloaded after allocator callbacks; no
source snapshot is substituted. The native `00BF7680` library body supports
backward overlap, so this copy uses `memmove` after the existing actual-header
resize helper. Zero-byte copies are omitted. The two float stores use inline
x87 instructions, including signaling-NaN quieting and load/store ordering.
Self-construction clears/abandons the original pointer and string, as native
does; it is not an assignment operation.

The destructor captures string data, captures length+1 with DWORD wrap, and
releases through `NativeStringStorage` without clearing that header. It then
reloads sample +0, performs the real atomic decrement at sample+4, and invokes
the required `GameplayEffectComponentLifetime::zero_references_slot_00` only
when the result is zero. That service dispatches the sample's current vtable+0
with ECX=sample and no stack arguments. The record slot is cleared after the
callback; a throwing callback leaves it uncleared. String-release callbacks
can change which sample is subsequently released.

The assignment helper captures source and destination pointers before comparing
them. Equal values have no effect. Otherwise it publishes the new pointer,
retains it, and releases the captured old pointer. It does not restore or clear
the destination after a zero callback, preserving callback changes. Besides
the retained-channel factory, its callers include `004ED2D0`, where a
`SoundEffect` property feeds `00A83FD0` and then this helper.

## Native calls and cleanup

| Containing function | Native site | Callee / contract |
| --- | --- | --- |
| `00A7C5F0` | `00A7C627` | IAT `00CE221C`, InterlockedIncrement(sample+4). |
| `00A7C5F0` | `00A7C653` | `0041DD40`, ECX name header; stack length,preserve=1; RET8. |
| `00A7C5F0` | `00A7C668` | `00BF7680`, destination/source/current destination length; caller ADD ESP,0Ch at `00A7C66D`. |
| `004C7FA0` | `004C7FD7` | `00419CC0`, no arguments, EAX pool. Three pushed release arguments remain on stack. |
| `004C7FA0` | `004C7FDE` | `00BD1510`, ECX returned pool; stack buffer,length+1,1; callee RET0Ch. |
| `004C7FA0` | `004C7FF5` | IAT `00CE2220`, InterlockedDecrement(captured sample+4). |
| `004C7FA0` | `004C8005` | Current sample vtable+0; ECX captured sample; no stack arguments. |
| `004E7BB0` | `004E7BCA` | IAT `00CE221C`, retain incoming sample+4. |
| `004E7BB0` | `004E7BD8` | IAT `00CE2220`, release captured old sample+4. |
| `004E7BB0` | `004E7BE8` | Current old sample vtable+0; ECX old sample; no stack arguments. |

Constructor handler `00CB51A8` selects FuncInfo `00DEB12C`; its one-state
unwind map `00DEB124` is `{-1,00CB51A0}`. The funclet loads saved destination
from [EBP-10h], then tail-jumps at `00CB51A3` to `004C3810`. It releases only
the current sample slot. C++ allocation failure reproduces this cleanup using
a local guard; a second exception during cleanup terminates. No string cleanup
is invented for the failed constructor.

Destructor handler `00C65288` selects FuncInfo `00D8D9D8`; map `00D8D9D0`
is `{-1,00C65280}`. Its tail at `00C65283` also reaches `004C3810` while
the native string stage is active. State becomes -1 before sample release.
The supplied storage interface makes release noexcept and abstracts the native
pool getter; native getter/SEH exceptions are consequently outside this host
interface. Original MSVC exception ABI and binary-call compatibility are not
claimed. These short funclets have no stored Ghidra functions; disk decoding
and live bytes establish their transfers without assigning them to a neighboring
function.

## Validation boundary

`reports/sound_retained_record.json` records inclusive final instructions,
lengths, full-span byte hashes, live/disk equality, original names and call sites.
Ghidra reads use the verified `C:/Users/sqz269/bsp.gpr` project and
`/battlestationspacific.exe`; this packet makes no Ghidra mutations.
`scripts/build.ps1` passed with MSVC Win32 /W4 /WX /fp:strict, including both
existing checks (`reconstructed_math`, `native_math_differential`) after
`verify-seeds`. A focused local manifested Win32 probe passed for deep string
copy, x87 signaling-NaN quieting and negative zero, retained-sample cleanup
after allocation failure, post-string-release sample reload, stale string
headers, and callback-visible assignment ordering. The local probe links this
packet's directly compiled source against the existing core library and is
not a native binary differential test. The call-site auditor verified all four
direct call rows; six IAT/vtable calls were inspected in their native listings.
These are new Win32 C++ interfaces; no FMOD playback, game behavior or
original-ABI replacement has been validated.
