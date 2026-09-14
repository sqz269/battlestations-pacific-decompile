# Native D3D9 renderer parent lifetime

Addresses: `00B32410`, `00B32920`. This packet restores and reviews the original parent bodies and unwind evidence. It does not provide full C++ constructor or destructor implementations.

The constructor occupies `B32410..B328F7` (1,256 bytes). The destructor occupies `B32920..B339C0` (4,257 bytes). Fresh bytes from the expected `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, match both the immutable worker captures and installed PE. The combined parent report records 133 call instructions: 91 direct calls passed the live call-site gate; 42 indirect calls are explicitly outside that mechanical check.

The previous saved destructor ended at `B32D72`, after a returning free. The integrator cleared the call-site flow overrides under the Ghidra write lock, disassembled the verified tail and recreated the function through its actual final return. The mutation reports retain the original range and repair sequence. No callee-wide no-return property was changed. The complete linear destructor contains 1,255 instructions.

The [unwind evidence](NATIVE_D3D9_RENDERER_UNWIND_EVIDENCE.md) preserves all 58 FH3 states, capture offsets, activation instructions and cleanup chains. Successful singleton constructions return to parent state 24 without acquiring a later parent cleanup state. Adding parent-wide rollback would therefore change behavior.

The substantive remaining conflict is constructor state 27: after native B5BF70 unwind, the parent directly frees the captured 10h allocation. The current source operation retains ownership and temporary state after failure and terminates if destroyed while failed. Its inner cleanup must be reconstructed before it can compose with this parent free. Clearing the guard or releasing that allocation while the operation still owns it would not implement the original behavior.

Raw base lifetime, control-worker lifetime, Lua ownership and four record reserve helpers are integrated at `6641c05f`. Cache, effect-registry, record destruction and active-frame dependencies continue independently. These dependencies and existing parameter/query fragments do not establish complete parent source, original FH3/SEH compatibility, application adoption or gameplay parity.

Evidence: `reports/native_d3d9_renderer_lifetime.json`, `reports/native_d3d9_renderer_flow_repair.json`, `reports/native_d3d9_renderer_function_definitions.json`, and `reports/native_d3d9_renderer_unwind_evidence.json`. The evidence-only changes require no new tests or rebuild; the separately recorded component build remains pinned to its exact source commit.
