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

## Out of scope or not reconstructed

Owned by cc9 (unit / mission state; listed, not touched): `0077A080`/`0077C880` formation
replication, `0077D600` `BSP_Entity_IssueCommand`, `0077B0C0` and `0077FAD0` (the `reconlevel`,
`lastBanto`, `_reconData` Lua property readers; `lastBanto` is a unit field, not a ban list),
`0077C160` (unit group), `00779BB0` recon level getter, `00920A20` debrief.

No ban-list code was found in this segment: the packet keyword `lastbanto` is the unit Lua field
`lastBanto` read by `0077C160`/`0077FAD0` and `LastBanto` in `0077D270`.

Routed but not reconstructed here (in segment, reachable from the ladder): `007746F0`
MT_SESSION_MANAGER (online stats write handshake, states 1..5 on `game+7ECh+i*118h`), `007752D0`
`MMultiPlayer::ServerAddPlayer` (23 callees, "Sending PIMPS to new client", "PeerTableToC" voice
peer table), and the presentation arms 2Bh..34h (blackout, countdown,
movie camera: `0076F4C0`, `007709C0`, `0076D420`, `0076F560`, `0076F630`, `0076D760`).

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

## Open questions

1. `0Dh` MT_CHECKSCENELOAD server arm `007F7460` (ECX = `msg+14h` record, byte `msg+18h`) was not
   opened.
2. The client stage at `[+18Ch]+9Ch` takes 3/4 on MT_KICKED and 8 on entering play; the other
   values and their readers are not mapped.
3. `msg+0Ch` is the send tick; whether 00777850's drop log can fire in a real session (state below
   0Ah with a queued message) is untested.
4. The mute table's xuid lookup ignores the player records; a xuid added by MT_ADDPLAYER stays in the
   table until MT_REMOVEPLAYER, so rows can outlive slot moves. Not observed in a run.

## Handoff

Totals for the packet: 68 Ghidra renames with evidence comments (ledger `config/names/`), 61
reconstruction records, about 60 reconstructed C++ functions in `src/multiplayer_session_cc13.cpp`,
all build-tested only.

Next steps, in order of value: (1) `007752D0` ServerAddPlayer, the server half of the
player-slot spine (XNet, the PIMPS/unlock sends and the P2P voice peer table; read past each
`_free` in the listing); (2) the client-side consumer that executes the MultiScore text (`msg+28h` of E0h) and
the `DFh` create message; (3) `007746F0` MT_SESSION_MANAGER together with the
online-stats builder `004C9780`; (4) the presentation arms 2Bh..34h. Leases for
`0076AA00-00787040` were released at the end of this packet.
