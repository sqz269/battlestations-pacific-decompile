# Actual application 2D texture reload

Independent Root runtime verification now passes for both the frozen f083 family
and combined Source `cdaf687819b79632f2d78713a696cb0b36f2aaa1`.
[Runtime receipt 32](CC12_TEXTURE_RELOAD_RUNTIME_32.md) pins the fresh executions,
Original/Source byte and owner/output comparisons, and successful shared drain.
Earlier Root device-creation failures remain historical evidence.

`GameNativeRendererApplication::reload_activation_texture_2d` now dispatches a
captured resource slot `+08` to the existing complete `B3FA90` Source body. The
entry requires a ready application, the resource's companion in that same
application's canonical owner registry, current profile `D61948`, and the
captured/current target `B3FA90`. It borrows the existing texture loading graph.
It neither retains the resource nor creates a second renderer, VFS, cache,
string pool, owner map, device or compiler graph.

The caller supplies `NativeTextureReloadOutputs` and retains
`NativeTextureReloadAcquired`. No output bytes are manufactured by the entry.
Both native output HRESULTs are ignored by the reconstructed body; any fields
left unwritten require explicit caller preimages. Failed acquired state and
already reached side effects remain visible under the existing core contract.
Replaying an acquired invocation is rejected. The `B3FA90` implementation was
not changed by this packet.

## Complete Original-body comparison

The ignored `local/platform_texture_reload` fixture forwards normal application
startup and links 137 current application objects plus the complete normal
`bsp_core`, Lua and zlib libraries. After application initialization it acquires
genuine installed `black.tga` resources through `B319B0`, once for each side.
Each acquisition is cold and publishes its actual canonical companion and
cache record. The first acquisition also creates the normal `error.tga`
fallback; the equivalent comparison states therefore begin with two records
and finish with the fallback record remaining.

The complete 747-byte `B3FA90..B3FD7A` body was matched to the installed PE and
live saved Ghidra bytes. Its copied execution differs only in 16 named direct
call operands and eight publication/import operand cells (96 operand bytes).
Every other byte is verified unchanged. The calls reach the existing concrete
Source children, real D3DX imports, actual COM methods and real Interlocked
decrements. Original child code is not claimed executed by those bridges.
Eight actual numeric virtual-table boundary words are temporarily relocated;
all are restored while Source children run and before Source comparison.

Original image information occupies native `ESP+34..4F`; its level-zero
descriptor occupies `ESP+50..6F`. The fixture seeds those exact, disjoint ranges
with `A5` and `5A` bytes before calling the real image-info import. Source has
separate caller-owned buffers with identical nonzero preimages. A temporary
hook on the actual COM `GetLevelDesc` entry checks the descriptor preimage and
calls the saved real method. Image-info, texture creation and descriptor calls
all return successful genuine results, once per side. All hooks are restored.

Both sides produced 32x32 DXT1 textures from images with six mip levels. The
complete 80-byte native owner preimages and postimages agree after normalizing
four independently allocated/assigned words: name pointer `+0C`, COM pointer
`+10`, serial `+20`, retained memory pointer `+4C`. Per invocation, those native
identities that the reload must preserve, including the retained source and
its reference count, are checked unchanged. The same canonical companion
survives each reload. Image-info and descriptor outputs agree byte-for-byte.
Original reached one actual VFS open, one conversion and two stream zero
terminals. Actual cache removal and final canonical texture retirement pass.
The retained Source invocation rejects replay.

The normal Release build and all three existing checks pass. The final fixture
exits zero, joins the renderer worker, clears native publications, and drains
the real device and API to zero. Binary pre/postimages, relocated Original
bytes, verification receipts, exact inputs and the recipe are frozen under
`local/checkpoints/cc14-platform-texture-reload/final/manifest.json`.

## Boundaries

This validates the successful real-provider 2D reload path. The actual renderer
`1B00` pointer array was empty, so nonempty array removal is not exercised.
Failed HRESULTs, invalid streams, provider exceptions, original FH3/SEH,
arbitrary profile changes and native drop-in ABI are not validated. The
isolated fixture forwards only two frames; GPU appearance and gameplay are
not established.

The full `B24FB0` activation composition remains separate: effect reload needs
the same current program service and retained frames, cube/volume reload needs
concrete provider bindings, and `F8D438` needs a distinct production authority.
This entry does not implement the abstract resource dispatcher, admit
`BED3B0`/`BECEE0`, or alter the separate lost-device hold behavior.

The first fixture run incorrectly expected the pre-fallback cache count; a
later artifact-capture attempt used a relative path after startup changed the
working directory. Both failed runs are retained. The final recipe accounts
for the real fallback and uses an absolute artifact directory.
