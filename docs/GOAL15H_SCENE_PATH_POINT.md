# Complete raw scene-path point transformation

The full `007AF800..007AF8D2` entry now consumes actual scene-path, pose, slot and point storage through the complete pose, vector and SDK policy providers. Its 211-byte Source body preserves the original instruction schedule. Original ECX is the raw path; output pointer and uint32 index are stacked, EAX returns the actual output, and RET8 consumes the two words. Source EDX is an ignored binding placeholder and is not an Original input.

The entry captures path+14 pose once, conditionally refreshes its actual byte-C8 owner, and retains the captured +CC matrix across validation. It checks +8/+C pointer slots with raw SAR2/unsigned arithmetic. A returning real SDK policy can repair slots; the entry reloads current +8 while preserving its earlier index and matrix. MOVSS snapshots XYZ, with W supplied by verified read-only Native D7A24C bits0000803F in current Source constant storage.

The genuine vector kernel writes a separate local float4 result. The subsequent full x87 spill, reciprocal and output sequence remains literal, including reuse of the ACTUAL incoming index word for the rounded reciprocal and late output-pointer read. There is no zero-w, NaN, normalization, default-pose or index repair. Every reached raw and control cell requires backing, dirty pose ancestry must terminate, and the canonical providers retain their free x87-slot requirements. Production changes no floating-point control state.

| Physical CALL site | Callee | Binding |
| --- | --- | --- |
| 007AF816 | 00414DB0 | Captured raw pose, conditional byte-C8 refresh |
| 007AF838 | 00BF6713 | Actual current SDK validation policy |
| 007AF87B | 00B62D10 | Genuine full vector kernel through Source physical-ABI adapter |

The accepted Source vector kernel takes destination in EDX instead of the Original stack word. A 13-byte naked adapter loads EDX, shifts the actual return slot by one discarded word, and tails into that complete kernel. Its single tail relocation is verified. The resulting end-ESP, return and receiver contract matches this caller; Source frame and stack addresses remain qualified.

Strict optimized MSVC Win32 /MD /W4 /WX compilation passed. All 195 fixed bytes agree between COFF, target-verified live bytes and original disk bytes. Three CALL relocations and the single absolute constant operand are independently verified by kind, actual Source symbol, Original target and data bits. No other operands are excluded. The whole 13-byte Source adapter shape and genuine-kernel target also pass.

One ignored genuine Source fixture passes with a dirty root pose, a returning thread-local SDK policy that repairs the real slots while changing path+14, retained old-matrix identity, finite reciprocal results, and zero-w/input-output alias behavior. It links the new object plus the existing registered archive for the historical overlay check; final archive-only validation is recorded separately in the batch report. No tracked test target or callback replacement is introduced.

This closes one raw child of avoidance construction. The current typed constructor still has explicit scene, allocator and CRT interfaces; raw clipping/winding children, world-bound global binding, parent virtual dispatch and exceptional ownership remain separate requirements. The group-add parent also remains incomplete. This packet does not assert Native private CRT, stack/frame, RTTI/SEH, virtual ABI or game-runtime equivalence.
