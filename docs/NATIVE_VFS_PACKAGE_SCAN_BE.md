# Native VFS package scan boundary (BE)

`0073CB10` is a complete 774-byte native body through the plain return at
`0073CE15`. Startup calls it at `0073D881` and `0073D888`. Each call makes a
fresh `00BDD990` query for directory `.` and extension `mpkg`, flag zero, then
consumes its native pooled list from the front. It computes priority before
the mounted-system-name query: 1000 except a leading case-insensitive `patch`
adds `atol` of the entire suffix, wrapping at DWORD width. It queries the
current manager through `00BDB120`; an absent system name is mounted through
`00BE1890` with the unchanged name, `.`, priority, flags zero and device ID
`FFFFFFFF`. The second scan sees registrations from the first; neither pass
sorts or repeats enumeration within that call.

The new source function `pop_native_vfs_package_name_00557a90` reconstructs
the complete normal path of the scan's pooled-list front copy/removal. It uses
the same actual 0Ch list/10h node layout as the existing render alias list,
the actual native string pool, and existing `004D0990` erase. It initializes
the output 8h header, copies the first node's string, removes that node, and
retains the sentinel. Its C++ ABI and exception domain are new; the native
FH3 unwind handler and arbitrary invalid-parameter continuation are not
reconstructed. The module also supplies complete normal-flow `00BDB120`
mounted-system-name lookup on actual manager+3Ch tree storage. It advances
the current iterator through existing `00BD97E0`, compares lengths, calls
`_stricmp` for nonempty equal-length names, and returns the first matching
provider. Equal zero lengths match directly. No root scan API is complete yet.

The first unresolved source dependency is actual-manager `00BDD990`. Existing
`enumerate_resources_00bdd990_fragment` takes a projected `VfsMountContext`.
Native `00BDD990` constructs a 14h visitor with `00BDBE20`, visits the actual
manager via `00BDD0A0`, and destroys through `00BDB6E0`. The visitor's
provider enumeration and actual pooled-list insertion dispatch need concrete
source bindings before the root can run. An empty installed archive inventory
does not imply an empty native enumeration result, since mounted providers can
enumerate names. `00BE1890` already has an actual-storage implementation. Integrating
this helper requires one `bsp_core` CMake registration for
`src/native_vfs_package_scan.cpp`; shared registration was reserved for the
primary integration pass. The new translation unit passed explicit MSVC Win32
`/W4 /WX /Zs`; the existing build does not compile it until registered.

Source and saved-owner evidence was read from `0073CB10`, `00557A90`,
`00BDD990`, `00BDBE20`, `00BDB6E0`, `00BDB120` and `00BE1890` in the
configured `bsp.gpr` program. No Ghidra state was changed. Source completeness
is limited to the `00557A90` and `00BDB120` normal paths; no package startup, runtime archive
or gameplay validation is claimed.
