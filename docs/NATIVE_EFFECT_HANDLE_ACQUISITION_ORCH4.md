# Native effect-handle acquisition prerequisites

Addresses: 0086B650, 0086F930, 00869990, 0086EBE0, 0086AB70.

Packet `orch4_effect_services_g7` closes the raw signed integer/pointer tree
providers used by the gameplay-effect manager cache. It does not add an effect
acquisition callback, a host map, or a cache-hit-only substitute for008700E0.
Names remain descriptive hypotheses rather than recovered symbols.

## Completed raw providers

| Address | Span | Original ABI | Source behavior |
| --- | ---: | --- | --- |
| `0086B650..0086B6B8` | 105 bytes | ECX tree; stack output iterator/key pointer; EAX output; RET8 | Signed lower-bound, fresh reverse comparison and owner/node publication; a miss returns the current sentinel. |
| `00869990..00869A18` | 137 bytes | ECX mutable owner/node iterator; RET | Checked predecessor. End moves to maximum; ordinary nodes use left/right-parent traversal. The CRT invalid-parameter service may return. |
| `0086AB70..0086ABAE` | 63 bytes | five stack inputs left/parent/right/pair/color; EAX node; RET14h | Allocate18h, copy links, signed key and unowned definition pointer, write color/nil and preserve padding16/17. |
| `0086EBE0..0086EDCB` | 492 bytes | ECX tree; stack output/left-byte/parent/pair; EAX output; RET10h | Unsigned count limit1FFFFFFEh, source-layout length error, allocation, current-count increment, link and red-black repair. Output node is published before owner. |
| `0086F930..0086F9E8` | 185 bytes | ECX tree; stack output result/pair; EAX output; RET8 | Signed unique search, checked predecessor, insertion or duplicate publication. Duplicate payloads are neither retained nor replaced. |

The actual tree begins at manager+4. Its header is opaque allocator word0,
sentinel pointer4 and unsigned count8. Nodes are18h bytes: links0/4/8, signed
key0C, weak definition pointer10, color14, nil15 and untouched padding16/17.
The source uses that storage directly. It reuses the already completed genuine
rotations00869810/0086A2F0, singleton allocation, legacy SBO/logic-error storage
and native invalid-parameter boundary. No `std::map` or parallel owner exists.

The length branch assigns the exact 19-byte `map/set<T> too long` message,
constructs the existing native-layout D69260 length error, and destroys the
completed temporary on unwind. The source C++ throw personality and RTTI are an
adaptation; original FH3 throw/catch identity is not claimed.

## Manager domain and acquisition boundary

The raw manager lifecycle was already complete before this packet:

- `004C1650` uses actual manager publication `01090AA0`, actual effect
  publication `00F87664`, current critical-section depth, raw10h allocation,
  constructor `00870370` and singleton registration.
- `00870370` builds the actual head/count representation through `0086AC00`;
  `00869C50` is its publication-reset unwind.
- `0086FE20` and `008703E0` erase/free the raw tree, clear publication and
  perform deleting-dtor storage release through the existing genuine providers.

The older `GameplayEffectManager` acquisition interface is a host projection:
it contains `std::optional<std::map<int32_t,void*>>`, and its Lua path uses
`GuiLua51Host` registry handles. Native `004C1650` returns a raw10h owner whose
map is the header above, while native Lua objects are14h tracked stack objects.
Reinterpreting either representation would corrupt storage. This packet closes
the raw cache mechanics instead of wrapping that incompatible projection.

## Original call edges

`0086B650` is called at `00870142`, `0086EF33` and `00870D49`.
`0086F930` is called at `008702A9`. Its own calls are `0086F98E -> 0086EBE0`
and `0086F9B0 -> 00869990`. Other checked-predecessor users are
`0086A5A3` and `0086AB63`. `0086EBE0` calls allocation at `0086EC64` and
the existing rotations at `0086ECF5`, `0086ED13` and `0086ED41`.

The length-error arm calls `00408720`, `00411700` and `00BF6885` at
`0086EC23`, `0086EC35` and `0086EC4C`. Raw node allocation calls
`00BF681B` at `0086AB72`. Checked predecessor reaches `00BF6713` at
`00869998`, and tail-transfers there at `008699B3` and `00869A0F`.
The report carries machine-checkable `address`/`native` rows for these edges.

## Remaining genuine acquisition closure

`00870CD0` remains source-absent. Its 42-byte body gets the actual manager and
forwards ECX output, EDX signed ID and the stack flag to008700E0. A faithful
wrapper therefore requires the complete raw008700E0 miss path, not only this
now-complete cache tree.

The raw miss path must compose the existing14h native Lua objects and actual
string-pool context with00870400. Five component families still block complete
component virtual dispatch:

| Family | Lua reader | Scalar deletion |
| --- | --- | --- |
| Particle | `00871D00..00871F90` | `0086BC60..0086BC7D` |
| Tracer | `00858700..00859231` | `0086B9A0` (stored body metadata absent) |
| WaterTracer | `0086D180..0086E591` | `0086D080..0086D09D` |
| Flare | `0086C240..0086CD0A` | `0086E870..0086E88D` |
| ThunderStorm | `0086F3B0..0086F8F5` | `0086FF00..0086FF1D` |

The readers are large, include register-sensitive component/resource work and
need separate Astra packets. This packet does not lease, name or claim them.
`0041DE40` remains the completed genuine one-word refcounted-handle release used
by damageable-class cleanup; no replacement owner or host reference map is added.

## Evidence and validation

All five complete spans have matching live-Ghidra and disk bytes. Their SHA256
values and exact inbound/outbound call rows are in
`reports/native_effect_handle_acquisition_orch4.json`. Ghidra was read-only;
there were no names, comments, body or flow mutations.

The strict MSVC Win32 build compiles the five providers into `bsp_core`. This is
source/build validation of raw tree behavior. It is not native differential,
FH3, concurrent-mutation, full effect acquisition, gameplay or runtime proof.
