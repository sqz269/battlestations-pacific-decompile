# Native material geometry query leaves

This packet reconstructs four complete native leaves called by
`BSP_MaterialPass_BindGeometryAndDraw` (`00B44750`). The source takes the actual
borrowed native stream or draw-section pointer in ECX. It does not own, check,
normalize, or initialize that storage. The descriptive C++ names are hypotheses;
the Ghidra names were already present and were left unchanged.

| Original | Complete PE/live body | Native read | Return | Direct calls from `00B44750` |
| --- | --- | --- | --- | --- |
| `00B48D50..53` | `8B 41 70 C3` | logical vertex stream `+70`, DWORD | EAX, RET | `00B44AAB`, `00B44AD1` |
| `00B48DE0..E3` | `8B 41 20 C3` | logical index stream `+20`, DWORD | EAX, RET | `00B44A8D` |
| `00B855A0..A3` | `8B 41 1C C3` | draw section `+1C`, DWORD | EAX, RET | `00B4478E`, `00B447E3` |
| `00B855F0..F3` | `8A 41 58 C3` | draw section `+58`, byte | AL only, RET | `00B44ABE` |

The byte getter preserves the original upper EAX bits. Consumers may use AL;
they must not infer a zero-extended EAX result. All four originals have one
two-instruction block and no callees or stack arguments. The `00B44750` caller
supplies zero when the index stream is absent; that policy stays with the caller.

Evidence is pinned to source base `97e0726455ef19c607fc8c55b3e2631ae4714d01`,
Ghidra project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and the installed PE SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The saved export assembly, current Ghidra bytes and xrefs, and installed PE
bytes agree. Exact per-function COFF evidence and build status are in
`reports/native_material_geometry_queries_bw.json`.

The native source is registered in `cmake/startup.cmake` and provides callable
Win32 entries for the geometry provider integrator. This establishes exact
four-byte function bodies and original register/field semantics. It does not
establish a drop-in binary replacement, a complete `00B44750` reconstruction,
or game/render validation.
