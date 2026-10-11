# Locale exit tail operands: primary review

Accepted immutable worker `d96689894f804250fe8eedf06cf138e655579353`. Root copied and physically pinned 36 helper bodies, verified both read-only single-archive replays, inspected fourteen fresh typed captures and seven identical raw pairs, and verified actual frozen/final Git membership and both outputs. All 4,217 current Source identities match the frozen Source epoch. Earlier raw CRLF variants, inherited captures and artifact evidence remain separate; no new C++ or runtime result is claimed.

The exact seven bytes at BF7000..BF7006 are `1B C0 F7 D8 59 48 C3`: SBB EAX,EAX; NEG EAX; POP ECX; DEC EAX; RET. Independent manual and Capstone decoding agrees with all five context-zero texts and the accepted unit/flow metadata. Native identity/count closes unchanged at 98/64730.

For unknown incoming carry bit C and ESP S, conditional instruction-local execution gives EAX=C-1 modulo 2^32, ECX=the unqueried DWORD at S, a near return through the unqueried DWORD at S+4, and final ESP=S+8 if both reads and the transfer complete. Final CF=C, OF=0, AF=SF=1-C, ZF=C, PF=1. Only if the preceding NEG of unknown R completes does C=(R!=0), giving zero for nonzero R and FFFFFFFF for zero R. Actual reach, stack words, return status, callback ownership and CRT behavior remain unknown.

BF6FB9 is a candidate for a separate direct entry metadata and prototype packet, selected from the preceding physical CALL operand. This acceptance does not admit that callee body or establish its behavior from a saved library name. The Application owner, retirement/failure policy, ABI and startup/gameplay goal remain unresolved.
