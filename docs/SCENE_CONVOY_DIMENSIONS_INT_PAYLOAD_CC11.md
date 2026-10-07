# LandConvoy owning integer dimensions

Packet `cc11_scene_convoy_dimensions_int_payload`; Source baseline `4f59c8d00`.
Only the actually called Rows and Columns reads in `retain_land_convoy_roster`
now prefer compatible I/i `integer_value` when `has_integer` is true. One local
called reader and two replacements are the implementation. The original shared
integer lambda, all Position1..4 reads, five scalar fields, Reverse, Path,
class/generation gates and downstream policies remain unchanged.

The fallback still scans `values.back()` with `scene_scan_int` for any type,
starting at zero for these two dimensions. Raw F/non-I incidental decimal-prefix
acceptance, failed/empty/missing fallback 0 and per-call assignments/reset remain
exact Source compatibility. This is not a no-reset path. Source Rows then
Columns order is preserved; native reads Columns then Rows. The existing product,
slot/symbol vector resizing and later slot writes are unchanged.

## Native data boundary

Read-only target: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.
Names remain descriptive hypotheses. Endpoints below are inclusive last bytes
with separate exclusive ends.

Parent `00743450` has stored body `00743450..00743B6F`, exclusive end
`00743B70`, 522 listed instructions and terminal RET at `00743B6F`.
Only `007434EB..0074351C`, end `0074351D`, 50 bytes, is adopted here:

| Field | Verified literal | Find -> 008F2260 | Stored +0C load | Convoy store |
| --- | --- | --- | --- | --- |
| Columns | 00CEE9F4, `Columns` plus NUL, 8 bytes | 007434F3 | 007434F8 | 00743501 -> +358h |
| Rows | 00CFF344, `Rows` plus NUL, 5 bytes | 0074350F | 00743514 | 00743517 -> +354h |

The Columns setup at 007434EB uses the holder bag at +8; 007434FB reloads
holder +C0 and 00743507 selects its bag for Rows. Neither field checks the
returned property for null or type, or converts it. The native successful I
payload is a DWORD; Source compatibility/type gating is explicitly narrower.
Native missing-key dereference/fault is not Source's zero fallback.

Prior setup support is unchanged: ESI receives ECX at 00743474; holder +C0
must exist and its kind at +4 must be 1; +8 supplies the bag. Preceding
0077E830 Lua/self attachment, later allocation/roster construction, holder/EH
and full ABI are external. The prior root 15-byte returning-free continuation
repair at 00743B4B..00743B59 and refreshed 522-instruction body are supporting
history, not new worker flow recovery. Prior group-merge tail/body limits remain.
Only the two Find calls are current report direct-call rows; the live verifier
passes both with zero failures.

Owning-I producer/copy support comes from
[SCENE_TYPED_INT_PAYLOAD_CC11.md](SCENE_TYPED_INT_PAYLOAD_CC11.md): parser arm
008F5B48..008F5BDF/end5BE0, fresh type0/+0C producer 008F3710, scalar
constructor 008EF140, clone 008F4FF9..008F502C/end502D and assignment
008F0716..008F0725/end0726. Native read-success metadata is not retained as
an action. Existing owning scalar/group copies are data, including zero.
Modern MSVC `%d` remains the unchanged Source provider for one recognized,
nonempty explicit I, ordinary in-range C-locale decimal-prefix token and closed
semicolon. Historical BF7533 CRT, overflow/error/locale/NUL, implicit or
conflicting declarations and empty-existing contexts remain unbound. No enum
identity/provider or default map is introduced.

## Existing production connection and fixture

The real class table has LandConvoy class 1Ah. SceneReader merges library groups,
then authored properties, and dispatches this reader for class 1Ah. The public
`GameSceneEntityRecord` retains Rows/Columns and immediately sizes its real slot
vectors from their product. `GameUnitsHost::build_land_convoy_roster_00743450`
uses those slots; `bind_land_convoy_motion` copies the retained dimensions into
formation state. Existing mission-frame and Lua GenerateObject callers reach
those methods under the current enabled convoy switches. No downstream source
or switch is changed.

One ignored manifested fixture includes the actual production TU, freshly
compiles the parser and changed standalone TU, and calls the actual private
roster reader into that public record. There is no tiny standalone public host
entry: whole SceneReader/VFS/descriptor/units/emitter services are compiled
connections and are not forced or approximated to execute this data path.

Actual installed 15 library inputs parse into 22 production Source groups.
`landconvoy.props` authors Rows I1 at line 3 and Columns I1 at line 42.
Production parser/library merge and independent copied data retain 1/1 and one
slot after cleared/opposite diagnostics. Authored compatible I2/i3 overwrite
produces six slots; the original group remains 1/1. A separate fresh decimal-prefix
I2tail/i3 case also retains six slots after owning copy and diagnostic removal.
The installed Type1.Position1 owns I0, but deliberately changed raw -1 still
prevents its slot write: the original shared Position scanner is unchanged.
No enum table is supplied by the fixture, and no ordinal/roster-type claim follows.

The same focused scenario checks owning I0, exact raw LAST token, F/B/S/E
incidental numeric fallback, malformed/empty/missing/bare-I fallback 0,
per-call dimension reset and independent fields. Five scalar values, Reverse
and Path remain unchanged; Reverse's raw-empty behavior is also retained.
Those malformed/non-I checks are Source compatibility, not native fault parity.
Fixture dimensions are 0..3 and product<=6. Negative/product-overflow/overcapacity
and native allocation behavior are outside its admission.

USN1 and JM06 parse through the actual parser and contain zero LandConvoy
entities. They provide parser regression inputs, not installed mission or game
execution evidence for this change. No tracked tests are added.

Strict Win32 compile/link/probe exit 0/0/0; PE014C embeds asInvoker via
`/MANIFEST:EMBED`. All 178 active production Source inputs (177 actual project
includes, including the changed CPP, plus the parser), 17 installed inputs and 77 support inputs
have equal pre/post hashes. Current d670 fully I/B-rebuilt support was frozen
as three libraries and 74 actual Game objects with original-pre/copy/original-post
equality; prior records provided only filenames, not reused binaries. Fresh
active parser/SceneContents inputs replace their older objects; game_main and
old SceneContents are excluded. Uninvoked real dependencies establish linkage,
not game execution. Recipes and complete hashes are in the ignored manifest
and JSON report. Primary post-merge full build/CTest and independent verification
remain pending; whole native attach/ABI/original-game validation is unclaimed.

## Primary integration

Main `a121efaa03744877e75c4ef324181d9744d863f0` passed the full Win32 build and all three existing CTests. Root rebuilt three actual parser/consumer/probe TUs against main, pinned 179 Source/header/fixture inputs (177 compiler includes), 17 installed files and 77 current support inputs before/after, and reproduced the owning/raw/reset/six-slot checks. The worker production-input prose is corrected to 178; its report manifest already had that correct count. All 50 native bytes matched disk/live and both direct rows passed. Whole host/units and enum/runtime binding remain unproved. The PE32 asInvoker manifest was verified. No tracked tests were added.
