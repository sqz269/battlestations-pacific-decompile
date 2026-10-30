# Raw locale map lookup

`find_native_locale_map_node_00a9ec70` supplies the read-only actual-map lookup
used beneath A9F4B0. ECX in Native is the bucket-array/map base, not a semantic
LocaleTables or the enclosing manager. The ordinary Source API returns the
borrowed node or null; the wide value remains at node+8 and is not copied/owned.

Evidence is the complete saved A9EC70..A9ED02 assembly. Hash captures key length
once and data only for nonzero length, folds ASCII A..Z, sign-extends each byte,
wraps multiply/add131 and masks4095. Each candidate captures node length before
CURRENT key length. Equal zero lengths hit without data reads. Otherwise current
key data precedes node data and the existing Source `_stricmp` receives node,
key. Only mismatch samples CURRENT node+14, including after the comparator.

All raw reads are compiler-fenced byte copies; no allocation, string/container
cast, bound, cycle repair, node cleanup, canonical consumer grant or loaded flag
is introduced. Caller proves accessible map/key/chain/data and serialized valid
lifetime across the current CRT callback. Current `_stricmp` locale/handler and
exception behavior are qualified Source boundaries, not Original CRT identity.

Original register/RET4 ABI, private-frame aliases, FH3/SEH/hardware, transitive
loading and runtime/game proof remain unproved. The existing semantic lookup is
unchanged. No test, probe/game, Ghidra mutation or library/vendor body credit is
part of this packet. Source caller/activation remains unbound.
