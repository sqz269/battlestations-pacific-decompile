# Native shader descriptor to pass route

The installed `shaderfx/common/debugshader.shfx` is a Lua-style descriptor that
includes `shaderfx/dx9_lua.inc`. It supplies render states, vertex inputs,
interpolators and VS/PS source fragments referring to generated symbols such as
`SYS`, `IN`, `OUT` and `cViewProjMat`. These fragments are not standalone HLSL;
compiling the literal VS/PS strings would omit the native source generator.

Exported native chain:

- `00b46950`: ECX effect, filename stack argument, RET4. With global0108d6f0
  clear, forwards filename/0/0 to `00b45ee0`. With it set, loads filename/0/3,
  calls `00b41b10` and `00b187a0`, then filename/1/3 and returns AL=1.
  Do not treat that branch's return as proof both loads succeeded.
- `00b45ee0`: allocates a 110h descriptor via `00b43700`, reads it using
  `00b43b00`, iterates configured combiners and forwards combined descriptors
  into `00b3c3a0`. The meaning of variant flags and complete merge behavior
  remain unresolved.
- `00b43b00`: reads the `Shader` table through helpers in the00b66xxx/00b69xxx
  range. Observed keys include PipeID, Priority, VertexFormat, ReceiveShadows,
  VSVersion, PSVersion, RenderStates and Constants. Ghidra removes several
  blocks as unreachable; raw assembly must be audited before porting the parser.
- `00b3c3a0`: constructs temporary builder state, forwards to `00b3b3c0`, then
  tears it down through `00b3a7e0`. Its decompiler ABI is incomplete.
- `00b3b3c0`: coordinates source/bytecode paths, metadata, pass state and shader
  wrappers. One branch obtains bytecode through `00b34890`, reflects it with
  `00b3aea0`, then reaches device vtable+16Ch/+1A8h (CreateVertexShader and
  CreatePixelShader). Wrappers are constructed by `00b5faf0`/`00b5f9b0` and
  attached through `00b5f0c0`/`00b5f080` before temporary references are released.

This investigation establishes the dependency route, not a complete shader
compiler or cache implementation. Source-generation helpers `00b36800`,
`00b34aa0` and `00b39880`, bytecode lookup `00b34890`, and descriptor merge rules
are the next bounded targets. The current probe's tiny compiled shaders remain
explicit diagnostic inputs. Raw exports are retained under ignored
`exports/bsp/functions/`; no original shader files are copied into the repo.
