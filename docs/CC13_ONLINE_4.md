# cc13-online4: segment 71 online code (finish)

Packet cc13-online4 is a reconstruction pass that finishes segment 71 (00A371C0-00A46930) after
cc13-online3 (docs/CC13_ONLINE_3.md). It covers the network client connection, the console sender,
the network server, and the larger XLiveSystem voice, friends and stats routines. It also covers the
three helpers that cc13-online2 left behind host interfaces.

At the start, 54 functions in the range had a ledger name but no record. This packet adds records for
51 of them. The other three belong to cc9 (see "For cc9" below).

Status of the code: everything is **reconstructed and build-tested**. None of it is ABI-compatible
or game-validated.

## Files

- `include/bsp/online_seg71_cc13.hpp` and `src/online_seg71_cc13.cpp` were extended. Everything after
  the "Packet cc13-online4" banner in each file comes from this packet. No CMake change was needed.
- `reports/cc13_online4_calls.json`: 164 call-site rows covering 54 functions. Each row is a direct
  CALL/JMP to an address that the online4 header names. `tools/verify_report_calls.py` checked all 164
  against live Ghidra and none failed.

Each function's calling convention, stack cleanup, return register and evidence addresses are in the
header comment above its declaration. Every RET form was checked against the disk bytes by sweeping
each body to the next function (`local/on4_rets.py`). Every host interface method names the native
routine, import or global it stands for.

Where the original holds MSVC checked-iterator validations (calls to `_invalid_parameter_noinfo`
00BF6713), the C++ keeps them in the server list walks and in 00A46150. It drops them from 00A44540 and
00A45C60, where they cannot fire on a well-formed list.

## Records (54)

- **cNetworkClient connection (12):** OpenSockets 00A39300, SendConnect 00A39870,
  EnumerateLanServers 00A39900, ResolvePacketSource 00A39B10, Connect 00A39B80, JoinLocalPlayer
  00A39C50, LeaveLocalPlayer 00A39CC0, DisconnectFromServer 00A39DC0, Destruct 00A39E30,
  RemoveRemotePlayer 00A39F00, Open 00A39F70 and HandlePacket 00A3A160.
- **Wide strings (2):** 00A3A0C0 AssignCString and 00A3A110 AssignPadded.
- **cNetworkConsoleSend (16):** 00A3ABA0, 00A3AC80, 00A3AEA0, 00A3B0F0, 00A3B780, 00A3B7F0, 00A3B870,
  00A3BBB0, the five IPv4 lookups 00A3BD20..00A3BF50, RecvFromLocked 00A3C080, SendToLocked 00A3C140
  and ReceivePacket 00A3C3C0. The sender's constructors, channel reset and queue workers were already
  reconstructed in `src/native_network_console_*.cpp`. Those files were not touched.
- **cNetworkServer (9):** CloseSystemLink 00A3D280, Open 00A3D480, AnswerLanProbes 00A3D540, Update
  00A3D6F0, SetKey 00A3D740, ResolvePacketSource 00A3DB10, Construct 00A3DC80, Destruct 00A3DCF0 and
  HandlePacket 00A3E000.
- **XLiveSystem (12):** 00A429E0, CreateStatsSession 00A43AF0, ClearFriendsList 00A43FA0,
  ResetVoice16 00A44230, Startup 00A44490, AddRemoteTalker 00A44540, AddFriend 00A45C20,
  RefreshFriends 00A45C60, RemovePlayer 00A46270, SendLocalVoice 00A46330, UpdateFriendsAndVoice
  00A46480 and ResetVoice 00A46620.
- **cc13-online2 helpers (3):** 005707A0 BSP_SessionRecord_Reset, 00571E50
  BSP_SessionRecord_Construct and 00A46150 BSP_StatsListMessage_Construct.

### Names

These changes are applied in the ledger and in Ghidra (logs: `local/ghidra-annotations-*.json` in this
tree).

- 00A3B7F0 `BSP_NetworkConsoleSend_RegisterAddress` is now `BSP_NetworkConsoleSend_RegisterSocket`.
  Both callers pass the console's game socket +858h (00A3A05E and 00A3D51B), and 00A3B870 removes the
  entry by socket.
- 00A46150 (it had no name) is now `BSP_StatsListMessage_Construct`. Its only caller is 00A47B80
  BSP_XLiveSystem_WriteSessionStats.

## The connection protocol, as the image shows it

- **System-link discovery.** The client sends a 9-byte probe from its broadcast socket to
  255.255.255.255:3EBh, at most 10 times per open. The probe is tag 0AAh followed by an 8-byte
  XNetRandom nonce. The server answers on the same port with a 14Ch-byte reply:
  - tag 99h
  - L"GameName" at +2
  - the nonce at +42h
  - XNKID at +4Ah, XNKEY at +52h, XNADDR at +62h
  - an 88h-byte host info at +86h, with its size at +148h

  The client accepts a reply only when it is exactly 14Ch bytes, the nonce matches and the XNKID flag
  bits are 0 or 40h. It hands the host info to the client vtable +1Ch.
- **Connect.** The server treats packet type 0 from an unknown sender as a connect request, and its
  vtable +24h gives the verdict.
  - Verdict 0 accepts: the new target is added to the peer list.
  - Verdicts 2, 3, 4 and 5, and any other non-zero verdict, send refusal types 3, 4, 9, 10 and 2. Type
    9 carries a wide refusal text from vtable +20h.
  - The client maps refusal types 2/3/4/9/10 to multi-error-screen messages 1..5 (global 00E08898,
    read by the BSP_MultiErrorScreen_* routines). For type 9 it puts the text in 00E1953C.
  - Type 1 accepts on the client side: the client wrapper goes to state 7.
  - Type 7 means disconnect in both directions.
  - When [00F8A2FC]+75h is set, the server forces verdict 1, which sends refusal type 2.
- **XSession flags.** Client Connect creates its session with 42Eh, or 43Eh when [00F8A2FC]+4Dh is
  set. CreateStatsSession uses 706h. The meanings come from the SDK's XSESSION_CREATE_* flag set:
  10h would be USES_ARBITRATION, and 706h would be presence + stats with invites, join via presence
  and join in progress all disabled. These readings are provisional; the image does not name the
  constants.

## Findings

1. **00A39300 waits without a limit.** It retries XNetGetTitleXnAddr until it succeeds. When
   XSocketCreate fails, it still binds and sets options on socket -1.
2. **00A39B10 attributes peer packets to the server.** When the sender matches one of the eight game
   players' targets, the function returns the server target +874h, not that player's target. A null
   `from` skips the first test, but the player scan still reads `from+4`.
3. **00A3A110 pads with U+2020.** It pads with `memset(.., 20h, 2*n)`, so each added character is
   2020h (a dagger), not a space. It always targets the static string 00E1953C.
4. **00A39F70 null-tests [00F8A2FC] only once.** The next line calls through the pointer anyway.
   Its stack argument (1000 from 00A41660) is never used.
5. **00A3B870 (ResetForSocket) has three faults:**
   - It empties the whole 82Ch list but frees only that socket's packets. Other sockets' packets are
     dropped from the list without being freed.
   - Its 820h sweep repeats while any entry remains, so an entry of another socket makes it loop
     forever.
   - After each removal it skips the entry that slid into the freed position.
6. **00A3B7F0 never checks capacity.** The socket array at +160h has room for 12 entries before the
   flags at +190h.
7. **00A3C140 tests the wrong result.** It reads WSAGetLastError for every non-zero sendto result,
   which includes every successful send.
8. **00A3D540 sends uninitialised stack and trusts the host info size.**
   - Reply byte +1 and the gaps between fields are uninitialised stack.
   - The vtable +28h host info size is not checked. Anything over C2h bytes runs into the size
     field and past the reply.
9. **00A3E000 skips three checks:**
   - The new 0DC0h target is used without a null test.
   - For a known-pointer but unregistered source, the XNADDR from XNetInAddrToXnAddr goes to a stack
     buffer and is dropped.
   - XSocketHTONS(+80Ah) is computed and discarded.
10. **00A3DB10 and 00A3DCF0 keep their list iterator in the object.** It is stored at +874h/+878h,
    so a nested walk would clobber it. 00A3DCF0 frees the list nodes but not the peer targets.
11. **00A43AF0 leaks its session handle.** It creates its stats session synchronously and never closes
    the handle with XCloseHandle. When the key appears in several stats categories, the last category
    wins.
12. **00A44540 runs its talker-flag pass twice.** The second pass is identical to the first.
13. **00A44230 and 00A46620 are adjacent vtable slots that disagree in size.** They are slots +0D8h and
    +0D4h of vtable 00D24878. 00A44230 empties 16 lists from +1F40h and sets 16 flags from +1D4Ch.
    00A46620 handles 4 of each and also clears the player list +10Ch. Whether +1F70h..+1FFFh hold 12
    more lists is not established.
14. **00A46330 assumes at least one packet.** It walks the chat packets with a do-while, so a zero
    packet count alongside a non-zero AX would wrap.
15. **005707A0 does not reset every field.** It leaves +40h, +44h, +4Ah, +4Bh, +80h and +84h
    (settings[0Ch..0Dh]) untouched, so a reused record keeps them.

## For cc9

These are gameplay-adjacent, and this packet left them out:

- 00A38DA0 BSP_Session_SpawnPlayerVehicles (2B0h). `multiplayer_session_cc13.cpp` only names it.
- 00A371C0 BSP_AiGlobals_Reload: CALL 004C1C50, MOV ECX,EAX, JMP 00A335D0. It belongs to the AI
  globals loader (`ai_target_weights.hpp`).
- 00A37250 BSP_AiGroup_FromLuaEntityArgument (`lua_binding_ai.hpp` names it).

## Handoff

Segment 71 has no unrecorded named function left outside the three cc9 items. Still open:

- No host wiring. The new routines are free functions over native offsets behind host interfaces,
  and nothing in the game host calls them yet.
- 00920690 (called by 00A43AF0 with ECX = [game+6B4h]) has no ledger name.
- 0056F280 (the checked wstring clear used by 005707A0) has no ledger name.
- The meaning of [00F8A2FC]+75h, which forces a connect refusal and is cleared by server Open.

Scripts are in this tree's `local/`:
- `on4_rec.py` and `on4_rec2.py` build the records.
- `on4_calls.py` builds the call rows.
- `on4_rets.py` checks the RET forms.
