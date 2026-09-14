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
provider. Equal zero lengths match directly.

`scan_native_vfs_packages_0073cb10` now composes those helpers with the
concrete actual-manager `00BDD990` visitor/provider enumeration and existing
`00BE1890` mount. Its borrowed context carries the live `0109CEEC` manager
publication, actual string pool, invalid-parameter callbacks, enumeration
context and mount-registration context. It allocates a real 10h sentinel,
initializes only the list's sentinel/count words, enumerates once, then
releases the `.` and `mpkg` query strings in reverse order. Each iteration
constructs `patch`, pops and owns the front name, computes priority before
looking up that name on the current manager, and mounts only when absent.
The per-mount `.` is released before the name, then `patch`. At exhaustion it
clears the list and frees the sentinel. CMake registers this source once in
`bsp_core`.

The source API does not reproduce original SEH/FH3 frames. C++ scope cleanup
also drains the list if a dependency throws; exact exceptional interleaving,
returning invalid-parameter repair, and impossible 32-bit huge-length wrap
cases remain outside proof. Ordinary native return paths are covered. This
source is not a binary ABI replacement. The installed game has no package
archives, so the source graph's ability to mount an authentic package in this
installation is not runtime-proven; the scan must still call the actual
enumerator rather than assuming an empty result.

Source and saved-owner evidence was read from `0073CB10`, `00557A90`,
`00BDD990`, `00BDBE20`, `00BDB6E0`, `00BDB120` and `00BE1890` in the
configured `bsp.gpr` program. No Ghidra state was changed. The bounded root
and both helpers cover their normal source paths; startup integration and
gameplay validation remain separate.
