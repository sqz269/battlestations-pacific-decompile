# Native texture filename route

Verified project `bsp`, program `/battlestationspacific.exe` before each live
analysis batch. Renderer constructor00b32410 installs primary vtable00d5f0a8;
its+64 entry is00b319b0. That valid function was absent from Ghidra's inventory
and has now been defined from the vtable/instruction boundary. Snapshot counts
are62615 total/62173 internal. Bytes are recorded in reports/resource_path.json.

## Renderer to resource manager

00b319b0 takes ECX renderer, stack native string pointer/flags, RET8. It enters
the established optional renderer guard, copies the requested name, performs
ASCII lowercase through004bcc00, and calls00b30b40 on renderer+1A74 with
stack normalized-name/flags/0/1. It frees its copy, leaves the guard and returns
the logical texture pointer. The renderer itself is not the manager's this.

Constructor00b32410 assigns manager vtable00d5f088 to renderer+1A74. Its slots
are destructor00b339d0, resolve-name00b31c20, load00b2c2d0,
retain/return00b31d80 and release00b31da0. Only the direct route has been
analyzed here; no generic resource-manager replacement is claimed.

00b30b40 normalizes resource names through00bee690, searches2Ch cache records
and their alias lists using length/case-insensitive comparisons, invokes the
manager's resolver/load virtuals on misses, and registers loaded resources and
aliases. Its SEH/string/container lifetimes and full alias logic remain unported.
The four stack arguments and call-site constants must not be collapsed into a
simple direct filesystem load.

Resolve-name00b31c20 takes ECX manager, stack output-string/request/flags,
RET0Ch; flags are not consumed in the inspected body. It initializes the
fallback when manager+1C is zero, copies the requested name, and calls
00bdf4c0. Failure logs `ERROR: TEXTURE NOT FOUND` and substitutes the manager's
fallback string at+14/+18. Initialization00b31bd0 writes `error.tga`, sets the
initialized byte before resolving/loading it, then caches the result at+20.
Full00bdf4c0 path search/overlay rules remain unresolved.

## File decoding boundary

00b2c2d0 opens the resolved name through file manager0109ceec virtual+4 with
mode2, checks virtual+18 validity, obtains a retained memory-file view through
00bef750 and calls D3DXGetImageInfoFromFileInMemory. Subsequent branches select
2D/cube/etc creation and renderer texture-quality adjustments. The visible2D
branch includes special name checks for detail.dds, noseart and
interface/textures/gui/units before mip/size reduction. These policies and
file ownership remain unported; the current diagnostic DDS loader cannot
substitute for this entire route.

## Implemented common path normalization

004bcc00 (ECX string, RET) iterates the native stored length, converting only
ASCII A..Z. High bytes, slashes, spaces and bytes after NUL are otherwise
preserved.00bee690 (ECX string, RET) then changes backslashes to slashes and
trims only byte20 spaces at both ends. Tabs and dot segments are not cleaned.
Empty/all-space input becomes empty. It copies the selected substring through
00469840, whose strncpy operation zero-pads after an embedded NUL without
shortening the stored length. A generic trim/lower/filesystem canonicalizer
would not preserve these details.

`resource_path.cpp` ports these semantics for typed strings up to INT32_MAX
bytes; larger paths fail before mutation. The host script-asset resolver now
uses this common operation in place of its ad hoc case/slash loop. That reuse
does not establish the native script file manager's complete resolution route.
Native string allocation, pointer aliasing and malformed storage are outside
the typed interface. MSVC Win32 build, existing CTests and the full installed
shader/material/DDS probe pass; unusual embedded-NUL/whitespace paths are
assembly-backed behavior, not covered by the ordinary installed-file fixture.
