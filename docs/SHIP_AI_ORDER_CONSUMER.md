# What a ship does with another ship's published navigation state

Addresses: 009D8CE0 009D8D2C 009E3C00 009E3DB0 00811D80 00815F30 006BC0C0 00415510 00415550
00415620 009F3E6D 009F3DD0 009EE649

Packet `cc_ai_throttle_ring`, worker `agent/cc-ai-throttle-ring`, 2026-09-12 UTC. Ghidra was
read-only for this packet: no renames, comments, prototypes, function creation or saves.
Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; every live query
verified both. Descriptive names are hypotheses, not recovered symbols.

## Answer to the packet question

**Of the four routines the follow-up named, only `009D8CE0` reads another unit's published
triple. It is a track-crossing predictor: it projects both units' headings into rays,
intersects them, and decides which of the two reaches the crossing point first. The other
three, `009E3C00`, `00811D80` and `00815F30`, work on the unit's *own* published record and are
a lateral-offset memory for the path follower, not a consumer of anyone else's state.**

The `this` `009E3C00` hands to `00811D80` is built from the navigator's own unit:

```
009E3DB1 MOV ECX,[ESP+2Ch]          ; the navigator
009E3DB5 MOV EAX,[ECX+3Ch]          ; its unit
009E3DB8 MOV EDX,[EAX+0B40h]        ; that unit's slot index
009E3DBE IMUL EDX,EDX,54h
009E3DC1 LEA ECX,[EDX+EAX+0A98h]    ; the unit's own published record
```

It is the same unit whose radius `009E3C00` reads at `+9C8h` (decompiled line
`*(float *)(*(int *)(param_1 + 0x3c) + 0x9c8)`). `00815F30` is likewise called on the record the
AI is writing this frame, at `009EE649` inside `009ED6B0`'s navigation arm. A unit does not read
its own bookkeeping out of somebody else's slot.

## `009D8CE0`, the crossing predictor

`void __thiscall(self)(void* candidate, void* other)`, `RET 8` at `009D913F`, body
`009D8CE0-009D9141`. `self` and `other` are the objects `009DA690` returns for a unit, not the
AI objects themselves: the caller gets both from that routine with `ECX = ai+60h`, which is
`blk`. On them, `+14h` is the unit, `+20h`/`+24h` the position, `+88h` the unit currently held,
`+8Ch` the crossing state and `+75h` a dirty byte.

Five guards first (`009D8CE0-009D8DCD`): `other` non-null, `candidate` non-null, `candidate` not
already `other->+88h`, `other->+88h` non-null, and `other`'s unit byte `+5Dh` clear. Any of them
failing writes `0` to `self+8Ch` and falls straight to the exit.

Both headings come from the published slot at `unit + 0A98h + 54h * [unit+0B40h]`, offset `+44h`
(`009D8D35` for self, `009D8D71` for the other), converted to a direction by `006BC0C0`
(`009D8D63`, `009D8DCE`). Self also reads its own `+40h` at `009D8DAA` and the other's `+48h` at
`009D8DB9`, which are the distance to the point being steered at and the path length left.

```
across   = (selfDir.z, -selfDir.x)                          ; self's across-track axis
lateral  = dot(otherPos - selfPos, across)                  ; 009D8E52 masks |lateral|
closing  = lateral - dot(otherPos + otherDir - selfPos, across)
if (|closing| <= 0.001f) return                             ; 009D8ED6, the tracks are parallel
t        = lateral / closing
cross    = otherPos + t * otherDir
selfRng  = dot(cross - selfPos,  selfDir)
otherRng = dot(cross - otherPos, otherDir)
```

The gate at `009D8FD3-009D9036` is a disjunction. The first half is fully resolved:

```
|lateral| < [otherUnit+9CCh] * 4.0        ; 009D8FDE, the double 00D7A328
```

The second half compares each range against a nested minimum built at `009D8F80-009D8FCB`:

```
selfRng  < min( min(otherRadius * 2.5, 400.0f), [ESP+3Ch] ) ; 009D8F8C, 009D9002
otherRng < min( min(selfRadius  * 2.5, 400.0f), otherSlot+40h ) ; 009D8FC6, 009D9027
```

`2.5` is the double at `00CE3DE0` and `400.0f` is `00CFD710`, loaded into `XMM0` at `009D8EEA`
and parked at `009D8F50` for the first minimum and reloaded at `009D8F9E` for the second.
`otherSlot+40h` reaches `[ESP+48h]` at `009D8E05`. The fourth operand, `[ESP+3Ch]`, is the one
loose end: no instruction in the function's stored listing writes that slot, so either the
listing is incomplete over the range or the image really reads an uninitialised local there.
The reconstruction therefore takes the whole disjunction as one caller-supplied predicate
rather than assert a value for it.

Past the gate:

```
margin = selfRadius / 1.8                                   ; 009D9044, the double 00D049A8
reach  = max(100.0f, selfRadius)                            ; 009D903C, 009D905A
if (otherRng + reach > otherPathLeft) -> state 1            ; 009D9073
if (selfRng <= 0)  conflict = otherRng > 0                  ; 009D908D
else if (margin + selfRng <= selfDistance) conflict = true  ; 009D9099
else -> state 1                                             ; 009D90A1
if (!conflict) -> state 0
if (self+8Ch was 2) selfRng -= margin * 0.5                 ; 009D90EC, the double 00D7A280
state = (otherRng <= selfRng) ? 1 : 2                       ; 009D906D
```

State `1` is the arm the routine falls into whenever the other unit will not get there in time
or self's own range is out of reach; state `2` only happens when self reaches the crossing
first. When the state is `2` the routine swaps the unit it holds for the other unit's held unit
(`009D90C7`), otherwise it keeps `candidate`, and it always sets `self+75h`.

`009D8CE0` does have a caller, contrary to the follow-up's note: Ghidra records an unconditional
call from `009F3E6D`. That site is in a routine Ghidra has not defined, `009F3E30-009F3E77`,
which sits between `009F3DD0`'s `RET` at `009F3E29` and the `int3` run at `009F3E78`. Scanning
`.text`, `.rdata` and `.data` for the absolute bytes `E0 8C 9D 00` finds nothing, so nothing
takes the routine's address and that relative call is the only reference.

```
009F3E30 PUSH EBX; PUSH ESI; MOV ESI,[ESP+0Ch]; TEST ESI,ESI; MOV EBX,ECX; JE 009F3E73
009F3E3E LEA ECX,[EBX+60h]; CALL 009DA690        ; blk -> EDI, self
009F3E60 CALL 009DA690                           ; -> EAX, other
009F3E65 MOV ECX,[ESP+14h]                       ; the second stack argument, candidate
009F3E69 PUSH EAX; PUSH ECX; MOV ECX,EDI; CALL 009D8CE0
009F3E75 RET 8
```

## `00815F30` and `00811D80`, the lateral-offset memory

A writer/reader pair over the two `1Ch`-byte sub-records of the published order record
(`bsp/ship_ai_navigation.hpp`, `UnitAiOrderSubRecord`). Field names below are that header's.

`00815F30`, `void __thiscall(record)(const float* xz, int direction, float low, float high)`,
`RET 10h`, body `00815F30-008160AC`, complete:

```
record+04h = 2.0f                                           ; 00815F43, 00CE3958
if (sub_a.flag_0c && |sub_a.pos - xz|^2 > 400.0)            ; 00815F63, the double 00CE3D90
    sub_b = sub_a; reset sub_a with field_08 = 1000.0f      ; 00CE3804
sub_a.flag_0c = true; sub_a.pos = xz; sub_a.field_18 = direction
if (direction == 1) { sub_a.field_04 = low;  sub_a.field_08 = high }
else                { sub_a.field_04 = -low; sub_a.field_08 = -high }
sub_a.value_00 = clamp(sub_a.value_00, into that pair, in the order it now has)
```

`00811D80`, `float10 __thiscall(record)(const float* xz)`, `RET 4`, body `00811D80-00811E7C`,
complete:

```
result = 30.0f                                              ; 00811D83, 00E0E304
for sub in {a, b}:
    if (sub.flag_0c && |sub.pos - xz|^2 < 400.0)
        if (sub.field_08 >= 0) result =  clamp(record+00h, sub.field_04, sub.field_08)
        else                   result = -clamp(record+00h, sub.field_08, sub.field_04)
                               and sub_b's negative arm returns at once   ; 00811E5F
```

The value being clamped is always `record+00h`, the blended value `0080E000` slews toward
`sub_a.value_00`: `ECX` is never reloaded in sub_a's arm after `00811D8C MOV ESI,ECX`, and
sub_b's two arms reload it explicitly with `MOV ECX,ESI` at `00811E49` and `00811E66`.

`009E3C00` uses the answer to push a waypoint sideways:

```
009E3DCD  r = 00811D80(ownRecord, &waypoint)
009E3DD9  waypoint.x += r * [zone+10h]
          waypoint.z += r * [zone+14h]
```

with `zone = [node+10h]`, the object hanging off the path node. So the pair remembers, per
position, how far off a waypoint this ship decided to pass, and reapplies it while the ship is
within 20 units of where the decision was made.

## Coverage

| routine | coverage |
| --- | --- |
| `009D8CE0` | partial: `009D8CE0-009D8DCD` (guards) and `009D8DCE-009D8F7C` (geometry) complete; `009D8FD3-009D9136` projected with the range half of the gate left as a caller predicate; `009D8F80-009D8FCB` unresolved |
| `00815F30` | complete |
| `00811D80` | complete |
| `009E3C00` | not reconstructed. Read for `009E3D8C-009E3DE8` only: what it passes to `00811D80` and what it does with the answer. The tree walk at `009E3C4A-009E3D81` and the whole tail past `009E3E07` are unread |
| `006BC0C0` | complete |

## Corrections

| what said it | what is true | evidence |
| --- | --- | --- |
| `docs/UNIT_AI_ORDER_SLOT_READER.md` follow-up `ship_ai_order_consumer`: "`009D8CE0`'s caller has to be found first; it has none in the call graph" | It has one, `009F3E6D`, an unconditional call recorded by Ghidra. No data reference to the address exists anywhere in the image | `python tools/bsp.py ghidra xrefs 009d8ce0`; `python tools/bsp.py scan-bytes "e0 8c 9d 00"` returns no matches |
| the packet brief frames `009E3C00`, `00811D80` and `00815F30` as consumers of another ship's published triple | All three address the unit's own published record. `009E3DC1` builds the record address from the navigator's own unit and its own `+0B40h` index | `009E3DB5`, `009E3DB8`, `009E3DBE`, `009E3DC1` |
| `docs/UNIT_AI_ORDER_SLOT_READER.md`: "The meaning of the two `1Ch`-byte sub-records is unread" | They are a two-slot, position-keyed lateral-offset memory: `+0Ch` and `+10h` are the bounds, `+08h` the held value, `+14h` the live flag, `+18h`/`+1Ch` the position, `+20h` the direction | `00815F30` and `00811D80` above |

## Uncertainties

1. `[ESP+3Ch]`, the fourth operand of `009D8CE0`'s gate, has no writer in the function's stored
   listing. Every other operand of the four `00415510` calls is resolved above. A flow-repair
   pass on `009D8CE0` would settle whether the listing is missing a block.
2. What `self+8Ch` is used for is unknown. `009D8CE0` is its only writer this packet found, and
   its reader was not looked for.
3. `009DA690`, which produces both objects `009D8CE0` works on, was not read. Until it is, what
   `self` and `other` are beyond "`+14h` is a unit" is a guess.
4. `record+00h`, the value `00811D80` clamps, is maintained by `0080E000`, which this packet did
   not read. `bsp/ship_ai_navigation.hpp` carries the rate from packet `cc_ai_order_hop`.
5. `[zone+10h]` and `[zone+14h]` in `009E3C00` are a two-float offset on the object at
   `[node+10h]`. Its producer was not read.

## Follow-up packets

| packet | addresses and files | what is left |
| --- | --- | --- |
| `ship_ai_crossing_gate` | `009D8CE0` `009D8E00..009D8FCB`, `00415510`, `self+8Ch` | Trace the x87 stack through the geometry so the three unresolved minimum operands come out, and find the reader of `self+8Ch`. That is what turns the crossing predictor from a shape into a rule |
| `ship_ai_path_follower` | `009E3C00`, `009D5930`, `009D6550`, `004218E0`, `009D9E50` | The path tree at `navigator+20h`: the node walk, the avoid-zone singleton and the tail that writes `navigator+60h`/`+64h`. `00811D80`'s contract is settled, so the remaining work is the walk |
| `unit_ai_order_blend_0080e000` | `0080E000`, `record+00h`, `record+04h`, `00815F30` | Who advances `record+00h` toward `sub_a.value_00`, and what the `2.0f` timer at `+04h` gates. `00811D80`'s answer is meaningless without it |

## no_ghidra_function

| start | end (inclusive) | evidence |
| --- | --- | --- |
| `009F3E30` | `009F3E77` | `009D8CE0`'s only caller. Start: six `int3` at `009F3E2A..009F3E2F` after `009F3DD0`'s `RET` at `009F3E29`, then a `PUSH EBX; PUSH ESI` prologue at `009F3E30` with `MOV EBX,ECX`. End: `RET 8` at `009F3E75`, then `int3` at `009F3E78` onward. Ghidra reports no function at or containing `009F3E6D` and names `009F3DD0` only as the enclosing candidate; `009F3DD0`'s own body is `009F3DD0-009F3E29` |
| everything else | | `009D8CE0-009D9141`, `00815F30-008160AC`, `00811D80-00811E7C`, `009E3C00-009E432A` and `006BC0C0-006BC111` all have Ghidra function bodies the bridge reports. No other routine in this doc was read from the raw listing and no other boundary is claimed |
