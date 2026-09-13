# Text native hardware layouts

`GuiTextNativeLayoutServices` connects the existing `B865A0` section rebuild to
the actual logical-stream declaration getter and renderer hardware-layout
factory. It reuses `B48CE0`, the `B2F710` tree/cache, canonical pools and
`B60CB0` constructor. Numeric profile checks select these recovered bodies.
They do not install a replacement callable vtable.

Each hardware companion borrows the native owner's `+04`. A cache hit reuses
that companion and preserves the factory's native increment. Final release
dispatches `BD30E0 -> B60770`, completes real COM/declaration/tree destruction,
returns the native pool slot, then removes the canonical host binding.
The connected `B483F0` path now reaches the existing CPU declaration companion
after its original decrement. This prevents a returned declaration slot from
leaving a stale host registration. Raw-only callers retain their existing API.

The factory's optional acquisition output records a completed constructor
before pair construction and tree insertion. Registration failures retain the
creator and any companion. No additional retain, native cleanup or rollback
is introduced. An interrupted operation must stay alive and cannot restart.

All contexts share the actual renderer publication, declaration pool, type and
profile views, string storage and geometry registry. The native fixed key and
scratch limits remain preconditions. These are new Win32 C++ interfaces;
arbitrary virtual profiles, full Text construction and game validation are
outside this packet. In particular, material creation still needs the actual
renderer `+48` effect-loading path.

See `reports/gui_text_native_layout.json` for call sites, ABI, validation and
the boundary between native effects and retained host diagnostics.
