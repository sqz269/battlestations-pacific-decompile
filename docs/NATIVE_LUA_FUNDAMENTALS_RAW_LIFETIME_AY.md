# Native Lua fundamentals on the raw singleton manager

Addresses: `00884770`, `00B68340`, `00B667D0`, `00B66B80`; deletion profile
`00D62C18` has slot zero `00B66B80` in both the live program and disk image.

The fundamentals getter now borrows `SoundLifetimeAccess`, the existing source
adapter for either the application's actual 01090AA0 publication or the retained
semantic fixture domain. It creates no private manager. The constructor's raw
0Ch cache, VFS calls, allocation preimages and scalar destructor are retained.
The raw manager's deletion bindings now admit D62C18 with the actual 0108FF1C
publication. They pass the popped allocation to the destructor even when that
publication has changed; deletion clears the current cell unconditionally.

The getter captures the first manager's section, enters and increments its
recursion word, then rechecks publication. Its inner SEH frame frees only an
unpublished allocation when construction fails. Success publishes first, resolves
the manager again, and registers the then-current publication on that captured
second manager. The outer section scope decrements and leaves the first section
before the final publication reload. This preserves manager replacement during
VFS callbacks. Source C++ exceptions release the section; original FH3/private
stack identity and hardware-fault behavior remain outside the interface.

| Function | Native ABI |
| --- | --- |
| 00884770 getter | No arguments; EAX owner; RET |
| 00B68340 constructor | ECX fresh owner; EAX owner; RET |
| 00B667D0 base cleanup | ECX owner; RET |
| 00B66B80 scalar destructor | ECX owner; stack flags; EAX original pointer; RET4 |

The retained original-instruction fixture still passes ten constructor
executions in five paired cases, native/source getter and scalar/base cleanup,
three semantic domain shutdowns and source read-failure cleanup. Its extension
passes four raw native/source getter executions, with and without manager
replacement, fast returns, captured lock-depth checks, actual manager registration
and changed-publication raw drain. The source read-failure case registers
nothing, releases the captured section and preserves the native stream/buffer
retention; the fixture explicitly frees that retained buffer after observing it.

The current saved scalar-destructor envelope was 70 bytes but did not own
instructions B66B96, B66B99 and B66BBD. Supported reconstruction of the same
function restores their ownership while preserving its name, prior comments and
labels. Live/disk bytes and every instruction owner are rechecked. No shared
callee no-return flag changes are made. The repair and annotation receipts are
linked by `reports/native_lua_fundamentals_raw_lifetime_ay.json`.

This supplies raw lifetime support, not a production VFS manager. Application
composition must retain the actual publications, strings and VFS services through
the drain, and bind the fundamentals deletion cell before registration. The
semantic fixture binding remains available. Original exception/CRT ABI,
malformed storage, game startup reachability and gameplay are unvalidated.
