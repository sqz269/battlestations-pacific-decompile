# cc13-online2: segment 72 online code, finished

Packet cc13-online2 finishes segment 72 (00A46B80-00A625D0): the XLive system, XSession, matchmaking,
host QoS and voice code, the movie decoder base classes, and the library entries. It continues the
"Part B not done" list in docs/CC13_GUI_2.md.

At the start, 62 functions in the range had a ledger name but no reconstruction record and no tag.
All 62 are now closed: 55 have reconstruction records and 7 have tag rows. The query
(`ledger_name` set, `recon_kind` empty, `tag_category` empty) now returns 0.

Status of the code: everything is **reconstructed and build-tested**. None of it is ABI-compatible
or game-validated, because online play was never run. Reads used live `bsp.py ghidra decompile`
and `disasm`/`disasm-raw`. No snapshot or bulk export was taken.

## Files

- `include/bsp/online_cc13.hpp` and `src/online_cc13.cpp` (extended). New sections start at
  "Packet cc13-online2".
- `include/bsp/movie_bases_cc13.hpp` and `src/movie_bases_cc13.cpp` (new). They are registered in
  `cmake/startup.cmake` under `bsp_core`, next to `movie_decoder.cpp`.
- `reports/cc13_online2_calls.json`: 58 call-site rows. `tools/verify_report_calls.py` checked all
  58 against live Ghidra and none failed.

Every routine takes the native object as `void*` and reads native offsets. XLive, XHV, CRT, STL and
game calls go through host interfaces, and each interface method names the native routine or import
it stands for.

## Online player record (40h)

`OnlinePlayerRecord` is the payload of the 48h std::list nodes in system +100h (Live players) and
+10Ch (LAN players). It is also the single record at system +128h, the local player (00A478D0
returns that address). Sources for the layout:

- the stack frames of 00A47130 (record base at ESP+0Ch) and 00A47220 (base at ESP+34h);
- 00A45F70, which calls operator_new(48h): next, prev, then 40h of payload;
- the copy constructor 00588640, which copies every field.

Fields:

- +0h: MSVC9 std::string, the name.
- +20h: identity. A Live player stores its XUID here. A LAN player stores the six abEnet bytes of
  its XNADDR at +20h and its user slot at +26h.
- +28h: copied, but neither builder writes it.
- +2Ch: 0.
- +30h: user slot.
- +34h: 0.
- +38h..+3Bh: flag bytes {0, 0, 1, 0}. 00A47220 also copies +3Ah to system +1D4Ch[slot].

## Session record (A0h) and matchmaking result (F6h)

The session record is the element of the result vector at system +174h. 00A48450 builds records
from the hosted QoS entry and from matchmaking results. 00A488D0 builds them from LAN servers.
Fields:

- +0h: a new 88h holder object (vtable 00CEED04) with XNKID at +4h, XNKEY at +0Ch and XNADDR at
  +1Ch.
- +4h: index byte. 0FFh for both kinds of search result; for LAN records, the server index.
- +8h: std::wstring, the name.
- +24h: property 10000030h.
- +28h: filled slots.
- +2Ch: total slots.
- +30h: median RTT. +38h and +3Ch: up and down bits per second. LAN records use 0 and 7D0000h.
- +48h and +49h: bytes of properties 10000047h and 10000048h.
- +4Ch: the 0Eh-dword settings block.
- +88h: property 200000D7h. +90h: 100000D9h. +94h: 100000DAh. +98h: 200000DBh.

00A49DF0 fills the F6h matchmaking result from an XSESSION_SEARCHRESULT and its properties. From
+3Ch onwards, the result layout is the session object's layout (as 00A4A890 writes it) shifted by 6.

## Records (55)

- **Movie decoder bases (32).** 00A4C470, which is the media-manager singleton base deleting
  destructor (publication [00F8AEF8]). The six trivial slots. The texture getter/setter at shared-8
  (00A4C750, 00A4D090). Twelve vtordisp thunks. Construct, destruct and scalar-delete for the
  decoder, MovieFileBase and MovieTextureBase. 00A4CCC0. 00A4D0B0 and 00A4D0C0.
  - The vbtable dword [1] values are read from the image: 00D24DE4 = 0Ch, 00D24DEC = 10h,
    00D24E3C = 8, 00D24F24 = 0Ch.
  - The host keeps vtable words as identities. It does not dereference them.
- **System (9).** 00A47130, 00A47220, 00A47AA0, 00A48330, 00A46D60, 00A46F60, 00A48450,
  00A488D0 and 00A47B80.
- **XSession (8).** 00A49C80, 00A4A790, its thunk 00A4B020, 00A4AF20, 00A4B030, 00A4AC40,
  00A4A890 and 00A4BB00.
- **Voice (1).** 00A493C0.
- **Matchmaking and QoS (5).** 00A4B490, 00A4B2A0, 00A4B7F0, 00A49DF0 and 00A4A4E0.
The large routines (00A488D0 Update, 5C0h bytes, plus 00A48450, 00A49DF0, 00A47B80 and 00A4B7F0)
were read from the listing, not only the decompile.

### Renames (ledger and Ghidra)

| address | old | new | why |
|---|---|---|---|
| 00A4CCC0 | BSP_MovieTextureBase_Slot04 | BSP_MovieDecoderBase_SetAllTrackVolumes | Clamps to [0, 1], multiplies by 32768.0 and calls BinkSetVolume for tracks 0-3. It is slot +4 of 00D24E78 and of 00D24ED8. |
| 00A48330 | BSP_XLiveSystem_EnsureLocalPlayerListed | BSP_XLiveSystem_JoinSessionRecord | RET 4 with one A0h record. It replaces the results with that record and calls 0057A9C0(record, 1). |
| 00A48450 | BSP_XLiveSystem_RefreshHostQosUi | BSP_XLiveSystem_RefreshSessionRecords | It rebuilds the result vector and then cancels or resets the search. It makes no UI call. |
| 00A47350 | BSP_XLiveSystem_BuildStatsRecord | STL_XLiveStatRecordVector_Assign_00a47350 | It is std::vector<50h record>::operator=. It is tagged, not reconstructed. |

The 00A474E0 rename from cc13-gui2 (`STL_PlayerList_CopyConstruct_00a474e0`) was also applied in
Ghidra. The annotation log is in this tree:
`local/ghidra-annotations-20261013T013851Z.json`.

### Tags (7, evidence id `cc13on2`, action `reviewed`)

- `stl_instantiation`: 00A47350.
- `block_telemetry`: 00A500E0, 00A53140, 00A59C80, and the two _vsprintf_s wrappers 00A586D0 and
  00A586F0. Their only caller is 00A59C80.
- `block_pipe_ipc`: 00A607EE CreateNamedPipeW (an IAT thunk).

## Findings

1. **00A4A890 publishes the previous game mode.** The mode properties 100000C9h..100000CDh are
   computed from the stored +58h, and the new settings are stored only at the end. So the mode
   properties always lag one call behind. Listing: 00A4A8A6..00A4A96C.
2. **Property 100000D5h is published twice.** Settings[6] and settings[7] both use it (00A4AAF8 and
   00A4AB18), so [7] overwrites [6]. The reader 00A49DF0 matches it: both its D5 tests store the
   same value into +6Eh and +72h.
3. **00A4B030 sends misaligned QoS data.** The data pointer is [00E0E3F0]+1 and the length is
   2*wcslen([00E0E3F0]+2). The wide string L"Battlestations..." at 00D24A3A is therefore sent
   starting one byte early. The QoS listen key is session +C7h throughout (00A49C10, 00A49C80,
   00A4AC40, 00A4B030).
4. **The host QoS entry array has room for one entry.** Entries start at +0 with stride 76h, and
   the count sits at +78h. 00A4A4E0 copies every search result anyway, so a second result
   overwrites +78h onwards. This adds to finding 1 of cc13-gui2 (the overlapping pointer arrays).
   00A4A4E0 also runs XNetQosLookup with 0 probes, where the matchmaking path uses 8.
5. **00A49DF0 drops hosts whose XNADDR platform is 1.** It calls XNetGetXnAddrPlatform and skips
   those results. It never initialises the mode field +5Eh before the property loop. The UNICODE
   property copy into +20h has no length limit.
6. **00A4B7F0 adds mode filters only for combined queries.** For each mode it tests
   `(mask & bit) == single`, where single = (query id in 1..5). A single-mode procedure therefore
   adds no mode property. A combined procedure adds a 0-valued property for every mode left out.
   Only q0 and the low dwords of q1 and q2 are read; q3..q5 are ignored.
7. **00A4B490 uses a prefix match.** The composed C/D/E/S/I key is matched against table rows
   5..30 at 00F8AD48 (stride 10h) with 0043E9A0, which tests whether the row key starts with the
   composed key.
8. **00A47B80 uses record[0] as scratch and reads the oldest list node.** Each stat record is
   copied into record[0] before it is appended to the list at +2200h. The list is never cleared
   there, so the WriteSessionStats arguments come from the first node ever appended.
9. **00A493C0 ignores the privilege value.** It calls 00A3EB30 but reads only its return; the
   output byte is never read.

## Uncertainty

- Every host method that stands for an STL instantiation is a contract taken from its call site:
  00573490 erase-all, 00575150 push_back, 00566310/005663B0/00573C90, 00A47560 sort, 00993990,
  004C6090 + 004CF800. The bodies were not re-read.
- These native routines are kept behind host methods and were not reconstructed here: 005707A0,
  00571E50, 004C5E60 (wide string from a C string), and the session message 00A46150 + 0076A9F0.
- `MatchmakingSearchGlobals` is the image's BSS block 00F8AC08..00F8AEE7 with its native layout.
  The query table is filled at static initialisation, so the host provides it. Its contents (the
  key strings) were not read.
- 00A47B80's empty-vector path hits a checked at(0), which calls 00BF6713. The C++ returns
  instead, and a comment marks this.

## Handoff

The packet is complete: segment 72 has no named function without a record or a tag. Possible next
steps:

- Reconstruct the host-side helpers listed under Uncertainty, starting with 00571E50 and 005707A0
  (session record constructors) and 00A46150 (the stats message).
- Read the query table strings that 00F8ACF8 receives at static initialisation, to name the
  combined-mode procedure ids.
- Scripts are in this tree's `local/` with the prefix `on2_`: `on2_targets.py` (the target query),
  `on2_dec.py` (cached compact decompile), `on2_mk.py` (record batches), `on2_tags.py`, and the
  batches `on2_b1.txt`..`on2_b3.txt`. The copied cc13-gui2 and scene2 tools were retargeted to this
  tree.
