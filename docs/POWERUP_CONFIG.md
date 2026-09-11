# Powerup configuration owner and Lua loading

Addresses: 008EDC60, 008ECEC0 and the 22 direct helpers listed with complete
byte spans, hashes, original ABIs and classifications in
`reports/powerup_config.json`. Descriptive names are hypotheses.

`construct_powerup_config_008edc60` constructs the native 1C4h owner's actual
container headers and 35 allocated sentinels. It preserves every allocator word
and the upper three bytes of the flag word at10. The native vectors start empty.
Only after all container construction does008EDD8A publish00F88C30. The parent
004DC6A0 publishes the returned pointer again before invoking008ECEC0.

The C++ shell carries the exact native constructor storage and separate canonical
standard-map payload projections. Its host allocation is larger than1C4h. Native
STL balancing, iterators, allocation timing and exception ABI are not reproduced
by those maps. The runtime lists/maps retain real native sentinel allocations;
their later payload behavior remains outside this packet.

The loader opens a temporary Lua owner with mask4 and runs
`Scripts/datatables/PowerupClasses.lua`. It walks integer groups1..3, captured
category keys, then captured class keys. It updates an existing class cell or
publishes the default descriptor before reading its fields. It keeps the native
name data/length captures across insertion and callbacks. A random descriptor
appends its name first to its category list and then to its group list (`A*`,
`S*`, `T*`). Reloading does not clear maps or existing random-name lists.

The E4h class descriptor contains two38h faction records, duration/cooldown,
target type/filter,16 multipliers, random byte, texture and UV fields. Constructor
untouched scalar bytes are explicit32-word allocation inputs. The two untouched
stack words passed as faction texture-size scratch are also explicit inputs.
Strings own pooled buffers. Descriptor/faction copies do not retain texture
pointers, and loading over a texture does not release its previous pointer.
Destructors use real atomic decrements and dispatch the current virtual slot0
only at zero. Duration/cooldown copying preserves the actual x87 load/store
operations. Target type/filter publication follows reader-reference cleanup.

Remaining bindings are the current GUI texture services, current selected
player's faction field28, and actual texture zero-reference dispatch. Existing
GUI Lua readers and `resolve_gui_texture_00aa2660` execute directly. Authored
category/class keys must be strings. Lua, CRT, compiler helpers and STL are
library boundaries; no fabricated native payloads or default service results
are supplied.

`release_powerup_configuration_storage` is a host cleanup convenience requiring
empty runtime vectors/lists/maps. It is not the native runtime destructor
008EDDA0, does not clear publication, and does not free the owner shell. C++
exception cleanup covers only the documented constructed storage, not native
SEH unwind tables. Native binary replacement and gameplay remain unvalidated.

The primary adopted saved worker source after its usage-limit failure, completed
the report and integrated the owner into004DC6A0. An independent worker compared
the complete constructor/loader listings and focused descriptor helpers; its
reader-cleanup correction is included. The combined MSVC Win32 Release build,
both existing tests and the real-Lua powerup fixture passed. The fixture covers
duplicate loads/overrides, numeric fields, sentinels, random lists and reference
ownership. Exact logs and library hash are in the report.

The primary defined missing008E6130 through RET008E6161 from matching live/disk
bytes, and restored the reachable ADD ESP,4 at008ED43C after the falsely terminal
free call008ED437. Unreachable alignment bytes008ED12D..12F and008ED159..15F
remain untouched. The owner constructor, Lua loader, class constructor,
non-deleting class destructor and class copy constructor were incorrectly tagged
as compiler helpers; their saved before/after records accompany this report.

Follow-up packets: recover008EDDA0 runtime owner destruction and the GUI texture
service dependencies only after checking current leases. Keep actual runtime
container state distinct from the configuration maps' standard-library projection.
