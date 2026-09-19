# Re-runnable x87/ESP trace of the dive-bomb FLY-OVER tick 009C62B0-009C7083.
#   .\tools\flyabove_trace.ps1 <from> <to>   -> listing window
#   .\tools\flyabove_trace.ps1 all           -> whole body to local/output/fa_trace.txt
# Callee effects are "<x87 net>:<RET imm>", each read from the callee's own tail
# (local/calleefx.py) rather than assumed.
param([string]$From = "009C62B0", [string]$To = "009C6300")
$args0 = @(
  "tools/x87trace.py", "trace", "009C62B0", "009C7083",
  "--basefb", "0x98",          # SUB ESP,88h + PUSH EBX/EBP/ESI/EDI: 0x98 = retaddr, 0x9C = arg1
  "--call", "00419010:1:20",   # BSP_Math_InterpolateClamped: RET 14h, float in ST0
  "--call", "00438aa0:1:8",    # BSP_Math_AddWrappedAngle:    RET 8,   float in ST0
  "--call", "00438b10:1:8",    # BSP_Math_SubtractWrappedAngle: RET 8, float in ST0
  "--call", "00414db0:0:0",    # EntityPose_RefreshWorld: void, __thiscall
  "--call", "0042e740:0:0",    # tuning singleton -> EAX
  "--call", "007c4810:1:0",    # __thiscall(esi) -> float in ST0 (009c4826 FSTP/009c4829 FLD [esp])
  "--call", "0099b630:1:0",    # __thiscall(ecx) -> float in ST0 (both rets FLD then RET 0)
  "--call", "009fa2e0:0:4",    # out-pointer getter, RET 4, writes [eax+4]/[eax+8], no x87
  "--call", "009fabe0:0:8",    # __thiscall(esi), RET 8, writes [esi+4]/[esi+8], no x87
  "--call", "009fb800:0:8",    # BSP_PilotBot_CommandPitchFromAltitude: RET 8 (009fb94b/96b/ba4d), void
  "--call", "007f0280:0:24",   # BSP_Bot_NearFieldUnitAvoidanceProbe: RET 18h, void
  "--call", "00bf701a:-1:0",   # atan2(ST1,ST0) -> ST0: net -1
  "--call", "00bf7030:0:0",    # sqrt(ST0) -> ST0: net 0
  "--icall", "009c62cf:0:4",
  "--icall", "009c6342:0:4",
  "--icall", "009c6404:1:0",   # vtable[50h] on [edi]->[+4], NO push: __thiscall -> float in ST0
  "--icall", "009c647b:0:4",
  "--icall", "009c64ec:0:4",
  "--icall", "009c6705:0:4"
)
if ($From -eq "all") {
  python @args0 --from 009C62B0 --to 009C7083 > local/output/fa_trace.txt 2>&1
  "wrote local/output/fa_trace.txt"
} else {
  python @args0 --from $From --to $To
}
