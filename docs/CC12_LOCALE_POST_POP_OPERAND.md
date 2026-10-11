# CC12 locale post-POP operand

`00CC8D45` is the one-byte encoding `C3`, decoded as `RET` in the accepted x86 32-bit mode. The exact typed gate remains unchanged: all instruction/code-unit exact and containing records identify one unowned `InstructionDB` byte, `NONE` override, default/effective `TERMINATOR`, null fallthrough and empty targets. Function-at/containing values and function/target lists remain empty. Function no-return metadata is unavailable, not false.

Fresh identity was `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, `x86:LE:32:default`, image base `00400000`. Before/after typed raw responses match at epoch 98/count 64730; count, code and exact text have three byte-identical pairs. Both text captures used context 0, returned only `00cc8d45`, and had empty neighbor arrays. One unique code byte and zero data bytes were admitted. All Native captures closed before packaging; no preceding refresh, adjacent/target/body read or saved-project mutation occurred. The recorded order is typed-before, count-before, code-before, text-before, code-after, text-after, count-after, typed-after. These finite equal samples are not an executed trace or atomic transaction.

For a successful ordinary decoded 32-bit near return with a normal 32-bit stack, let entry ESP=V and Q be the DWORD read from SS:V. This `RET` sets EIP=Q and ESP=(V+4) modulo 2^32. EAX/ECX/EDX/EBX/EBP/ESI/EDI and EFLAGS are preserved relative to this RET entry. The ordinary instruction reads one stack DWORD and performs no explicit memory store. V and Q remain unknown; the analysis captured no stack data. C3 has no immediate operand, which establishes no caller argument policy, calling convention, cleanup contract or function signature. No actual return, target/caller identity, fault behavior or runtime completion is observed.

The preceding POP contract remains separately inherited. Post-BF6FF5 ESP, GPRs, flags and stackword contents are unknown. Deriving ECX=CD9410 from that POP needs both an explicit returned-ESP hypothesis and an unchanged-stackword hypothesis; a plain return or zero-argument-cleanup hypothesis alone is insufficient. Earlier EAX=E1768C credit ends at BF6FF5 entry. RET preserving unknown incoming registers does not restore those earlier values. No registration, initializer completion, callback/EH/FH3, canonical ownership or activation proof is composed. The epoch 92 prefix, epoch 95 PUSH, earlier epoch 98 packets and this fresh operand capture remain distinct snapshots.

The exclusive unit end `CC8D46` is arithmetic only and was never queried. This RET has no static fallthrough; its dynamic destination comes from an unobserved stack value. No successor address or new function extent is admitted.

Frozen Source HEAD is `639ba6fa83343dcfa263676b8a0f3d9aa903c5aa`. The worker advanced only after comparing the previous accepted metadata branch. All 4217 `src/`, `include/`, `cmake/` and `CMakeLists.txt` paths are closed with actual Git bytes and subtree traversal. 4215 paths equal the prior metadata Source snapshot; the following two match independently accepted mesh commit `aaa1e25d4819621bba02f9c869758b8fc1de8c1b`. Current closure contains 4286 selected blob occurrences, 69 named dependencies and 11 commit/tree objects, with nine complete enumerated searches. No prior proof is rebased and no mesh build/runtime credit transfers.

| Source path | Prior Git Source SHA256 | Current Git Source SHA256 |
| --- | --- | --- |
| `include/bsp/native_mesh_owner.hpp` | `540fa95aafcd3b67697998361f9a6b1689bf90348c9c0cdc09bdeb6b9c6ac42c` | `f0f4b0d87721717033e823aee5494a280481c6b4a02a7fd2bea51970b46d5636` |
| `src/native_mesh_owner.cpp` | `fb263bbe9a631354e3e6943293befdf18fcd7da26a7969df4827e4ea97fb6fd3` | `8725dee8119cf400bf58464d6dd2dcb6771502b6edd8f527f65ec3f3c7fe40fe` |

The separate raw physical Root Source3356 mesh manifest is 1264559 bytes, SHA256 `2f2e85784563abd5bef2e5dc8fa002116aa20e61bd8575b25f0888b4bd411a75`. Within its 3351-path overlap with this closed Source census, 3346 raw pins match exactly; five actual retained copies differ solely by CRLF line endings. Their raw pins are verified, raw bytes are archived, and CRLF-pairs-to-LF conversion produces the exact frozen Git bytes and blob identities during replay. They are **EOL representations, not sound Source changes**. The other five named-input pins and four build-artifact pins outside this Source scope remain opaque metadata; their files were not opened. Historical saved-renderer manifest `9db7bcdc7f5e9db339286ad26c43f96f8f5340034fe7d5f757e7bbb08c7abdb8` remains inherited only. Neither manifest transfers build/runtime credit. Later Main changes require a separate qualified comparison.

Core archive `local/cc12_locale_post_pop_operand/evidence.zip` is 235172237 bytes, SHA256 `776103644e42a5d6f16a96386e3a0359629b7c191c5ab2aa8be6552eada4cf82`. Original and distinct physical-copy guarded replays passed. Prior metadata core (211241548 bytes, SHA256 `dc4d6f1ebc173ead41e3bf63f1e7543de137681c952a926e6a240d4f9cfeb2a6`) and immutable-output delivery (291885 bytes, SHA256 `e62b4f70c4071edfe0af4661485a57cb1663d9a178a99cfbc10a27c79a535cd4`) passed copied replay and recur under the new guard. Prior document/report blobs from `44b9cce88fe9575b780815abbe6afba026894182` equal the corresponding current dependency blobs.

Keep all 19 physical helpers below beside `evidence.zip`, then run `python guarded_replay.py evidence.zip --output result.json`. Helpers must match archived Tools before import. Replay permits only the supplied ZIP read and result write, denies external reads/process/network/native loading, passes three negative self-probes and records no unexpected denials.

| Required core helper | Bytes | SHA256 |
| --- | ---: | --- |
| `replay.py` | 15964 | `cc5a8f5f70a642e6eb1ae7c90450ad123cca63f5f5643979505c319452dfac9b` |
| `guarded_replay.py` | 3859 | `82c4e1fcd16bfc2a7e2979448b0904f40174e5fdd87d91bde133986ce42d6ee4` |
| `prior_replay.py` | 5817 | `5787d5059905efafa5cbf041ddd604aa7c455abe1799b9a530d081669bef3f25` |
| `prefix_replay.py` | 10761 | `a1b73d8c05fb3d6ddbe0be426db625ef7572da6bc27891df7cc34b55178e2d40` |
| `prefix_delivery_replay.py` | 2465 | `a126e45579611ca05c5e4b02c42567c01ad0e8f4eaf645b59b0744747eb847f6` |
| `metadata_replay.py` | 9853 | `b31b71745833305917e8367b8b18575b539b71dd18d6346de0b065c1e02fb16b` |
| `metadata_delivery_replay.py` | 2467 | `25147f2eff8a3ba2c819b76d950eca1ce6d078c2b44314124ac32524b7cbad77` |
| `operand_replay.py` | 10585 | `1c941b6bc0eae0c2ff25ab87af765e66fbbff0dc581b6f175b1bf038a61f7f63` |
| `operand_delivery_replay.py` | 2480 | `11e83e2c8ec79cfeadf4ee19ec68e8c022ef1a81681466364ed36fe1eff43503` |
| `post_metadata_replay.py` | 12435 | `20ecd3c6cf8aeb12d774ba59ca76cda39a0c07a4cdabdada589e9fedb40a4434` |
| `post_metadata_delivery_replay.py` | 2462 | `647cb76904c1a7d4cccd49a0de2ba2d9b0212ce9863d6fe205c98dec63feff04` |
| `call_replay.py` | 12906 | `018f614495c0a6f551cd8d481047a67d66ec4ec6cac22e93b70b43c5c98e53bf` |
| `call_delivery_replay.py` | 2466 | `2252ea9d80d691df785d1211472f4fa44fc8da105aca39802fca8fbe31b4ebe0` |
| `return_metadata_replay.py` | 10363 | `02e9da53e519c568fdd960418674950a81bd294e23477d85901eac004bbac35d` |
| `return_metadata_delivery_replay.py` | 2453 | `977f708c138accaf6692198296378067f1478642401b092f3f2b73bbe72cddbd` |
| `return_operand_replay.py` | 13268 | `c47081e1a52c18656155650973251acfa2018b0817d5389135ae95a002da687a` |
| `return_operand_delivery_replay.py` | 2563 | `8024b7026c805e2ddd5d1b439b5309fdbba69e23797bd6778aef45bd7a3f2fed` |
| `post_pop_metadata_replay.py` | 12097 | `022a113ea89a32f9e80a10aca6cb9df296c08c1d59d6f198797f2056db466665` |
| `post_pop_metadata_delivery_replay.py` | 2408 | `19afc228d031f2df2ee44dea72d2c0029638950238853c123600b95614978e87` |

Post-commit actual-output delivery uses `guarded_delivery_replay.py`, `delivery_replay.py` and `prior_replay.py`; its pin is external to these immutable outputs. The report records full helper pins, exact Native records, Source deltas, raw/LF distinctions, Root manifest pins and nested proof limits.

No C++, CMake, caller or ledger change, native test/probe/build, ABI/game/OS/audio execution or Ghidra write belongs to this packet. Any further target, ownership or caller investigation requires a separately selected bounded Root packet. No automatic continuation at CC8D46 is justified.
