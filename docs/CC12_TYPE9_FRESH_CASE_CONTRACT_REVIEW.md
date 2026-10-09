# Type9 future constructor cases (CC12)

This worksheet contains exactly four proposed Source-only cases, **Source credit0**. Current Type9 CPP/HPP and canonical allocation/free CPP/HPP are the only Source inputs. No old Root family/helper, Native/Ghidra/provider/compiler/runtime or compiled artifact is accessed. No Source edit, new test or fixture generator is introduced.

The actual current symbol is `bsp::construct_native_scene_property_record_type9_four_byte_array_storage_008ef360`. Its Source29–68 saves ESI/EDI, loads count, doubles it once, stores phase/tag9, doubles it again and compares only the low byte of the full flag word. The current header17–41 requires fresh/unowned writable56 root bytes, positive count[1,3FFFFFFF], exactly4*count readable bytes, disjoint/nonwrapping root/input/frame and DF0 for the successful current-provider domain.

| Proposed case | Count / bytes | Full flag / low byte | Root poison | Complete opaque input hex |
|---|---|---|---|---|
| retain_large | 3 /12 | 8D37B400 /00 | C7 | 5E00C837A1D46BF20985E74C |
| retain_small | 1 /4 | 52EA9C00 /00 | 3A | B37200E9 |
| copy_large | 3 /12 | C61F7280 /80 | 96 | 8AF12D00C659E47310BB956F |
| copy_small | 1 /4 | A943E501 /01 | 6D | D80046AF |

The four full flag words are distinct and have nonzero high24 bits. Both high-only/low00 cases retain despite the full word being nonzero; low80 and01 copy without Boolean normalization. Each input contains an interior NUL and later bytes. Copy expects the entire12/4-byte binary span, not a string prefix. Inputs remain unchanged. Root chooses future Source/Original/ordinary lanes; none is assigned or observed here.

The whole56 expectation comes from this Type9 Source's actual stores/header, with no Type10 assumption:

| Offset range (hex, exclusive end) | Expected contents |
|---|---|
| [00,04) | D4 89 CE 00, literal phase bits only |
| [04,08) | 09 00 00 00 |
| [08,18) | Original poison16 bytes |
| [18,20) | Zero8 bytes |
| [20,24) | Exact input address on retain; actual newly allocated child base on copy |
| [24,28) | 0C 00 00 00 or04 00 00 00 |
| [28,2C) | Original poison4 bytes |
| [2C,2D) | 01 in both branches |
| [2D,34) | Original poison7 bytes, including all four owner+30 bytes |
| [34,38) | Zero4 bytes |

This is29 written/27 preserved bytes. The compact JSON stores the common complete template once and four case rows. Dynamic pointer bytes require actual future pointer identity; no address is invented. Owner remains the poison DWORD, not an owner publication. Marker1 cannot discriminate borrowing/ownership or authorize destruction.

Physical Source interface: ECX=root, EDX unused register formal, target-entry stack+4=count,+8=input,+C=full flag. Both exits return full EAX=root and RET0C consumes three argument DWORDs. Source restores ESI/EDI. Retain leaves ECX=root/EDX=input and terminal CMP0,0 flags **8D5=44, including AF0**. Copy ECX/EDX provider residuals are unconstrained; EBX/EBP preservation depends on the genuine providers' normal ABI.

For copy, **T means actual target-entry ESP**. Final Source ADD ESP,10h takes `a=uint32(T-24)` to `s=uint32(a+16)=uint32(T-8)`, defining CF/PF/AF/ZF/SF/OF under8D5. Root must derive its actual future capture-to-entryT mapping; no pre-CALL/frame/capture value or fixed copy flag word is guessed. DF0 is the current-CRT precondition. No blanket ES/FPU/MXCSR/provider/wrapper preservation, naked-frame unwind or noexcept promise is added.

The Source bridge12–16 requests object/actual_bytes/actual_bytes. Canonical `singleton_lifetime.cpp:51–63` uses malloc(host_bytes), _callnewh retry or bad_alloc;65–67 calls free(pointer). Source copy stores the actual child+20 before memcpy and later returns the root. There is no constructor free on successful paths. The retained input stays caller-managed and is never child-freed. Each copied child is exclusively live/undisposed until its actual base is current-freed once, before the root's disposal. If Root selects four actual56-byte current-heap roots, this proposes four root plus two child allocations/frees, six pairs; children are12 and4 bytes. No retired read or global address-uniqueness rule is imposed. Manual child/root cleanup must not also invoke recursive release/destruction that frees the same child again.

Failure/zero/overflow/alias/reentry, private Original CRT/EH, literal-phase dispatch, class ownership, whole clone/publication/World/game remain separate. Numeric/vector/string payload meaning is not inferred. Existing independently qualified Source domains remain unchanged.

Root owns future lane selection, actual root/input/capture/guard layout, whole fresh Source/Native/helper/caller/provider gates and a sole fresh process. Root56 provides no adjacent heap guard/metadata proof. No observations or ABI blanket come from this worksheet.

Four actual Source/header files and four frozen input files have strict before/after hashes. `local/t9cases/receipt.json` plus `artifact_manifest.json` seal every actual recursive file, excluding only those two exact root filenames. Source/input/matrix copies, utilities, logs, stops and doc/report/commit snapshots remain included. Main metadata is generation history, not a forever-live pin.
