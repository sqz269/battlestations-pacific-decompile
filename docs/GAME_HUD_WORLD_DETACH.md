# HUD world binding cleanup

Addresses: none (source owner composition).

`GameHudHost::detach_world_2k()` clears the canonical unit pointer and pending/applied unit-interface bookkeeping, then destroys the marker and minimap binding owners. Those child owners borrow the unit/Lua objects; their default destructors release source caches without invoking the borrowed owners. The frontend HUD manager and its registered screens remain owned by the menu.

This supports the continuing unit observer lifetime binding before mission frame replacement or unit destruction. `GameMenuHost::Impl` declares `hud` before `mission`, so reverse member destruction destroys the mission/frame before the HUD. `GameStartupHost` deletes the menu before draining the raw singleton manager. Thus frame cleanup can call this API while both HUD and observer runtime remain alive.

This method is source lifetime cleanup, not a recovered native HUD destructor or native scene transition. The current S build checks it; its actual call paths and repeated-load behavior belong to the continuing T unit binding.
