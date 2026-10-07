# MaxPlayerNum owning integer consumption

Packet `cc11_scene_max_players_int_payload`; Source baseline `b7b612e31`.

The actual mission-header MaxPlayerNum read now prefers an I/i property's
existing owning `integer_value` when `has_integer` is true. One private called
reader and one call replacement are the behavior change; the old default-site
comment is corrected from 004F2145 to 004F2157. Public `scene_property_int32`,
CompetitiveModeParty, player/slot/pool readers, headers and all other policy
remain unchanged.

The existing caller presence guard preserves missing default 8 with
`max_player_num_authored=false`. A present property keeps authored=true. Outside
compatible owning I, the exact old I/i-only front-token strtol reader remains:
non-I/F, missing diagnostics or failed raw conversion return zero; no new clamp,
reset, default, enum resolution or property-order rule is added.

## Native receipt and ABI limit

Read-only target verified: `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. Names are descriptive hypotheses. Spans use an
inclusive last byte and an explicit exclusive end.

| Containing function | Span | Coverage |
| --- | --- | --- |
| `004F1D70` | Stored body `004F1D70..004F2406`; end `004F2407` | Partial Source adoption: only the MaxPlayerNum word path below. Whole header applier, other fields/services/EH and original ABI remain external. |
| `004F1D70` | `004F2136..004F2160`; end `004F2161`, 43 bytes | Key 00CEA450 is MaxPlayerNum; PUSH 004F2136 and ECX=EBX at 004F213B. The intervening 004F213D store belongs to prior CompetitiveModeParty and is support only. Find 004F2143 -> 008F2260; TEST 004F2148/JZ 004F214A; word +0C at 004F214C -> record+988h at 004F214F; missing default 8 at 004F2157. Native guards null but not type. |
| `004F1D70` | `RET 4` at 004F23E0 and 004F2404 | Both terminal byte receipts are C2 04 00. They contradict historical RET8 metadata; this packet does not bind a formal/native replacement ABI. |

Primary/Astra independently observed entry 004F1D8F `EBX=ECX` and 004F1D96
`EBP=EDX`; sole caller 00469D45 is preceded by 00469D3C loading [ESP+24h],
00469D40 PUSH EDX, 00469D41 EDX=EBP and 00469D43 ECX=ESI. The observed boundary
is ECX bag, EDX record, one pushed DWORD and RET4. Its stack-argument meaning
and full EH ABI remain unclaimed. Old record-on-stack/RET8 descriptions are
stale history; primary will preserve them while appending its correction after
lease release. No worker formal recovery, ledger or Ghidra mutation occurs.

Prior owning-I support is in
[SCENE_TYPED_INT_PAYLOAD_CC11.md](SCENE_TYPED_INT_PAYLOAD_CC11.md): explicit I
arm 008F5B48..008F5BDF (end 008F5BE0), type0/+0C producer 008F3710,
constructor 008EF140, clone arm 008F4FF9..008F502C (end 008F502D), assignment
008F0716..008F0725 (end 008F0726). Source int32/presence are data, including
zero, not parser success/+2Ch/replay metadata. The unchanged modern MSVC
sscanf `%d` owning provider is admitted for one in-range ordinary decimal-prefix
I token with closed semicolon; historical BF7533 CRT, overflow/error/locale/NUL,
implicit/declaration conflicts and empty-existing contexts remain external.
Source's fallback strtol/type policy is compatibility, not native untyped parity.

## Actual connection and focused Source proof

GameMission parses the actual scene header, calls
`read_scene_record_slot_table_004f1d70`, stores `table.max_player_num` in its
`participant_scene_counts`, and passes the retained count pointer to the existing
frame host `run_scene_load_004dfb70`. This is an active production connection;
whole Mission/VFS/frame/participant copying is not forced by the fixture.

The manifested probe links fresh parser, changed public reader and probe objects
with only the frozen current core library. It actually calls the public reader
and existing data-only `scene_slot_records_written`. No Game object, fake
provider/class/enum/allocation service or broad test scaffolding is required.

Three exact installed explicit-I8 witnesses pass: scene.props line 4 in its
first SceneRootProps declaration, and authored USN1/JM06 header line 401.
Both actual mission documents parse and retain count 8/authored=true after
independent owning copy, cleared diagnostics and opposite diagnostics. Multiplay
presence and other raw header/party metadata remain unchanged. The library
witness is parsed directly; no whole library/group/default provider is claimed.

The one focused scenario also covers fresh I8/i2/I2tail/I0, raw front-token
prefix/failed/empty, Source-authored wrong-type/F metadata, missing default8/
authored=false, present empty0/authored=true, bare-I Source fallback, and
unchanged public generic/CompetitiveModeParty behavior. The generic reader still
returns zero for cleared diagnostics; the single MaxPlayerNum read returns
owning data. Fixture counts are ordinary 0..8; participant overcapacity/fault
behavior is not exercised or repaired. No tracked tests are added.

Final strict MSVC Win32 compile/link/probe are 0/0/0, PE014C with embedded
asInvoker. The first probe compile used an incorrect C-array/equality assumption;
only that fixture assumption was corrected. All 22 active production Source
inputs (20 project headers plus parser/reader CPP), three installed inputs and
one required frozen core pre/post hashes match. Current fully B/I-rebuilt
bc083d99d core was copied before the next root build, with original-pre/copy/
original-post equality; no old generated copies or live support are linked.
The live report verifier passes its one direct Find row with zero failures.
Exact recipes and hashes are in the ignored artifact manifest and JSON report.
Primary main full build/CTest and independent verification remain integration
steps. No original-game/runtime, whole header applier, participant array safety,
formal ABI or native allocator/EH proof is claimed.
