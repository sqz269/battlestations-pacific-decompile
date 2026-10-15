# Multiplayer session: receive ladder, voice chat, player slots (packet cc13-net1)

Partition segment 43, `0076AA00..00787040`. Reconstruct-only: nothing here is wired into
the game hosts. Source: `include/bsp/multiplayer_session_cc13.hpp`,
`src/multiplayer_session_cc13.cpp` (namespace `bsp::mp_cc13`).

Status words used below: **exported** (raw Ghidra pseudocode in `exports/`),
**reconstructed** (C++ written from pseudocode plus listing checks), **build-tested** (compiles in
`scripts/build.ps1`). Nothing in this packet is fixture-tested, ABI-compatible or game-validated.
All C++ names and Ghidra `BSP_*` names are hypotheses; the only recovered identifiers are the
`MT_*` message names (read from the image's own table) and the class name `MMultiPlayer` (from
log strings).

## The session object

The session is the `MMultiPlayer` instance embedded in the game at `game+1EF0h`
(`[00E188A8]`; constructor `0076EDE0`, already reconstructed as
`construct_native_game_embedded_state_0076ede0`). Log strings name the class:
`"MMultiPlayer::SetMuted(), XUserCheckPrivilege return: %d"` (00771630) and
`"__MMultiPlayer::ServerSendSceneScoring()"` (0076D1B0). Fields used by this packet:

| offset | role | evidence |
|---|---|---|
| `+F4h` | mode: 0 local, 1 host, 2 client (`SessionMode` in `session_message_dispatch.hpp`; same field as `game+1FE4h`) | every host/client split below |
| `+188h` | server transport; non-null selects the server arm of the ladder | `0077808B CMP [EDI+188h],EBX` and 15 more |
| `+18Ch` | client transport; `+9Ch` its stage (3/4 kicked, 8 playing), `+C4h` scene-match byte | 0076D170, 007712F0, 007724A0 |
| `+190h` | voice mute table, `0x88` bytes (below) | 0076D915 `LEA EDI,[EBX+190h]`, 0077174A |
| `+248h` | set to 2 by MT_LETSPLAY on a client | 00771FC0 |
| `+24Ch/+250h/+258h` | receive queue (array, count, cursor) | 00777850 |
| `+268h/+26Ch` | 64-bit session nonce | 0076C7B0 |
| `+274h` | loading-info flag | 007778F4 |
| `+284h` | MT_MOVCAMNEW_ADDPOSITION counter | 0076D760 |

## The message-type table 00E0AB68

`00E0AB68` holds 235 `char*` entries, `MT_INVALID` (0) through `MT_LAST` (`EAh`). The table is
static data (no writer; three readers 0076C68F, 0077839E, 00788280 index it as
`[type*4+00E0AB68]` for the "multi msg (%s) from tick (%d) is dropped" logs). The strings were read
from the installed PE by resolving each pointer through the section table. None of the 235 names
appeared in the repository before this packet; `message_type_name_00e0ab68` returns them verbatim.
Examples relevant here: `04h MT_ADDPLAYER`, `0Ah MT_REFRESHSCENE`, `15h MT_SCENESCORING`,
`19h MT_SELECTPLAYER`, `1Ah..1Ch MT_SWAPSLOTREQUEST/ACCEPT/REFUSE`, `25h MT_MYPLAYER`,
`26h..29h MT_ADDREMOTEVOICE/REMOVEREMOTEVOICE/MUTEREMOTEVOICE/VOICEDATA`, `4Dh MT_MULTISELECTUNIT`,
`DDh MT_PLAYERUNLOCKUNIT`, `E4h MT_SESSION_MANAGER`, `E8h MT_VOICECHANGE`, `E9h MT_METRICSSESSION`.

## The receive ladder 00777850 (reconstructed: routing and queue outcome)

`00777850` (`__fastcall`, ECX = session, RET) is the receive pump that
`docs/SESSION_MESSAGE_DISPATCH.md` left as "partially read". Before the walk it calls `007849C0` on
`+188h` (or `+18Ch` when that is null). For each queued message (ESI) it sets the status local
`[ESP+14h]` to 2 and asks `msg->vtable[0Ch](type)` for each type below, in this order; the first
true answer selects the arm. Where the handler column shows two addresses the arm splits on
`session+188h`: non-null takes the server handler. Argument forms: `SessionMsgStatus` = ECX
session, push `&status`, push msg; `SessionMsg` = push msg only; `TwoDwords` = push `[msg+1Ch]`,
push `[msg+18h]`; `SessionPeer` = push `[msg+8]`; `PayloadWord` = push `[msg+20h]`, push `msg+18h`;
`MessageThis` = ECX msg; `Other` = a callee outside this segment, not opened.

| type | MT_ name (00E0AB68) | is-a site | handler | args |
|---|---|---|---|---|
| 0Eh | MT_LOADINGINFO | `007778E5` | inline | None |
| 0Fh | MT_CONNTEST | `00777919` | inline | None |
| 10h | MT_CONNSTATE | `0077792C` | `0076CD20` | SessionMsgStatus |
| 11h | MT_LETSPLAY | `0077794D` | `00771FC0` | SessionMsgStatus |
| 04h | MT_ADDPLAYER | `0077796E` | `00773120` | SessionMsgStatus |
| 06h | MT_REMOVEPLAYER | `0077798F` | `00773420` | SessionMsgStatus |
| 05h | MT_SERVERINFO | `007779B0` | `0076A7B0` | SessionMsgStatus |
| 35h | MT_MISSIONINFO | `007779D1` | `00734DB0` | MessageThis |
| 36h | MT_OBJECTIVE | `007779EA` | `008E2390` | MessageThis |
| 41h | MT_STATIONARY_SET_DEADMEAT | `00777A17` | `0084EC10` | MessageThis |
| 42h | MT_SPAWN_FROM_RES | `00777A30` | `0094B600` | Other |
| 43h | MT_POWERUP | `00777A84` | `008EE020` | Other |
| 44h | MT_POWERUP_EFFECT | `00777AAC` | `008EA4E0` | Other |
| 45h | MT_POWERUP_BINDING | `00777AD4` | `008E6D30` | Other |
| 07h | MT_CREATED | `00777AFC` | inline | None |
| 08h | MT_SCENENAME | `00777B1A` | `0076E710` | SessionMsgStatus |
| 09h | MT_SCENEIDX | `00777B3B` | `007724A0` | SessionMsgStatus |
| 0Ah | MT_REFRESHSCENE | `00777B5C` | `007717A0` | SessionPeer |
| 0Bh | MT_SCENELOAD | `00777B7B` | `0076D030` | SessionMsgStatus |
| 0Dh | MT_CHECKSCENELOAD | `00777B9C` | server `007F7460` / client `00772710` | Other / SessionMsgStatus |
| 0Ch | MT_SCENELOADED | `00777BD7` | server `0076D090` / client `004DA6C0` | SessionMsgStatus / Other |
| 12h | MT_SCENETERM | `00777C1C` | server `0076D260` / client `00772990` | SessionMsgStatus |
| 13h | MT_QUIT | `00777C58` | server `0076D140` / client `0076D0E0` | SessionMsgStatus |
| 14h | MT_KICKED | `00777C94` | `0076D170` | SessionMsgStatus |
| 15h | MT_SCENESCORING | `00777CB5` | `0091C650` | MessageThis |
| 16h | MT_SCENEOBJECTIVE | `00777CD7` | `009105F0` | MessageThis |
| 19h | MT_SELECTPLAYER | `00777CF0` | server `0076C840` / client `007705A0` | SessionTwoDwords |
| 1Ah | MT_SWAPSLOTREQUEST | `00777D2F` | `00771990` | SessionTwoDwords |
| 1Bh | MT_SWAPSLOTACCEPT | `00777D52` | `00771AC0` | SessionTwoDwords |
| 1Ch | MT_SWAPSLOTREFUSE | `00777D75` | `00771CC0` | SessionTwoDwords |
| 17h | MT_CHANGEMULTISETTINGS | `00777D98` | `005E50D0` | MessageThis |
| 18h | MT_SUBSCENE | `00777DB1` | `007708E0` | SessionPayloadWord |
| 1Dh | MT_READYPLAYER | `00777DD4` | `0076CAF0` | SessionMsgStatus |
| 1Eh | MT_AIENABLE | `00777DF5` | `0076CC20` | SessionMsg |
| 1Fh | MT_AISKILLLEVEL | `00777E11` | `0076CCD0` | SessionMsg |
| 20h | MT_PLAYERSURRENDER | `00777E2D` | `00770850` | SessionMsgStatus |
| 21h | MT_CHAT | `00777E4E` | `007722E0` | SessionMsgStatus |
| 22h | MT_HINT | `00777E6F` | `00777820` | SessionMsg |
| 23h | MT_COUNTER | `00777E8B` | `0076CEA0` | SessionMsg |
| 24h | MT_PLAYER_PING | `00777EA7` | `008E2DA0` | MessageThis |
| 4Dh | MT_MULTISELECTUNIT | `00777EC0` | `0076CFD0` | SessionMsgStatus |
| 25h | MT_MYPLAYER | `00777EE1` | `0076C7B0` | SessionMsgStatus |
| 2Ah | MT_MUSICCONTROL_SETLEVEL | `00777F02` | `00A788A0` | Other |
| 2Bh | MT_BLACKOUT | `00777F25` | `0076F4C0` | SessionMsgStatus |
| 2Ch | MT_COUNTDOWN | `00777F46` | `007709C0` | SessionMsgStatus |
| 2Dh | MT_MOVCAM_HANDMODETARGETED | `00777F67` | `0076D420` | SessionMsgStatus |
| 2Eh | MT_MOVCAM_REFPOSMODEPOLAR | `00777F88` | `0076F560` | SessionMsgStatus |
| 2Fh | MT_MOVCAM_REFPOSMODERADIUS | `00777FA9` | `0076F630` | SessionMsgStatus |
| 30h | MT_MOVCAM_PATHWALKPATHPOINTPROPERTYP | `00777FCA` | `0076A870` | SessionMsgStatus |
| 31h | MT_MOVCAM_ADDINTERPOLATOR | `00777FEB` | `0076A8A0` | SessionMsgStatus |
| 32h | MT_MOVCAM_SETDEVIATIONFACTOR | `0077800C` | `0076A940` | SessionMsgStatus |
| 33h | MT_MOVCAMNEW_ADDPOSITION | `0077802D` | `0076D760` | SessionMsgStatus |
| 34h | MT_MOVCAM_BLACKBAR | `0077804E` | `0076A970` | SessionMsgStatus |
| C2h | MT_PLANE_SYNC_ACK | `00778072` | inline | None |
| 26h | MT_ADDREMOTEVOICE | `00778085` | server `00772D20` / client `0076D840` | SessionMsgStatus |
| 27h | MT_REMOVEREMOTEVOICE | `007780C0` | server `00772D80` / client `0076D890` | SessionMsgStatus |
| 28h | MT_MUTEREMOTEVOICE | `007780FB` | `0076D8E0` | SessionMsgStatus |
| 29h | MT_VOICEDATA | `0077811C` | `0076D9C0` | SessionMsgStatus |
| 55h | MT_CAMERA | `0077813D` | `007F7C60` | Other |
| D7h | MT_HIGHLVLAI_SPAWNBANN | `00778174` | `00A38DA0` | Other |
| 46h | MT_NETENT | `00778193` | `00780670` | Other |
| DDh | MT_PLAYERUNLOCKUNIT | `007781BF` | `0076CB30` | SessionMsg |
| DEh | MT_SERVERDLCUNLOCK | `007781DE` | `0050A820` | Other |
| E1h | MT_MANAGE_DIALOGS | `00778204` | `00451E50` | Other |
| E2h | MT_MANAGE_MESSAGES | `0077822D` | `00706760` | Other |
| E3h | MT_PLAYERINFO | `0077825C` | `007F8210` | MessageThis |
| E4h | MT_SESSION_MANAGER | `00778278` | `007746F0` | SessionMsgStatus |
| E5h | MT_HITMARKER | `0077829C` | `0096F090` | Other |
| E6h | MT_HITWARNING | `007782C4` | `00978A70` | Other |
| E7h | MT_VOICEOVER | `007782EC` | `0097B960` | Other |
| E8h | MT_VOICECHANGE | `00778311` | `0076DA10` | SessionMsgStatus |
| E9h | MT_METRICSSESSION | `00778332` | inline | None |

Inline arms: `0Eh` stores `(msg+18h == 0)` into `+274h` in client mode, else 0 (007778EB); `07h`
stores 8 into `[msg+8]+0Ch` (00777B02); `0Fh` and `C2h` jump straight to the tail; `E9h` builds a
string from `msg+18h` and calls `00753350`/`00753810(9,0)` on `[00E1AED4]` when it is non-null.
`0Ch`'s client arm calls `004DA6C0` only when `game+624h` is 0. `15h` clears `[00F8A2FC]+74h` after
its call. `36h` sets status 0 when `msg->vtable[10h]()` is false (00777A01), which defers the
message. A message no test claims keeps status 2 and is destroyed.

Queue outcome (0077836B..0077841E): status 0 is logged and destroyed only when the game exists, its
state (`game+5D4h`) is below `0Ah` and `msg+0Ch` is positive; otherwise the message stays queued and
the cursor advances (0077843D). Status 2 unlinks and calls `vtable[0](1)`; any other status unlinks
without destroying. A null game or null entry is unlinked (and destroyed when non-null) without
routing (007778C0..007778D6).

**Assumption (stated in the header):** the reconstruction routes by type byte. The native routes by
the is-a test, so a class that answered true for several types would take the earliest arm. Every
handler opened in this packet reads only the fields of its own type.

## Voice chat

### The mute table, session+190h (reconstructed, byte-exact layout)

| offset | contents |
|---|---|
| `+00h` | `muted[8][8]`, byte per (row, column); `muted[a][b] = 1` = a muted b |
| `+40h` | eight 64-bit xuids (lo, hi) |
| `+80h` | eight active bytes |

Constructor/reset `00778610`/`00778590` were already reconstructed as "peer pairs"
(`native_game_embedded_state`); the table's purpose is new: `0076D8E0` (MT_MUTEREMOTEVOICE) writes
`muted[find(a)][find(b)]`, and the chat filter `005D0770` asks `007786E0` on `game+2080h`
(= session+190h) whether to drop a line (`include/bsp/hud_widgets.hpp`, `ChatMuteHost`).

| address | name (hypothesis) | ABI | behaviour |
|---|---|---|---|
| 007785D0 | find | ECX table, (lo, hi), RET 8 | first active row with matching xuid, else -1 |
| 00778620 | add | RET 8 | absent xuid takes the first inactive row; row and column cleared; full table: no-op |
| 00778690 | remove | RET 8 | xuid, active, row and column cleared |
| 00778780 / 007787C0 | set / clear | RET 10h | `muted[i][j]` = 1 / 0 when both found (i == j allowed) |
| 007786E0 | is-muted | RET 10h, AL | both found, distinct, either direction muted; the false path returns `index_b & FFFFFF00h` (AL = 0) |

### Voice messages

| type | handler | behaviour |
|---|---|---|
| 26h/27h server | 00772D20 / 00772D80 | trace, record `+89h` = add, `00772BB0(xuid, add)`, then `[00F8A2FC]->vtable[ACh]/[B0h](xuid)` |
| 26h/27h client | 0076D840 / 0076D890 | trace, record `+89h` = add, `vtable[ACh]/[B0h](xuid)` |
| 28h | 0076D8E0 | set/clear `muted[a][b]` (msg+18h, msg+20h, flag msg+28h); with a server transport and a != b, rebuild (0076BFE0) and broadcast to all |
| 29h | 0076D9C0 | record by xuid msg+18h: `007F7490(record, 1)` (record+8Ch = game+64Ch, last-talk time) then `vtable[A8h](msg+18h)`; unknown sender traces `"WOICE ERROR"` (sic) |
| E8h | 0076DA10 | record by xuid msg+20h: `+89h` = msg+18h; host forwards the message to every peer except that record's |

`00772BB0` (RET 0Ch) builds 26h/27h (vtables 00D0323C/00D03250, xuid at +18h). In host mode it
broadcasts with `00784790` on `+188h`, excluding the owner's peer (`record+50h`, or no exclusion
when the xuid is unknown); otherwise it sends to the server with `00770AF0`. It then sets `+89h`.

`00771630` `MMultiPlayer::SetMuted(xuid, mute)` (RET 0Ch). `ESI = [00F8ABE8]` (the Xenon system
manager) supplies the user slot. `XUserCheckPrivilege(FCh)` (communications) allowed keeps the
request. Otherwise `XUserCheckPrivilege(FBh)` (friends only): denied forces `mute = 1`; allowed asks
`XUserAreUsersFriends` and forces the mute for a non-friend. An API error is traced and keeps the
request. Then: a client (`+F4h == 2`) whose target is not its own xuid (`00A3EB00`) sends
MT_MUTEREMOTEVOICE (own, target, mute) to the server and returns **without touching its own
table**; the server's 0076D8E0 applies and rebroadcasts. Host, local mode or self-target set the
table directly and trace `"__SetLocal_VOICE__  MUTE(%llx, %llx) %d"`.

## Player-slot bookkeeping

Player records are the 8 x 118h array at `game+748h`; `game+18CCh` holds eight current-record
pointers per slot (`session_participant_pools.hpp`), `game+18C8h` the local player index and
`game+18ECh` the local slot. Record fields this packet adds to the map:

| offset | role | writer / reader |
|---|---|---|
| `+0Ah` | AI enabled (current record) | 0076CB90 |
| `+0Ch/+0Dh` | ready flags | 0076CA40 |
| `+0Eh` | scene loaded | 0076D090 |
| `+10h` (u16) | state: `FFFDh` on ready/loaded, 1 on MT_LETSPLAY | 0076CA40, 0076D090, 00771FC0 |
| `+19h` | cleared on entering play | 007712F0 |
| `+1Bh` | surrender | 00770750 |
| `+1Ch` | owning player index (current record) | 00771AC0 |
| `+20h` | bound slot index, -1 none | 0076C840, 00771AC0, 00773420 |
| `+2Ch` | AI skill level (current record) | 0076CC40 |
| `+50h` | transport peer (null = local) | 00771990, 00772BB0, 0076DA10 |
| `+58h` | name (inline chars) | 007705A0, 00771AC0, 00773420 |
| `+80h/+84h` | xuid | 0076D7D0 |
| `+89h` | remote voice registered | voice handlers |
| `+8Ch` | last voice time (game+64Ch) | 007F7490 |
| `+90h/+94h` | MT_ADDPLAYER bytes msg+60h/+61h | 00773120 |
| `+110h` | unlocked class | 0076CB30 (`game+858h + i*118h`) |

Setters that the host echoes to all peers with `00784790(msg, 0)` after writing locally:
`0076CA40` ready (1Dh, receiver 0076CAF0 skips in host mode unless game state is 8), `0076CB90` AI
enable (1Eh, receiver 0076CC20), `0076CC40` AI skill (1Fh, receiver 0076CCD0), `00770750`
surrender (20h, receiver 00770850; also posts `"surrender"`/`"notsurrender"` to `00985920` for slot
`record+20h` when `[00F8A0C4]` is set). `0076CB30` stores the unlocked class and rebroadcasts the
received message when `game+1FE4h` is 1.

Slot moves: `0076C840` (host) requires the target slot to be -1 or AI-held and the player record to
be claimed and not AI-held. It releases the old slot (`004BB660`, broadcast `{-1, old}`) and binds
the new one (`004BB630`, broadcast `{player, slot}`); the local player also gets `004B4740`.
`007705A0` is the client's application of the same message; on release it keeps the departing
name on the restored mission record in game states 0Ah, 0Dh and 10h. Swaps: `00771990` request
(local slot -> `005D8800` prompt; host forwards to the holder's peer, or refuses to the requester
via `0076BA80` when no remote human holds the slot), `00771AC0` accept (host: three `0076C840`
moves, then `005CF320` with both names), `00771CC0` refuse.

## Lifecycle receivers

| type | handler | behaviour |
|---|---|---|
| 0Bh MT_SCENELOAD | 0076D030 | `game+1EE3h = (game+1FE4h != 0)`, `[00F8A2FC]->vtable[54h]()`, `004D7920(10)` |
| 0Ch MT_SCENELOADED (server) | 0076D090 | when `game+624h == 0`: sender record `+0Eh=1`, `+10h=FFFDh`, `+18h=0` |
| 11h MT_LETSPLAY | 00771FC0 | client: `+248h=2`, `00874640(0)`, local current record `+10h=1`, `007712F0`; host: sender record `+10h=1` |
| 12h MT_SCENETERM (server) | 0076D260 | `[[msg+8]+D50h]+1Ah = 0` when the metadata exists |
| 13h MT_QUIT (server) | 0076D140 | `00784B10(+188h, msg+8, 0)`: MT_KICKED to the sender |
| 14h MT_KICKED | 0076D170 | `[+18Ch]->vtable[3Ch]()`, stage `= msg+18h ? 3 : 4` |
| 23h MT_COUNTER | 0076CEA0 | client with HUD root `+C8h`: `0052CF60` |
| 25h MT_MYPLAYER | 0076C7B0 | `game+18C8h = msg+18h`; nonce `= (msg+1Ch << 32) + msg+20h` (00BF9640 `__allshl` with EDX = 0, then ADD/ADC), copied to `[game+207Ch]+9A8h` |
| 4Dh MT_MULTISELECTUNIT | 0076CFD0 | formats a `"**LOFING..."` line into an unused buffer; for the local slot stores the u16 at msg+1Ch into `[[00E198C4]+40h]+78h` (no HUD null check) |

`007712F0` (client enter-play): only while `[+18Ch]+9Ch < 8` (signed, 0077131C): set it to 8, clear
`current[local]+19h` for a slot in 0..7, clear `game+193Ch`, `00644220(0)`, send MT_CREATED (07h)
to the server. `00784B10` sends MT_KICKED (vtable 00D03174, `+18h` flag) to one peer when the peer
is in the transport's list; `00784790` serializes a message to every peer except one, each under
the peer's critical section (`peer+4h`).

## Chat relay, scene selection, removal, scoring (batch 2)

`00771DD0` request-ready (player, ready, aux): host applies `0076CA40`, a client sends
MT_READYPLAYER to the server, local mode does nothing. MT_SCENEIDX calls it as
`(local player, 0, 1)` when the announced scene differs from the selected one.

Chat (`MT_CHAT`, 21h). `007722E0` hands the line to `00772020` in host mode; a client shows it with
`005D0770` when the chat screen `[00E198B4]` exists. `00772020` relays by channel `msg+20h`:

| channel | action |
|---|---|
| 0 | show locally (player 0 is the host); `005D0770` gets channel argument 0 |
| 1..7 | whisper: `00770B50` to `players[channel]+50h`, **no mute filter** |
| 8 | `00770C50(msg, sender xuid, 0)` to every unmuted player, then show locally |
| 9 / 10 | `00770D40(team 0 / 1, ...)`, then show locally when `current[local slot]+28h` is that team |

`00770C50` skips AI-held, unclaimed and excluded records, records muted with the sender
(`007786E0`), and the sender itself; a record without a peer is traced in Hungarian
(`"###%d#Nem_kuldom_el mert nincs peerInfoja!_4 %x"`, "not sending it, no peer info") instead of
being sent. `00770D40` asks the mute table in both directions (redundant, since `007786E0` is
symmetric), requires a peer and `record+28h == team`, and is silent for a missing peer.

Scene selection. `0076E710` (MT_SCENENAME) calls `004E1D70(name or "", 1, msg+28h, 0)` and counts
successes in `[00F1AF2C]`. `007724A0` (MT_SCENEIDX) compares `[game+5FCh]+90Ch` with the announced
name before and after `004C6890(msg+20h)`: a change before un-readies the local player and leaves
the tactical library; a mismatch after clears `[+18Ch]+C4h` and requests MT_REFRESHSCENE, a match
sets it; it then refreshes the lobby scene panel and stores `msg+28h/+2Ch` in `game+2060h/+2064h`.
The server's MT_REFRESHSCENE arm `007717A0` (batch 5, ECX session, push `[msg+8]`) re-sends every
scene to the asking peer, then the selected index. Scenes are the `game+5F0h` nodes of the list at
`game+5F4h` (node `+4h` next, `+8h` record; record `+90Ch` name, `+920h` second string); the walk
stops at the last node when a next pointer is null. Builders: `00770FF0` MT_SCENENAME (vtable
`00D036D0`, strings `+18h`/`+20h`, index `+28h`) and `00771190` MT_SCENEIDX (vtable `00D036F8`,
name `+18h`, index `+20h`, `game+2060h/+2064h` at `+28h/+2Ch`), which is the layout `0076E710` and
`007724A0` read on the client.

`00773420` MT_REMOVEPLAYER (msg+18h = player): with the online manager present, unregister the
remote talker and remove the xuid from the mute table; destroy the record's peer; slot > 0 is
restored to its mission record, keeping the departing name in game states 0Ah/0Dh/10h (the compare
is `JLE` at 007734B2, so **slot 0 is never restored here**); slot -1 clears `+0Ah` of the current
record whose name matches (`004BB690`, case-sensitive); then `[00F8A2FC]->vtable[15Ch]({xuid,
name})` without a fresh null check, and `004D0FC0(record)`. Ghidra's export ends cleanly after the
`_free` call; the listing 00773587..007735AC confirms the tail (`RET 8`).

`0076D1B0` `__MMultiPlayer::ServerSendSceneScoring`: trace, `00905370` on the scoring object
`[game+21A0h]` (a plain `RET` in this image), `0090ABB0` on the eight 284h rows at
`[game+21A0h]+4h`, then a tail jump to `007848F0` on `+188h` (or `+18Ch`). Callers: 007746F0
(MT_SESSION_MANAGER state 2 on the host) and 00920A20 (cc9's debrief bring-up). The per-row writer
`0090ABB0` and the scoring object belong to the mission scoring area, outside this packet.

## The MultiScore net table (batch 3)

`MultiScore` is an entity (constructor `00780DB0`, already named `BSP_MultiScore_Construct`; it
creates the Lua globals `MultiScore` and `MultiScore_save`, each with eight sub-tables) whose job is
to replicate the per-player Lua table `MultiScore[p]` from the server to the owning peer. Its
message is type `E0h`, which the image's table names **`MT_MNETTABLESYNCENTITY_SYNC`** (and `DFh`
`MT_MNETTABLESYNCENTITY_CREATE`), so the class is a net table sync entity; "MultiScore" is the
table it syncs.

| address | role | vtable slot |
|---|---|---|
| 00782020 | sync to one peer (reconstructed: change rule and statement text) | `00D04230 +10h` |
| 00781F60 | build the E0h message from the change list (`0075B430(E0h)`, vtable `00D03824`) | |
| 00781590 | E0h write | `00D03824 +4h` |
| 00781910 | E0h read (reconstructed: the Lua text it produces) | `00D03824 +8h` |
| 00779FC0 | `msg+18h` (u16) = `entity+174h`, then `00770B50(peer, msg)` | |

`00782020` (`__thiscall(entity, holder)`, `RET 4`; `[holder]` is the peer and `p = peer+20h`) does
nothing for a null peer, for `p == 8`, or when `MultiScore` or `MultiScore[p]` is not a table. It
iterates `MultiScore[p]`; for each pair the old value is `MultiScore_save[p][key]` (by index for an
integer key, by name for a string key) when the save table exists. The listing (00782200..0078245A)
fixes the objects: `[ESP+24h]` key, `[ESP+38h]` new value, `[ESP+68h]` old value, `[ESP+A8h]` save
table. A pair is unchanged when old and new have the same `lua_type` and, for numbers, compare equal
or, for strings, are the same interned string (pointer compare); any other same-type pair (tables,
booleans) is unchanged. A changed pair is collected only when the key is an integer (`00B66A60`) or
a string and the value is an integer (sent as `(float)int` through `008869F0`) or a string
(`00B660A0` is an exact `LUA_TSTRING` test, no number coercion). **Fractional numbers, booleans
and nested tables in `MultiScore[p]` never replicate.** After the loop it always runs
`MultiScore_save[p]=copy_table(MultiScore[p])` through `00B65EC0` (`luaL_loadbuffer(..., "DoBuffer")`
then `lua_call`), and sends one E0h message when anything was collected.

Wire order (writer 00781590, reader 00781910, after the extended header): the count in **5 bits**
(the writer does not clamp, so a 32nd change would wrap), then per entry `bool key_is_string`, the
key (owned string or 32 bits), `bool value_is_string`, the value (owned string or numeric float
`(0, 1, [00D7A248], 32 bits)`). The reader rebuilds `msg+28h` as Lua source, one statement per
entry: `MultiScore[p][key]=value;` with `p = current[local slot]+20h` on the receiving side, string
keys and values in double quotes without escaping, integer keys in decimal (`004CACA0`) and float
values as `"%f"` (`0043BDF0`). The punctuation constants are `"["` 00CF5980, `"]["` 00D0424C,
`"]="` 00D04248, `'"'` 00CE436C and `";"` 00CE5698. Which routine executes `msg+28h` on the client
was not traced.

## MT_ADDPLAYER and three small receivers (batch 4)

`00773120` MT_ADDPLAYER (session, msg, &status), `RET 8`. `004BB550` fills the player record from
the message (player `+18h`, name `+28h`, `&msg+48h`, xuid `+20h/+24h`, bytes `+58h`/`+54h`, `+5Ch`,
`+B0h..+BCh`); then record `+90h = msg+60h`, `+94h = msg+61h`. The local player index is read after
the fill (007731D1). The record's peer (`+50h`):

- player 0 (the host, seen from a client): the first node's `+8h` of the client transport list
  (`[+18Ch]+0Ch`), or null when `[+18Ch]+10h` is 0;
- any other player: a new `0xDC0` peer target (`007839D0(0,0)`, vtable `00D03860`, then
  `007827D0` with ECX = `target+D6Ch`). Its `sockaddr_in` at `+D74h..+D83h` is AF_INET, port
  `htons(1000)` (`PUSH 3E8h` at 0077325B), address from `XNetXnAddrToInAddr(XNADDR copied from
  msg+62h, [game+207Ch]+96Ch, &sin_addr)`, written only when `[00F8A2FC]->vtable[198h]()` is true.
  `target+D50h = record` in both cases (007732F6).

So clients hold direct targets to every other client on port 1000, which matches the
`"__P2P_VOICE__ PeerTableToC"` string in `007752D0`. **Correction to earlier docs:** the "peer
metadata" at `peer+D50h` that `docs/SESSION_TEARDOWN_LATCHES.md` and the MT_SCENETERM arm
(`0076D260`, `[[msg+8]+D50h]+1Ah = 0`) touch is the sender's player record (written at 007732F6;
for the server's own peers the same field is presumably set by `007752D0`, not checked here).
With the online manager present it then calls `vtable[158h]({xuid, name}, msg+9Eh)`,
`vtable[ACh](xuid)`, `00778620` on `session+190h` and `vtable[28h]()`; the local player's record
finally goes to `007F8690(record, {game+734h, game+730h, 0})`. `vtable[198h]` is called without a
null check, so a client without the online manager would fault on a non-zero player.

`0076CD20` MT_CONNSTATE composes the existing `update_native_smoothed_remainder_triplet_007862C0`:
for i = 0..7, ECX = `session+0Ch+i*1Ch`, floats `msg+18h+i*0Ch` and `+4h` (the third float of each
group is unused). `00777820` MT_HINT: outside host mode, `00690FD0` on `004E1CA0(msg+18h, 1)`.
`00772710` (MT_CHECKSCENELOAD client arm): `r = [00F8A2FC]->vtable[168h]()`, `005D6CD0()`, send
0Dh with `+18h = (r == 0)` (SETZ at 00772746) to the server, `session+4h = 0`.

## `_free` flow repair in this lease (2026-10-12)

`docs/CC13_FREE_FLOW_REPAIR.md` skipped five `_free` (`00BF65AC`) call sites inside this lease that
still carried `CALL_RETURN` overrides with undefined bytes after the call. They were repaired with
copies of cc13-flowfix1's scripts (same checks: instruction present, override `CALL_RETURN`, Ghidra
bytes equal disk bytes, rel32 call to a `_free` target), under the Ghidra write lock, and saved:

| site | result |
|---|---|
| 007846E6, 0078471C, 00784756 | all three belong to `007844E0`: overrides cleared to NONE, fall-throughs disassembled (45, 49, 37 bytes), function deleted and re-created with `disassemble_first=false`; name, signature and plate comment compared equal; body end 007846EA -> 00784773 (`RET`, then INT3 padding) |
| 00784083, 00784097 | the flowfix census listed them outside any function. They are the body of a function at `00784060` (INT3 padding before it, `RET` at 007840A7, reached by the `JMP` thunk at `00784340`) that had no Ghidra function and no ledger record. Overrides cleared, fall-throughs disassembled (11 and 20 bytes), and the function re-created as `FUN_00784060` (`007840A7` end); it frees every node of the list at `this+4h`, then the head, and zeroes `+4h`/`+8h` |

Fresh exports of both functions now run past every `_free` call.

## Packet cc13-net2: ServerAddPlayer, SESSION_MANAGER, presentation arms, 0Dh server arm

Written by cc13-net2 and committed by cc13-net3 after a power outage interrupted net2 before its
commit; net3 re-checked that every Ghidra rename below survived the restart (all 24 present) and
rebuilt. All C++ here is reconstructed and build-tested only; nothing is game-validated.

### `007752D0` MMultiPlayer::ServerAddPlayer (session, peer), `RET 4`

Named by the log string at `00D03C88` ("MMultiPlayer::ServerAddPlayer: slot: %d/%d, private: %d/%d,
will: %d  Rank:[%d,%d]"). The order, from the listing:

1. 007752EE..007754C7, claim a record through `004BB440`. The peer is the *local join* when it is
   the first node of the server transport list (`[+188h]+0Ch`, count `+10h`) and not
   (`session+F4h == 1` and `session+168h != 0`). A local join takes its name from the profile
   (`game+6A0h`, else `+68Ch`), its tag from the char* at `00F88A38`, xuid `game+698h/69Ch`, and
   rank from `00574050` (ECX `game+650h`); it also sets `game+18C8h = record+1Ch`. A remote join
   first posts the host template chat `005CF320(8, 7, "", name)` and takes name/tag/xuid/bytes/rank
   from the join info at `peer+8h`. `004BB440` returns null when all eight records are claimed, and
   007752D0 dereferences it without a check. `peer+D50h = record` (the field the net1 correction
   predicted).
2. 007754C9..00775552: record `+94h` = the XNet platform (`XNetInAddrToXnAddr` then
   `XNetGetXnAddrPlatform` on `peer+D78h`), or 2 when the peer has no address.
3. MT_MYPLAYER (25h, vtable `00D030C0`) to the peer: player, then the session nonce
   `+26Ch`/`+268h`.
4. 007755C1..00775885: for every other claimed record, MT_ADDPLAYER built by `0076B760`, the
   `"_ADD_PLAYER_ Server:... Peer:..."` trace, then MT_SELECTPLAYER (19h) when its slot `+20h >= 0`
   and MT_READYPLAYER (1Dh, both bytes constant 1) when `+0Ch` is set.
5. 0077588B..00775960: MT_AIENABLE (1Eh) and MT_AISKILLLEVEL (1Fh) for the eight current records
   (`game+18CCh`).
6. The `"__P2P_VOICE__ PeerTableToC"` trace and the new record as MT_ADDPLAYER to everyone
   (`00784790`, no exclude), then MT_CHANGEMULTISETTINGS (17h, the 38h-byte BSS block `00E1955C`)
   and MT_SUBSCENE (18h via `007710E0`) to the peer.
7. 00775BF8..00775D01: `session+12Bh` ("will") decrements for a remote join, `0076DBE0` recounts the
   slot counters, the log above, and MT_SERVERINFO (05h, `0076B670`) to everyone with the mean peer
   quality from `00786030`.
8. 00775D06..00775DC9: an automatic slot choice through `0076C840` HostSelectPlayer: in effective
   game mode 7 (`004BCA50`) the first AI-held current record; otherwise the first AI-held record of
   the team (`+28h`) with fewer human records (team 0 on a tie), -1 when that team has none.
9. A local join ends in `007F8690(record, game+734h, game+730h)`. A remote join gets the "PIMPS":
   `"Sending PIMPS to new client %d:"`, then for every other claimed record MT_PLAYERINFO (E3h,
   record `+E0h..+E8h` and the player) and MT_PLAYERUNLOCKUNIT (DDh, record `+110h`), each traced.

Helpers reconstructed with it: `0076B760` MT_ADDPLAYER builder (04h, vtable `00D03048`, `RET 80h`),
`0076B670` MT_SERVERINFO builder (05h, vtable `00D02FF8`, `RET 30h`), `007710E0` MT_SUBSCENE
builder, `0076DBE0` slot recount (ECX `session+100h`: `+128h` claimed records, `+12Ah` those whose
join `+58h` is set), `00786030` mean of the conn-state quality floats of rows 1..7 (1.0f when none;
row 1 carries a no-op `FADD double 0.0`).

### `007746F0` MT_SESSION_MANAGER (E4h) and `004C9780`

`004C9780` builds E4h (vtable `00CE7608`, `RET 8`): `+18h` state, `+19h` player, `+1Ch` an empty
`std::list` of 38h-byte stats entries. `007746F0` logs " - - ONLINE - - SESSION_MANAGER PlayerIndex
%d, State  %d" and runs a five-state handshake on record `+A4h`:

| state | side | action |
|---|---|---|
| 1 | client, first time (`session+4h`) | `00772A40` sends the local score (MT_SCENESCORING 15h), reply state 2, flush peer buffers (`007848F0`) |
| 2 | server | record `+A4h = 2`; when `0076C560` says every remote player is at 2: `00920A20` debrief (cc9's), `0076D1B0` ServerSendSceneScoring, online `+74h = 0`, online `vtable[184h]`; the local current record goes to 4 either way |
| 3 | client | replace the online stats list `[00F8A2FC]+2200h` with the message entries (word 0 overwritten with `[game+207Ch]+9CCh`), then `004C04C0` on the first entry and `00626C50` |
| 4 | server | record `+A4h = 4`; when all are at 4, online `vtable[58h]` and broadcast state 5 |
| 5 | client | online `vtable[58h]`, reply state 5 |

The status word is never written. `004C04C0` (XUSER property writes) and `00626C50` are not opened.

### Presentation arms 2Bh..34h

All `(session, msg, &status) RET 8`; none writes the status word or touches unit or mission state.

| type | address | effect |
|---|---|---|
| 2Bh MT_BLACKOUT | `0076F4C0` | `005B9BA0` fade on `[HUD+A4h]` with an empty completion name (no Lua) |
| 2Ch MT_COUNTDOWN | `007709C0` | start (`00733FF0`) or stop (`007340A0`) the countdown at `game+21E8h` and show/hide the HUD counter (`0052B9B0`/`0052AB90`); the client copy has an empty callback, so expiry runs no Lua |
| 2Dh..2Fh | `0076D420`, `0076F560`, `0076F630` | resolve the uint16 entity id at `msg+18h` (`0076BD70`/`0076BE10`/`0076BEA0`, identical bodies, no bounds check) and call the movie camera's hand-mode / polar / radius setters (`007A6B40`, `007A6E60`, `007A7070`) |
| 30h | `0076A870` | `007A7C60(camera, msg)` |
| 31h | `0076A8A0` | new 18h interpolator (`0078E370`) added by `0079F6D0`, also when the allocation failed |
| 32h | `0076A940` | `005CAF90` deviation factor on the screen `[HUD+98h]` |
| 33h | `0076D760` | `007A2D30` with (tick - `[00F876B0]`)/1000.0 seconds; `session+284h += 1` outside host mode |
| 34h MT_MOVCAM_BLACKBAR | `0076A970` | black bars on `[HUD+9Ch]` (`005CBC30`/`005CBCC0`) and `[HUD+A0h]` (`005CB0F0`/`005CB130`) |

The first camera use switches the interface to mode 2Bh (`005CD160` -> `004CC460(HUD, 2Bh, ...)`).

### `007F7460` MT_CHECKSCENELOAD (0Dh) server arm

`__thiscall(record, byte) RET 4`, ECX = the sender's record `[msg+14h]`: record `+14h` = (byte != 0)
as a dword (SETNZ). The byte is the client's `([00F8A2FC]->vtable[168h]() == 0)`. Readers of
record `+14h` are not mapped.

### Never-defined function bodies (24)

The 24 bodies `reports/cc13_undefined_function_sweep.json` listed in this lease were disassembled
and created with `disassemble_first=false` under the write lock, and the program saved; the event
log is `reports/cc13_net2_defined_functions.json`. They are unnamed (`FUN_`):
0076AAB0, 0076AB30, 0076AC40, 0076B090, 0076E190, 0076E240, 0076EC50, 00774D70, 007761D0,
00776490, 00778950, 00778980, 007789C0, 00778E70, 00779090, 0077B780, 0077D910, 007810A0,
00782910, 00782A80, 00783950, 00786E10, 00786E30, 00786F30. All 24 were present in Ghidra after
the restart.

## The MultiScore client side: who runs the E0h text (cc13-net3)

The MultiScore primary vtable is `00D040B8` (written at 00780DEE by `00780DB0`; net1's
"`00D04230 +10h`" is slot `188h` of it). Its net slots:

| slot | address | role |
|---|---|---|
| 17Ch | `00780AE0` | send MT_MNETTABLESYNCENTITY_CREATE (DFh) to one peer (reconstructed) |
| 180h | `00780A00` | schedule a sync to one peer holder (reconstructed; Ghidra function created by net3, `BSP_MultiScore_ScheduleSync`) |
| 188h | `00782020` | sync to one peer (net1) |
| 18Ch | `00780B90` | apply a received E0h (reconstructed) |

**Receive route.** E0h's is-a (`00768440`) answers E0h, 48h and 46h. The ladder's 46h arm
(MT_NETENT, 00778193) hands every such message to `00780670`: the game must be in state 0Dh, the
message tick (`msg+0Ch`) must be due, `msg+18h` must resolve through `00521E30` to an entity with
`+BDh` set; then is-a 48h (MT_SYNC) selects `0077C710` (007807A4), which drops a message older than the
sender's 16-bit stamp in the list at `entity+2A4h` (node `+8h == msg+14h`; a stamp more than
30000 ahead counts as wrapped and accepts), skips a sender with no node, and otherwise stores the
new stamp and calls
`entity->vtable[18Ch](msg)` (0077C79B). For MultiScore that is `00780B90`: a borrowed Lua state
owner on `[[[00E188A8]+1A08h]+4]+4`, then `00B65EC0` with data `msg+2Ch` (`00F871AC`, an empty
string, when null) and length `msg+28h`, which is `luaL_loadbuffer(L, text, length, "DoBuffer")`
and `lua_call(L, 0, -1)` with no check of the load result. The entity itself is not read. So the
client executes `MultiScore[p][key]=value;...` exactly as 00781910 rebuilt it, with `p` the
receiver's own local slot.

**Create.** `00780AE0` builds DFh inline (`00766FC0(DFh, 0, entity, 0)`, vtable `00D03804`,
`+C4h = 0`, `+10Ch = game+18C8h`) and sends it through `00779FC0`. `00780A70` (Ghidra function created by net3, `BSP_NetTableSyncCreate_Construct`; no xref found) is the same constructor as a `__thiscall(msg, entity) RET 4` routine. Both C++ builders run the
base constructor through `net_cc13::build_entity_create_00766fc0` (corrected by cc13-seg1; the
first version never called it), then set the vtable and overwrite `+C4h` and `+10Ch`. The `+10Ch`
value is read from `game+18C8h` through `NetTableCreateHost::game_local_player_18c8` after the base
constructor returns, as both image routines do (corrected by cc13-fix4; the caller used to pass it
in, read before `00766FC0`). DFh uses the
shared entity-create codec (write `00767D70` -> `00765000`, read `007683B0` -> `00767E30`) and its
is-a answers DFh, 47h (MT_CREATE) and 46h, so `00780670` splices it onto `00F871A0` for `0077EC20`, which calls
`msg->vtable[1Ch]()` (0077EC8E) when `msg+1Ch` names no entity yet. For DFh that is `007813B0`:
`operator new(314h)`, zeroed, `00780DB0(obj, (u16)msg+1Ch)`. The host creates its own MultiScore
at 004D557F (net1 cites the gate: game mode not 8 and `game+1FE4h == 1`).

**Sync schedule.** `0077C540` walks the net entities (`00F87198`); for each peer holder whose
`[holder+50h]+0Ch == 8` it calls `vtable[180h](holder)`. MultiScore's `00780A00` queues
`(entity, holder, 0)` into the bucket counted by `00F1B14C` (arrays `00F86550`, `00F56550`,
`00F26E50`, capacity 300h) when `entity+310h` is set or when `[00F876B0] - holder+4h`, as an x87
int, exceeds `1/0.05f` (`00D0DE84`), i.e. 20 ticks or more; a queued row clears `+310h`. The same
function then drains all 40h buckets with `vtable[188h](holder, byte)` (0077C62A).

**Correction:** `00782020` is `RET 8` (0078274A), not `RET 4`: the drain pushes a holder and a
byte. In the stretch read (0078261D reads the holder at `[ESP+644h]` with the EH-state slot at
`[ESP+63Ch]`) the byte is not used; the rest of the body was not swept for it. The ledger
evidence and the header comment are amended.

**The force flag `+310h` (corrected by the follow-up scan).** The constructor stores 0 (00780E2F,
`BL` from `XOR EBX,EBX` at 00780DE1), and `00780A00` reads and clears it. Its setter is the Lua
binding **`ForceMultiScoreSend`**: the binding table entry `00E0C1F8` holds the name pointer
`00D0F988` and the function `008AFEC0`. When `game+1FE4h == 1` (EBX = 1 at 008AFEDB), that function
walks the list `[[game+19CCh]+49Ch]` (count `+498h`) and stores 1 at `[node+8]+310h` (008AFF95).
Scan method: the opcodes `C6 /0 imm8` (with and without SIB) and `88 /r` for displacement 310h.
The positive control is the clear at 00780A58, which the scan finds. The scan finds only these
two register stores (00780E2F and 008AFF95) and no immediate store of 1. `008AFEC0` is a Lua
binding (cc9's area), so it was not renamed.

**`holder+4h` (the last-sync tick).** A holder is the node payload `node+8h` of the list at
`entity+2A4h`. `0077EB50` (`__thiscall(entity, record)`, called at 0077ECC8 in `0077EC20`, 0077F072 in
`BSP_UnitOwnerEntity_Construct`, 0077F0C0 in `0077F090` and 0077F26C in
`BSP_UnitOwnerEntity_ActivateAndRegister`) appends `{record, FFF0BDC1h = -999999, u16 stamp 0, 0}` to it, appends
the record to the list headed at `entity+29Ch`, and sets `00F87190 = 1`. `0077DA20` removes both. The only refresh of
`holder+4h` is `00779F80` (`holder+4h = [00F876B0]`, `RET 4`). A rel32 scan of `.text` finds four
call sites for it (positive control: the four `0077EB50` call sites are found), all unit sync
senders: `0074D16B`, `007C29CB` (BSP_Plane_SendControlStateMessage), `00816B97`
(BSP_UnitInstance_SendShipSyncMessage) and `00859AED` (BSP_TurningGun_WriteAimSnapshotFields).
Nothing in MultiScore code refreshes it, and the only literal `[00F876B0]` read there is in the
scheduler. So **for MultiScore the 20-tick test always passes**: every `0077C540` pass (once per
`BSP_Session_FlushOutboundStep` on a server, from the fixed-step fan-out) queues a sync to every
in-game peer. Each sync diffs `MultiScore[p]` and runs `MultiScore_save[p]=copy_table(...)`. That
makes `ForceMultiScoreSend` redundant while the tick stays at its seed. This is an inference from
the listing and has not been observed in a run.

Not opened: `0077C540` beyond the lines cited.
## The generic net-entity create message (cc13-net3, second packet)

Reconstructed in `src/net_entity_create_cc13.cpp` / `include/bsp/net_entity_create_cc13.hpp` on the
existing bit-cursor primitives (`native_bit_cursor_*.hpp`); build-tested only.

`00766FC0` (`__thiscall(msg, type, word, entity, with_name) RET 10h`) is the base of the `*_CREATE`
messages built by its 11 callers. It calls `0075B430(type)`, sets the base vtable `00D03720` and
snapshots the entity into a 110h-byte message. The base vtable is `[0] 007669F0, [4] 00765000 write,
[8] 00767E30 read, [Ch] 006D44D0 is-a, [10h] 004499C0, [14h] 006E0650, [18h] 006D44C0`, and slot 1Ch is
`00BF698E`, which looks like the pure-call stub (not checked): each derived class supplies its own
factory there, as DFh does with `007813B0`.

| field | source | wire (writer 00765000) |
|---|---|---|
| +10h type | constructor argument | u8, 8 bits |
| +18h | 0 (`00779FC0` later stores `entity+174h`) | word, 12 |
| +1Ah | 0 | bool |
| +1Ch net id | `entity+174h` | word, 12 |
| +1Eh | constructor argument | `00429030` 10 bits; read back **unsigned** (`00428CB0`) |
| +20h parent id | `[entity+3Ch]+174h`, 0 without a parent | word, 13 |
| +24h..+2Ch | position `entity+A4h..+ACh` | 3 numeric floats |
| +30h..+38h | `0042D2E0` angles of the matrix at `entity+74h` (ECX matrix, EDX `&msg+30h`) | 3 numeric floats |
| +3Ch | `entity+5Ch` | bool |
| +40h | `entity+28Ch` | u32, 16 |
| +44h, vector +48h | 0 in the base | bool, then a 16-bit count (written signed, read unsigned) and 2 floats per entry |
| +58h | 0 | u32, 2 |
| +5Ch | 1 | (value+1), 4 bits |
| +60h, +64h..+84h | `entity+188h..+1A8h`; +60h set when all nine are equal | bool, then 9 x 7 bits, or one 7-bit value when uniform |
| +88h, +8Ch..+ACh | set when the entity is not kind 2Ah (`vtable[5Ch](2Ah)`): `entity+1ACh..+1CCh` | bool, then 9 x signed 7 bits |
| +B0h | `entity+54h` | u32, 4 |
| +B4h, +B8h | kind 5: `entity->vtable[12Ch]()` | bool, u32 3 |
| +BCh, +C0h | the `with_name` argument; `00438E40` copy of `entity->vtable[10h]()` | bool, byte-length string |
| +C4h | `entity+58h` | u8, 4 |
| +C5h.. | kind 2 with a unit group at `entity+284h`: +CAh `group+4FCh`, +CBh `group+500h`, +C8h `[group+14h]+174h`, +C6h `[group+14h] == entity`; when not the leader, kind 18h copies 3 floats from `0070D980(group, &out, entity)` to +CCh, kind 6 copies record +10h..+2Ch of `0070D080(group, entity)` to +D8h..+E4h / +ECh..+F8h | bool, u8 8, u8 8, word 12, bool, then 3 floats (kind 18h) or 4 interleaved float pairs (any other kind) |
| +100h, +104h | +104h = the length of matrix row 0 (`entity+74h..+7Ch`, 0 when its square is at most 1e-10), +100h = (abs(length - 1.0) > 1e-4) | bool, then one float |
| +108h | `entity+304h` | numeric float |
| +10Ch | `entity+180h` (DFh replaces it with the local player) | u32, 4 |

All numeric floats use `(zero 0, signed 1, scale [00D7A248] = FLT_MAX, 32 bits)`, the raw 32-bit
path. The reader `00767E30` (cursor at `stream+4`) mirrors the writer: it decrements +5Ch and
fills all nine words of +64h from one uniform value. It resizes the +48h vector through `00766E30`
(`resize(count, {0, 0})`) and allocates the name through `00428DA0`.

Observations (from the listing, not seen in a run):
- A negative +1Eh does not survive the trip: it is written as 10 raw bits and read zero-extended.
- In a group member that is not the leader and whose kind is neither 18h nor 6, the writer still
  sends the eight floats at +D8h/+ECh, which the constructor never wrote. The derived classes may
  zero them; that was not checked.
- The writer passes +C0h to `004290D0` unchecked whenever +BCh is set, so a null name would fault.

**For cc9 (not opened, not touched):** what creates units on the receiving side is the per-type
factory in `msg->vtable[1Ch]`, called by `0077EC20` with the follow-up `vtable[154h]` calls for
+8Ch and `0077EB50(entity, [msg+14h])`. The builders and their message types are AIRFIELD `006D4C70`
(A3h), GUN `0072A1D0` (ABh), LANDCONVOY `00741A40` (CFh), LANDFORT `00743DB0` (D0h), GRNDVEHICLE
`0074CDB0` (CDh), PATH `007B2950` (D8h), PLANE `007BD3E0` (C0h), PLANESQUADRON `007EF490` (BBh). Two
are intermediate bases that take the type as an argument: `006E0D20` (vtable `00CF9324`) and
`00813A20` (vtable `00D09410`). The derived writers and readers that call `00765000`/`00767E30`
directly (for example `00741450`, `006E0690`, `006D5130`, `00813BD0`) carry unit-specific fields
and belong with those builders.

## `004C04C0` the online stats write (cc13-net3)

`__thiscall(game, 12 dwords) RET 30h`, named `BSP_OnlineStats_WriteSessionStats` (hypothesis).
Callers: `004CAA90` BSP_OnlineStats_UpdateWrite, `007746F0` (state 3), `00A43AF0`, `00A47B80`. It
returns at once when `game+22DDh` is set. Otherwise it builds `XUSER_PROPERTY` records (id `+0h`,
type `+8h`, value `+10h`) in the scratch slots `game+2210h..+2288h` and `XSESSION_VIEW_PROPERTIES`
at `game+22A0h + i*0Ch`, then calls `XSessionWriteStats(session, xuid, count, views, overlapped)`
(`00A4D428`, 004C0846):

- **Listed path.** Taken when `id_5` is in the int vector `00E19458..00E1945C` and
  `[00F8A2FC]->vtable[198h]()` returns true.
  - Ranked (`ranked_7` low byte set): view `FFFF0000h` with two int32 properties, `1000800Ah` =
    arg 10 and `1000800Bh` = arg 9.
  - Not ranked: the views are built only when the xuid argument equals `00A3EB00`'s local xuid.
  - Then each positive value adds one int64 view:
    - arg 12 as `table[row][2]` in view `table[row][ranked]`. The 8x3 table is built on the stack
      at 004C04DE: (0Dh,0Eh,2000008Eh), (0Fh,10h,2000009Dh), (11h,12h,20000081h), zeros,
      (0Bh,0Ch,20000091h), (9,0Ah,20000092h), (7,8,20000083h), (5,6,20000082h). The row index is
      not bounds-checked.
    - arg 13 as `20000084h` in view 4.
    - arg 8 as `200000BCh` in view 1.
- **Otherwise.** A non-zero arg 12 becomes one view: id 2 with `20000080h` when `id_5 - 1 < 14`
  (unsigned), else id 3 with `2000008Fh`.
- **Tail.** The `XOVERLAPPED` at `game+7038h` (1Ch bytes) is zeroed and passed only when
  `game+1FE4h` is non-zero; otherwise the write runs synchronously. `XGetOverlappedExtendedError(0)`
  (`00A4D422`) runs for any result other than `ERROR_IO_PENDING` (3E5h). When nothing was written,
  the result compared is the whole `ranked_7` argument. Finally `game+22DDh = 1` when
  `game+1FE4h` is non-zero, so in a networked session each game writes stats once.
- Arg 11 is never read.

**The listed-id vector is probably always empty.** `00E19458`/`00E1945C` lie in `.data` past its
raw size (`00E18000`), so they are zero at load. A byte scan for their absolute addresses finds
only the two reads in this function (positive control: both reads found). Nothing references
`00E19450..00E19463` by literal address either. Unless something fills it through another base
(a neighbouring leaderboard object at `00E19430..00E1946C` is read by `BSP_LeaderboardsScreen_*`),
the listed path never runs, and every stats write is the single view 2/3 property.

## Out of scope or not reconstructed

Owned by cc9 (unit / mission state; listed, not touched): `0077A080`/`0077C880` formation
replication, `0077D600` `BSP_Entity_IssueCommand`, `0077B0C0` and `0077FAD0` (the `reconlevel`,
`lastBanto`, `_reconData` Lua property readers; `lastBanto` is a unit field, not a ban list),
`0077C160` (unit group), `00779BB0` recon level getter, `00920A20` debrief.

No ban-list code was found in this segment: the packet keyword `lastbanto` is the unit Lua field
`lastBanto` read by `0077C160`/`0077FAD0` and `LastBanto` in `0077D270`.

`007746F0`, `007752D0` and the presentation arms 2Bh..34h, listed here by net1 as routed but not
reconstructed, are now covered in the cc13-net2 section above. Still not opened: `004C04C0`
(XUSER stats property writes), `00626C50`, `00920A20` (cc9's debrief), and the movie-camera
internals `007A6B40`/`007A6E60`/`007A7070`/`007A7C60`/`007A2D30`.

Outside the segment, routed by the ladder: see the table (the `Other` and `MessageThis` rows).

## Checks

- Build: `scripts/build.ps1` in the worktree, exit 0, no warnings from the new file.
- `tools/verify_report_calls.py`: 52 call sites cited in this doc (CALL and tail-JMP rows, each with its
  enclosing function) checked against live Ghidra, 0 failures.
- Listing checks: 0076D840 (ECX session, `RET 8`), 0076D140 (tail jump to 00784B10 with
  `[ECX+188h]`), 0076D170 (`NEG/SBB/ADD 4`), 0076C7B0 (`__allshl` operands), 007712F0 (signed
  `JGE`), 00771630 (`ESI = [00F8ABE8]` for 00A3EB00, `LEA ECX,[EDI+190h]`), 00773420 (`JLE`, tail),
  007724A0 (both compares use `[game+5FCh]+90Ch`), and the ECX of every `00784790` call in this
  file (`[session+188h]`).
- `_free` callers among the reconstructed bodies: 00773420 and 00773120; both tails were read in
  the listing (00773587..007735AC and 007733C7..0077341D).
- cc13-net2 section: 40 cited callee calls (in 007752D0, 007746F0 and the 2Bh..34h arms) checked
  with `tools/verify_report_calls.py` on a JSON built from the live listing, 0 failures. Listing
  checks by net3: 007F7460 (SETNZ, `RET 4`), 0076A970 (both screens, `[ESP+8]` = msg after the
  push), the local-join test at 0077531A..00775332 (`session+F4h == 1` and `+168h`).
- MultiScore client section: 9 cited calls checked the same way, 0 failures; listings read for
  00780B90, 00780AE0, 007813B0, 0077C710, 00780A00/00780A70 (disasm-raw, INT3 padding on both
  sides), the 0077C540 drain loop and the 00782020 `RET 8`.

## Open questions

1. Who reads record `+14h`, the scene-check result `007F7460` stores (the 0Dh server arm). Searched
   by net3 without a hit: the listings of every function in this segment and in `004BB*` that uses
   the 118h record stride (looking for `[reg+14h]`, which also covers the biased base
   `game+750h`), and a byte scan for the game-indexed displacement `75Ch`. Its 34 hits all belong
   to other structures. A reader through a record pointer elsewhere would not show up in either
   search.
2. Record `+A4h` is the SESSION_MANAGER handshake state. It is written only by `007746F0` (00774809
   = 2, 0077486B = 4, 007749FE) and read only by `0076C560` (through the base `game+750h` as
   `+9Ch`), as far as the same searches reach (game-indexed `7ECh`: 11 hits, the only record one in
   `007746F0`).
3. The client stage at `[+18Ch]+9Ch` takes 3/4 on MT_KICKED and 8 on entering play; the other
   values and their readers are not mapped.
4. `msg+0Ch` is the send tick; whether 00777850's drop log can fire in a real session (state below
   0Ah with a queued message) is untested.
5. The mute table's xuid lookup ignores the player records; a xuid added by MT_ADDPLAYER stays in the
   table until MT_REMOVEPLAYER, so rows can outlive slot moves. Not observed in a run.
6. Whether anything fills the listed-id vector `00E19458` (see `004C04C0`).
7. Whether the MultiScore per-step resync (see the holder paragraph) costs anything noticeable in a
   hosted session.

## Handoff

Packet cc13-net1 totals: 68 Ghidra renames with evidence comments, 61 reconstruction records,
about 60 reconstructed C++ functions. Packet cc13-net2 (net2, finished by net3 after a power
outage): 29 more renames, 24 reconstruction records, 26 never-defined bodies created in Ghidra.
cc13-net3's second packet: 7 renames (`00766FC0`, `00765000`, `00767E30`, `004C04C0`,
`00779F80`, `0077EB50`, `0077DA20`) and 4 reconstruction records, with the codec in
`src/net_entity_create_cc13.cpp`. All C++ is build-tested only; nothing is game-validated.

Done: the net2 packet items (1)-(6); the generic entity-create codec; `004C04C0`; the `+310h`
setter (`ForceMultiScoreSend`, `008AFEC0`, cc9's Lua binding, not renamed); the `holder+4h`
writer (`00779F80`, never called for MultiScore). Ghidra functions `00780A00` and `00780A70` were
created under the write lock on the lead's go-ahead; `_free` still reads not-noreturn afterwards.

Left: the open questions above; `00766E30` (vector resize, STL-like, unnamed); `0077C540`'s
per-class bucket choice (`00F87150 = tick % 12`) for the other entity classes; the derived create
codecs and unit factories, which belong to cc9.
