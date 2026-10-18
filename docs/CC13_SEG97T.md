# CC13 seg97T: segment 97 tag-only triage

Packet cc13-seg97T. Segment 97 runs from 00BE4460 to 00C30570. The input was the 660 rows in
local\cc13_undone_seg97.txt: functions with no tag and no reconstruction record. The cc13 lead's
local\cc13lead_seglist.py generated the list on 2026-10-18 from main 9d085f779.

This packet only adds tags. It reconstructs nothing.

## Result

| class | count | action |
| --- | --- | --- |
| MSVC 2005 runtime inside [00BF55BE, 00C2DDD0) | 659 | `crt_unmatched`, action `reviewed`, evidence `cc13seg97t_crt`; bind-only |
| game code | 1 | listed below for a later packet; not tagged |
| already tagged on main | 0 | `config/tags/*.jsonl` checked before adding |

**Why these rows were untagged.** The library inventory tag e0615 covers the same span: the CRT,
the C++ EH runtime, libcpmt and undname, from 00BF55BE to 00C2DDD0. It only claimed the
*unmatched* bodies as `LIBCRT_unmatched_*`. The 659 rows here are the bodies that Ghidra's FID or
an earlier pass already named with their runtime symbols. They were therefore never tagged.

By name, 471 rows start with `_` or `__` and 33 are `FID_conflict:` rows. 89 lie in the undname
block 00C21615..00C26936, and one of those also starts with `_`. The other groups overlap these, so
the table below lists examples only:

| group | examples |
| --- | --- |
| CRT entry points and helpers | leading `_` / `__` names |
| printf / wprintf internals | `write_char`, `write_multi_char`, `write_string` |
| scanf / strtox / xtoa | `vscan_fn`, `strtoxl`, `xtoa_s` |
| startup | `entry`, `parse_cmdline`, `fast_error_exit`, `check_managed_app` |
| multibyte code pages | `setSBCS`, `getSystemCP` |
| C++ EH runtime | `FindHandler`, `CallCatchBlock`, `terminate`, `unexpected`, `type_info` helpers |
| libcpmt locale | `facet_Register`, `~_Locimp`, `tidy_global` |
| undname demangler | `DName`, `UnDecorator`, `get*` |

Every row keeps its existing name, and none is renamed. Under the 2026-10-16/17 library ruling
these bodies are bound, not reconstructed.

## Game-code rows (for a later packet)

| address | name | note |
| --- | --- | --- |
| 00BF3ACC | `BSP_PhysicalDirectory_GetFileDate_EnabledContinuation` | Not a separate function. It is the enabled continuation of 00BF3A80, reached by the JZ at 00BF3AA5 after the shared SEH prologue. 00BF3A80 is already reconstructed twice: `src/vfs_file_date.cpp` and `src/native_physical_file_date.cpp`, whose record gives `native_end_exclusive` 00BF3C0F and so covers this address. It is now recorded as a fragment of 00BF3A80 (cc13-small2, 2026-10-18), and needs no new code. **Polarity:** 00BF3A98..00BF3AA5 load byte [[0109CEEC]+78h] and `JZ 00BF3ACC`. When the byte is non-zero, 00BF3AA7..00BF3AC9 zero the five output dwords and return. 00BF3A80's ledger evidence names that byte the date-*disabled* flag ("zero if set"). So the continuation is the date-query arm taken when date reporting is enabled, and the name `EnabledContinuation` is correct and kept. |

No other row in the list is game code.
