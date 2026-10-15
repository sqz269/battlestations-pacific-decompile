# Borrowed platform publication in the renderer

The new `GameNativeRendererApplication` entry accepts an existing
`NativePlatformLoadPublication`. DeviceGraph, reset and loading consumers keep
that wrapper, which points to the caller's publication cell; they do not keep
a snapshot of its receiver. This closes a Source binding gap for actual raw
platform callers. It does not install the game's address0109CF04 or replace
the current projected host startup path.

Report: `reports/native_platform_publication_binding.json`. Six pre-change
Source/API files are preserved under
`local/checkpoints/cc14-platform-publication-borrow/before`.

## Explicit entry and compatibility

An actual raw caller passes the same mutable cell already borrowed by
`NativePlatformConstructionContext`, explicitly wrapped:

```cpp
GameNativeRendererApplication renderer(log, singletons, vfs, lua, data,
    clock, NativePlatformLoadPublication(platform_context.actual_platform_0109cf04));
auto publication = renderer.borrow_platform_publication_0109cf04();
```

The wrapper and its copies contain pointers to the cell, not copies of the
cell's current receiver. The caller retains that cell and every reached owner
through renderer drain. The new accessor returns metadata without reading the
receiver, changing phase or entering native code. It can be passed directly to
the existing loading/sampler interfaces that already accept this wrapper.

The old final `const void*` constructor argument remains available. It retains
its pointer-value behavior in an explicitly named
`Impl::legacy_platform_focus_publication` cell. The existing
`loading_platform_focus_publication()` reference signature and ready-phase
guard are preserved for that entry. A caller using the new borrowed entry is
rejected by the legacy accessor and uses the wrapper accessor instead; no
const-pointer alias or fabricated cell is returned.

GameStartupHost still passes its original projected44h focus storage through
the old constructor. That storage covers +30/+41; it is not the raw184h owner
or height+28. No host callsite, platform constructor selection or native
message admission changes here. Wrapper borrowing alone never proves that a
cell is the canonical game-process0109CF04 publication.

`NativeRendererResetProcessContext::actual_platform_0109cf04` changes from a
`const void* volatile&` to the existing wrapper value. Existing aggregate
initialization from a persistent const-pointer cell still compiles; mutable
raw publication cells now bind without a pointer-alias cast or copied value.
The source context's Win32 size changes from24 to28 bytes. It is metadata, not
an original binary interface; out-of-tree callers must rebuild. No native ABI
claim follows from this interface change.

## Current loads versus captured receivers

The four established B2ABD0 load sites remain separate:

| Original site | Source behavior |
| --- | --- |
| B2AC9A | Load current platform for the existing lost-device branch's captured focus check |
| B2ACDF | Load current platform, retain that explicit receiver across GetFocus |
| B2AD47 | Reload current platform after release calls and sleeps, then read its HWND |
| B2ADD2 | Reload current platform for the later B20C50 fallback focus check |

Each existing source read now invokes `load()` at that site. No import or
provider is moved, replaced or added. The existing deliberate lost-device
hold policy still excludes the B2AC9A recreation path in the normal build;
this packet does not alter or validate that divergence. The renderer's startup
focus observation also loads through the same wrapper.

BEC3B0's original36 bytes still read current0109CF04 on every call. The raw
BED3B0 handler remains unbound; its explicit active receiver and captured
WindowExtra receiver must stay distinct from the current publication. This
packet neither supplies that handler nor admits WM_ACTIVATE/full BECEE0.

## Bounded validation

Four fresh Ghidra/installed-PE spans and saved listings cover660 bytes:
B2ABD0[589], B20C50[31], BEC230[4], BEC3B0[36]. The four exact CF04 loads and
thunk operand are checked. Live queries verify the configured bsp project and
`/battlestationspacific.exe`; configuration names `C:/Users/sqz269/bsp.gpr`.
Ghidra annotations, function counts, shared packet states and CMake are
unchanged; this is binding credit, not another reconstructed native body.

The focused manifested fixture constructs two raw388-byte platforms using the
current Source constructor and one mutable cell shared with its construction
context. Explicit fixture preimages set both +41 bytes to1, A's HWND to null
(matching the isolated thread's real null focus), and B's HWND to the hidden
window. These are not BECEE0-produced window state. Copied Original31 and
current Source B20C50 capture owner A. A fixture
GetFocus import wrapper calls real GetFocus and then replaces the publication
with B. The active call still compares A's HWND; a later wrapper load sees B
and its different HWND. A persistent const-pointer cell also retains its
identity and current-value behavior. All776 owner bytes remain unchanged.

The fixture also enters current B2ABD0 from the normal built core. It uses an
isolated thread, hidden window and actual private HAL device. A COM wrapper
calls the real TestCooperativeLevel (S_OK observed), then returns E_FAIL solely
to select the existing other-HRESULT path. The first captured focus passes,
the publication changes to B, the later current focus fails and prevents
recreation; the real final Sleep(100) runs. The observed order is cooperative,
GetFocus, GetFocus, Sleep. No release/recreation/resource child is reached.
The unchanged historical reset-fixture provider graph rejects unexpected
boundary calls; it is not a newly supplied application provider.

Process IAT and COM slots are restored, the device/window are released and
both raw queues, title/base/publications and singleton manager are retired.
The fixture makes no foreground, display-mode or power-policy changes.
Both constructor signatures and the legacy reference-accessor signature are
compile-checked. Normal application COFF contains both constructors and both
accessors; dataflow inspection checks the DeviceGraph and reset wiring.

`./scripts/build.ps1` Release passed3/3. The reset translation unit and focused
probe passed `/W4 /WX /fp:strict`; the probe links the current normal core and
embeds an asInvoker manifest. The new complete application construction path
was not executed by this fixture, and no whole application, renderer
recreation, native exception or game validation is claimed.

Next work is full raw message ownership plus concrete B24FB0 resource/material
activation bindings. B19000's existing Source, B469A0's descriptor domain and
B3FA90's existing reload Source must be composed in their actual canonical
resource domains before full handler/window admission.
