# Bounded raw scene-property lookup

| Native entry | Source or mapping | Span | Coverage |
| --- | --- | --- | --- |
| `008F2260` | `find_native_scene_property_bag_008f2260` | `[008F2260,008F23D7)` 375B/123ops | whole control flow over stated bounded providers |
| `00CA4770` | completed actual-key guard, state0 | `[00CA4770,00CA4778)` 8B/2ops | qualified action semantics only |
| `00CA4778` | completed actual-key guard, state1 | `[00CA4778,00CA4780)` 8B/2ops | qualified action semantics only |

General `source_ready` is false. This implements every branch of the parent
using actual existing Source providers, while inheriting their admission. It
does not establish unrestricted parent raw/locale/alias completeness or all
101 indexed caller functions. Those upstream admission obligations stay open.
The old std::string/vector `ScenePropertyBagModel` lookup remains separate.

The existing whole raw map provider admits a live 108h map, finite correctly
bucketed stable chains, matching closed NUL-free ASCII headers/buffers, null
data only when empty, stable nonnull NUL fallback roles and current C locale.
Its bucket output must be writable and nonaliasing with its borrowed lookup
storage. The parent inherits these requirements, valid reached C strings,
records, scratch extent, recursion/frame cells and current raw pool releases.
It adds no input normalization, capacity clamp, null/type recovery, callback,
stub, lock, private string copy or blanket concurrency/reentry guard. Outside
the provider's stable lookup interval, real shared-scratch/publication effects
are retained when all reached contracts remain valid.

Native ABI is ECX bag, one live stacked key DWORD at entry P+4, RET4/EAX
record-or-zero. Capture key before strchr and retain the first-dot address.
No dot constructs the actual eight-byte key header from that captured input.
Dot copies the head into actual shared F89658, writes its terminating byte,
then constructs the same header from the shared buffer. The native BF7680
boundary explicitly detects destination inside source and selects backward
copy at BF7844 (STD/REP MOVSD/CLD). Source uses actual current `memmove`,
including a zero-byte copy, without a new non-overlap requirement.

The key constructor zeros its own actual header before measuring text. No
caller initialization or semantic NativeString owner is substituted. Only a
successful constructor return arms state0 (dot) or state1 (no dot). Map lookup
uses actual bag+4 and writes its full bucket into the original parameter word.

On a dot hit, capture mapped node+8 BEFORE writing only the parameter's low
byte to1. On a miss write only that byte to0, THEN capture the full resulting
partial word. Dot cleanup captures current header data, disarms, and only
for nonnull data reads current length+1 and resolves the actual pool. Captured
block/size survive getter activity; return uses the live shutdown gate. A
throwing getter sees an unarmed guard and is not retried. After returning
cleanup, reread CURRENT low byte. Only if nonzero read captured record+4; type6
loads the nested bag POINTER from record+Ch, then tail=dot+1 is stored into the
reached child parameter before real recursion. No record+Ch address substitution.

No dot captures mapped node+8 on a hit. A miss still reads the current bucket
DWORD before the native AL/NEG/SBB/AND mask returns0. The final whole result
is captured before cleanup and returned unchanged afterward. Unlike the dot
branch, no-dot cleanup disarms at2393 BEFORE capturing header data at23A1.
Both branches capture nonnull length+1 after disarming, then resolve the pool
and return the captured block/size without an unarmed retry. This branch does
not introduce a parameter-byte store or post-cleanup success recheck.

Handler CA4780 resolves FuncInfo DD7360 (magic19930522, maxstate2, mapDD7350).
States0 and1 each transition directly to-1 through CA4770/CA4778. Both actual
actions are LEA ECX,[EBP-14h]; tail JMP0041DD20. Source maps them to the armed
actual-header guard, disarming before that direct real cleanup helper. Failed
construction is unarmed. Host C++ unwinding, including a throwing cleanup
during another exception, remains the qualified transport; no original FH3,
SEH, private action entry or private-stack exception ABI is claimed.

Bindings are explicit: actual parameter4 at P+4..P+7, key header8 at
P-14h..P-Dh, real shared scratch F89658, distinct property-node/query fallback
roles E177E4/E17654, and NativeStringRawPoolContext borrowing publicationAA8,
return gateAA4 and canonical raw-manager publicationAA0. Reached recursive
entry is P-2Ch, its parameter P-28h..P-25h and its key header P-40h..P-39h.
Only the reached recursive path inspects child metadata. Metadata/binding
addresses stay stable/disjoint from described payload; no hidden physical
Source/native argument-home or arbitrary private-frame aliases are supplied.

| Containing routine | Native listing | Contract |
| --- | --- | --- |
| `008F2260` | `008F2285 CALL 00BF86F0` | key,dot character; caller ADD ESP,8 |
| `008F2260` | `008F22A2 CALL 00BF7680` | actual shared head,input,head bytes; caller ADD ESP,0Ch; overlap-capable |
| `008F2260` | `008F22BA CALL 0041E870` | actual8B header/shared head; callee RET4 |
| `008F2260` | `008F22D4 CALL 0043B8B0` | actual bag+4,key header,actual parameter bucket; callee RET8; state0 |
| `008F2260` | `008F230B CALL 00419CC0` | no consumed arguments; three captured return words remain stacked |
| `008F2260` | `008F2312 CALL 00BD1510` | actual pool,block,size,unused1; callee RET0Ch |
| `008F2260` | `008F232B CALL 008F2260` | loaded nested bag pointer,tail; callee RET4 |
| `008F2260` | `008F2361 CALL 0041E870` | actual8B header/captured original key; callee RET4 |
| `008F2260` | `008F237B CALL 0043B8B0` | actual bag+4,key header,actual parameter bucket; callee RET8; state1 |
| `008F2260` | `008F23B4 CALL 00419CC0` | no consumed arguments; captured return words remain stacked |
| `008F2260` | `008F23BB CALL 00BD1510` | actual pool,block,size,unused1; callee RET0Ch |
| `00CA4770` | `00CA4773 JMP 0041DD20` | state0 actual header cleanup; Source guard mapping only |
| `00CA4778` | `00CA477B JMP 0041DD20` | state1 actual header cleanup; Source guard mapping only |

The previous preserved comment placed strchr at228A and described recursion
as record+Ch. Actual instructions place the transfer at2285 and load the bag
pointer from that field. Those corrections append evidence rather than erase
old comments. This packet does not assert exhaustive incoming-caller admission.

All providers are concrete existing raw header/map/pool implementations or
actual current CRT services. The map domain is bounded, not a named missing
body; unrestricted upstream admission is still unproved. Exact live/installed
spans, Source/provider hashes, original ABI and prior annotation evidence are
recorded in JSON. Required `scripts/build.ps1` passed at registered revision
`f0d3fc0d9`, targeting MSVC Win32. Both enabled existing checks passed in
6.94 seconds: math and tool tests. Optional native differential fixtures are
not enabled in this worker build. The existing unrelated LNK4006 duplicate
`spawn_request_id_matches` warning remains. Locked annotation/export awaits
the held 8F ledger turn; Source and build evidence do not bypass that boundary.
No new tests are added. Original
static-CRT heap/errno/handler/locale, RTTI/FH3/SEH/register/fault/binary identity,
new-entry native differential, application and game proof remain unclaimed.

Primary review accepted the full 123-op parent and both two-op actions against
the complete Source/header at `ab49fb7b5`, after correcting the distinct normal
cleanup orders. Reviewed hashes were asserted unchanged before recording the
receipt. This accepts whole control flow at the explicit bounded provider and
host-guard admission; it does not discharge the open unrestricted-parent or
101-caller obligations.
