# zlib 1.2.1 and other borrowed C code in battlestationspacific.exe

Read-only inventory (Ghidra bsp project, exports/bsp/functions.json). Goal: identify code that should be imported and matched rather than reconstructed.

## Summary

- **zlib 1.2.1 is statically linked, stock, and contains both deflate and inflate.** Code block `[00bc94d0, 00bcf910)` = 25664 bytes; 50 Ghidra-defined functions, all mapped to zlib source functions below, plus at least two pockets of undefined code holding unreferenced zlib functions (`00bcacf0` inflateSetDictionary family, `00bcbf40` deflate_fast).
- Objects present: adler32, compress, crc32 (BYFOUR, 8x256 table at `00d657c0`), deflate, inffast, inflate, inftrees, trees, uncompr, zutil. Absent: gzio, infback.
- The game touches zlib through 8 wrapper functions only (compress, uncompress, inflateInit_/deflateInit_, inflateReset, inflate, inflateEnd). Default allocator is stock zcalloc/zcfree over CRT malloc; no game allocator hook.
- **MT19937** (already reconstructed as src/random.cpp) is the only other well-known algorithm found. Its seeding is non-stock (seed|1, 69069 LCG), so keep the reconstruction.
- Negative sweep: no MD5/SHA/AES/FNV/TEA/Murmur/Perlin/inv-sqrt/CRC-16 constants; no libpng/jpeg/expat/tinyxml/squish/stb/lzo/lzma/bzip2/base64 signatures. XML and UTF-8 handling are in-house (iometrics/iostdnet).

## zlib address map

| Address | zlib source | Conf. | Ghidra fn | Evidence |
|---|---|---|---|---|
| `00bc94d0` | uncompr.c:uncompress | high | yes | passes ZLIB_VERSION '1.2.1' (00d643f8); calls inflateInit_, inflate, inflateEnd; game callers FUN_00b6c0e0, FUN_00bd3ec0 |
| `00bc9570` | compress.c:compress2 | high | yes | passes '1.2.1'; calls deflateInit_ 00bccd40, deflate 00bcb270, deflateEnd 00bcb690 |
| `00bc9610` | compress.c:compress | high | yes | calls compress2; game callers FUN_00b6d1b0, FUN_00bd48f0 |
| `00bc9650` | inflate.c:inflateReset | high | yes | called by inflateInit2_ and game FUN_00bbbe10 |
| `00bc96a0` | inflate.c:inflateInit2_ | high | yes | calls inflateReset |
| `00bc9770` | inflate.c:inflateInit_ | high | yes | called by uncompress |
| `00bc97b0` | inflate.c:updatewindow | high | yes | called by inflate; only callee memcpy |
| `00bc9890` | inflate.c:inflate | high | yes | references every inflate.c error string; body 00bc9890-00bcac2a, switch jump table 00bcac30-00bcacaf; game caller FUN_00bbbf00 |
| `00bcacb0` | inflate.c:inflateEnd | high | yes | body 00bcacb0-00bcaced; called by uncompress and game FUN_00bbc320 |
| `00bcacf0` | inflate.c:inflateSetDictionary (+ likely inflateSync/inflateSyncPoint) | medium | **no** | UNDEFINED in Ghidra: ~0x230 bytes of real code (push ebx/esi; cmp [state],0xa = DICT; adler32(0,NULL,0) prologue). Unreferenced, so never auto-defined |
| `00bcaf20` | inflate.c:inflateCopy | medium | yes | 0x2d0 bytes, only direct callee memcpy; allocs go through the zalloc pointer |
| `00bcb1f0` | deflate.c:putShortMSB | high | yes | 0x30 bytes; callers deflate and deflateParams |
| `00bcb220` | deflate.c:flush_pending | high | yes | 0x50 bytes; callers deflate, deflate_stored/fast/slow, deflateParams |
| `00bcb270` | deflate.c:deflate | high | yes | calls putShortMSB, flush_pending, _tr_align, _tr_stored_block, crc32, adler32, memset (CLEAR_HASH); dispatches through configuration_table; emits gzip header 1f 8b |
| `00bcb690` | deflate.c:deflateEnd | high | yes | called from deflateInit2_ failure path and compress2 |
| `00bcb940` | deflate.c:lm_init | high | yes | memset = CLEAR_HASH; called by deflateReset |
| `00bcb9c0` | deflate.c:longest_match | high | yes | decompiled body: scan_end/scan_end1 checks, chain_length>>2 on good_match, nice_match clamp, 8-byte unrolled compare to strend (0x102) |
| `00bcbb40` | deflate.c:longest_match_fast | high | yes | decompiled body: single-candidate match used for Z_RLE (1.2.1 compiles it unconditionally) |
| `00bcbc10` | deflate.c:fill_window | medium | yes | 0x150 bytes; called by deflate_stored/fast/slow. Direct memcpy xrefs from the deflate_* bodies are unexplained (read_buf/zmemcpy expected inside fill_window) - verify during matching |
| `00bcbd60` | deflate.c:deflate_stored | high | yes | configuration_table[0].func = 00bcbd60 (table at 00d65568, 12-byte entries, func at +8) |
| `00bcbf40` | deflate.c:deflate_fast | high | **no** | configuration_table[1..3].func = 00bcbf40. NOT a defined function in Ghidra/functions.json (0x360 bytes of code after deflate_stored) - needs create_function |
| `00bcc2a0` | deflate.c:deflate_slow | high | yes | configuration_table[4..9].func = 00bcc2a0; calls longest_match, longest_match_fast, fill_window, _tr_flush_block, flush_pending |
| `00bcc720` | deflate.c:deflateReset | high | yes | calls crc32(0), adler32(0), lm_init, _tr_init |
| `00bcc7b0` | deflate.c:deflateParams | high | yes | decompiled: level/strategy validation, configuration_table compare, deflate(strm,Z_PARTIAL_FLUSH) inlined, reloads good/max_lazy/nice/max_chain from 00d65568+level*12. No callers (dead but linked) |
| `00bccb20` | deflate.c:deflateInit2_ | high | yes | decompiled: version[0]=='1', stream_size==0x38, installs zcalloc/zcfree, windowBits/memLevel checks, calls deflateReset, deflateEnd on alloc failure |
| `00bccd40` | deflate.c:deflateInit_ | high | yes | calls deflateInit2_; called by compress2 and game FUN_00bbc1d0 (via '1.2.1' xref) |
| `00bccda0` | zutil.c:zcalloc | high | yes | default zalloc installed by deflateInit2_; calls malloc |
| `00bccdc0` | zutil.c:zcfree | high | yes | default zfree installed by deflateInit2_ |
| `00bccdd0` | inffast.c:inflate_fast | high | yes | references 'invalid distance too far back' / 'invalid distance code' at 00bcd1ee/00bcd201; called by inflate |
| `00bcd290` | inftrees.c:inflate_table | high | yes | references lbase 00d656c0 (00bcd446), lext 00d65700 (00bcd454), dbase 00d65740 (00bcd430, 00bcd50f) |
| `00bcd720` | adler32.c:adler32 | high | yes | called by inflate, deflate, deflateReset, deflateParams |
| `00bcd860` | crc32.c:crc32_little | high | yes | references crc_table[0..3] at 00d657c0/00d65bc0/00d65fc0/00d663c0 (BYFOUR build) |
| `00bcdb00` | crc32.c:crc32_big | high | yes | references crc_table[4..7] at 00d667c0 |
| `00bcddf0` | crc32.c:crc32 | high | yes | calls crc32_little; called by inflate, deflate, deflateReset, deflateParams |
| `00bcde20` | trees.c:init_block | high | yes | called by _tr_init and _tr_flush_block |
| `00bcde90` | trees.c:pqdownheap | medium | yes | called only by build_tree; source order right after init_block |
| `00bcdf60` | trees.c:gen_bitlen | medium | yes | 0x220 bytes; called only by build_tree |
| `00bce180` | trees.c:scan_tree | high | yes | called by build_bl_tree |
| `00bce270` | trees.c:send_tree | high | yes | called by send_all_trees; large because send_bits is inlined |
| `00bce780` | trees.c:send_all_trees | high | yes | called from _tr_flush_block with (l_max+1, d_max+1, max_blindex+1) |
| `00bcea80` | trees.c:compress_block | high | yes | references extra_dbits 00d67848 at 00bced35; called from _tr_flush_block with (ltree, dtree) |
| `00bcee90` | trees.c:set_data_type | high | yes | called when data_type == Z_UNKNOWN (2) in _tr_flush_block |
| `00bcef40` | trees.c:bi_flush | high | yes | called twice from _tr_align |
| `00bcefc0` | trees.c:bi_windup | high | yes | called at end of _tr_flush_block when eof |
| `00bcf040` | trees.c:copy_block | high | yes | called from _tr_stored_block with header=1 |
| `00bcf0d0` | trees.c:_tr_init | high | yes | called by deflateReset; calls init_block |
| `00bcf140` | trees.c:gen_codes | medium | yes | 0x80 bytes; called only by build_tree |
| `00bcf1c0` | trees.c:build_tree | high | yes | called twice from _tr_flush_block with &s->l_desc (+0xb10) and &s->d_desc (+0xb1c); calls pqdownheap, gen_bitlen, gen_codes |
| `00bcf3c0` | trees.c:build_bl_tree | high | yes | returns max_blindex; calls scan_tree and build_tree |
| `00bcf490` | trees.c:_tr_stored_block | high | yes | decompiled: send_bits((STORED_BLOCK<<1)+eof, 3) then copy_block(..., 1) |
| `00bcf530` | trees.c:_tr_align | high | yes | decompiled: static-tree code 2 (3 bits) + EOB (7 bits), bi_flush, second align if 1+last_eob_len+10-bi_valid < 9, last_eob_len = 7 |
| `00bcf720` | trees.c:_tr_flush_block | high | yes | decompiled: set_data_type, build_tree x2, build_bl_tree, opt_len/static_len compare, static_ltree 00d67910 / static_dtree 00d67d90, send_all_trees, compress_block, init_block, bi_windup |

## Data tables

| Address | Item |
|---|---|
| `00d643f8` | ZLIB_VERSION "1.2.1" (xrefs: compress2 00bc9592, uncompress 00bc94ee, game FUN_00bbc1d0 @00bbc2ba) |
| `00d65530` | deflate_copyright (unreferenced, as in stock) |
| `00d65568` | configuration_table[10] (good, lazy, nice, chain as ushort + func ptr): level 0 -> 00bcbd60, 1-3 -> 00bcbf40, 4-9 -> 00bcc2a0 |
| `00d655e8-00d6565f` | z_errmsg strings (incompatible version ... need dictionary) |
| `00d65680` | z_errmsg[] pointer table (read by deflateInit2_ at 00bccd05) |
| `00d65690` | inflate_copyright (unreferenced) |
| `00d656c0 / 00d65700 / 00d65740` | inftrees lbase / lext / dbase (dext follows) |
| `00d657c0-00d677c0` | crc_table[8][256] (BYFOUR) |
| `00d677e0 / 00d67848` | trees.c extra_lbits / extra_dbits |
| `00d67910 / 00d67d90` | static_ltree / static_dtree (used by _tr_flush_block) |

## Game-side entry points (not library code)

- `FUN_00bbbe10` -> inflateReset
- `FUN_00bbbf00` -> inflate
- `FUN_00bbc1d0` -> inflateInit_/deflateInit_ (references '1.2.1')
- `FUN_00bbc320` -> inflateEnd
- `FUN_00b6c0e0` -> uncompress
- `FUN_00bd3ec0` -> uncompress
- `FUN_00b6d1b0` -> compress
- `FUN_00bd48f0` -> compress

`FUN_00bbbe10`..`FUN_00bbc320` look like one streaming-inflate reader class; `FUN_00b6c0e0`/`FUN_00bd3ec0` and `FUN_00b6d1b0`/`FUN_00bd48f0` are whole-buffer unpack/pack helpers.

## Stock-ness and build flags

- deflateInit2_ checks `version[0]=='1'` and `stream_size==0x38` (sizeof z_stream on 32-bit) and installs zcalloc/zcfree: stock.
- deflate emits the gzip header (`1f 8b 08 ...`) when wrap==2: NO_GZIP not defined. gzio.c simply was not pulled from the static lib.
- crc32 dispatches to crc32_little/crc32_big with 8 tables: BYFOUR (default for STDC builds).
- longest_match_fast exists alongside longest_match: matches 1.2.1 (Z_RLE branch in both deflate_fast and deflate_slow).
- deflateParams is linked with deflate() inlined into it and has no callers; MSVC function-level linking kept it. Other unreferenced API (deflateSetDictionary, deflatePrime, deflateBound, deflateCopy, inflateSync, inflateSyncPoint, zError, zlibVersion, get_crc_table) is either dropped or sits in undefined code pockets.
- No modified semantics found. Direct `memcpy` xrefs from the deflate_* bodies are the one oddity (expected inside fill_window/read_buf); verify while matching - likely inlining or xref attribution from the undefined deflate_fast pocket.

## Recommendation

1. Vendor stock zlib-1.2.1 (checksum-pinned, like Lua) under third_party; add it to the build instead of reconstructing anything in `[00bc94d0, 00bcf910)`.
2. Create Ghidra functions at `00bcbf40` (deflate_fast) and `00bcacf0` (inflateSetDictionary family), run a code-gap scan on the block, then apply the names above as *library matches* in the ledger / ghidra_names with the evidence column. Exclude these 52+ functions from the reconstruction denominator.
3. Parity needed by the game is the public API used by the 8 wrappers; a compress/uncompress round-trip fixture against the native functions is sufficient validation.
4. Byte-identical confirmation would need VC8 (VS2005) `/O2`; with the installed VS2022/2026 toolchains, use a local zlib build plus Ghidra fuzzy hashing (bulk_fuzzy_match / find_similar_functions_fuzzy) to confirm the symbol assignment, especially the medium-confidence trees.c entries (pqdownheap/gen_bitlen/gen_codes) and fill_window.
5. MT19937: nothing to import; the reconstruction already exists and the seeding is non-stock.

## Other borrowed code: findings

- **MT19937**: `00bf0cf0` seed (69069 LCG, seed|1), `00bf0d20` refill (227/397 split, 0x9908b0df at 00bf0d30), `00ba2c20` next + temper, `00bd2e00` ctor (seed 0x1105). Already in src/random.cpp.
- Nothing else well-known was found by constant or string sweep (list in the JSON `negative_sweep`). detect_crypto_constants is unimplemented on this MCP server; the sweep used search_byte_patterns.
- Out of scope but noted: Lua 5.1.1 (separate agent), Dinkumware STL copyright `00e15410` (CRT agent), DLL-imported middleware (FMOD, Bink, D3DX, XLive, DirectInput, XInput).
