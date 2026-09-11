# Text glyph-child boundary and UTF16 code-unit constructor

Addresses: `00AB98F0`, `00AB81C0`. Names describe reconstructed behavior; they are hypotheses. This packet makes no Ghidra changes and does not enable the Text factory.

| Routine | Coverage | Native ABI and body |
| --- | --- | --- |
| `construct_gui_text_code_unit_00ab81c0` | Complete ordinary valid-storage behavior using the existing actual string pool; new C++ interface | ECX actual eight-byte UTF16 header, one DWORD code-unit argument, EAX same header, `RET 4`; `00AB81C0..00AB8242`, final instruction `00AB8240`, length 3 |
| `GuiTextGlyphChildCallFrame` for `00AB98F0` | Analyzed continuation contract only. No instruction of optional child range `00AB9D33..00AB9FAD` is implemented by this record | ECX Text, ten DWORD arguments, `RET 28h`; full body `00AB98F0..00AB9FC2`, final instruction `00AB9FC0`, length 3 |

The existing mapped writer stops before `AB9D33`, which loads the Text pool receiver for the first missing call at `AB9D38`. The new record borrows the caller's canonical `GuiTextLifetime`, stable float3 and mapped streams. It neither reruns the quad writes nor advances the pending builder. It is an evidence contract, not a native stack image or a resumable implementation of the child tail. Pending single-line/wrapped builders must keep their mappings and enclosing content continuation alive.

## Actual UTF16 storage operation

The sole caller is `AB9E51` in `AB98F0`. Its local is an actual `{DWORD code_unit_length, DWORD buffer}` header, not the byte-counted `NativeString` wrapper. `AB81C0` zeroes both fields before getting the sized-storage singleton and allocating four bytes. The implementation uses the existing concrete `ActualNativeStringPoolStorage`, whose operations resolve the same live publication/lifetime domain each time.

After allocation, the native routine reloads the current header. Unsigned length at least one copies its first UTF16 unit; unsigned length below one implies zero and calls `memcpy` for zero bytes. The C++ leaf omits that zero-byte call. A current nonnull old buffer is returned with DWORD-wrapped `length * 2 + 2`, then the original allocation is published, length becomes one, the terminator is stored, and the supplied low 16 bits are written through the reloaded published pointer. This preserves the native stores and live reads around actual pool operations. The constructor expects a fresh/unowned header: it does not release a preexisting buffer before the initial zero stores.

No alternate allocator, wrapper ownership or nullable-allocation fallback is introduced. The caller must later return a nonnull buffer with its current UTF16 length times two plus two through the same pool, as `AB9E8A/AB9E91` do. Native SEH, exceptional pool-construction behavior and the original callable x86 ABI are not claimed.

## Ten-argument frame

Let entry ESP be `P`; after the SEH frame, `SUB ESP,3Ch` and four saved registers, steady ESP is `S = P - 58h`. The return address is at `S+58h`. Temporary pushes must be included when interpreting subsequent ESP-relative operands.

| Slot | Original location | Caller value | Full-body use / value at `AB9D33` |
| --- | --- | --- | --- |
| 1 | `S+5C` | Borrowed font glyph | Prefix reads metrics; slot is reused, finally first vertex plus one |
| 2 | `S+60` | Borrowed stable float3 pointer | Initial x/y at `AB991E..AB992B`; original pointer survives and x is reread at `AB9F5E/AB9F62` |
| 3 | `S+64` | Actual logical vertex stream | Prefix retains stream in EAX; slot becomes first vertex plus two |
| 4 | `S+68` | Actual index slice | Used by prefix; original pointer remains |
| 5 | `S+6C` | Same float3 address in both known callers | **Never read by the full callee** |
| 6 | `S+70` | Same float3 address in both known callers | **Never read by the full callee** |
| 7 | `S+74` | Quad index DWORD | Prefix creates low-16 index values from quad index times four |
| 8 | `S+78` | Height DWORD; low 16 bits used | Reused for intermediate values and packed white, then allocation/child pointer in the tail |
| 9 | `S+7C` | First vertex DWORD | Original slot remains; do not confuse it with pushed-ESP accesses to slot 8 |
| 10 | `S+80` | Code-unit DWORD | Low 16 bits used for substitution membership and `AB81C0` |

Both native call sites, `ABA1F8` in `AB9FD0` and `ABA730` in `ABA270`, push the same float3 address as slots 2/5/6. Full-function assembly corrects the earlier suggestion that slots 5/6 might be additional outputs: they are unused. Actual argument 2 is a read-only input to this callee; its stable allocation remains necessary for late x reads and caller continuation lifetime.

Full-listing register-write filtering establishes the boundary state: EDI is the original parent Text (restored from `S+10` after packed-color arms); ESI is first vertex from `AB9961`; EBP is first vertex plus three from `AB9BA7`; EBX is quad index times four plus two after `AB9C45`; EAX is the actual vertex stream. ECX/EDX/XMM0 are dead prefix temporaries. Tail instructions replace ESI with the child, EBP with minus one and EBX with one. Live inputs needed after the boundary are parent EDI, original argument 2 and original argument 10. Prefix geometry locals are not retained outputs: subsequent string locals and the reference-position getter overwrite them.

## Native child order, currently unexecuted

All sites below belong to `00AB98F0`; the JSON report carries numeric callee addresses and containing functions for mechanical checking. The four indirect targets are established by the current Text/Model profiles, not by the verifier alone.

| Call site | Callee / cleanup | Native operation and arguments |
| --- | --- | --- |
| `AB9D38` | `AB78A0`, `RET` | Raw slot from Text pool `F8BDF0`; **first missing call** |
| `AB9D4F` | `AB9650`, `RET` | Construct Text in nonnull allocation; native null arm later dereferences zero |
| `AB9D70` | `005DAD40`, `RET 4` | Append address of local child pointer to parent's existing `+194` vector, before clone or attachment |
| `AB9D87` | Reference node current `+10`, Model `B752B0`, `RET 8` | Clone current `[parent+1B0]+4C` with `(0x26, 0)` |
| `AB9D8C` | `AA6720`, `RET 4` | Bind clone to child |
| `AB9D93` | `AB8530`, `RET` | Ensure actual child draw sections after binding |
| `AB9DA5` | Child current `+34`, Text `AA8530`, `RET 4` | Visible `(1)` |
| `AB9DB2` | `AA6BC0`, `RET 8` | Child listener `([parent+1AC], 0)`; stores borrowed `+DC` and byte `+79`; next direct store sets child `+78=1` |
| `AB9DC9` | `AB8C30`, `RET 4` | Font name from current reference Text `+1C8` |
| `AB9E05` | `AB6C30`, `RET 10h` | Shadow `(1, 0, float[D5C5C0], &RGBA{0,0,0,float[D7A24C]})` |
| `AB9E13` | `0041E870`, `RET 4` | Construct temporary shader name from `CEFD78`, `GuiFontBilinear.mshd` |
| `AB9E1F` | `AB8E70`, `RET 4` | Set child shader name; releases/clears cached shader even when name text is unchanged |
| `AB9E39`, `AB9E40` | `00419CC0`, `RET`; `BD1510`, `RET 0Ch` | Get actual string pool, return captured name buffer `(buffer, length+1, 1)` |
| `AB9E51` | `AB81C0`, `RET 4` | Actual UTF16 temporary containing original argument 10 low word; new leaf is available independently |
| `AB9E61` | `ABA8D0`, `RET 4` | Fully update child's content from temporary wrapper; this can recursively need a glyph child |
| `AB9E71` | Child current `+50`, Text `AB6B50`, `RET 4` | Color from live child `+50`, only after content completes |
| `AB9E8A`, `AB9E91` | `00419CC0`, `RET`; `BD1510`, `RET 0Ch` | Return captured UTF16 temporary `(buffer, current_length*2+2, 1)` |
| `AB9E9B` | `AAA5A0`, `RET 8` | Attach existing child to parent `(child, 0)`; no constructed `+74` call |
| `AB9EC0` | Child current `+30`, Text `A9E0B0`, `RET 4` | Pivot `(0.5, 0.5)` from `CE3800` |
| `AB9ECD` | `AA6750`, `RET 4` | Get current reference Text resolved position into three-float local |
| `AB9FA9` | `AA8240`, `RET 4` | Set computed child resolved position; final child call ends at `AB9FAD` |

`00419CC0` takes no arguments. The three pushes before it belong to the subsequent `BD1510`, whose `RET 0Ch` removes them. The one-code-unit constructor's zero-byte `memcpy` at `AB81F2` uses three cdecl arguments and `ADD ESP,0Ch` at `AB81F7`.

## Producer and remaining prerequisites

`00531130` produces this mode: ECX is Text, arguments are substitution UTF16 wrapper, borrowed listener and borrowed reference Text; all ordinary returns use `RET 0Ch`. It copies the existing `+1A4` string and stores `+1AC/+1B0` without retain/release. All nine calls were checked: four in `00531380`, one in `0054C050`, three in `005D26D0`, and one in `0061FBE0`. Each passes the target Text again as reference; listener receivers differ. In `00531380`, direct stores also set byte `+1B4=1` and placement floats `+1B8/+1BC`. Constructor `AB9650` and copy constructor `ABB2C0` clear the borrowed pointers; the copy constructor copies byte `+1B4` without copying those two placement floats. The old frontend host name for the third argument does not establish an event-object type.

The final tail reloads the reference and branches on current byte `+1B4` and reference string length. Offset mode adds current `+1B8/+1BC` to reference x/y. Otherwise reference length one uses reference x directly, while other lengths first compute `(live_arg2.x - 26) / 960 + reference.x` with a float spill. Non-offset x then subtracts double bits `3f847ae140000000`; y subtracts `3f8c432ca0000000`; every branch subtracts double 100 from reference z. These are resolved-position/pivot operations, not scale operations. No arithmetic-only substitute is exposed ahead of the missing child operations.

Existing canonical lifetime, draw buffers, styles, content stages, mapped builders and actual string pool are reusable. Peer resource-name functions now bind the same real font registry/resource association and shader owner. The current Text factory is still unavailable. `GuiWidgetOwnerRuntime::create_with_scene_00aa6640` creates a fresh model and invokes current `+74`, so it cannot replace the native raw-constructor, pointer-append, reference-model-clone sequence above. Model clone `B752B0` itself uses actual pool/model construction, `B6F150` base/node copy, source geometry current `+10`, actual geometry binding and reference effects; no such complete owner composition is supplied by this packet. Full recursive child content, canonical nonnull reference identity, live `+1B8/+1BC` storage and ownership transport into `AAA5A0` also remain prerequisites. No fallback factory, completion callback, second Text state/tree or automatic pending-frame cleanup is added.

## Verification

The new source compiled as MSVC Win32 C++17 with `/W4 /WX /O2 /fp:strict`. The stronger live report verifier passed all 39 numeric call-site rows with zero failures; four indirect targets additionally rely on the documented vtable/body evidence. No new tests were added. The child path is not enabled in the executable, so there is no reachable runtime, native differential or game/render validation claim. The parent owns combined build registration and integration.
