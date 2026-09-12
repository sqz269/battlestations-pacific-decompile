# Canonical GUI manager frame

Addresses: 00AA4F80, 00AA0F70, 00AA7EF0.

`update_gui_manager_00aa4f80` publishes the raw manager70 byte before callbacks,
calls the required AA3910 pointer provider only for zero, resets highlights,
copies the registered actual page pointers, and dispatches each visible or live
Screen through the existing frame runtime. It clears70 before destroying the
snapshot on normal completion. Callback exceptions retain completed effects and
the current raw70 value. Registry insertions after the snapshot do not join it.

AA5D70 leaves70 unwritten. `GuiResourceState::blocked_70` therefore starts
disengaged and is produced by this frame sequence. Listbox A9D030 reads this
same field and rejects an unproduced value when its gates reach that read.

AA0F70 reads live D7A260 with MOVSS before testing74, passes negative/negative/
positive-zero to AA8240, reloads74 for current34(false), reloads the constant
after that callback, and repeats with fresh78. AA7EF0 captures signed8C first:
nonpositive counts return false without reading88 or allocating a companion.
Positive arrays use the same canonical timed allocation as AA87B0.

Native AA4F80 is ECX manager plus two stack DWORDs, RET8; AA0F70 and AA7EF0
take ECX and RET, with the latter returning AL. Descriptive names are hypotheses.
Pointer identity, callback ordering and floating spills are reconstructed in a
new C++ interface. Native allocator/debug iterator/SEH ABI is not reproduced.
AA3910 still requires actual input, cursor, hit-test and global providers.

Validation: combined MSVC Win32 Release build and both existing CTests passed.
Independent assembly review found and corrected the empty timed-list allocation
mismatch. Focused fixture results and exact promoted commit are recorded in
`reports/orch5_menu_frame_storage_batch.json`; no gameplay/render claim.

Follow-up packets: bind AA3910 to its canonical typed input/cursor providers;
exercise actual registered Screen traversal with live resource ownership.


## Correction from docs/ORCH5_MENU_INPUT_BATCH.md

AA3910 now has a complete normal caller in GuiPointerRuntime, sharing the
frame runtime's actual input publication and manager pointer fields. Native
AA2F10/AA8BD0 hit testing and actual enabled cursor execution remain separate.
See GUI_INPUT_POINTER_RUNTIME.md and ORCH5_MENU_INPUT_BATCH.md.
