# Native GUI group, mesh and scene ownership integration

Addresses: `00aa31f0`, `00aa5840`, `00aa8320`, `00aa9730`, `00ac6600`,
`00b8f5e0`, `00b8f680`, `00b73d70`, `00b73e60`, `00533fa0`, `00b857f0`,
`00b86420`, `00aa5070`, `00aa4b30`, `00b724e0`, `00b72430`.

Seven worker packets and the primary GUI scene bridge are integrated. The
canonical group, mesh and section pools allocate actual18Ch/C0h/64h slots.
Their objects borrow the same native reference counters, retained owners,
hierarchy and group attachment array. Screen, Group, Icon and FrameBox adapters
share the existing widget/layout tree. The camera store uses one ordered map;
the outer24h scene owns its separate lighting resource and native root chain.
Names are descriptive hypotheses and the C++ interfaces are not binary replacements.

The cleanup integration resolved a concrete lifetime defect. Native manager
`00AA31F0` calls current20 before deleting04(1); the adapter had mixed derived
teardown into logical node release. It now preserves the native two-step order,
so a final outer-scene release cannot consume still-bound widget roots.
`GUI_NATIVE_SCENE.md` records the assembly and the actual scene/group/model
fixture, which uses no extra node retains. Its camera/weak-base services remain
controlled. `GUI_TYPE_DISPATCH.md` also corrects AC6600 to(name,node,flag),
preserving the exact third argument byte and Screen pre-base property ordering.

Tested source is `0af6d9d81dda601671edd6ba8a66c0e0371587ff`. MSVC Win32 /W4 /WX /fp:strict,
both existing CTests, eight seed ranges and eight focused probes passed. The
group pool native/host probe compared622973260 slab bytes; the mesh constructor
matched256 preimages; the section constructor matched all100 slot bytes against
104 installed code bytes. Mesh/section pool and destructor coverage is bounded
host checking, not original full-EH equivalence. No permanent tests were added.

85 names/comments were read back and their exports refreshed; prior
comment fields were preserved. Thirteen missing function entries were defined
from verified byte extents. Returning-free continuations were decoded under the
write lock, but stored function bodies for B8F680/B73CB0/427880 still have known
tail limits. Four incorrect inventory classifications were removed with old
values retained. The report records source hashes, artifact hashes, and the
90 preserved worker artifacts before tree retirement.

Full Screen acquisition, actual weak-handle ownership and directional-light
reference composition continue in separate packets. Actual GUI geometry still
needs the renderer/material services connected to these mesh/section owners.
This batch establishes ownership and ordering; it does not validate GUI
rendering or gameplay. See `reports/native_gui_owner_integration.json`.
