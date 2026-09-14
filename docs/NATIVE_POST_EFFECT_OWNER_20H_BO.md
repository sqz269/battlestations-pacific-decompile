# 32-byte post-effect owner lifetime (BO)

`00B4E1F0..00B4E2AC` and `00B4E430..00B4E44D` reconstruct the actual **20h/32-byte D61EC0** owner destructor and deleting destructor (219 native bytes total). This is the B4E470 family, distinct from the 24h/36-byte D61EC8 owner. Names are descriptive hypotheses; the C++ interfaces are new source contracts.

The destructor stamps D61EC0, captures material+14 before capturing the current CE2220 decrement function, then releases material, unlinks nodes+0C/+10, frees the raw40-byte draw record+18, and releases frame+08. It uses the same captured decrement for material and frame. Null branches leave fields untouched; nonnull fields clear only after their captured release returns, overwriting any callback write to that field. Later fields and zero-reference vtables are current reads. There is no logical-stream member in this allocation.

Material final zero requires the real D5E520/BD30E0/B194B0 profile and its canonical NativeMaterialReference borrowing the exact actual+04 count. Node unlink goes through the actual node lifetime runtime and B6DFA0. Frame final zero uses D5E600/B1FCF0 and its concrete context. The raw+18 draw record is freed through CRT, without reference operations.

The outer canonical reference binds transactionally in the same geometry registry, without retaining or resetting the actual counter. Final zero checks the current D61EC0 profile, invokes real B4E430(flags1), then unbinds by captured identity before notifying host disposal. Unbind must preserve both companions until that final notification. Host storage and registry metadata lifetime remain explicit; registry binding can allocate.

## Saved analysis and exception boundary

Caller flow repairs restore B4E269..B4E26E (ADD ESP4 and clear+18) and B4E445..B4E447 (ADD ESP4), without changing CRT callee no-return flags. Raw10-byte handler CBFAF8 now has a saved function definition, with no reconstruction credit. It selects DF8684, maxState1, mapDF867C; state0 dispatches CBFAF0 to BD30F0. Original comments/names will be preserved during naming integration, and exports are force-refreshed after repairs.

The C++ exception projection restores only the base and marks the host owner dead; it does not release later members, retry a failed callback, or free the outer allocation. Direct destruction while its canonical reference is bound is outside the contract. The canonical terminal is noexcept, so native FH3/SEH equivalence is not established.

## Validation and remaining producer work

The report pins seven live/original-PE spans, the complete189/30-byte bodies, provider sources and every direct call. The combined Win32 build and both existing math CTests passed at `afee0acdebab4e3b8152159b83e13de51fb8068c`; [batch validation](NATIVE_POST_EFFECT_BN_BO_INTEGRATION.md) records the exact evidence. No new native fixture or gameplay claim is made.

B4E470 must begin the actual atomic counter lifetime at its native count-one store when reconstructed. This terminal packet neither manufactures that constructor nor admits arbitrary failed initialization. In particular, service+70 is published after +1C4 becomes one. See [producer map](NATIVE_RENDER_RESOURCE_MEMBER_PRODUCERS_BM.md) and [source evidence](../reports/native_post_effect_owner_20h_bo.json).
