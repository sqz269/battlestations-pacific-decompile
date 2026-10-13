# cc13-online3: segment 71 online code (partial)

Packet cc13-online3 is a reconstruction pass over segment 71 (00A371C0-00A46930): the XLive
system's vtable slots and session control, the Xenon system manager, the MNetwork*XLive wrappers,
the network console, and the cNetworkClient session wrapper. It continues cc13-online2
(docs/CC13_ONLINE_2.md) and reuses that packet's structs from `include/bsp/online_cc13.hpp`.

At the start, 198 functions in the range had a ledger name but no record and no tag. 144 now have
records, and 54 are left (listed in the handoff).

Status of the code: everything is **reconstructed and build-tested**. None of it is ABI-compatible
or game-validated.

## Files

- `include/bsp/online_seg71_cc13.hpp` and `src/online_seg71_cc13.cpp` (new, namespace
  `bsp::online_seg71_cc13`). They are registered in `cmake/startup.cmake` under `bsp_core`.
- `reports/cc13_online3_calls.json`: 57 call-site rows. `tools/verify_report_calls.py` checked all
  57 against live Ghidra and none failed.

Each function's calling convention, stack cleanup, return register and evidence addresses are in the
header comment above its declaration. The records point there.

Every host interface method names the native routine or import it stands for.

## Records (144)

- **Back-records (16).** These functions were already reconstructed, but had no record:
  - the 14 AI mission Lua bindings 00A37310..00A38A50 in `src/lua_binding_ai.cpp` (commit
    4c9ddf1c0). Their ABI is `__thiscall(ECX = Lua call frame)`: each prologue does `MOV EDI,ECX`.
  - 00A44EC0 and 00A45010 in `src/network_online_cc13.cpp` (cc13-sweepA1).
- **New reconstructions (128).**
  - XLiveSystem slots 00A42690..00A429D0 (28).
  - Session control 00A42A30..00A43650 (21): countdown, voice restart, host create/refresh, search
    cancel, arbitration, mute refresh, and the voice change message.
  - MNetwork*XLive wrappers 00A410F0..00A42520 (42): constructors, destructors, create
    factories, peer queries, ConnectToServer and ConnectToServerInner, StartHosting, DisconnectPeers,
    PostPeerAddressMessage, and XLiveHostInfo_Assign.
  - Network console 00A39060..00A39500 (8).
  - cNetworkClient session wrapper 00A39570..00A397D0 (7).
  - Xenon system manager accessors 00A3E420..00A3EB30 (9).
  - Small routines (13): client Update, SetServerAddress and Close; the HandlePacket thunks; the
    send/receive thread entries; the record-by-value slots 00A44800 and 00A44820; 00A454F0,
    00A45D90 and 00A46240 StopVoice.

### Rename

00A422F0 `BSP_MNetworkServerXLive_ResolvePacketSource` is now
`BSP_MNetworkServerXLive_DisconnectPeers`. Its ECX is the server subobject (+0A4h).
- With a peer argument, it calls outer vtable +8(server vtable +4(peer)).
- Without one, it sends every value of the list at +868h to outer vtable +8, then empties the list
  and sets +81Ah = 1.

The rename is applied in the ledger and in Ghidra (log: `local/ghidra-annotations-20261013T021620Z.json`).

## Findings

1. **00A42710 repeats the misaligned QoS title data.** It starts QoS listening with the same
   misaligned title data as 00A4B030 (finding 3 of cc13-online2): the pointer is
   [00E0E3F0]+1, and the length is 2*wcslen of the string at [00E0E3F0]+2.
2. **00A416B0 logs the key's address instead of its bytes.** Its "Unreg key" log pushes one
   pointer (00A416CB) where the format expects eight `%x` values. Its XNetRegisterKey result is
   ignored.
3. **00A43130 reuses an argument slot and ignores the query result.** XUserMuteListQuery writes
   `muted` into the low-dword XUID argument slot, and the query's return value is never checked.
4. **00A3EB30 hard-codes user 0.** It checks the privilege of user 0, not the active slot. Its
   output byte means "friends only": it is 1 only when COMMUNICATIONS (0FCh) is denied and
   COMMUNICATIONS_FRIENDS_ONLY (0FBh) is granted.
5. **00A3E470 and 00A3E4B0 scan one slot.** Each scans a single slot and returns 1 (the slot count)
   when nothing matches.
6. **The two create factories never test their allocation.** 00A41C70 and 00A42170 store the new
   object and call its vtable +4 without a null test. 00A41D80 and 00A42480 also write into their
   new objects without a null test.
7. **00A43330 always writes system +1C24h = 1.** This is the voice sub-object's first word. It is
   written after every create attempt, whether the create succeeded or failed.
8. **00A395E0 zeroes an XOVERLAPPED it never passes.** It clears +8E8h..+903h, but passes a null
   overlapped to both XSessionArbitrationRegister calls.
9. **00A39570 leaves two flags uninitialised.** The constructor never sets the ended (+90Dh) and
   deleted (+90Eh) flags, and 00A397D0 tests both.

## Handoff

The packet is partial. 54 targets remain. Size here means the function body in bytes.

- AI / game:
  - 00A371C0 AiGlobals_Reload: three instructions (CALL 004C1C50, MOV ECX,EAX, JMP 00A335D0).
    It belongs to the AI globals loader in `ai_target_weights.hpp`.
  - 00A37250 AiGroup_FromLuaEntityArgument.
  - 00A38DA0 Session_SpawnPlayerVehicles (2B0h). `multiplayer_session_cc13.cpp` only names it.
- cNetworkClient (11): 00A39300 OpenSockets; 00A39870 SendConnect; 00A39900 EnumerateLanServers;
  00A39B10; 00A39B80 Connect; 00A39C50; 00A39CC0; 00A39DC0; 00A39E30 Destruct; 00A39F00; 00A39F70
  Open; 00A3A160 HandlePacket (1C0h).
- Wide strings 00A3A0C0 and 00A3A110.
- cNetworkConsoleSend (17): 00A3ABA0..00A3C3C0. These are the latency and channel tables keyed by
  IPv4, plus the locked send and receive.
- cNetworkServer (10): 00A3D280..00A3E000. HandlePacket is 3A0h.
- XLiveSystem (14): 00A429E0, 00A43AF0 CreateStatsSession, 00A43FA0, 00A44230, 00A44490 Startup,
  00A44540, 00A45C20, 00A45C60, 00A46270, 00A46330 SendLocalVoice, 00A46480 UpdateFriendsAndVoice,
  00A46620.

Other open items:
- The open helpers from cc13-online2 (005707A0, 00571E50 and 00A46150) were not reached.

Scripts are in this tree's `local/`:
- `on3_scan.py`: target sizes and existing mentions.
- `on3_defs.py`: existing definitions per target.
- `on3_rec.py`: records from header declarations. It skips `virtual` host methods.
- `on3_names.py`: ledger names.
- the copied `on2_*`, `gui*` and `sc2_*` tools.
