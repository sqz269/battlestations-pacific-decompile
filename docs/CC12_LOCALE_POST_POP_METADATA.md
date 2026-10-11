# CC12 locale post-POP metadata

Fresh metadata at `00CC8D45` identifies one unowned byte of `InstructionDB`, with exact/containing instruction and code-unit records agreeing. Override is `NONE`; default/effective flow is `TERMINATOR` (`is_terminal=true`; call/jump/conditional/computed false), both fallthroughs are null, and target arrays are empty. The opcode and instruction text remain unread. This is a metadata classification, not proof of an instruction identity, return, function no-return status or actual runtime termination.

The strict address lease is `CC8D45` only. Verified identity is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, `x86:LE:32:default`, image base `00400000`. Before/after typed raw responses and count bytes are identical at epoch 98 and count 64730. Capture order was typed-before, count-before, count-after, typed-after. Every Native CLI batch checked project/program identity; typed records retain the marker and epoch. All Native captures closed before packaging. These are finite equal samples, not an atomic transaction or execution trace.

The exact/containing function values are null, with empty function and direct-call-target records. Function no-return is therefore unavailable, not false. Unit start and inclusive end are `CC8D45`, length 1; exclusive end `CC8D46` is arithmetic only and was never queried. There were **zero code bytes, zero data bytes, zero instruction-text/context/decompiler reads, zero adjacent or target queries and zero saved-project mutations**.

The preceding accepted `POP ECX` contract remains inherited and conditional: if reached and successfully completed in decoded 32-bit mode with a normal 32-bit stack, entry ESP=U yields ECX=DWORD[SS:U], ESP=U+4 modulo 2^32 and next EIP=CC8D45, preserving other GPRs/EFLAGS relative to that POP entry. This packet observes none of those runtime values. Post-BF6FF5 ESP, GPRs, flags and stackword contents remain unknown. Deriving ECX=CD9410 requires both a returned-ESP hypothesis and an unchanged-stackword hypothesis; a plain-return or zero-argument-cleanup hypothesis alone is insufficient. Earlier EAX=E1768C credit ends at BF6FF5 entry. No actual callee return, cleanup, parameter/signature, registration, initializer completion, callback/EH or activation proof is composed.

Epoch 92 prefix, epoch 95 PUSH, previous epoch 98 CALL/metadata/POP operand and this fresh epoch 98 metadata capture remain distinct snapshots. The preceding CALL's encoded return-site address does not prove that its target returns or that either later unit executes.

Frozen Source HEAD is `b196a650e8c6e9e896cd630002099ac4ac2bf975`. The worker advanced only after comparing Source against its accepted operand commit. All 4217 closed `src/`, `include/`, `cmake/` and `CMakeLists.txt` paths equal the prior operand Source snapshot. The package contains 4278 selected actual Git blob occurrences, 61 named dependencies and 11 commit/tree objects, complete Source subtree traversal, eight complete enumerated searches and checked excerpts. Later Main Source changes require a separate qualified comparison. Historical Source3356 renderer manifest `9db7bcdc7f5e9db339286ad26c43f96f8f5340034fe7d5f757e7bbb08c7abdb8` supplies no fresh build/runtime credit.

Core archive `local/cc12_locale_post_pop_metadata/evidence.zip` is 211241548 bytes, SHA256 `dc4d6f1ebc173ead41e3bf63f1e7543de137681c952a926e6a240d4f9cfeb2a6`. Original and distinct physical-copy guarded replays passed. The prior operand core (187586630 bytes, SHA256 `6ccb66dad98a6e08dcfa1ea8ef10aa96523c0c46c7fc501741fce35885c2839e`) and its actual-output delivery (291538 bytes, SHA256 `d27f047ade00f2e35058d2b3c045e82764f63652b2040c0ca1e00e7d8808df78`) passed physical copied replay and recur under the new guard. The immutable prior document/report blobs from commit `1bf7cebc4b81185ca2ac2e9cadc176b3dcfd4a8d` equal the corresponding current dependency blobs.

Keep all 17 helpers below beside `evidence.zip`, then run `python guarded_replay.py evidence.zip --output result.json`. Physical helper bytes must match archived Tools before importing the graph. Guarded replay admits only the supplied ZIP read and result write, denies external read/process/network/native-load operations, passes three negative self-probes and records no unexpected denials.

| Required core helper | Bytes | SHA256 |
| --- | ---: | --- |
| `replay.py` | 12097 | `022a113ea89a32f9e80a10aca6cb9df296c08c1d59d6f198797f2056db466665` |
| `guarded_replay.py` | 3788 | `47552f5fae73772dea010573759d19ececd393f15f55f3a0ef7358db4fb2baf7` |
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

Post-commit actual-output delivery uses `guarded_delivery_replay.py`, `delivery_replay.py` and `prior_replay.py`; its pin is external to these immutable outputs. The report records all helper pins, exact Native records, named dependencies, Source deltas, search counts and nested proof pins.

No C++, CMake, caller implementation, native test/probe/build, ABI/game/OS/audio execution, function creation or saved Ghidra mutation belongs to this packet. The smallest next proposal is **exact one-byte/context-0 text admission at CC8D45**, separately authorized and gated by fresh identity plus unchanged exact owner/unit/flow metadata. Nothing here permits adjacent, target, data or body expansion.
