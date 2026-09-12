# Section texture, Icon frame, menu listener and PointLight population

Addresses: 00ac0280, 00abf6f0, 00abf4f0, 00abf590, 00ab1150, 00ab1110,
00ab27f0, 00581970, 00581b20, 004fa100..004fa160, 00b7b090, 00b72140,
00b6ef20, 00b6e8c0, 00b8f100, 00b0ca40.

This batch was developed on `agent/orch5-20260911`, starting at 95d2e955, with
three independent workers in their own leased worktrees. It extends the same
canonical owners from the previous Section/frame/PointLight batch. It does not
provide a runnable game or binary-compatible GUI replacement.

Section now reads its nine properties and runs the actual texture-name/resolver,
texture getter, atlas and retain sequence. The setter preserves the native
overwrite without releasing the old texture. The native pair of unwritten size
scratch DWORDs is an explicit required caller input. Derived retirement releases
the current texture and string before the shared base. Actual Section current4
now composes that cleanup with common Group/Text/Section child deletion and the
existing C++ wrapper disposition. Native pool and SEH behavior remain outside
this interface. See [GUI_SECTION_TEXTURE_OWNERSHIP.md](GUI_SECTION_TEXTURE_OWNERSHIP.md).

Icon supports rotation44, temporary-state84, AutoRotate and its frame40 tail.
The primary frame integration holds one active owner guard across the base call
and derived tail, spills a copy for the base argument, and retains the original
delta for the timer. The numeric code preserves x87 intermediate precision,
unordered tests and the post-callback countdown clear. See
[GUI_ICON_FRAME_RUNTIME.md](GUI_ICON_FRAME_RUNTIME.md).

The seven empty base listener leaves are now defined and documented. Five
callbacks are exposed through the canonical C++ listener. Main-menu current0C
and current18 operate on the existing screen and scrollers, current Text content,
and actual Icon/FrameBox state operations. The main-menu class remains abstract
at its incomplete current04 command handler, 005993a0. See
[MAIN_MENU_WIDGET_LISTENER.md](MAIN_MENU_WIDGET_LISTENER.md) and
[GUI_WIDGET_LISTENER_BINDING.md](GUI_WIDGET_LISTENER_BINDING.md).

PointLight population now follows B7B090 through the actual root/node traversal,
preserving strict sphere comparisons, repeated sphere queries, current sibling
loads and duplicate reciprocal physical backlinks. The particle fragment
B0CAE1..B0CBA6 produces the actual raw center/radius fields and preserves
alias-sensitive stores. Model and cached/static Group bounds are supported;
dynamic Group aggregation and a real Text-light provider remain unresolved. See
[NATIVE_POINT_LIGHT_PROVIDER.md](NATIVE_POINT_LIGHT_PROVIDER.md).

Validation includes the combined MSVC Win32 Release build and both existing
tests. Five local probes passed: production Icon numeric kernels; Section atlas
resolver/storage; same-screen menu/scroller alias behavior; existing recursive
Group teardown; and actual PointLight pool/population/backlink behavior with both
teardown orders. The latter linked the combined library. No permanent tests were
added. These probes do not establish a complete resource-backed Icon/Section
runtime, original-byte differential agreement, render parity or gameplay.

Independent reviews covered the Section setter, Icon-frame/common-base
integration, main-menu source/listing and PointLight raw identities and ordering.
All passed. Seventy numeric call rows passed the mechanical check: resolved
indirect destinations additionally rely on the recorded table evidence. The
symbolic resource and import calls are retained as documented contracts and are
not counted as mechanically proven direct calls.

The parent defined Icon44, seven listener leaves and two analyzed provider leaves
under the Ghidra write lock. ABF4F0 was already defined when checked. The names,
prior comments and saved annotation archive are recorded in the batch report;
affected exports are refreshed. Every descriptive name remains a hypothesis.

Follow-up packets are recorded in `reports/orch5_texture_icon_listener_batch.json`:
the actual 5993A0 menu command chain and screen provider; resource-backed GUI
configuration/validation; Section native copy/pool retirement; and the Text
PointLight provider with dynamic Group/particle dependencies. Their incomplete
dependencies must be resolved before treating them as independently ready work.
