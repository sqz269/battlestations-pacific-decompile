# Orchestrator 6 reconstruction batch Z

Addresses: `00B67400`, `00B67530`, `00B67580`, `009292B0`, `00B67460`,
`00B66790`, `00B675D0`, `0087BCC0`; supporting definitions `00CA71A0`,
`0080DF80`, `006D1DF0`.

This batch supplies six complete Lua field setters, the complete unit class Lua
binding, and the complete normal health/parts initializer: 1,591 original body
bytes across eight routines. These are prerequisites of the existing killed-unit
and scene initialization callers. Their C++ interfaces still require actual
unit/world bindings; health initialization retains explicit full native providers.

| Component | Recovered behavior and proof |
| --- | --- |
| Numeric Lua setter | Exact x87 float widening and current owner/key/index reads. 27 original-byte/source pairs include NaNs, subnormals, four rounding modes and allocator rebinding. |
| Lightuserdata/new-table setters | Current stack objects, length-delimited keys and repeated field loads. Five native/source cases; 44 direct call rows. |
| Class Lua binding and three setters | Actual Lua and pooled strings; later world reload, target-state object-index semantics and normal tracked-object cleanup. 100 direct/tail rows; original exception transport remains unproved. |
| Health/parts initializer | Both allocation/detail branches, checked vector and numbering tail. Five native/source pairs plus source constructor cleanup; required part-set/model providers remain explicit. |

Review corrected compiler reordering of key and owner/index reads. The optimized
objects now establish the documented sequence. The unit `+190` leaf returns six
with a bare RET: the previously pushed float belongs to the following `+8` call,
alongside the returned selector. Both `FLD` and `FLD1` detail paths are retained.
Older helper coverage and the affected argument descriptions are corrected.

The two supporting unit leaves and class-binding EH selector were defined from
matching live/disk bytes. Confirmed names and evidence comments were saved in
Ghidra with prior values retained; affected exports were refreshed. Supporting
definitions do not constitute additional C++ provider implementations.

The combined MSVC Win32 Release build and both existing CTests passed at
`fdbab48b5a87fd024d6d767ead02d3e32303a0c4`. Executable SHA256:
`decd4be3212198d82d0028eef56f81579a09670f96341e5368d2742ddf80c507`. The 120-frame USN01 compatibility run produced
18,557 finite trajectory rows and 241
unchanged Airfield2 samples, 10,080 generic ticks, and 420
valid world nodes. All 77 unit observer prefixes were torn down with
the owner live. The full run record is in `reports/orch6_reconstruction_z.json`.

The compatibility run does not prove execution of these new initialization or
killed-unit paths. Original FH3/SEH, Lua error transport, complete provider/runtime
binding and full gameplay/visual validation remain incomplete. The full game
reconstruction goal remains active.

The root retains 1,674 worker artifacts, the numeric setter's exact
compiler/source/library/fixture proof, and annotation receipts. Worker commits
are merged, and 47 archived production source/header hashes match the
integrated tree. Worker trees may be removed only after the final archive and
clean-worktree checks. Historical fixtures retain their own compiled objects
and libraries; the final executable is identified separately above.
