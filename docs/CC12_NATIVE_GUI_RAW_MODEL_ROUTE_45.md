# Ordinary GUI models use the bound actual string storage

`d623a5c28` removes an unconditional semantic-string requirement from ordinary
`GuiWidgetOwnerRuntime::create_model`. When `models.actual_names` is bound, the
temporary model name now uses that same storage as the existing model
constructor and destructor. With no binding, the existing semantic pool path
is preserved. The change adds one private helper; it creates no name pool,
model pool, map, reference count, native owner, or constant profile.

The production provider exists: the renderer's `game_grid_context().strings`
is the same `ActualNativeStringPoolStorage` borrowed from the VFS service
domain. It borrows the canonical string-pool publication, return gate, and
singleton-manager publication. A future GUI composition must bind that exact
stable storage to `NativeModelEnvironment::actual_names` and keep it alive
through all model destruction. This packet does not instantiate that future
composition or activate native Text drawing.

The allocation/map/reference/publication body is unchanged. Ordinary temporary
name release still precedes reference creation; the untracked auxiliary path
still publishes its caller's slot before releasing the temporary. Tracked Text
auxiliary construction, clone construction, model retirement, and the genuine
direct-raw model overload are unchanged.

This route deliberately uses the already established `NativeStringStorage`
interface. Its release operation is `noexcept`; a C++ failure while recreating
the raw string pool terminates at that interface. This is evidence for the
returning-getter domain, not a reconstruction of a throwing raw getter's FH3
cleanup, native SEH, or Original object/entry ABI. Existing host failure
handling is unchanged. No new Original function was recovered or annotated.

Relevant Original bodies remain `B75030` (ECX model slot, stack name pointer,
EAX same slot, RET4), `41E870` (ECX string header, stack C string, RET4), and
`41DD20` (ECX string header, RET). The Source owner/runtime helpers use new C++
interfaces. See [model ownership](NATIVE_MODEL_OWNER.md),
[GUI ownership](GUI_WIDGET_OWNER.md), and the
[composition contract](CC12_NATIVE_GUI_COMPOSITION_CONTRACT_44.md).

The canonical MSVC Win32 build passed with **3/3 existing suites, 9.08 s**.
Source inspection also verified that the allocation/publication/cleanup body
is identical after factoring the storage binding. Seven relevant source files,
the executable and build log are retained in a hash-checked evidence archive;
see the [machine receipt](../reports/cc12_native_gui_raw_model_route_45.json).
No new test suite or GUI/runtime probe was added for this narrow routing change.
