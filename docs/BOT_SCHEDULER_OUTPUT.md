# Where the bot scheduler's plan leaves the AI object (packet `cc_unit_orders_apply`, part B)

Addresses: 00911E80, 00912A60, 00914390, and read-only 00914EF0, 0090EDE0, 0076A9F0, 0075B430,
00910570, 00911C00, 00911CE0, 00914270, 0090CA30, 0090D220, 00907240, 00910F10, 00907D40,
005070C0.

Worker `agent/cc-unit-orders-apply`, 2026-09-11 UTC. Ghidra was read-only for this packet.
Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. Descriptive names are
hypotheses, not recovered symbols.

## Answer to the packet question

**The bot scheduler at `game+21A0h` never writes a unit or a controller.** Its three callees
contain no virtual call at all - `00911E80`, `00912A60` and `00914390` together decompile
without a single `(**(code **)...)` site - and no store outside the scheduler's own eight
per-side records. It is a strategy layer that keeps score and publishes named demands; it does
not issue ship orders, and it is not a producer of `unit+980h`/`+984h` or of any other unit
command field.

The plan leaves the AI object by exactly four routes:

1. **The demand list at `record+78h`** (count `record+80h`), cleared and rebuilt every think
   tick by `00911E80`.
2. **The goal list at `record+88h`** (count `record+8Ch`), cleared and rebuilt every think tick
   by `00912A60`.
3. **Six category totals at `record+1C8h..+1DCh` and their sum at `record+1E0h`**, recomputed
   every retarget tick by `00914390`.
4. **Session message `15h`**, one per side per retarget tick when `game+1FE4h == 1`: a stack
   object with vtable `00D035B8` carrying `record+1E0h`, the literal `12h` and the side index,
   handed to `0076A9F0`. This is the only value that leaves the process.

Alongside those, both think-tick routines call `0090EDE0`, the general in-mission award grant
(`docs/AWARD_GRANT.md`), with `record+250h` as the player slot and a token string as the
counter name. That is statistics, not command.

## The scheduler tick, `00914EF0` (read only, correcting the earlier record)

`__thiscall(this = game+21A0h, float delta)`, `RET 4`. The eight per-side records are a plain
array in the scheduler:

```
record[i] = this + 4 + i * 284h       ; 00915026 LEA ESI,[EBX+4]; 0091503E ADD ESI,284h
```

Eight records end at `this+1424h`, inside the `14B8h` allocation. The scheduler's own scalars
follow: `+1494h` think countdown, `+149Ch` retarget countdown, `+14A4h` enable byte,
`+14A8h`/`+14ACh` the two reload periods, `+1470h` a demand threshold read by `00911E80`.

The retarget pass walks the same array through a pointer biased by `1E0h`:

```
EDI = this + 1E4h                     ; 00914F88, that is record[0] + 1E0h
loop i = 0..7:
    if (this+14A4h) 00914390(ECX = EDI - 1E0h)      ; 00914F99/00914F9F
    if (game+1FE4h == 1):
        message.payload = [EDI]                      ; record[i]+1E0h, the aggregate
        message.code    = 12h
        message.side    = i
        0076A9F0(ECX = game+1EF0h, &message, 0)      ; 00914FD1
    EDI += 284h
```

The think pass is the plain array walk with `00911E80` then `00912A60` on each record.

## `00914390`, the per-side asset accounting

`void __fastcall(record)`, body `00914390..00914EE9`. It zeroes six counters and refills them
from four nested list walks over the record, then sums them.

| record offset | decompiler index | contents |
| --- | --- | --- |
| `+1C8h` | `[0x72]` | sum of `node+14h` over one list |
| `+1CCh` | `[0x73]` | sum of `node+14h` over a second list |
| `+1D0h` | `[0x74]` | sum of `node+14h` over a third list |
| `+1D4h` | `[0x75]` | `00910570(node+0Ch) * node+10h` over the list at `+DCh`, plus a plain `node+14h` sum |
| `+1D8h` | `[0x76]` | sum of `node+14h` over a fifth list |
| `+1DCh` | `[0x77]` | `max(00914270(name), 0)` over a sixth list; the name is copied through `BSP_NativeString_Resize` and `memcpy` first |
| `+1E0h` | `[0x78]` | `+1D8h + +1D0h + +1C8h + +1DCh + +1D4h + +1CCh`, in that native order |

List heads the routine reads: `+34h`, `+4Ch`, `+70h`, `+A0h`, `+A4h`, `+ACh`, `+B8h`, `+BCh`,
`+C4h`, `+DCh` (decompiler indices `0xd`, `0x13`, `0x1c`, `0x28`, `0x29`, `0x2b`, `0x2e`, `0x2f`,
`0x31`, `0x37`). Two flags are also set at the end: `record+18Ch` (`[99]`) when
`(float)record+14h` equals `_DAT_00D7A218`, and `record+188h` (`[0x62]`) when both `record+BCh`
and `record+A4h` are empty.

The three deepest walks use `00911CE0(a, b, c, d)` and `00911C00(a, b, c, (float)d)` as scoring
kernels; neither was opened.

## `00911E80`, the per-side unit demand rebuild

`void __fastcall(record)`, body `00911E80..00912A1E`. Shape:

1. Clear the list at `record+78h` (`0058B520` on its head, then the three self-pointers and
   `record+80h = 0`).
2. `00908190` returns an iterator pair over the map at `record+B4h`/`record+B8h`. For each leaf
   of a three-level walk, switch on the leaf's type code at `+0Ch` and index a
   case-insensitive string-to-int map with one of the tokens
   `BU_DM`, `BU_PTM`, `BU_BM`, `BU_CM`, `BU_VLB`, `BU_VDB`, `BU_VTB`, `BU_FA`, `BU_VR`, `BU_SM`,
   incrementing the count.
3. Copy the map at `record+120h`/`+124h` into the same demand map by key.
4. Walk `record+15Ch`/`+160h`; for each entry whose `+10h` float exceeds the scheduler's
   `game+21A0h`+`1470h` threshold, increment the count for `BO_CV`. The same pattern then adds
   `BO_DCM`, `BO_KSS`, `BO_KSP`, `BO_SC`, `BO_EE`.
5. Walk the rebuilt list at `record+78h`. For each entry whose count reaches the first element
   of the config vector at `[0050FC30(key)]+48h`, and whose key resolves through `004BF370` to
   the same bucket as `record+70h`, call `0090EDE0(record+250h, key, 1, 0)`.

The `BU_` tokens read as build/unit-type demands and the `BO_` tokens as bot-order demands, but
that is inference from the prefixes and the switch, not from any string the game prints.

## `00912A60`, the per-side goal request rebuild

`void __fastcall(record)`, body `00912A60..00913A0E`, `ECX` captured into `EBP` at `00912A82`.
Same shape against the second list: clear `record+88h` / `record+8Ch`, then build counts for
`RUA_CU`, `RUA_FU`, `RUA_SU`, `RUA_TBU`, `GA_SH`, `GA_DB`, `GA_AO`, `GA_PL` and call
`0090EDE0(record+250h, key, 1, 0)` at `009133C9` and `009139C4`. Helpers `0090CA30`, `0090D220`
(self-recursive), `00907240`, `00910F10`, `00907D40`, `00907240` are container plumbing;
`0090D220` is the only one that reaches further scoring code (`00907F40`, `00907F80`,
`009085D0`, `0090C430`).

`RUA_` reads as "requested unit action" and `GA_` as "goal action"; again inference from the
prefixes.

## Where ship orders actually come from

Since this layer produces none, the per-unit chain in `docs/UNIT_COMMAND_PRODUCERS.md` stands as
the only producer: `009998A0` (bot tick) -> `0099B450`, `0099C270`, `0099D300`, and the player
path `00816A40` -> `00815440` -> `0080DAD0`. Both end in the order ring at `unit+838h` that
`docs/UNIT_STATE_MESSAGE.md` recovers. The scheduler and the per-unit bots are separate objects
with no call edge between them in either direction.

## Corrections

* `docs/GAME_WORLD_ENTITIES.md`, section "00914ef0 - bot scheduler", says the passes run "over
  the 0x284-stride records at `this+0x1E4`" and flags as uncertain that "eight records of 0x284
  bytes starting at `+0x1E4` would end at `+0x1604`, past the 0x14B8 allocation". The records
  start at **`this+4`** (`00915026 LEA ESI,[EBX+4]`) and end at `this+1424h`, comfortably inside
  the allocation. `this+1E4h` is `record[0]+1E0h`: the retarget loop biases its pointer so that
  `MOV ECX,[EDI]` reads each record's aggregate directly. The uncertainty is resolved and the
  stride, the count and the base are all proven.
* The same section says the retarget pass fills the stack message with "the record's first
  dword". It is the dword at `record+1E0h`, the aggregate `00914390` computes.

## Uncertainty

* The six counter categories are proven as six independent sums over six lists; which asset
  class each list holds is not proven. `00910570`, `00911C00`, `00911CE0` and `00914270` were
  not opened.
* The token strings are proven as keys into a case-insensitive string-to-int map and as award
  counter names. Reading `BU_`/`BO_`/`GA_`/`RUA_` as build unit, bot order, goal action and
  requested unit action is a naming hypothesis.
* `0076A9F0` is an adjustor thunk; the transport behind session message `15h` and the meaning
  of the literal `12h` were not chased.
* `00908190` and the map at `record+B4h` were read only through the decompiler; the three-level
  container was not identified.

## Follow-up packets

* `bot_side_record_layout` - addresses `00914390`, `00910570`, `00911C00`, `00911CE0`,
  `00914270`; name the six asset lists and the two scoring kernels, and settle whether the
  aggregate at `+1E0h` is a threat score or a resource budget.
* `bot_demand_vocabulary` - addresses `00911E80`, `00912A60`, `005070C0`, `0050FC30`,
  `004BF370`; recover the demand-token table and the config vector `[0050FC30(key)]+48h` that
  gates publication, which should turn the prefixes into names.
* `bot_side_sync_message` - addresses `0076A9F0`, `00D035B8` as data, `0075B430` with id `15h`;
  recover the message class the scheduler broadcasts and its receive side.

## `no_ghidra_function`

None. Every address in this packet already has a Ghidra function.

## State reached

| address | state |
| --- | --- |
| `00914390` | analysed, reconstructed (aggregation rule only), build-tested |
| `00911E80`, `00912A60` | analysed |
| `00914EF0` | read only; record base corrected |
| `0090EDE0`, `0076A9F0` and the container helpers | read only |
