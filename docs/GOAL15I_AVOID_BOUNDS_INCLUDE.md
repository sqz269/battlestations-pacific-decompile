# Raw bounds inclusion and actual argument-slot reuse

The complete127-byte entry00415010..0041508E now preserves Original ECX bounds, stacked point, RET4 and every x87/MOVSS operation. It captures the point pointer in EDX, then overwrites its ACTUAL incoming point word for each float spill. Four ordered comparisons reload the source, so prior bound stores can change later point reads. Equal/unordered comparisons skip their corresponding stores. There is no point snapshot, typed object view, repair or FP reset.

The existing geometry_helpers public typed wrapper remains separate. Its private kernel uses a private PUSH slot for the temporary, rather than this Original physical argument word. An adapter to that typed interface alone would not establish the raw slot contract needed by the constructor.

Strict optimized MSVCWin32/MD/W4/WX compilation passes. All127 COFF, target-verified live and installed PE bytes agree exactly, with no masks or owned relocations. One ignored raw Source fixture verifies the concrete alias risk: maxX changes before the same cell is read as point.y, and the caller observes40000000 in the actual consumed argument word. Final archive-only validation belongs to the batch report. No new tracked test target is added.

Physical CALL0041CFB9 targets00415010 in the construction parent; CALL00423298 targets00415010 in corner-clearance scaling. Both consumer rows are live checked. This does not establish Native stack-address/object ABI, all floating environments/faults or game execution. All reached raw/control storage requires backing; production preserves ambient FP control/status/traps.
