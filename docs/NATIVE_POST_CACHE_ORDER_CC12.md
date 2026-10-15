# POST effect acquisition and the Original shader-cache bracket

This is a read-only contract audit for `fakemotionblur.mshd`, the real name used by the render-service `+664` producer. It does not admit that effect, the POST draw, or a complete scene to runtime validation. The captured current application sources are exactly Main `37eb2b90a97ecf0bf5851c15bbd4bf9d7d71d10e`; worker prefix commit `6d84c7d68f3752f2ed34d3c58c62acf0ba0d1f3a` and its sealed 62 artifacts plus six committed snapshots remain unchanged.

The decisive result is that Original closes the binary shader cache **before** constructing the post resources. The effect registry and the preload caller credits survive that close. A later `+664` constructor must reuse the preloaded effect through the same renderer registry.

## Original order and ownership

| Stage | Original evidence | Consequence |
| --- | --- | --- |
| Common initialization | `0073D410`: shadow constructor `00A8FE30` at `0073DD04`; preload `0073BF80` at `0073DD12` | Shadow service exists before preload. |
| Cache and Lua | `0073BFA6 -> 00B3A600`; then Lua construction `0073BFAF -> 00B66BD0`, open-mask 1 `0073BFC3 -> 00B6A020` | One cache bracket surrounds descriptor acquisition. |
| Actual preload script | `0073C00D -> 00B69D40`, `shaderfx/shaderpreload.lua`; `FileNames`, first/next at `0073C0AD/0073C196` | Preserve native Lua iteration; textual file order is not runtime iteration proof. |
| Acquire and retain | Current renderer `+48` at `0073C12F`, canonical `00B318B0`; append result at `0073C15B`, increment application vector count at `0073C161` | Application `+8/+C/+10` keeps each returned caller credit. |
| Normal close | `0073C1BE -> 00B3B140`, followed by Lua value/key/table cleanup and owner close at `0073C204` | The binary cache ends before preload returns. |
| Later common initialization | Resource-service constructor `0073DF01 -> 00B14A10`; GameOnInit `0073E477 -> 004E5540`; settings `0073E485 -> 008D5B50` | The render-resource initialization is later than the cache bracket. |
| Settings to resource initialization | `008D60CC..008D60E6` passes settings `+58/+8D/+8C`, loads current `00F8D39C`, calls `00B107F0` | `00B107F0` distinguishes an existing service using byte `+1C4`; this audit concerns its fresh initialization path. |
| Real `+664` producer | `00B1297A`: allocate 20h, name `00D5E240` = `fakemotionblur.mshd`, count 3, null optional argument; `00B129C9 -> 00B4E470`, publish `+664` at `00B129D9` | Keep the genuine descriptor and construction inputs. |
| Material acquisition | `00B4E5D8 -> 00535320`; factory calls current renderer `+48` at `00535345`, creates the material, then retires its temporary effect credit at `00535377` | The material retains its own effect credit; the factory does not reopen the binary cache. |

`00B3A600` allocates/constructs the 10h binary-cache owner and publishes `0108D6EC` after construction. `00B3B140` captures that current owner, destroys it at `00B3B14D`, frees it at `00B3B153`, and only then clears `0108D6EC` at `00B3B15B`. It does not release material-effect references.

The normalized alias hit in `00B31090` branches at `00B311CA` to `00B3142A`, invokes current registry `+0C` at `00B31436`, and jumps to return cleanup. It bypasses name resolution and cold registry `+08` dispatch at `00B3130D`. The actual registry `00D5F074` has `+08 = 00B2EBB0` and `+0C = 00B31FF0`; the latter increments the effect's actual `+04` via import `00CE221C` and returns the same pointer. A closed binary cache is therefore compatible with a **verified warm effect-registry hit**.

Original shutdown `00737F72..00737FB5` walks the application's retained vector in ascending order, decrements each nonnull actual effect `+04` through `00CE2220`, dispatches virtual slot 0 on zero, and clears its captured slot. The binary-cache close is not the lifetime boundary for these credits. The current bounded GUI probe's explicit caller-reference retirement is a fixture lifecycle; it is not evidence that Original released preload credits immediately.

## Forward cursor and installed asset evidence

With compiler `source_mode` byte `0108D6F1 == 0`, `00B34890` starts at the binary owner's current cursor, compares the current row, and advances that cursor on misses (`00B348EE`) **and hits** (`00B34901`). It neither rewinds nor wraps. A nonzero source-mode gate returns null before reading the owner. This packet does not change either mode byte.

The installed physical `shaderfx/shaders.bin` is 2,250,700 bytes, 1,628 records, SHA-256 `dcc65686e26e0b7b4f4179b45b153aaa353696b0b1b08c32251fbc7c97c0ecf8`. A fresh complete parse reaches exact EOF:

| Family | Record indices | Names |
| --- | --- | --- |
| Genuine GUI font | 700–703 | `guifont0F3.vso/pso`, `guifont0T3.vso/pso` |
| Genuine bilinear font | 704–707 | `guifontbilinear0F3.vso/pso`, `guifontbilinear0T3.vso/pso` |
| Genuine fakemotion | 1076–1079 | `fakemotionblur0F3.vso/pso`, `fakemotionblur0T3.vso/pso` |

The installed preload script includes these names at textual lines 80, 81 and 143. Its hash is `083b751decc24e70b370c9d291baf3b3112f66ad763d5839e6bc47435786bcee`. The actual `postprocess/fakemotionblur.shfx` includes `dx9_lua.inc`, uses the dummy combiner, and declares the `SceneTexture` sampler and its camera/motion parameters. These are physical-file observations, not proof of selected VFS records, overrides, native Lua traversal, query order, or successful compiler consumption.

The physical rows permit a bounded GUI-font, bilinear-font, then fakemotion acquisition sequence using the existing cursor. That sequence must still be observed at runtime: every genuine child lookup consumes the same cursor, so a returned descriptor can introduce a failure before the expected rows are reached. Preserve the first failure instead of restarting, replacing a descriptor, resetting the cursor, or changing compiler mode. The descriptor source-1 sampler work under compiler mode 0 is a separate child frontier; compiler source mode 1 is not an alternate admission path for this packet.

## Failure and graph contract

Original preload handler `00C868AD` references FuncInfo `00DB5E88`, eight states at `00DB5EAC`. Actions are `00C86860 -> 00B669A0` (Lua owner), `00C8686B/00C868A2 -> 0041DD20` (name), and `00C86876/81/8C/97 -> 00B67700` (Lua objects). **No unwind action closes the binary cache.** This is decoded static EH evidence, not a native FH3 execution test.

The existing `NativeShaderPreloadOperation` keeps its cache operation, Lua objects, name and stable list of per-material frames. A Source exception leaves it failed and retains those native obligations. `GameNativeGuiEffectApplication::acquire_effect_00b318b0` already provides the required guarded entry for a genuine named effect:

- Validate the same renderer, VFS, strings, effect registry, compiler/pass owners, sampler domains, scalar cells and Lua service binding before native entry.
- Publish a stable `unique_ptr` operation before effects; record the stable actual8h input name and current renderer.
- Activate the renderer's existing `NativeLuaServiceBindings` across the entire nested descriptor/DoFile operation; call the private cache through the genuine `00B318B0` Source provider.
- Record the returned actual owner/credit or retain the failed frame and parents. A failed/active graph cannot replay, retarget, drain or reopen. A successful return transfers the caller credit; the operation's metadata remains stable until the renderer's actual owner drain completes.

The renderer `ShaderGraph::create()` is already one-shot. Its `release()` requires the same completed creation and same current cache publication. Calling the full `NativeShaderPreloadOperation` directly while that wrapper also owns creation would create two authorities: production integration must reconcile ownership of **one** `NativeShaderBinaryCacheOperation`, not call both constructors or create a second effect graph.

## Smallest next implementation boundary

For a bounded acquisition proof, the existing guarded application method is sufficient; no private cache/program accessor is needed:

1. In the current probe's existing one-shot cache bracket and same existing `GameNativeGuiEffectApplication`, add a stable actual8h `fakemotionblur.mshd` name and returned-credit slot after the two genuine font acquisitions. Call the existing `acquire_effect_00b318b0` method. Preserve actual descriptor/sampler/compiler child behavior and failure frames.
2. Require a complete genuine result and record operation identity, actual registry owner, cache publication, source/load-variant bytes, and consumed cursor/query evidence. Keep its caller credit alive while the ordinary existing `release_shader_cache()` closes the cache. Do not extend the cache into post-resource construction.
3. With `0108D6EC` null, call the same application method with the same stable name. Require the same actual effect pointer, a completed warm-hit operation at `00B31436`, no cold-loader entry, and an additional actual caller credit. Retire both credits through existing actual-owner lifetime operations only at the bounded probe's explicit normal teardown, before renderer/parent drain. On any exception retain the entire process-owned graph; do not run a generic cleanup/retry path.

This is a bounded projection of Original preload membership, cache bracket and warm effect reuse, not a replacement for the full native Lua preload order. The exact production operation is `preload_native_application_shaders_0073bf80` with the application's actual retained vector and native Lua traversal, inside the same application-owned Lua activation and unified cache-operation authority. Preserve the resulting caller credits through the proper application shutdown stage.

Only after that proof should a separate guarded application operation compose the actual `00B4E470`/`+664` producer. That operation must own and publish its construction frame before effects, internally borrow this **same** private material-effect cache, activate the same Lua binding, validate the supplied renderer/targets/model/camera/layout/parameter domains, and retain its frame plus parents on failure. It must not expose a bare cache reference. This packet does not implement that operation or claim the remaining active `00B4DCC0` material draw graph is bound.

## Receipt and validation boundary

`reports/native_post_cache_order_cc12.json` pins the ignored receipt `local/world_post_cache_contract_20261015/`: 60 copied input files, 25 source copies verified against captured Main, 26 Original spans totaling 2,186 bytes, each matched through `bsp.py ghidra bytes` to the installed PE, plus exact decoded call/EH/credit/cursor checks. `audit.py` passes 261 checks and revalidates sealed prefix artifacts without modifying them. The original capture is preserved; six additional warm-hit/profile spans live in a separate supplement.

No shared C++, CMake, ledger or Ghidra annotation changed. No build was needed for this documentation-only packet. There was no Original preload execution, fake effect acquisition, POST construction/draw, native exception execution, ABI replacement or game validation. Existing registered providers remain distinct from proof that their production runtime composition is admitted.
