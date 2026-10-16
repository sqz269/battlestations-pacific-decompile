# Property lookup ordered raw accesses

| Native routine | Span | Coverage |
| --- | --- | --- |
| `0043B760` | `[0043B760,0043B7B9)` 89B/39ops | whole body at current qualified CRT boundary |
| `0043B8B0` | `[0043B8B0,0043B919)` 105B/48ops | whole body at current qualified CRT boundary |

General Source readiness remains false. This packet strengthens the existing
ordinary property providers' raw access schedule; it does not close Original
changed-locale, negative-byte, errno/TLS or unrestricted parent admission.
The existing enum helpers and property publication fragment are unchanged.

The hash receives one stacked actual eight-byte header and returns EAX with
RET4. It captures data+4 before length+0 through property-only volatile DWORD
reads. Its separate unsigned hash, remaining count and fixed
`(length|20h)>>5` step reproduce the whole native loop. Each sampled byte is a
volatile signed BYTE promoted into real current SDK `toupper`; only its low
return byte enters the modulo-DWORD mix. Cursor advancement is explicit
wrapping Win32 address arithmetic, matching native ADD EBP,1. Null data selects
the distinct actual E177E4 role; zero length performs no byte/CRT read.

The lookup receives ECX map and two stacked query/output pointers, returns a
borrowed node/null in EAX and uses RET8. The new raw overload accepts actual
`void* actual_bucket4`; the prior `uint32_t&` interface delegates to it.
The actual map and query pointer values are captured as Source parameters;
there is no implicit Source/native parameter-home or private-frame alias.

The raw body captures the hash, stores the actual output DWORD, then reads the
head using that captured hash, never by rereading the output. A nonnull head
allows one fresh query-length capture after publication. Every node-length
comparison is current; a length match reads node.data before current query.data.
Each null data value selects its distinct actual fallback role. Pointer identity
succeeds before both native null gates and real current SDK `_stricmp`.
Only comparison zero succeeds; failures reload the current next word at+C.
The query length stays captured across all subsequent candidates and calls.
Mapped+8, allocator+10 and map count are untouched.

Bucket publication may alias map/query/node payload. Its effects are visible
to subsequent actual reads, including the head and captured query length.
Valid reached DWORD/BYTE storage, representable Win32 addresses and current
string/sample extents remain required. Ordinary C++ word alignment/lifetime,
stable/disjoint binding metadata and the current CRT boundary remain qualified;
arbitrary faulting/unaligned/private-stack identity and concurrent data races
are not established. No alias guard or input normalization is introduced.

Correct bucketing, a recorded-length/strlen match, NUL-free payloads, globally
constant borrowed field values and a nonaliasing bucket are not native tests.
They are no longer imposed as algorithm admission. Hash samples need readable
backing; a reached CRT comparison needs its actual NUL-terminated spans.
Pointer identity can return without reading their bytes. Native traversal is
uncapped; a hit or terminal next gives termination, while cycles are not repaired.
The existing sampled-byte ASCII/current-C-locale ctype qualification stays.

| Containing routine | Native listing | Contract |
| --- | --- | --- |
| `0043B760` | `0043B789 CALL 00BF924E` | signed BYTE promoted to int; low AL used; caller ADD ESP,4 |
| `0043B8B0` | `0043B8BA CALL 0043B760` | stacked actual query header; callee RET4; captured EAX bucket |
| `0043B8B0` | `0043B8E2 CALL 00419CA0` | ECX query; complete13B accessor expanded as current data+4/fallback |
| `0043B8B0` | `0043B8F5 CALL 00BF7FBF` | query,node strings; caller ADD ESP,8; zero result alone consumed |
| `008F2260` | `008F22D4 CALL 0043B8B0` | dot actual bag+4/header/parameter bucket; callee RET8 |
| `008F2260` | `008F237B CALL 0043B8B0` | no-dot actual bag+4/header/parameter bucket; callee RET8 |

These selected parent rows record inspected incoming bindings, not exhaustive
caller admission. No unowned caller annotations are changed. The typed wrapper
is a new Source convenience edge, not a claimed extra native delegation edge.

The actual 39B toupper wrapper tests Original global109DE1C. Its initial-zero
branch changes only signed inputs a..z; its other branch invokes BF9136 with
the signed integer and null locale. BF9136 has no EOF shortcut. For -1 in a
single-byte locale, BF91E4/BF91E9 writes errno=2Ah before its mapping call.
Installed SDK 26100 (selected by the worker build) and 22621 instead return EOF
before locale/errno work in `common_tox_l`. Therefore no unsigned input cast,
assumed AL equality or current-C-locale assertion closes Original semantics.
This packet adds no hand-built Original fast path. Actual CRT imports remain
the provider boundary, with Original locale/TLS/tables/mapping/errno parity open.

Original BF7FBF selects unsigned-byte ASCII comparison when its locale-change
flag is zero, otherwise locale-aware comparison. The latter needs Original
LocaleUpdate/tolower/table transport, not a guessed locale-independent compare.
The two explicit fallback roles remain genuine supplied bindings; no new
cell, callback, semantic string/container, stub, capacity clamp or rollback exists.

Live full-body bytes match the installed PE across all194B. The JSON report
records their full bytes/hashes, exact original ABI, current Source/header pins,
prior comments, typed transfers and verification status. Corrected Source at
`eb6f4dcd5` passed the normal MSVC Win32 build and both enabled existing checks
in 7.46 seconds; native differential fixtures were not enabled. Primary whole
87-op and complete13B accessor review passed after the cursor correction.
The emitted raw-overload body stores output at+63, loads head at+68 using its
captured ESI bucket, then captures query length at+70. It does not reload the
bucket output. Node/query data reads are ordered at+7E/+83; failed comparison
reloads next at+A9. Actual toupper/stricmp import relocations remain. These are
current production COFF observations, not native replacement/ABI proof.
Locked annotation preserved names/comments, saved the original project and
refreshed both exports; prior values are in
`local/ghidra-annotations-20261015T235321Z.json`. Existing module registration
is reused; no CMake addition is needed. No new tests,
Original binary entry, private CRT/SEH/ABI, fixture or game proof is claimed.
