# Actual raw C-string assignment

The four-argument raw-context0041E350 overload covers all32 normal Native
instructions through0041E392 RET4. Original ECX=destination8, nullable source
stack argument and EAX=SAME destination are normalized to ordinary Source.
The existing storage overload, typed xlive path and all providers stay unchanged.

Capture original source/destination identities without clearing the header.
NULL source requests0 without reading bytes; otherwise scan unsigned BYTEs
through NUL using wrapping DWORD subtraction. Genuine raw41DD40 receives the
SAME AA8/AA4/AA0 context, requested length and preserve=false. After return,
capture CURRENT destination DATA FIRST. Null DATA skips LENGTH and copy.
Nonnull DATA captures CURRENT destination LENGTH and ALWAYS invokes genuine
BF7680, even count0/NULL source, using ORIGINAL source and actual0109EEA4 ref.
No terminator copy, measured-count override, post-resize source guard, forced
empty bytes, mode preload, host memcpy/memmove or semantic adapter is added.

Header reads use fenced byte copies; source BYTE reads are volatile/fenced.
Scan-before-resize and real copy overlap/current-count behavior are retained.
Caller proves reached raw/copy extents, canonical cell/feature/pool/code graph,
DF/CPU/private-frame restrictions and complete lifetime. BF7680 alone decides
feature-read gating. A changed header is not repaired or clamped.

No local EH/Frame, catch, destructor, cleanup, retry or rollback exists. Retain
actual header/backing/provider effects and partial debt on escape. Normal
return proves only the real operation returned. Native ABI/private-stack/FH3/
SEH/hardware/current-provider behavior stays qualified; mission-machine
holder/result/error caller, VM/table/loading/startup/runtime and Native/vendor
credit remain separate. Validation is one separately recorded strict object;
no test/probe/entry/game/Ghidra mutation belongs to this packet.
