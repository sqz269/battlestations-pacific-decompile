# Native material-effect reload

Addresses: `00B469A0` (new complete 207-byte body, through `B46A6E`),
`00BDF310` (existing VFS callback composition), `00B61280` (existing compiler
metadata lifetime correction). Names are descriptive hypotheses.

`reload_native_material_effect_00b469a0` uses the actual 178h effect and its
canonical retained owners. Native ABI is ECX effect, no stack operands, RET.
The source interface adds contexts and a retained operation frame.

The gate is **108D6F1, the same byte used by source compilation and binary-cache
lookup**. With it clear, reload leaves every effect byte unchanged. With it set,
the routine reads the current B8h name for a verified bare-RET diagnostic, calls
B41B10 then B187A0, rereads the name, and calls B46950 on the same effect. A false
AL repeats both cleanup calls, constructs the actual D61C88 literal
`shaderfx/common/error.shfx`, calls B46950 again, ignores its result and returns
the pooled temporary. The native name/serial/reference and cache entry survive.

FH3 handler CBF658 points to DF8020: one unwind state, map DF8018, action CBF650
calling 41DD20 on the fallback string. The C++ guard arms only after string
construction and disarms before the normal pool return. This is source cleanup
behavior, not proof of original FH3 or binary ABI compatibility.

The existing loader installs a callable binding on the actual descriptor.
Cleanup honors that current table. A raw D61A44 descriptor instead uses the
existing numeric-profile terminal context. Merely supplying that context does
not overwrite the loader's callable binding or create another owner registry.

Two existing integration faults surfaced in the real run:

- BDF432 captures current manager+90. In the explicit native VFS route it now
  uses `NativeVfsStartupCallbacks`, already implementing the verified one-byte
  530620 RET installed by application startup. It no longer attempts to execute
  that original address inside the source executable. Unknown native targets
  remain explicit errors; the existing callable service route is preserved.
- B61280 branches directly from B61336 to its epilogue when D3DX returns no
  code. That bypasses the B61AEC error-buffer release. Completed C++ operation
  metadata now preserves this native leak without terminating its caller or
  adding a Release. Guards still reject running, failed and other live states.

The strict Win32 build and both existing CTests pass. A fresh composed fixture
uses the existing full B318B0/B31090 renderer cache producer, producing the
resolved shader path, two aliases, one record and cache/caller references on the
same effect. It checks three states:

- Disabled: all 178h bytes, canonical-owner count and VFS read count unchanged.
- Enabled: the actual shared source-mode byte drives real D3DX9_40 HLSL
  compilation. Four primary modes and one secondary return; canonical owners
  remain 28, reads increase 14 to 25, old descriptor binding retires, and effect,
  name, serial, references and registry identity remain unchanged.
- Invalid local HLSL: a valid Lua descriptor in the asset mirror is edited to
  reference an undeclared symbol. Real D3DX returns E_FAIL with retained error
  text. Native cleanup and fallback build the real error shader on the same
  effect: one primary, one secondary, 21 canonical owners including preserved
  compiler-orphan state. The pooled fallback header is released; the original
  local shader bytes and source-mode byte are restored.

Five source program compilations succeed and one deliberately fails. Native
debug shader writes stay inside a local mirror of 269 hash-matched installed
inputs. All mirrored inputs are checked against the unchanged installation
afterward. The earlier cold-code-byte comparisons still concern cached
admission; they do not establish byte equality of newly generated shaders.

The source path also required correcting old fixture assumptions: register
limit E13078 is **77**, verified in live Ghidra and the installed PE; 256 had
never been exercised by the cached-only fixture. A direct loader call with a
bare basename bypasses the real cache producer and is not valid reload state.
The loader transfers its completed program frame into canonical owner metadata,
so the emptied caller pointer cannot be used to inspect the old binding.

No permanent tests were added. Failed attempts, current sources, compiler/link
inputs, real runtime modules, original bytes, HLSL diagnostics and local debug
outputs are retained under `local/native_material_effect_reload`. The passing
run is `fixture_result_11.json`, compile log `compile_fixture_10.log`, link log
`link_fixture_12.log`. The unchanged earlier platform/device/Reset/focus/movie
and raw-root checks also pass; control worker exit, timer balance and focus
restoration remain checked.

Renderer registry scans, texture reload, shader texture rebinding, full window
activation and frame/draw/shutdown integration remain. Reload frames must stay
alive through their descriptors and failed native acquisitions. Native compiler
leaks are preserved. This is not whole-function original execution or gameplay
validation. See `reports/native_material_effect_reload.json` for bounded claims.

The final capture recompiles all 25 explicit units (24 selected) with the final
headers; the link also selects 311 current production providers. A semantic
ledger comparison confirms that only the three leased addresses changed.

Immutable closure: `local/checkpoints/84c6fe88/native-material-effect-reload/validation.json`, SHA-256 `1ea8a4152186e00f8fb8d6953d298db091f51ed898ea3363a95528bfc141fd76`; 6875 artifacts, 82 mapped Win32 modules, 311 production providers.
