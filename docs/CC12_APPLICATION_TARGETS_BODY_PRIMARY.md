# Application unwind target bodies: primary acceptance

Root accepts the exact seven-byte profile reset and twenty-five-byte guard
cleanup bodies. All 32 bytes passed exact ownership/boundary/per-byte gates
and matching before/after typed records at epoch95/count64730. Ten decoded
instructions and stored/default/effective flows agree; no repair was made.

Reset writes CE3818 at the ECX receiver, then plain RET. Guard captures +4,
tests it, writes CE37FC on both paths, and branches to RET when captured is
null. Otherwise ADD [captured+18h],-1 precedes PUSH captured and real indirect
CALL [CE2210], then RET. ADD updates arithmetic flags and wraps the DWORD.
The saved zero call count is inconsistent with the opcode and typed COMPUTED_CALL.

The intended nonnull return requires the external callee to remove its one
four-byte argument. No IAT contents or external code/behavior was read. The
body has no local argument cleanup or rollback. No direct guard+4 write is
present; aliases or imported effects are outside any preservation claim.

Existing explicit-pointer Source providers match the directly observed memory
order and stored unsigned decrement on valid storage. They do not preserve the
original ECX/flags/stack ABI. Conditional action-frame receiver/guard arguments
remain compatible only if the previously unproved FH3 binding is supplied.

Root copied726 finite pins, passed guarded replay, checked8492 actual current/
prior Git occurrences and4210 closed Source paths, and verified all619 prior
metadata pins unchanged. Only the separately accepted two-file GUI helper delta
differs in current Root Source; this packet adds no renderer build credit.

The next packet is Source-only exceptional composition readiness, with zero
new Native bytes or C++ implementation. Actual FH3 activation, IAT binding,
callable owner, current publication, escaped custody and retirement remain open.
No C++ change, build/test or game run is added by this body contract.

Evidence: reports/cc12_application_targets_body_primary.json.

Root appended two locked comments and name-ledger evidence, retained existing
names and old comments, saved GPR95 to98 and refreshed six export files.
Both complete typed records remain exact except modification numbers; bodies,
listing, flows, prototypes, Source types and no-return flags are preserved.
Existing Source function records remain byte-identical, preserving their prior
build/fixture coverage without transferring it to Application frame activation.
