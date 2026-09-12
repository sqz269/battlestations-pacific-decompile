# The scene command-type registry (`00E19A6C`, `006F9110`, `00523900`)

Addresses: 00E19A6C 00E19A68 00E19A70 00E08EF8 00E08FC0 00CCE500 00CCEB40 00CDC6A0 00CDCCE0
00CE2BEC 006F7D40 006F7F30 006F7F40 006F7F90 006F9110 00523900 005234F0 00CFB378 00521EF0

Packet `cc2_entity_orders`, part 2. Follow-up of `docs/SCENE_DEFERRED_REFS.md` ("The global command
registry (`00E19A70`)"). The 26 command names below are string literals in the image, not
hypotheses; every other name here is a hypothesis.

## Headline

There are exactly **26** command-type objects. Each is an 8-byte global `{vptr, ordinal}` in
`00E08EF8..00E08FC7`, built by one of the 26 constructors at `00CCE500 + 40h * k` that the CRT
initialiser table at `00CE2BEC` runs in ascending address order. Each constructor takes the next
value of the counter `00E19A68` as the object's ordinal, appends the object to the list at
`00E19A6C`, sets the object's real vtable, and registers the matching destructor thunk at
`00CDC6A0 + 40h * k` with `atexit`. The ordinal is what the `MT_COMMAND` message carries.

## Correction to `docs/SCENE_DEFERRED_REFS.md`

| Was | Is | Evidence |
| --- | --- | --- |
| "the static initialisers at `00CDC6A0..00CDC7E0`" populate the list | `00CDC6A0..00CDCCE0` are the **destructors**: each writes the base vtable `00CFB378` over the object and unlinks it from the list through `00522B70`. The constructors are `00CCE500..00CCEB40`. | `00CDC6A7: MOV [00E08EF8],0x00CFB378` (the base vtable, whose `+4h` is `006F7F30` "undefined command name"); `00CCE526: MOV [00E08EF8],0x00CFB3A4` (the derived vtable) after `00CCE51C: CALL 006F7D40`, the list append. `00CCE521` pushes `00CDC6A0` to `atexit` at `00BF6FF5`. |
| the list is "`00E19A70`, head of a second list, `{?, next@+4h, object@+8h}`" | The **container** is at `00E19A6C` and is `{count +0h, head +4h, tail +8h}`; `00E19A70` is its head pointer. Nodes are `{prev +0h, next +4h, object +8h}`, 0x0C bytes from `00BF681B`. | `006F7D40`: `MOV ESI,ECX` with `ECX = 00E19A6C` at `00CCE50D`; `MOV [ECX],EDI` where `EDI = [ESI+8]` (prev = old tail), `MOV [ESI+8],ECX` (new tail), `ADD [ESI],0x1` (count). |
| "the thirteen authored spellings collapse to eleven command types" | The 13 spellings in that table's own counts collapse to **ten** distinct names (`Cruise`/`cruise`, `Follow`/`follow` and `Moveto`/`MoveTo` are the three case pairs). Flagged, not re-scanned: this pass read the doc's table, not the corpus. | `docs/SCENE_DEFERRED_REFS.md` lines 185-192. |

## Registry layout

| Address | Field |
| --- | --- |
| `00E19A68` | `int` registration counter, pre-incremented; ends at 26 |
| `00E19A6C` | `int` node count |
| `00E19A70` | head node (`0046AAB0`, `00467170` and `006F9110` all start here) |
| `00E19A74` | tail node |

Node: `prev +0h`, `next +4h`, `object +8h`. `006F7D40 __thiscall(list, object)`, `RET 4`, appends;
the walk order is therefore registration order, which is what makes "first match wins" in
`0046AAB0` well defined.

Command object: `vptr +0h`, `int ordinal +4h`, 8 bytes. The vtable slots this packet uses:

| Slot | Base body (`00CFB378`) | Meaning |
| --- | --- | --- |
| `+0h` | `006F7F90` | scalar deleting destructor |
| `+4h` | `006F7F30`, returns `00CFB388 "undefined command name"` | the command name; every derived body is `MOV EAX,<literal> ; RET` |
| `+8h` | `006F7F40`, `XOR AL,AL ; RET` | predicate; every derived body is `MOV AL,1 ; RET` or `XOR AL,AL ; RET` |
| `+0Ch` | `00BF698E` `__purecall` | category id; every derived body is `MOV EAX,<imm> ; RET` or `XOR EAX,EAX ; RET` |
| `+10h` | `006F7F50` | not read (`contract: unread`) |

## The 26 command types

`ordinal` is `object+4h` and equals the row number, because the CRT table at `00CE2BEC` lists the
constructors `00CCE4F0, 00CCE500, 00CCE540, ... 00CCEB40` in ascending order and only the 26 from
`00CCE500` touch the counter. `name` is the literal returned by `vtable[4]`; `cat` is
`vtable[0Ch]`; `tgt` is `vtable[8]`.

| # | Object | vtable | name fn | name | cat | tgt | dtor |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | `00E08EF8` | `00CFB3A4` | `006F8010` | `settarget` | 1 | yes | `006F91A0` |
| 2 | `00E08F00` | `00CFB3B8` | `006F80B0` | `cleartarget` | 0 | no | `006F91F0` |
| 3 | `00E08F08` | `00CFB3CC` | `006F8150` | `clearorders` | 0 | no | `006F9240` |
| 4 | `00E08F10` | `00CFB3E0` | `006F81F0` | `artillery` | 1 | yes | `006F9290` |
| 5 | `00E08F18` | `00CFB400` | `006F8290` | `torpedo` | 2 | yes | `006F92E0` |
| 6 | `00E08F20` | `00CFB41C` | `006F8330` | `divebomb` | 2 | yes | `006F9330` |
| 7 | `00E08F28` | `00CFB43C` | `006F83D0` | `levelbomb` | 2 | yes | `006F9380` |
| 8 | `00E08F30` | `00CFB45C` | `006F8470` | `dropkamikaze` | 2 | yes | `006F93D0` |
| 9 | `00E08F38` | `00CFB480` | `006F8510` | `depthcharge` | 2 | yes | `006F9420` |
| 10 | `00E08F40` | `00CFB4BC` | `006F8650` | `strafe` | 1 | yes | `006F94C0` |
| 11 | `00E08F48` | `00CFB4A0` | `006F85B0` | `rocket` | 2 | yes | `006F9470` |
| 12 | `00E08F50` | `00CFB4D8` | `006F86F0` | `kamikaze` | 2 | yes | `006F9510` |
| 13 | `00E08F58` | `00CFB4F8` | `006F8790` | `dogfight` | 1 | yes | `006F9560` |
| 14 | `00E08F60` | `00CFB518` | `006F8830` | `follow` | 3 | yes | `006F95B0` |
| 15 | `00E08F68` | `00CFB534` | `006F8920` | `moveto` | 3 | yes | `006F9600` |
| 16 | `00E08F70` | `00CFB554` | `006F8A30` | `cruise` | 3 | no | `006F9650` |
| 17 | `00E08F78` | `00CFB590` | `00521EF0` | `attackmove` | 2 | yes | `006F96F0` |
| 18 | `00E08F80` | `00CFB63C` | `006F8EF0` | `moveonpath` | 3 | no | `006F98D0` |
| 19 | `00E08F88` | `00CFB5A4` | `006F8C00` | `stop` | 3 | no | `006F9740` |
| 20 | `00E08F90` | `00CFB5C0` | `006F8C90` | `retreat` | 3 | no | `006F9790` |
| 21 | `00E08F98` | `00CFB5DC` | `006F8D20` | `returntobase` | 3 | no | `006F97E0` |
| 22 | `00E08FA0` | `00CFB600` | `006F8DB0` | `land` | 3 | yes | `006F9830` |
| 23 | `00E08FA8` | `00CFB61C` | `006F8E50` | `closetoship` | 3 | yes | `006F9880` |
| 24 | `00E08FB0` | `00CFB660` | `006F9000` | `Leave` | 0 | no | `006F9920` |
| 25 | `00E08FB8` | `00CFB67C` | `006F90A0` | `disband` | 0 | no | `006F9970` |
| 26 | `00E08FC0` | `00CFB570` | `006F8AD0` | `tutorial` | 3 | no | `006F96A0` |

`attackmove` is the one class whose name getter (`00521EF0`, literal `00CECCA8`) lives outside the
`006F8xxx` block, and the only ordinal the binary compares against a fixed address: `00E08F7C` is
`00E08F78 + 4h`, read by the gate at `0077D776` (`docs/ENTITY_ORDER_MESSAGE.md`).

Categories: 0 = no category (`cleartarget`, `clearorders`, `Leave`, `disband`), 1 = gunnery-style
(`settarget`, `artillery`, `strafe`, `dogfight`), 2 = weapon run (`torpedo`, `divebomb`,
`levelbomb`, `dropkamikaze`, `depthcharge`, `rocket`, `kamikaze`, `attackmove`), 3 = movement (the
remaining ten). The 0/1/2/3 split is the only thing the binary proves; the labels are a reading.

`tgt` (`vtable[8]`) drops a scene record that named no target (`0046ABA6`), so "requires a named
target" fits every row except `moveto`, which is `yes` while three quarters of the corpus's
`Moveto` records name no target. Treat the reading as provisional; what is proven is the drop rule.

## Which authored names exist as objects

Of the ten distinct names in the corpus scan of `docs/SCENE_DEFERRED_REFS.md`, **all ten** have an
object, matched case-insensitively through `00438E10`:

`settarget` (1), `divebomb` (6), `levelbomb` (7), `kamikaze` (12), `follow` (14), `moveto` (15),
`cruise` (16), `attackmove` (17), `moveonpath` (18), `stop` (19).

The other 16 classes are registered but never named by the shipped scene files: `cleartarget`,
`clearorders`, `artillery`, `torpedo`, `dropkamikaze`, `depthcharge`, `strafe`, `rocket`,
`dogfight`, `retreat`, `returntobase`, `land`, `closetoship`, `Leave`, `disband`, `tutorial`. They
reach `0077D600` from the code instead: the call-site table of `docs/ENTITY_ORDER_MESSAGE.md` names
`00E08EF8`, `00E08F08`, `00E08F18`, `00E08F70`, `00E08F80`, `00E08F90`, `00E08F98`, `00E08FA0`,
`00E08FA8` and `00E08FB8` among the pushed constants.

## `006F9110`, the by-category collect

`__fastcall(entity /*ECX*/, int category /*EDX*/, void* outList /*[ESP+4]*/)`, `RET 4`.
Coverage: complete, `006F9110-006F9184`.

```
005234F0(outList);                                   // 006F911B, clear
for (node = [00E19A70]; node; node = node->next) {   // 006F9120, 006F9177
    if (!entity->IsKindOf(2)) continue;              // 006F9139, vtable[5Ch]
    if (node->object->vtable[0Ch]() != category) continue;   // 006F9147
    if (entity->vtable[168h](node->object->vtable[4]())) // 006F915D, 006F9164
        006F7D40(outList, node->object);             // 006F9172, append
}
006F7C20(&LAB_006F7B30);                             // 006F917F
```

The `IsKindOf(2)` test does not depend on the node, so a false answer empties the result; it is
re-evaluated per node because the compiler did not hoist it. `vtable[168h]` takes the command's
name string and answers whether this entity accepts that command; its body was not read
(`contract: unread`), and neither was `006F7C20`.

## `00523900`, the caller

`__thiscall(this, int category)`, `RET 4`. Coverage: complete, `00523900-0052393C`.

```
this->category_2Ch = category;       // 0052390D
005234F0(this + 3Ch);                // 00523910, clear the result list
if ([00E188D8] == 0) { this->cursor_48h = 0; return; }   // 0052391B, 00523934
006F9110([00E188D8], this->category_2Ch, this + 3Ch);    // 00523923
this->cursor_48h = this->head_40h;   // 00523928, 0052392C
```

`[00E188D8]` is the player-controlled unit (`docs/UNIT_INSTANCE_UPDATE.md`). So `this+3Ch` is a
`{count, head, tail}` list of the command objects the player's own unit accepts in one category,
and `this+48h` is a cursor reset to the head. `00523900` has no direct callers: it is reached
through a vtable. The owning class was not identified (`contract: unread`), but `00525250`, which
issues an order with the command `00523130` returns and the flags `playerUnit->IsKindOf(18h)`, is
the same screen family.
