# Raw mission-score record destruction

Addresses: `00593570`, `00590B40`, `00585FF0`, `007FD510`,
`0058AD40`, `0058AE10`, `0058AF30`, `0058B000`, `0058B0D0`, `0058F100`,
`00591A30`, `00591C00`, `00592F10`, `00593180`, `00582B00`, `00582B80`,
`00582C00`, `00582C80`, `00582D00`, `0058BCD0`, `005914B0`, `00591800`,
`00592A20`, `00592EA0`. Read-only producer: `0091CE90`.

## Result

Mission-score subtree cleanup now invokes the complete normal raw score-record
destructor by default. The former required payload service has a concrete binding
through `NativeProfileCollectionCalls::call_00593570(record, strings)`. It destroys
the actual record's owned storage, including populated nested maps.

`00593570..00593C9F` is 1,840 bytes, ECX record, RET. It was previously analyzed only
and truncated in Ghidra. This is one new unique reconstructed game function. Its
22 supporting library functions are scoped normal storage contracts; the existing
`7FD510` caller receives composition evidence. These are not generic STL ports.

Sources: `include/bsp/native_mission_score_record.hpp`,
`src/native_mission_score_record.cpp`, and `native_profile_collections.hpp/.cpp`.
Reports: `reports/native_mission_score_record_r108.json` and
`reports/native_mission_score_flow_r108.json`.

## Owned storage and schedule

The 288h extended storage contains the 284h base record and its extra count.
Destruction preserves unrelated scalars, allocator words, padding, string headers
and the extended count. It does not free the record itself.

| Native fields | Normal cleanup |
| --- | --- |
| Strings +264,+25C,+254 | Read each current pointer/length when reached; release length+1; preserve headers. |
| Objective maps +240,+234,+228,+21C,+210,+204 | Right/current/saved-left tree traversal; each node destroys its second NativeString before its first. |
| Counters +1F8,+1EC | Existing native string/int tree clear, current sentinel free, head/count zero. |
| +1BC | Scalar tree with nil byte +3D in 40h nodes. |
| +1B0,+1A4,+198,+15C | Scalar trees with nil byte +15 in 18h nodes. Payloads are not interpreted. |
| Plain vector begins +150,+140,+130 | Free nonnull current begin, clear begin/end/capacity; preserve preceding allocator word. |
| Counter +120; scalar/float +114,+108,+FC,+F0,+E4,+D8 | Full current range cleanup and current sentinel retirement. |
| Nested counters +CC | Destroy each child tree at node+10 before freeing its containing node. |
| Three-level counters +C0,+B4,+A8,+9C | Recursively destroy both nested levels, preserving each captured left continuation. |
| Counters +90,+84,+78,+6C,+60,+54,+48,+3C,+30,+24,+18 | Clear/free in this order; final nine calls use the native `590B40` wrapper. |

This is **36 trees, three strings and three vectors**, totaling 42 owned fields.
Read-only producer `91CE90` establishes all 36 sentinel offsets and nil bytes;
the fixture layout is mechanically checked against those instructions. This
packet does not reconstruct or execute a new source version of that constructor.

All trees traverse right first and save left before destroying the current payload.
Nested nodes use nil +1D, inline child header +10 and child head +14. Each child
sentinel is reread for its free; child head/count are zeroed before outer-node free.
The objective pair helper `585FF0` receives node+C and destroys header+8/data+C
before header0/data4. It differs from the transient pair helper from R107.

Four 18h scalar subtree bodies have identical bytes after masking only relative
CALL operands. The wide-scalar body differs only at its two nil offsets. Four
nested subtree bodies likewise match after CALL masking, with distinct child
range targets. These comparisons support sharing the storage algorithms without
inventing payload semantics.

The ten newly bound range functions implement only destructor-generated complete
current ranges. Partial iterator ranges, returning validation on malformed graphs,
concurrent mutation and native private-stack alias behavior remain outside scope.

## Ghidra repairs and preserved evidence

All mutations used the existing `bsp.gpr` / `/battlestationspacific.exe`, owner lease
and write lock. No installation bytes or callee no-return flags changed.

`593570` originally ended at `59363F`. Explicit tail decoding recovered 1,632 bytes
but left the stored function truncated. Locked recreation then restored the full
range through RET at `593C9F`, preserving its reviewed name and prior comment.
`590B40` similarly required recreation through `590B73` after a 16-byte tail decode.
Both creation requests used `disassemble_first=false`.

Ten internal free-site gaps concealed saved-left continuations; nested variants
also concealed child-header clearing and outer-node frees. Dedicated flow repair
restored all ten, with zero remaining call gaps. Projects were saved, exports
refreshed, and the snapshot forced because function count was unchanged.

The first call audit then found that the outer-node frees at `5914FC`, `59184C`,
`592A6C` and `592EEC` were still outside Ghidra function membership despite being
visible in the repaired listings. A further locked recreation of the four complete
102-byte nested subtree functions corrected those internal membership holes.
The failed audit and subsequent full passing audit are both retained. Production
source and compared native bytes were unchanged by that metadata correction.

Unlisted bytes are unreachable alignment: one seven-byte LEA in the previously
reviewed profile reset and 12 single-byte NOPs following RET14 in range helpers.
Each NOP has a terminating predecessor and no branch target into it.

## Validation and limits

- Strict MSVC Win32 build and all three existing CTests pass. No repository tests
  were added.
- 11,365 bytes match live Ghidra and the installed PE: 9,375 bytes in 39 copied
  cleanup/composition bodies, 1,930 bytes in the read-only producer, and 60 bytes of
  constants/literals. All 329 direct CALL rows are checked; 293 are fixture
  relocations and 36 belong to the producer.
- Eight original/source cases match **1,019 ordered observations and 65,354,952
  bytes**. The populated score case alone has 610 observations. It contains three
  real-layout records, one with all 36 trees populated, including complete
  three-level counter maps, three owned strings and three vectors. The other two
  records have empty containers. The enclosing score tree's left/key mutation
  during record cleanup agrees with saved-left/current-key behavior.
- The remaining cases retain full game/profile construction, null allocation,
  name alias, publication substitution, repeated reset, returning validators and
  transient-tree coverage from R107. Required map/settings/game providers remain
  controlled; score payload and nested cleanup now execute actual bodies.
- Source-only checks cover nested construction failure, a free exception inside
  actual score cleanup and late Dyn failure. The mission parent retains its partial
  graph and `7FD81B`/state -1 and rejects replay. Original score-record FH3 cleanup
  and native exception identity are not implemented or claimed.
- Source-only real heap checks cover 42 counted allocations/frees: three mission
  sentinels plus 36 score sentinels and three score vectors; three score strings and
  two nonempty alias-list strings/nodes are also released. Opaque fields and the
  extended record count remain unchanged.

The application executable matches R107 except timestamps, and all 53 application
objects are identical. These raw game/profile paths remain unconnected to actual
application ownership; runtime was not rerun. Fixture/build evidence is not native
ABI, FH3, gameplay or visual validation. Tested and combined build archives are
recorded in the report.

## Follow-up

Recover the actual `91CE90`/`91D620` record construction and connect the remaining
profile counter-map/settings providers. Raw record copy/reset, native unwind and
application game-owner construction/destruction remain required for the full goal.
