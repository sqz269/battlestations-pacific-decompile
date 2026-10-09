# Pending registry tick registration constructor: 00875890

The complete139-byte Native body is42 instructions and returns with RET8. It
initializes selected raw node fields, calls the pending registry getter, enters
its returned section if nonzero, appends the node to the pending intrusive list,
copies two scalar SSE words, then leaves the section and returns the receiver.
The whole live byte span matches the installed PE. This is readonly readiness;
no Source, build, fixture, ledger, Ghidra mutation or reconstruction credit is added.

## Machine inputs and exact owned schedule

Let D be incomingECX, S entryESP, payload=[S+4], group=[S+8]. IncomingEAX is
overwritten by the group load. IncomingECX is copied to savedESI, while the
payload is captured intoECX before the getter call. IncomingEDX has no owned
read and can reach that getter unchanged. EDI/ESI/EBX are saved then repurposed;
EBP is untouched. XMM0 is defined by XORPS before any owned use. There is no
owned x87 operation, localFS frame or exception handler. Child preservation
and extra-frame accesses remain external contracts.

Before the getter, stores occur in this order: DWORD D+0=D0DEC8; DWORD
D+14=group; BYTE D+18=0; BYTE D+19=0; DWORD D+20=0; DWORD D+1C=0;
DWORD D+24=0; DWORD D+28=payload. The payload itself is not dereferenced.
The sole direct child call is875280 at8758BA. There are two Win32 import calls:
EnterCriticalSection throughCE2218 at8758C7 and LeaveCriticalSection through
CE2210 at87590D; PE import metadata confirms their names. No Native descendant,
caller, handler, profile table, timer word or pending-list data body was opened.

After the getter returns, D retains its captured receiver and the current
registry result is dereferenced at+4 without a registry-null guard. The section
pointer is captured inEDI. If nonzero, real Enter runs before the actual
DWORD ADD[section+18],1 RMW. There is no locally protected cleanup scope.

The section is compared with zero again at8758D1. Its flags survive the entire
pending-link and SSE schedule to the JE at875906; those intermediate MOV,
XORPS and MOVSS operations do not define arithmetic flags. In particular the
unlock decision is not a late reread of a mutable registry/section field.

The pending-link stores are deliberately ordered: load current[E0B708] asA;
storeA atD+4; store literalE0B704 atD+8; reload current[E0B708] asB; storeD
atB+8; storeD at[E0B708]. A and B can differ under alias mutation or concurrent
writes. A Source reconstruction must retain both actual current-cell reads,
and cannot reuseA for the neighbor store. The fixed tail address is opaque
Native pointer data, not a verified Source sentinel object or callable table.

XORPS XMM0,XMM0 precedes the link stores. MOVSS[D+2C],XMM0 writes the zero
32-bit word after linking, then MOVSS loads the current word[D7A260] intoXMM0
and stores its low32 bits atD+30. These are scalar SSE transfers, with no
floating arithmetic or x87 conversion. The opaque timer word was not read or
assigned a value by this packet. Its load remains after all link stores, so an
explicit Source timer binding must be read at that point, not cached at entry.

If the captured section was nonzero, actual ADD[section+18],-1 precedes real
Leave. ReceiverESI is returned inEAX after normal completion; saved registers
are restored andRET8 removes only the payload/group stack arguments. XMM0 and
final flags are not restored; they depend on this schedule and import effects.

## Footprint, holes and failure boundaries

The minimum direct receiver address extent is34h. This is not a complete class
size claim. DWORDs+C/+10 and bytes+1A/+1B are untouched, as are all bytes beyond
the selected accesses. There is no owner allocation/free, group validation,
node null check, profile invocation or duplicate-link policy in the body.

Initial receiver fields precede the getter. A getter failure leaves those
effects. A null registry faults on its+4 load. Null section skips Enter/depth
and Leave/depth but still performs all linking and SSE stores. Nonzero section
and every selected receiver/cell/neighbor/stack access require valid backing;
no alias/disjointness checks exist. Faults or nonreturning children stop the
later effects. The owned routine supplies no cleanup or rollback on those paths.

## Current Source route and admission requirements

The admitted Source getter takes two actual volatile publication references;
it has a complete250B compiled root and real providers, with Original entryABI
and application execution unproved. The canonical retained process contains
F878CC and1090AA0 cells, but this packet adds no production caller for them.

Current address searches find metadata/constants and abstract host calls for
875890. unit_instance_layout.cpp delegates node construction at unit+310h to
UnitConstructionHost::construct_tick_node; there is no concrete override in
src/include. The fixed-step host comments identify unreached Native construction
paths; those comments are not newly executed proof that every runtime list is
empty. Plane-squadron Source is a typed state projection, not this raw node body.

A bounded Source candidate can use explicit raw receiver/payload/group inputs,
the actual two getter publication references, an actual pending-last cell and
tail object binding, and a late current timer-word binding. Preserve raw holes,
all stores/reloads and unsigned section counter operations. Do not fabricate
a global producer, assume Source numeric profiles callable, insert a boot-time
getter call, or substitute the differentF899E8 pending-entity registry.

Such an ordinary Source interface would not reproduce this OriginalECX/RET8
entryABI, residualXMM0/flags, hardware-fault timing, imported exception identity
or arbitrary frame aliases. Actual pending sentinel/last/timer producers and
the concrete unit/aircraft/objective lifecycle route remain separate work.
Application startup and gameplay validation remain pending.
