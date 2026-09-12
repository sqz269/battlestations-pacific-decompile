# Prompt navigation focus helpers

Addresses: `00530670`, `00530C20`.

The existing `FrontEndPromptScreen` and `FrontEndPromptHost` now implement the two
normal native bodies. `530670` selects ordinal zero only when the same screen's
input-mode word at `+264` is zero. `530C20` tests that word once, reads the current
navigation Listbox count, subtracts one with DWORD wrapping, then resolves the
current navigation widget again for selection. Empty count therefore passes -1.
Neither helper writes the distinct requested-focus word at `+284`.

Assembly, callee bodies and direct caller sites establish ECX screen, no stack
arguments and a bare RET. The count getter `A9AC50` returns the Listbox list header
at `+FC`; count is its `+8`, the same Listbox `+104`. Host operations must resolve
the navigation widget at the same screen's `+24` each time and use its actual
Listbox companion. There is no separate navigation state or silent fallback.

Evidence: `reports/frontend_prompt_navigation.json`; implementation is in the
existing `src/frontend_prompts.cpp`. Validation of this batch is recorded in
`reports/orch5_menu_listbox_group_batch.json`. Names are hypotheses. These are
semantic C++ interfaces, not native callback ABI or a game-validated prompt UI.
