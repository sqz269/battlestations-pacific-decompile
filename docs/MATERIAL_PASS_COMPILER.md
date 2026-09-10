# Native material shader compiler calls

The original game imports `D3DXCompileShader` from **`d3dx9_40.dll`** and uses
backwards-compatibility flags for normal material compilation. VS wrapper
`00b60f60` passes `0x1200`; PS wrapper `00b61280` normally passes `0x1400`, with
a name-specific zero-flags exception for lowercase `shore`. A host call with
flags zero does not reproduce the normal native material compiler settings.

This audit provides the native contract to the primary-owned
`src/compiled_material.cpp`. The worker changed only this document and
`reports/material_pass_compiler_audit.json`; it did not alter installed shader
source, shared compiler code, Ghidra annotations or ledgers.

## Imported API and flags

The original PE import directory maps IAT `00CE23EC` to
`d3dx9_40.dll!D3DXCompileShader`. Thunk `00C2E00A` is the six-byte
`JMP [00CE23EC]`. Both wrapper callsites target that thunk. The complete
604-byte VS body and 2183-byte PS body match live Ghidra memory and the original
installed PE. The audit also captures the name-construction slices and three
approved string helpers: 20 spans, 4324 matched bytes in total.

The official SDK header already fetched by `cmake/d3dx.cmake`,
`build/win32/_deps/dxsdk_d3dx-src/build/native/include/d3dx9shader.h`, defines
these bits at lines 88..90:

| Stage/path | Native flags | Official flag names |
| --- | --- | --- |
| VS, `00B60FB6` | `0x1200` | `D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY` plus `D3DXSHADER_AVOID_FLOW_CONTROL` |
| PS normal, `00B61318` | `0x1400` | `D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY` plus `D3DXSHADER_PREFER_FLOW_CONTROL` |
| PS name contains lowercase `shore`, `00B612F6` | `0` | No flags |

The backwards-compatibility bit is `1 << 12`; these values do not include
`D3DXSHADER_USE_LEGACY_D3DX9_31_DLL` (`1 << 16`). The imported DLL and actual
flags together identify the native request; the DLL name alone is insufficient.

Both wrappers call the ten-argument API in the official header with:

| Argument | Native value |
| --- | --- |
| Source | Borrowed NUL-terminated generated HLSL pointer supplied by caller |
| Source length | `strlen(source)`, excluding the terminating NUL |
| Macro definitions | null |
| Include handler | null |
| Entry point | `"main"`, literal at `00D582A0` |
| Profile | Caller EDX pointer, unchanged |
| Flags | Stage/name-specific values above |
| Compiled shader output | Local `ID3DXBuffer*`, initialized null |
| Compiler messages output | Separate local `ID3DXBuffer*`, initialized null |
| Constant-table output | null; later reflection remains a separate operation |

The source is scanned and passed synchronously. Neither wrapper transfers its
ownership, edits its contents, nor frees it. `std::string::size()` is equivalent
to the native length only when no embedded NUL exists; an exact projection uses
the first NUL boundary. The normal caller obtains profile strings from primary
descriptor `+34/+38` for VS and `+3C/+40` for PS, using the data pointers at
`+38/+40`. The wrappers do not select a default profile themselves.

## Original ABI and success paths

VS `00B60F60` receives the native engine-name string in ECX, profile C string
in EDX, source and `IDirect3DVertexShader9**` on the stack; EAX returns the
result and the function ends with `RET8`. PS `00B61280` receives engine name
in ECX, profile in EDX, then source, `IDirect3DPixelShader9**`, TEXCOORD masks
and COLOR masks on the stack, `RET10`.

Both branches test whether a compiled-bytecode buffer was produced. They do
not use `FAILED(hr)` as that branch's test. If code is absent, they return the
compiler HRESULT without a retry or substitute shader. Their successful code
paths consume `ID3DXBuffer::GetBufferPointer`, create the D3D9 shader object,
then release owned compiler buffers. There is no retained pointer into source
text or into a released bytecode buffer in the established success path.

The VS wrapper disassembles the compiled code before device shader creation,
writes its native diagnostic, releases the disassembly buffer, calls device
virtual `+16C` (`CreateVertexShader`), releases code and compiler-message
buffers, and returns the creation result. The native debug-write path itself
uses VFS/string helpers and remains outside the host compiler projection.

The PS wrapper calls device virtual `+1A8` (`CreatePixelShader`) first.
When both mask pointers are nonnull it disassembles the code, runs the already
reconstructed component-usage parser unless `texcoord[0] == 500`, and releases
the disassembly buffer. The sentinel selects the `.psa2` diagnostic path;
ordinary usage collection selects `.psa1`. Without both mask pointers the
wrapper skips this disassembly/usage branch. The optional disassembly HRESULT
overwrites the saved return value, so it can be the returned status rather than
the earlier device creation result. This is why output presence and individual
call results should remain visible in a safe host integration.

The successful paths release code and compiler-message buffers. The native
code-null early return bypasses that cleanup; copying this possible error-path
message-buffer leak is unnecessary. Host RAII can release every returned COM
buffer on all exits while reporting the failure explicitly. That cleanup is a
host improvement, not evidence that the original had the same failure behavior.

## Disassembly and diagnostics are separate

The native disassembler calls are:

```cpp
D3DXDisassembleShader(static_cast<const DWORD*>(code->GetBufferPointer()),
    FALSE, nullptr, &assembly);
```

Both VS `00B60FD8..00B60FEC` and PS `00B6137A..00B6139E` push zero for color
coding and null for comments. A host adapter using `FALSE` is therefore correct.
The VS writes `shaderfx/debug/<engineName>.vsa`; PS diagnostics use `.psa1` or
`.psa2`. Those writes explain native diagnostics; they do not justify writing
to the original installation during reconstruction. The host may keep bounded
failure diagnostics in repository-local storage.

Neither audited wrapper retries with `D3DCompile`, loads a different compiler
DLL, removes shader statements, or selects a fallback shader. Any diagnostic
probe using another API or synthetic source must retain a separate validation
label. Likewise, proving these native settings does not alone prove they fix
the observed `X3025` failure: the primary integration must compile the unchanged
installed generated shader and report the resulting status.

## Exact input to the shore exception

The PS test is case-sensitive CRT `strstr(engineName.data, "shore")`. Null name
data uses normal flags. The additional native subtraction/result-`-1` comparison
does not alter valid-buffer substring behavior. No case conversion appears in
this wrapper or the inspected name construction.

Both wrapper callers pass the builder's **complete string at `+9C`**, whose
data pointer is at `+A0`. They do not pass the selected combiner filename. The
normal `00B45EE0` loader constructs that name as:

```text
base_filename_before_last_dot + unsigned_decimal(mode) + T_or_F + "3"
```

The exact chain is established by these bounded reads:

- `004BCB80` reverse-searches a literal byte set and `00469840` copies the
  selected substring. `00B45FB3..00B4603B` uses `"."`, index `INT_MAX`, then
  copies from start zero to the final dot. It searches the complete filename,
  not only its final path component.
- `00B46109..00B46193` copies that stem and appends `00711370(mode)`.
  The complete helper uses `sprintf("%u", DWORD)`, then copies the result into
  a native string; it does not fold case or append a combiner name.
- `00B461E5..00B46267` appends `"T"` for a nonzero selected variant byte,
  otherwise `"F"`. The semantic role of that external byte is not renamed by
  this compiler audit.
- `00B4628B..00B462C4` appends `00711370(3)`; this literal generation value is
  the one supplied by this inspected normal loader path.
- `00B3C3CB..00B3C419` copies the constructed name into the temporary builder's
  `+9C` string. `00B3B3FB` retains its address in EBX for the initial PS call;
  later VS and PS calls explicitly use `builder+9C` again.

`00B3B3C0` also creates a slash-trimmed local name for other work. That local
copy does **not** replace builder `+9C`, and the shader compiler sees the full
constructed path. Thus lowercase `shore` in a directory name counts too.

For ordinary `.shfx` descriptor paths, a case-sensitive search of the original
base descriptor path is equivalent: the removed `.shfx` and added decimal/T/F
suffix cannot introduce or remove `shore`. This equivalence does not authorize
lowercasing an arbitrary name or testing the combiner path. A general host
compiler can accept the exact engine name explicitly; unusual extensions,
embedded NULs and any earlier VFS case/canonicalization policy should not be
silently inferred from this normal-path proof.

## Validation boundary

The configured existing project/program were verified before native queries.
All captured native ranges match the original PE, and the official local
headers establish flag names and call signatures. This packet is a compile
contract audit, not an additional C++ implementation or new test target.
The primary owns the shared compiler implementation and existing focused
material probe. Native binary ABI, full cache/diagnostic behavior and original
game execution remain outside any successful host shader compile.
