# CC16 host-origin inverse-script encoding erratum

This NEW evidence erratum supplements frozen `8f9f9942bfc3bd8f23d385f6269d01375b72511b`. No Source, old evidence or old local snapshot is changed. The original report pinned raw local script bytes while embedding Python universal-newline text without a separate embedded-text hash or an explicit mixed-newline recipe.

The header inverse script, entry2, has these exact representations:

| Representation | Bytes | SHA-256 |
| --- | ---: | --- |
| Original raw local artifact | 2702 | `7a505ddd9683f4e2ca71e13937ea23982d7edd65e8d39a6509fa01a5232fe188` |
| Embedded UTF-8 LF text | 2701 | `c3d0cfd466eb0d39c48c870965091f6b4dbee8d0224daad2f5b16ae4c02d729b` |

The old raw file STILL matches its declared hash. It has no BOM: its body uses LF, while ONLY its final newline is CRLF. There are14 LF bytes and exactly1 CR. Python `read_text(encoding='utf-8-sig')` normalized that terminal CRLF before JSON embedding. The exact reversible relationship is `embedded = raw[:-2] + b'\n'`; all-LF/all-CRLF conversions do not reproduce the mixed raw artifact. The first two scripts' raw/embedded hashes already match. Both header representations and all old raw scripts were preserved separately in a NEW ignored snapshot.

All three EMBEDDED scripts were independently materialized from frozen8f9 into a new isolated text-only snapshot, with four current Source files and four baseline preimages recreated from pinned Git blobs. All three exited0 and all FOUR whole-file inverses matched exactly: ChangedCPP `721a3bff0f346ae3506deda59f5a7e892e7ca46eefb8fcaa4d57d04a9c1753a1`; MainCPP `9cea2fa635091c01ad61fabad16a406843709a1d267d830a2aedfc97ebd2b7a1`; ChangedHPP `13cc02308c28bf3ba443adfe1334627dffae05231a0c05e7bff6cbd634da5cc8`; MainHPP `2fffebb5ba867421eacc9bec461648004238f870d671337544ebdf3989bccc72`.

[The report](../reports/cc16_native_gui_text_host_inverse_erratum.json) pins both representations, all preserved preimages/results, the unchanged eight-file8f9 packet and an independently executable reproduction recipe. The recipe extracts embedded scripts and Source directly from Git into a NEW ignored directory; it requires no original local verifier file and never writes to the production checkout or old snapshots.

This was only Python text-inverse verification, not a C++/Native behavioral test or probe. No C++ Source execution, build, behavioral probe, game, Ghidra, CMake, ledger or process-control action occurred. No build, positive Native-origin, ABI, runtime, full Text96 or activation admission is granted by this erratum.
