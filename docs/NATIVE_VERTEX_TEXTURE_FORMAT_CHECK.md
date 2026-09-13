# Native vertex-texture render-target format check

Addresses: 00b20190.

The complete 42-byte body at `B20190..B201B9` is reconstructed alongside the
existing general renderer format query in `native_renderer_format_check.cpp`.
Its original ABI is ECX renderer, one stacked format DWORD, and RET4; native
EAX is exactly zero or one. The new ordinary C++ interface is not an original
register/stack ABI replacement. Descriptive names are hypotheses.

`B32410` establishes the pointer layout: it calls `Direct3DCreate9(20h)` at
`B32479` and publishes the returned actual interface at renderer `+1990` at
`B3247E`. This helper captures that current pointer, then its table, then method
`+28`, the SDK `IDirect3D9::CheckDeviceFormat` entry. Its seven stdcall arguments
are the interface, adapter0, HAL1, X8R8G8B8(16h), usage100001h, texture3, and the
caller's format. The installed Windows SDK identifies usage100001h as render
target plus vertex-texture query. Both known callers, `B3B763` and `BA15BA`,
pass format71h (`A16B16G16R16F`) and consume AL.

The original NEG/SBB/ADD sequence accepts only HRESULT zero. Positive nonzero
success results are false here. The existing `B21EC0` helper separately translates
caller resource flags and accepts all nonnegative HRESULTs; its body is unchanged.
The new helper performs no caching, allocation, ownership transition, renderer
write or format normalization. Readable aligned raw renderer storage through
offset1993h and a live COM interface/table/method are caller preconditions.
Private stack aliases, precise faults and concurrent mutation are outside the
new C++ interface; no replacement renderer schema is introduced.

All 16 original instructions and all 42 bytes were checked against saved Ghidra
and the installed executable. There are no gaps, absolute operands or direct
callee relocations. One focused local fixture executes this whole original body
unchanged from a protected executable copy and compares it with the registered
library source. Three COM capture rows check every argument, current factory
selection, exact-zero/positive/negative HRESULTs and unchanged renderer bytes.

Two further rows use an actual local Direct3D9 interface. The direct SDK query,
copied native body and source agree: format71h returns zero and true; FFFFFFFFh
returns 8876086Ah and false on this machine. This creates no device or window.
The fixture supplies raw storage for the renderer publication rather than
executing the full renderer constructor. It proves this query path, not renderer
startup, shader execution, rendering or gameplay. Default MSVC Win32 build and
both existing CTests pass. The report pins the bytes, code, runner and results.
