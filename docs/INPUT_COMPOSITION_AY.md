# Input and Lua lifetime composition (AY)

`GameInputSettingsRuntime` owns stable script, keyboard-storage, table and
lifetime contexts while borrowing the actual E198E8/01090AA0 cells and supplied
Lua, string, constant and stack-policy inputs. Construction performs no native
registration. Its `get()` invokes the complete 005547D0 getter; callers bind
deletion before the first call and retain the runtime through manager drain.

`GameSingletonHost` now owns the E198E8 cell, exposes borrowed references to it
and its existing raw 01090AA0 manager, and binds the settings context to the
existing CF81CC destructor route. `GameInputActions` exposes its existing binding,
configuration and tick contexts so the full configuration loader can share them.
The accessors create no second action owner or callback state.

The fundamentals getter can now use that same raw lifetime manager through the
existing lifetime-access adapter. D62C18 dispatch deletes the registered raw cache
and clears the same actual 0108FF1C publication even when its value changed.
See `NATIVE_LUA_FUNDAMENTALS_RAW_LIFETIME_AY.md` for native order and coverage.

The full settings-table/lifetime fixture passes using the retained runtime:
122,545 lifetime, 309,711 tree-range, 168,378 insertion, 1,448 vector-storage and
335 checked-string values match the copied native paths, including registration,
fast getters, populated constructor unwind and actual manager drain. The raw
fundamentals extension and retained semantic-domain comparisons pass separately.
The integration report pins the final combined source, build and fixture hashes.

These changes prepare source composition. `GameStartupHost` does not yet call
the raw settings getter: it still needs actual VFS ownership and complete native
Lua callback services. Full configuration additionally needs the actual game+3C
owner, defaults/keyboard contexts and real timing/deadline callbacks. No projected
VFS/settings object is cast into these raw interfaces; no game or device calls
are made by the validation fixtures. Original ABI and gameplay remain unproven.

The isolated source host fixture additionally binds deletion before the getter,
registers one CF81CC settings owner and drains it through `GameSingletonHost`.
Shutdown clears both settings and manager publications; all 2,763 pooled string
allocations are released. It links current singleton/observer objects and exactly
checked GameHostLog methods. The VFS audit in `NATIVE_VFS_HOST_GRAPH_AZ.md` records
the remaining actual owners, readable table data and missing MPAK libraries.
