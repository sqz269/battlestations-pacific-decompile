# Text-owner activation and reset (CC10)

`set_text_editor_active_00a966e0` projects the complete normal instruction
schedule of `00A966E0` into references to an **existing** text owner. It does
not construct a callback owner or a second platform queue. Original ABI is
ECX=owner, stack enabled byte and context DWORD, `RET 8`. The C++ binding is
not that ABI.

| Owner field | Producer / use |
| --- | --- |
| +04h | `00A972CB` initializes disabled; `00A966EC` writes the supplied enabled byte. |
| +18h/+1Ch | `00A97297..9A` zeroes the eight-byte native string; enabling resizes it to zero through `0041DD40`. The existing `NativeString` and caller-supplied storage provide this field. |
| +20h | Cursor reset to zero at `00A9673B` only on enable. Constructor `00A97260` does not initialize it. |
| +24h | `00A966EF` stores the caller's context DWORD. For the main-menu enable call this is the current selected row returned by `00425E50`. Constructor `00A97260` does not initialize it. |

The producer `00A97260` initializes the embedded owner at screen+0Ch when
`005902E0` constructs the main-menu screen; `005CFDA0` also calls it for the
multiplayer chat screen. The main-menu destructor path begins at `00590D60`
and calls `00590600`. Its later embedded-owner cleanup is not established by
this packet because the saved pseudocode truncates the destructor's cleanup
flow. The binding borrows the owner fields and must not outlive their screen.

Native order is exact: write owner+04h, write owner+24h, call platform
`00A965A0` with the same enabled byte (which always clears the existing queue),
get the current input-action singleton with `004BEC00`, update it at zero
seconds through `00A92C40`, and, only when enabled, resize the text at +18h
to zero and reset cursor+20h. The native branch reloads text data/length after
resize and has a conditional `memcpy` from the shared empty string. A valid
zero-length `NativeString` has null data, so that branch does no copy. The C++
projection retains the conditional zero-length copy. The required input service
must call the application's current `GameInputActions::update(0.0f)` or the
equivalent current `004BEC00`/`00A92C40` binding. It cannot be an empty method.
If that service or string storage throws, earlier stores and queue clearing
remain; the later text/cursor reset has not happened.

At `00583DE0`, the main-menu caller first clears a separate screen string at
+24h, updates its listbox presentation and byte+77h, calls `00425E50` on the
current listbox at screen+1B8h, then passes that selected-row result and `1`
to the owner at screen+0Ch (`00583E36`). `00590E60` passes zero, zero to the
same owner at `00590EAF`, disabling and clearing it. The other saved direct
call sites also supply zero or one. `005D1D0C` is a saved call instruction
outside a defined Ghidra function body; its containing function is not claimed.
These caller-specific listbox and screen operations are not part of `00A966E0`.

The future frontend bridge must supply the actual screen owner's
`TextInputCallbacks` instance and the other field references from that same
owner, the application's persistent `PlatformTextInput`, its
current string storage and input-action services, and the selected row returned
by the real listbox. The existing `MenuUpdateBinding` still lacks the enabled
owner callback and queued-event dispatch; activating input without that owner
would strand events. This packet supplies the activation contract, not a fake
owner or an editable UI claim.

Read-only native inspection used the saved `bsp.gpr` project and
`/battlestationspacific.exe`. `reports/text_editor_activation_cc10.json`
records call-site containment, build results, and the remaining runtime
boundary. `scripts/build.ps1` built Win32 Release `bsp_core` and `bsp_game`
and passed both existing CTests. No Ghidra writes were made.
