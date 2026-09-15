# Native vehicle camera rows and counted array lifetime

## Reconstructed scope

| Entry | Inclusive end | Enclosing bytes | Native ABI |
| --- | --- | ---: | --- |
| `005CD070` | `005CD0E2` | 115 | ECX destination, stacked source, RET4, EAX destination |
| `005CD260` | `005CD354` | 245 | ECX data/count/capacity header, signed stacked capacity, RET4 |
| `005CD640` | `005CD703` | 196 | ECX header, signed stacked count, RET4 |
| `0078E230` | `0078E265` | 54 | ECX four-word destination, RET, EAX destination |

The source provides complete ordinary bodies through new Win32 C++ interfaces
in `native_vehicle_camera_array.hpp/.cpp`. Names are descriptive hypotheses.
Camera rows occupy 5Ch bytes: native string length/data at +0/+4, sixteen
raw matrix DWORDs at +8..47h, four scalar words at +48..57h, and a byte at
+58h. Bytes +59..5Bh are untouched padding. The vehicle Lua reader at
`009625CB` initializes the four scalars; its array resize at `00962840`
and the camera binder audit establish the row stride and field consumers.

## Copy and initialization

`005CD070` compares source/destination identity before clearing both
destination string-header words. A self-copy therefore orphans the previous
string storage; it is not an early no-op. For distinct rows it calls the
existing actual-header `0041DD40` with current source length and preserve=true,
then rereads source length. If nonzero, it captures destination length,
source data and destination data in native order before the byte copy.
The native `00BF7680` handles overlap; the source uses the same explicitly
bounded host `memmove` approach as the existing raw native-string source.
A zero byte count performs no buffer access. Exact CRT dispatch/fault access
order and native argument-frame aliasing are outside this boundary.

The tail copy uses the native REP MOVSD order for all sixteen matrix words,
then interleaved load/store pairs for +48h,+4Ch,+50h,+54h, then only byte+58h.
No x87 conversion, padding copy, item retention or destination destruction
is introduced. The source contract requires DF=0, as the surrounding CRT
interfaces do; it does not normalize DF.

`0078E230` reads the actual borrowed constant cells and writes one word before
reading the next. Live image bits are CE684C=`C0490FDB`, D7A264=`40490FDB`,
CE3CCC=`BFC90FDB`, CE3C64=`3FC90FDB`. These are approximately negative/positive
pi and half-pi, but the interface uses their raw words, not decimal floats.
MOVSS preserves signalling-NaN payloads and performs no arithmetic. Context
objects remain outside storage modified by these routines.

## Reserve and resize schedule

Reserve clamps the signed request to at least one and returns if the current
signed capacity is sufficient. It allocates wrapped DWORD capacity*5Ch using
the canonical `singleton_lifetime_allocate/free` source CRT boundary. Each
iteration rereads current count/data, skips a zero destination row address,
and calls the reconstructed row copy. It then releases old row strings in
forward order, rereading current header/count after calls, frees the current
backing pointer, and only then publishes replacement data followed by capacity.
The count is unchanged. There is no allocation-size validation or rollback.

Resize first reserves when the signed request exceeds capacity. Growth starts
at the current count, computing wrapped addresses and initializing only the
two string-header words and four scalars. Matrix, byte+58h and padding remain
untouched. Shrink decrements published count before computing each removed row
and returning its string storage; it reloads the count for the next comparison.
The final count write occurs after these loops, including signed-negative
requests. No bounds check or successful invalid-input substitute is added.

Both release loops compose `destroy_native_string_header_0041DD20` with the
existing `NativeStringRawPoolContext`. That implementation captures nonnull
data, then length+1, invokes the canonical singleton getter and BD1510 return,
and does not clear the row header. This matches the inlined original calls;
there is no hosted string object or fake pool callback.

## Exception and listing evidence

The native reserve handler C736C7 selects FuncInfo D9F65C and one unwind state
at D9F654: {-1,C736B0}. That action passes the failed range to `00401130`,
whose entire native body is RET. It does not free the replacement buffer or
destroy completed copies. Native reserve exceptions therefore leak those
resources; automatic rollback would change the observed ownership schedule.

Resize handler C73721 selects D9F6BC and states at D9F6AC:
{-1,C73700}, {0,C73719}. C73700 also calls RET-only00401130. C73719 jumps to
string destroy0041DD20 for the current growing row. The only call made with
that second state armed is the reconstructed no-throw raw MOVSS initializer.
Ordinary C++ propagation preserves the relevant allocation/pool failure
schedule; original FH3 machinery, hardware faults, native exception identity
and asynchronous exceptions are not reproduced by this source.

The false returning-free gap after `005CD32F` was repaired under the Ghidra
write lock, restoring `005CD334..005CD342` including data/capacity publication.
The enclosing 245-byte reserve span includes three skipped alignment bytes at
5CD2FD..5CD2FF; listed instructions occupy 242 bytes. Saved project and refreshed
exports use `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. The receipt
is `reports/native_vehicle_camera_array_flow_orch4.json`. No executable bytes
or global no-return flags changed.

## Validation boundaries

All 610 enclosing bytes matched the installed executable and live Ghidra;
all fourteen direct-call checks passed. The strict MSVC Win32 Release build
and all three existing CTests passed. A separate reviewer checked
all four complete listings, raw string-pool reuse, both unwind maps, copy
order and publication timing.

One ignored original/source lifecycle produced seven identical snapshots.
It executed all four original bodies with their internal edges intact,
binding external allocation/string-pool edges to real source operations.
A constructed, prepublished native pool handled nonempty `camera` and `end`
strings. It checked copied bytes, untouched growth bytes and padding,
self-copy header clearing, published count at shrink return, and the old
header still visible during free. Pass-through source observers supplied
matching event records. The compiled source hash matches this implementation;
the pool/CRT provider sources are unchanged from the fixture's build.

The fixture covers the published-pool fast path. It does not execute original
CRT allocation, lazy pool registration, native FH3 handlers, throwing calls,
hardware faults or gameplay. Its reproducible source, byte manifest, build
receipt and outcomes are archived in ignored
`local/worker_evidence/ship-indexed-and-camera-b2/`.

These helpers close the camera-array lifetime
dependency; they do not implement the full vehicle binder, native heap/ABI
replacement, original exception runtime or game validation.
