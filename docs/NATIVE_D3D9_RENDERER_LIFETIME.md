# Native D3D9 renderer parent lifetime

Addresses: `00B32410`, `00B32920`. This packet restores and reviews the original parent bodies and unwind evidence. The original evidence packet did not provide full C++ constructor or destructor implementations; the current integration status is recorded below.

The constructor occupies `B32410..B328F7` (1,256 bytes). The destructor occupies `B32920..B339C0` (4,257 bytes). Fresh bytes from the expected `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, match both the immutable worker captures and installed PE. The combined parent report records 133 call instructions: 91 direct calls passed the live call-site gate; 42 indirect calls are explicitly outside that mechanical check.

The previous saved destructor ended at `B32D72`, after a returning free. The integrator cleared the call-site flow overrides under the Ghidra write lock, disassembled the verified tail and recreated the function through its actual final return. The mutation reports retain the original range and repair sequence. No callee-wide no-return property was changed. The complete linear destructor contains 1,255 instructions.

The [unwind evidence](NATIVE_D3D9_RENDERER_UNWIND_EVIDENCE.md) preserves all 58 FH3 states, capture offsets, activation instructions and cleanup chains. Successful singleton constructions return to parent state 24 without acquiring a later parent cleanup state. Adding parent-wide rollback would therefore change behavior.

The original constructor state 27 conflict was: after native B5BF70 unwind, the parent directly frees the captured 10h allocation. The current source operation retains ownership and temporary state after failure and terminates if destroyed while failed. Its inner cleanup must be reconstructed before it can compose with this parent free. Clearing the guard or releasing that allocation while the operation still owns it would not implement the original behavior.

Raw base lifetime, control-worker lifetime, Lua ownership and four record reserve helpers are integrated at `6641c05f`. Cache, effect-registry, record destruction and active-frame dependencies continue independently. These dependencies and existing parameter/query fragments do not establish complete parent source, original FH3/SEH compatibility, application adoption or gameplay parity.

Evidence: `reports/native_d3d9_renderer_lifetime.json`, `reports/native_d3d9_renderer_flow_repair.json`, `reports/native_d3d9_renderer_function_definitions.json`, and `reports/native_d3d9_renderer_unwind_evidence.json`. The evidence-only changes require no new tests or rebuild; the separately recorded component build remains pinned to its exact source commit.

## Constructor integration and current lifetime boundary

The complete B32410 constructor is now implemented in
`native_renderer_constructor`, preserving its 29-state cleanup schedule and
actual publication cells. The integrated raw B5BF70 registry constructor
provides substantive inner cleanup, so the former retained-operation conflict
at parent state 27 is resolved for this raw route. Its B5BF50 zero-resize
specialization requires nonnegative current count and capacity.

The constructor's owner writes and cleanup operands were checked against the
full original listing and installed PE. Strict Win32 compilation and a
link/import probe passed; the probe did not invoke the constructor. The
caller still supplies the documented live subobjects, scratch preimages and
borrowed manager/control contexts. Full destructor source is being recovered
separately, and several resource-owner contexts still require actual-AA0
manager access before the full lifetime graph can be exercised. EndFrame
also retains three outer providers after debug-line integration. Full parent
execution, original FH3/SEH identity, application adoption and gameplay remain
unvalidated. See `reports/native_renderer_constructor.json` for the exact
integration checkpoint and scoped validation.

## Destructor and actual manager integration

The full B32920 destructor, B339F0 scalar delete and B32900 secondary adjustor
are implemented in `native_renderer_destructor`, including the 29-state source
cleanup schedule. Query terminal cleanup is substantive, and physical,
surface, layout and shader lifetime contexts can now borrow actual-AA0
resource support. Canonical manager drain dispatches D62B64 support owners
through B61D60 using the popped owner and the current publication cell.

These changes resolve the earlier query/support source gaps. The final parent
probe covers linking and import resolution only; it does not invoke either
parent. Full execution still needs a valid initialized renderer/device graph
and the documented live subobjects, preimages and borrowed contexts. Static
caller evidence identifies +19E4 as a generated debug-sphere model and +19E8
as a generated sprite model; +19E0's writer remains unproved. Canonical model
bindings and the actual factory composition remain separate requirements.

The nested material-effect +C4 descriptor needs D61A44/B46930/B458A0 cleanup;
its sampler children use D621F4/B56FC0/B56EA0/B56DE0 and an actual state-list
pool. Their raw integration is being recovered independently. Existing
retained-operation/noexcept boundaries, physical-lock and shader-construction
projected contexts, three outer EndFrame providers, original FH3/SEH identity,
application adoption and gameplay remain open. See
`reports/native_renderer_destructor.json` for the pinned validation scope.

## Correction from descriptor, generated-model and raw shader integration

At `56780ba7`, raw descriptor/sampler terminals, the complete B4C700 generated-model
body, raw535320 material route and actual-AA0 shader constructors are integrated.
Existing callable descriptor/sampler producers still require explicit raw adoption.
Full B4C700/raw material execution and the cold B3B3C0 compiler continuation remain
open. Normal original/source shader fixtures use real HAL shaders; native FH3/SEH
execution and complete compiler ownership composition remain unproved.
Constructor/destructor probes still cover linking/imports only. Projected physical
locking, EndFrame providers and application/gameplay validation remain separate work.
