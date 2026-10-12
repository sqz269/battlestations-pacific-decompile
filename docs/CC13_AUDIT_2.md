# cc13 adversarial audit 2

Addresses: 00765000 00767E30 00766FC0 00777850 007746F0 0076C560 00782020 0043BDF0 00B66A60
006B1E40 006ADE91 00435D30 0043CC20 0043C1C0 0043C530 00B68460 00BEFDC0 00BF0050 00BEF8F0
00690FD0 005FFB20 00560430 0057A9C0 0057AC10 00568930

Reviewer: cc13-audit2 (Claude), who wrote none of the routines below. Scope: the cc13
reconstructions that `docs/CC13_AUDIT_1.md` did not sample, codecs and parsers first. Every
verdict was checked against the Ghidra listing (`bsp.py ghidra disasm`, uncapped into
`local/au2_*.txt`) or the disk bytes (`bsp.py disasm-raw`, `ghidra bytes`), with strings read from
the PE on disk. Status words: the reconstructions are *reconstructed, build-tested*; nothing here
was game-validated, and no routine below was run in `bsp_game.exe` for this audit.

## Verdicts

| Routine | Source | Coverage of this review | Verdict |
|---|---|---|---|
| 00765000 entity-create writer | src/net_entity_create_cc13.cpp:146 | complete: every field offset, width, signed/unsigned helper, branch and loop count (9 x 7 bits, 4 interleaved pairs at +D8h/+ECh via EBX-14h), FLT_MAX at 00D7A248 = 7F7FFFFFh, RET 4 | CONFIRMED-CORRECT |
| 00767E30 entity-create reader | src/net_entity_create_cc13.cpp:212 | complete: widths, the uniform branch (one read replicated to +68h..+84h), 00766E30(count, {0,0}) resize, RET 4 | CONFIRMED-CORRECT |
| 00766FC0 entity-create builder | src/net_entity_create_cc13.cpp:56 | complete: arg order (RET 10h, type/word/entity/with_name), the x87 row-length sum `(y*y + x*x) + z*z`, 1e-10 (00CE3820) and 1e-4f widened (00D7A268) thresholds, NaN paths, `uniform_60` scan, group arms 18h/06h | CONFIRMED-CORRECT (note N1) |
| 00777850 receive ladder | src/multiplayer_session_cc13.cpp:93 | all 72 rows by script (`local/au2_ladder.py`): type byte, test site, handler(s), and for the 7 split rows that the +188h-non-null arm is the host handler; queue tail 0077836B..0077843D | CONFIRMED-CORRECT |
| 00777850 queue outcome | src/multiplayer_session_cc13.cpp:244 | complete (status 0 log/keep, 2 destroy, else unlink) | CONFIRMED-CORRECT |
| 007746F0 session manager | src/multiplayer_session_cc13.cpp:1358 | complete: five arms, mode word game+1FE4h, `local_current` +A4h store reached on both outcomes, the 12-argument 004C04C0 push order traced slot by slot, 0076C560 (state in ECX, record base game+748h stride 118h) | CONFIRMED-CORRECT |
| 00782020 MultiScore change rule and entry classification | src/multiplayer_session_cc13.cpp:933, 944 | 007822B3..007824B7 (type compare, FUCOMIP/LAHF/JP number test, interned-pointer string test, key/value classification incl. BL flag), snapshot text 00D04070.., send gate | CONFIRMED-CORRECT |
| 00781910 MultiScore source text | src/multiplayer_session_cc13.cpp:896 | key/value formatters only: 004CACA0 -> 004260B0 "%d" (00CE3A34), 0043BDF0 "%f" (00CE4334) via `_sprintf` | CONFIRMED; the "signed int (assumption)" comment is now settled: `%d` |
| 006B1E40 save pickler, float key | src/mission_savedata_lua.cpp:85, 163 | key truncation (CVTTSS2SI then 00B67720), segment "|" + ("" or "#" 00CF828C) + "%f", RET 18h | CONFIRMED-CORRECT |
| 006ADDC0 table-ref path walker | src/mission_savedata_lua.cpp:324 | 006ADE80..006ADF3C: `_stricmp(Substring(1,1), "#")`, end = 1 + strcspn(data+1, "\|") or length | CONFIRMED-CORRECT |
| 00435D30 checkpoint slot writer | src/profile_checkpoint.cpp:62 | complete: key classes, value jump table 00435F28 (types 1..5, light userdata skipped), whole test (floor spilled to float, JNP arm), vtable +4/+8/+0Ch, RET 0Ch | CONFIRMED-CORRECT |
| 0043CC20 play-time text | src/profile_checkpoint.cpp:296 | complete: CVTTSS2SI, magic divides /3600 and /60, IDIV 60, secs = total - 60*(total/60), strings "." 00CE3A70, "\|.:\|" 00CE43C0, "0" 00CE43BC | CONFIRMED-CORRECT |
| 0043C1C0 Lua value text | src/lua_text_dump.cpp:39 | complete: predicate order, every string 00CE4338..00CE4380, WORD +174h handle | CONFIRMED-CORRECT |
| 0043C530 Lua table dump | src/lua_text_dump.cpp:76 | prefix, depth gate, recursion args (indent+1, depth-1), closing "},\n"/"}\n" on indent | CONFIRMED-CORRECT |
| 00B68460 type names | src/lua_text_dump.cpp:32 | complete: JA bound 9, eleven strings from the PE | CONFIRMED-CORRECT |
| 00BEFDC0 TGA decode | src/file_store_cc13.cpp:93 | complete: bpp table 00BEFF7C/00BEFF68 (8->L8 for type 3/11, 16->18h, 24->14h, 32->15h), raw read of ByteSize, row swap through the scratch buffer | CONFIRMED-CORRECT (the short-header `nullptr` is a documented interface deviation: the native ignores the read result) |
| 00BF0050 TGA encode | src/file_store_cc13.cpp:137 | complete: format table 00BF0244/00BF0230, header offsets, depth*8 into +10h and +11h, bottom-up rows | CONFIRMED-CORRECT, all three "native quirk" claims confirmed |
| 00BEF8F0 CR/LF collapse | src/file_store_cc13.cpp:67 | complete | **DEFECT D1** (fixed in this branch) + DOC-CLAIM-WRONG (header) |
| 00690FD0 TriggerHint | src/gui_screens_3.cpp:309 (read only) | 00690FF4..006912BA (setting gate, active-hint arm, storing rules, cooldown), 00691704..006917CB (previous timed hint) | CONFIRMED on sampled ranges (note N2) |
| 00560430 controls update | src/frontend_menus.cpp:446 | 00560430..00560785 (hint timer NaN path, XGetOverlappedResult args, hold test vs 0.0 at 00D7A218, page 0 arm, +16Ch..+17Ch) | CONFIRMED on sampled range |
| 0057A9C0 session browser join | src/menu_screens_2.cpp:866 | complete: wide name -> 00436630 low bytes -> "sessionname", the search-result block copy slot by slot, EDI = 2 still live at 0057ABE8, RET 8 | CONFIRMED-CORRECT |
| 0057AC10 session browser finish join | src/menu_screens_2.cpp:901 | complete: /0A0h record size, find-vs-end test, timer -1.0 (00D7A260) before the insert, tail stores, 2.5 (00CEECEC) | CONFIRMED-CORRECT |
| 00568930 filter-row layout (register/x87 pick) | src/menu_screens_2.cpp:436 | complete: CMOVNE EBP,EBX, {x, y, 0} position vector, FILD*float+double pad, FUCOMIP equality skip, constants 0.075f / 0.0111.. | CONFIRMED-CORRECT |

## Defects

### D1 00BEF8F0: the result keeps the input length (stale tail)

The compaction loop (00BEF957..00BEF986) writes through EAX into the same buffer it reads from,
and never stores EAX. The buffer object was built by 008D43C0 with its length at +0Ch set to the
whole stream length; 00BEF6D0 then wraps it with `end = base + [backing+0Ch]` (00BEF73D..00BEF743).
So the native stream is as long as the input: the compacted text followed by the input's last
`k` bytes, `k` = number of dropped CRs. `"a\r\nb"` gives `"a\nbb"`, not `"a\nb"`. The header
comment also said a CR "before anything else" is kept; a CR followed by another CR is dropped
(the second one only re-sets the pending flag).

Impact: none observed. No call or data reference to 00BEF8F0 exists (`ghidra xrefs` and
`scan-bytes 'f0 f8 be 00'` both empty, `bsp.py callers` 0), and nothing in `src/` calls
`collapse_crlf_00bef8f0`. Fixed in this branch (`src/file_store_cc13.cpp`,
`include/bsp/file_store_cc13.hpp`):

```diff
         pending_cr = false;
     }
+    out.append(bytes, out.size(), std::string::npos);
     return out;
```

## Notes (no behaviour change for reachable inputs)

N1 (00766FC0 / 00765000). For a non-leader group member whose kind is neither 18h nor 06h, the
writer still sends eight floats from +D8h/+ECh, which the builder never filled. In the image they
are whatever the message object held; in C++ they are the zero-initialised members. The values are
unspecified in the image, so this is not a defect, but a byte-exact comparison of such a message
would differ.

N2 (00690FD0). The active (+60h) and previous-timed (+1Ch) lookups call 005FFB20, which is
`std::map::operator[]` (lower_bound 005FD0C0, then insert of a default 005FD650 value), not the
find 0068E450 used for the new name. The C++ host treats a miss as `nullptr`. Every decision is the
same (a default definition is neither critical nor flagged and has no storing rule), but the image
inserts an empty definition under that name, after which a later TriggerHint of the same name
finds it instead of returning at 00691054. Both names come from definitions that were shown, so a
miss needs the table to be reloaded between triggers. Worth one sentence in the host contract.

Checked and dismissed: the `cooldown` rule test at 00691242 calls 00BF7FBF directly rather than
through 00425850, which looked case-sensitive; 00BF7FBF is `__stricmp`, the same comparison
00425850 uses, so `equals_insensitive` is right.

## Method notes

- Scripts and listings are in the worktree's `local/` (`au2_ladder.py` compares the ladder table
  against the listing; `au2_cstr.py` reads C strings from the PE on disk).
- Argument counts were taken from `RET n` of each callee where a list was transcribed (00765000
  RET 4, 00766FC0 RET 10h, 007746F0 RET 8, 0057A9C0 RET 8, 00435D30 RET 0Ch, 006B1E40 RET 18h).
- Register provenance at 0057ABE8 (EDI = 2) was checked by filtering the whole listing for EDI.
