# Native Page constructor property tail

`continue_native_gui_page_properties_00ac6806` supplies the previously absent
normal `[00AC6806,00AC6878)` source tail. It composes with the existing raw
Page prefix, script stage and reader stage on their same actual allocation.
This is an inactive source prerequisite. It neither admits a Page application
nor constructs the GUI manager, and it is not Step 4 runtime acceptance.

The original ABI is `__thiscall`, ECX actual124h Page, stack native8B name,
actual scene node, low-byte flag; `RET 0Ch`, EAX same Page. This source tail
uses an explicit C++ interface. It does not replace that binary ABI or its
private FH3/SEH and Lua nonlocal transfer machinery.

## Recovered sequence

Saved analysis in `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` was
verified through the live program bridge before examining the raw table.
The existing 162-instruction export establishes these boundaries:

| Site | Reached operation |
|---|---|
| AC6806 / AC6812 | Read current Page profile and current+18, call with same reader |
| AC6814 / AC681F | Read current reader profile and current+08 after properties |
| AC6821 / AC6825 | Reload current Page+4C and propagate root with captured EBX=0 |
| AC682A..AC6847 | CE44FC reader profile, +4 vector tidy, CE374C retired profile |
| AC684F / AC6857 | EH state1, close same Lua owner through B669A0 |
| AC6864 / AC6876 | Return same Page, RET0C |

EBX becomes zero at AC6636 and is preserved across reached calls. The live
CE44FC first three DWORDs are `00441A70 00BD8E20 00BD7A20`. BD7A20 adjusts the
reader by four then jumps to BD7130. The implementation reuses the existing
actual `NativeLuaReaderStorage::pop_back_00bd7130` and destructor. Its destructor
performs the same profile/vector/profile sequence; diagnostic EH state is
sideband and does not install an original native unwind frame.

## Borrow and retirement contract

The reader stage now returns its own existing Lua service binding with the
borrowed reader and interpreter. The tail activates that exact domain across
all callbacks and cleanup. It creates no reader, Lua state, property tree,
scene, model, reference count, manager or page registry entry.

Current table resolution is explicit borrowed metadata. Page+18 requires a
genuine full reached implementation on the raw Page and same reader; no stub
visitor or typed `GuiLayoutPage` receiver is supplied. The reader leave admits
only the genuine current CE44FC/BD7A20 domain. Root propagation uses the existing
raw B6D890 and required native scene-child dispatch. Unknown domains fail.

The tail guard enters before the reader-stage raw borrow. All entered failures
remain sticky and retain every partial property, node, reader and Lua effect.
Its destructor rejects interruption before touching raw metadata; the host must
retain all dependencies and exit before ordinary drain. No rollback, replay,
forced close or destructor-time cleanup is introduced.

On normal completion the prefix gains a `constructed` phase. Script and reader
stage disposal still requires the actual retired reader stamp, closed owner
and quiescent borrows. These observations do not authenticate caller-provided
property dispatch, native-private cleanup or the history of a raw memory cell.

## Remaining Step 4 requirements

This tail needs the genuine raw Page property implementation and all reached
child factories/lifetimes. Screen acquisition still calls the GUI manager at
AC59C5, AC5B32 and conditionally AC5EE6. The GUI lane must borrow cc15's warm
actual88h manager and same native string/pool/scene domains, without entering
the getter's slow construction path. One authoritative Page/tree retirement,
all18 class admission and ON mission proof remain unaccepted. Input/button
dispatch is outside this task.

Build and default-OFF validation are recorded in `OCT24_GUI_RENDER.md` when this
source prerequisite batch is integrated. No native Page game proof is claimed.
