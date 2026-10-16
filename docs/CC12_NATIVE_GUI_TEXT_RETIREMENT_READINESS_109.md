# CC12 raw Text retirement readiness 109

The next missing raw body is `AB80C0`, the glyph-child clear engine. Its complete valid-storage algorithm is ready to implement using the existing actual `AA83A0` detach and required genuine current child-delete dispatch. The current implementation takes `GuiTextContentBinding`, `GuiWidgetOwner` and a semantic `std::vector`; it cannot receive93's actual raw Text by a cast. No duplicate node, string, vector, material, font or base destructor is needed.

Whole raw `AB8250` is therefore not closed by current Source. Its remaining direct raw-body gap is this 247-byte helper. Nonempty Text-child deletion also recurses through current virtual4 → `AB8EE0` → `AB8250`; implementing the helper alone does not close that recursive terminal or admit production. Fresh93's empty glyph vector has a closed path through the real helper, but an empty no-op must not replace its whole body. The eventual raw scalar/profile binding must borrow the SAME actual Text+04 and existing creator/storage lifetime, without a semantic GUI companion or another counter.

This packet changes only this document and its report. Clean baseline is Root `a17278d8358e1234a2dc10aecec9bf5a72e092f7`. The installed Original remains SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Live wrapper reads verified the existing `bsp.gpr` / `/battlestationspacific.exe`; there were no Source, CMake, ledger or Ghidra writes, builds, tests, probes or raw execution.85/93/101 hashes are unchanged.

## Complete outer schedule

`AB8250[AB8250,AB83D0)` is 384 bytes / 113 instructions. Original ABI is ECX actual Text, no stack arguments, RET, preserving EBX/ESI/EDI; EAX is unspecified. It installs an FH3 registration with handler `CB7EEC`, saves the receiver for funclets, and starts from state-1. The decompiler's signature is not a recovered C++ ABI. The whole live byte range equals the installed Original and all 113 saved listing starts agree. The saved historical false-no-return warning on `_free` does not truncate this audit: code continues after `AB8345` through `AB83CF`.

| Original sites | Current capture, action and post-return mutation |
| --- | --- |
| `AB826F..AB827F` | Stamp Text+0=`D5C6C8`; capture+1EC into EDI; install state6 after its comparison. |
| `AB828D..AB829F` | If captured shader is nonnull, load CURRENT `CE2220`, decrement that actual+04; only on observed zero load its CURRENT vtable and slot0 and call it. Clear Text+1EC only after return. Initially null skips that store. |
| `AB82A5..AB82B4` | Load CURRENT+188 after the shader callback. If nonnull, call genuine `B6DFA0`; clear+188 after return. Initially null skips that store. |
| `AB82BC` | Clear glyph children with SAME raw Text. Neither +188 nor the glyph vector is an entry snapshot. |
| `AB82C1/82C9/82D0` | Capture data+1CC first, state5 next, then only when data is nonnull capture CURRENT length+1C8; return length+1 bytes. |
| `AB82E9/82F1/82F8` | Data+1C4 → state4 → conditional CURRENT length+1C0; return length+1. |
| `AB8311/8319/8320` | Data+1A8 → state3 → conditional CURRENT length+1A4; return wrapping `2*length+2`. |
| `AB833A..AB8359` | Capture begin+198 once; free it iff nonnull; always clear+198,+19C,+1A0 in that order. State remains3: there is no normal state2 store. Preserve allocator+194. |
| `AB835F/8367/836E` | Data+F8 → state1 → conditional CURRENT length+F4; return length+1. |
| `AB8387/838F/8395` | Data+F0 → state0 → conditional CURRENT length+EC; return wrapping `2*length+2`. |
| `AB83B1/83B9` | Set state-1 before genuine whole raw `AA9730` on SAME receiver. Restore FS registration and RET. |

Each of the five normal string returns calls a fresh `419CC0` getter, then `BD1510(data,captured_size,1)`, borrowing the SAME actual string-process publications and return-mask cell. Headers are not cleared. Calling the ordinary string destructor after changing the outer state would reread data too late; future Source must explicitly pass the already captured data into a genuine pool-return sequence, then read length after the state store. Exception funclets use the ordinary raw string destructors directly.

No explicit Text+04 decrement occurs in `AB8250`; callbacks can still mutate it. It does not directly release borrowed+180/+184/+1AC/+1B0, a font companion, or cursor184. The cursor's parent/Model hierarchy is retired through the actual main node/base path. Shader and shadow post-return clears overwrite their slots even if the callback replaced them; later glyph callbacks can also install another188, which the stamped base profile need not recognize. Retain independent creator/publication records rather than inferring those replacements were consumed. This is not permission to discard the independently retained creator records.

## Seven-state unwind

Live handler `CB7EEC` loads FuncInfo `DEECAC` and tail-jumps `BF6B43`. FuncInfo magic is `19930522`, maxState7, unwindMap `DEECD0`, no try-map/IP-map, EH flags1. The seven entries have next states -1,0,1,2,3,4,5. Each funclet reads SAME receiver from `[EBP-10]` and tail-jumps the actual cleanup body:

| Active state | Funclet | Actual cleanup | Next |
| --- | --- | --- | --- |
| 6 | `CB7EDE` | narrow Text+1C8 → `41DD20` | 5 |
| 5 | `CB7ED0` | narrow Text+1C0 → `41DD20` | 4 |
| 4 | `CB7EC2` | wide Text+1A4 → `436430` | 3 |
| 3 | `CB7EB4` | pointer vector Text+194 → `5D6CF0` | 2 |
| 2 | `CB7EA6` | narrow Text+F4 → `41DD20` | 1 |
| 1 | `CB7E98` | wide Text+EC → `436430` | 0 |
| 0 | `CB7E90` | SAME raw base → `AA9730` | -1 |

State6 is active throughout shader, shadow and glyph clear calls. Failure there cleans the five strings/vector/base, with **no extra shader/shadow rollback** and no attempt to finish the interrupted glyph loop. Data→state→length consumption prevents retrying the normal string being returned. State3 remains armed during inline vector free/clears; if that Native call could throw, the native vector unwind action is still armed. Ordinary Source CRT free is nonthrowing; do not invent a normal state2 transition to simplify this distinction. A Source projection should consume each unwind action before invoking it and terminate on a second cleanup exception, while recording which actions actually returned.

`provider_call` in85 is a boundary marker, not proof that a nested exception originated in Native code. Source metadata/registry/argument failures may follow consumed native mutations; outer failure policy must state this limit rather than infer Native cleanup solely from that enum. Source C++ exceptions, Native FH3/private frames, SEH and arbitrary modified import targets remain distinct contracts.

## Raw glyph clear frontier

`AB80C0[AB80C0,AB81B7)` contains 247 bytes. Live Ghidra has 79 instruction starts; the saved listing has those 79 plus a gap marker. Linear disk decoding additionally sees the unreachable 6-byte `LEA EBX,[EBX]` at `AB80CA`, skipped by the entry jump. That padding is byte-covered, not an extra executed path.

The loop uses unsigned index EBP and CURRENT begin+198 / arithmetic-shifted byte difference to end+19C. It repeats the native bounds check before detach, then reloads begin and bounds after genuine `AA83A0(parent=Text,child=current[index])`. Null children are passed to detach too. After detach it captures the CURRENT slot address and child. If nonnull it captures CURRENT child vtable+4, calls flags1, and clears that **captured** slot only after return. A null child skips the clear. It then increments index and rechecks current size. A delete callback can replace headers/elements for later iterations but cannot invalidate the captured slot before its post-return write. No protective retain, erase-before-call, fallback progression or replacement ownership transfer is present.

At loop exit it captures end, checks both ordered ranges, compares current begin with captured end, and performs the inlined erase tail. In valid serialized storage no callback intervenes, so its memmove length is zero and only end=begin is needed when nonempty. It preserves allocator194, begin198, capacity1A0 and backing; it does not free the vector. Invalid ranges reach `BF6713`; a nonzero `_memmove_s` arm is outside this packet's valid serialized-vector domain. A Source helper must state its CRT/error boundary and cannot turn an invalid post-detach index into successful truncation.

Existing `detach_native_gui_widget_child_00aa83a0` already handles actual parent/child storage, current Model re-resolution through SAME canonical Node registry, real `B6E680(null)`, `B6D890`, `B6D940`, complete raw `A9BD50` list removal, then child+70 clear. It needs no `GuiWidgetOwner`/`unique_ptr` transport. Existing `NativeGuiWidgetLifetimeBindings` provides current-table lookup and genuine `call_child_delete_04`; those remain required external providers, with no successful default. A Text target `AB8EE0` cannot currently be supplied by the typed Text implementation for raw93 storage.

The smallest next Source packet should implement only this whole raw clear engine in a distinct raw API, preserving the old typed function. Inputs are actual Text, SAME existing `NativeNodeDestructionRuntime`, required current-table/child-delete bindings and fresh persistent diagnostics/prepared recursion frames. Preflight belongs before entry; all frames, contexts, vector backing, parent/list storage and actual creators must be held and disjoint from writable diagnostics. Each reached recursive call gets distinct retained state. An empty fresh93 vector requires no child dispatch; nonempty actual Text children require the later genuine recursive raw `AB8250`/`AB8EE0` closure. There is no Source change authorization in109.

## Existing providers and lifetime boundaries

- Raw `B6DFA0` tree retirement already exists, with parent/root unlink, current profile/current18 dispatch and explicit persistent diagnostics. The canonical Model path also exists. Use the SAME model creator/registry/node services from85/93, not a new Model or typed widget cast.
- Cached shader/effect retirement already has actual-count canonical providers. `NativeMaterialEffectReference` borrows SAME effect+04, checks its current established profile, and calls whole base/derived effect scalar retirement. Generic observed release records decrement-before-lookup and a failed already-zero obligation. For exact AB8250 import timing, load CURRENT `CE2220` at the native site; do not silently substitute a helper that bypasses that cell. Current-profile projection is required on zero; no arbitrary vtable/default terminal is implied by the existing normal profiles.
- Raw `41DD20` / `436430` and actual pool getter/return exist. `5D6CF0` from93 is the genuine16-byte-header tidy: preserve+0, conditionally free captured+4, always zero+4/+8/+C. Its body can be reused while the outer state remains3.
- Whole raw `AA9730` exists. It stamps `D5C130` before its `AA8320` class query, deletes current GUI children04(1), detaches from the parent's SAME list, retires replacement main4C/timed entries, destroys backing, then stamps `D5C104`/`CEB130`. Child/timed current scalar bindings remain required; no count barrier or automatic Text recognition is added. Fresh default's empty child/timed containers use the existing closed domain.

`AB8EE0[AB8EE0,AB8F00)` is 32 bytes / 11 instructions: call `AB8250`, **then** read the caller's flags byte at `AB8EE8`; if bit0, return the SAME raw slot via actual `F8BDF0` / `AB75A0` at `AB8EF5`; return SAME pointer bits and RET4. No decrement/count test occurs. A failing destructor does not reach flag read or pool return. Flags0 still destroys. The genuine Text pool return already exists and requires the SAME pool/slab/trailer+1F4/allocation domain; no creator/companion should be unbound or storage returned early. A future Source frame should preserve this late flag-word read, not snapshot a mutable caller frame at entry.

The material count-held1→2→1 domain from106 remains valid only while caller-held storage/creator actually survives. Count/creator retention is **not** a scalar-delete, page/ancestor or recursive child-retirement barrier. Normal raw Text admission still requires complete destructor/scalar/current-profile binding and all reached actual child/timed providers.

Before any callback or unwind, hold the whole independent85/93 creator/acquisition graph, context services, profiles, raw string process, Model/mesh/section/material records, canonical registries, raw slot/pool and every recursive frame. On partial failure record captured shader/count/zero-call progress, shadow tree progress, detached/captured child slot and returned scalar/pool actions, consumed string/vector/base cleanup and any ended lifetimes. Preserve completed creator/diagnostic metadata even if widget fields have become dangling or base cleanup ended its lifetime. Retained raw aliases are not extra owners; a terminal may already have destroyed its payload and retired its canonical record. Never rediscover it from dead Text fields, retry a consumed release, roll back Shadow, automatically drop a retained creator, or return the Text slot during destructor failure.

The report seals exact ranges, direct-call verification, EH data, Source/provider hashes and local evidence. Byte/listing/call proof is static readiness evidence, not a build, Native ABI, whole raw Text execution or game validation result.
