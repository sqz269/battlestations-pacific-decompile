# Native FileStore open and stored-stream conversion

Addresses: `00BE5E90`, `00BE5FA0`, `00BEF750`, `00BB8F60`, `00BEF540`.

The actual FileStore open consumer now searches its native primary tree and
returns an actual 14h memory-stream owner. It reuses the existing 10h backing,
intrusive lifetime, allocator, tree reader and native string comparator. No
semantic FileStore/MemoryStream object, callable replacement table or owner
registry is introduced. The original numeric D642C0 and D15AD8 identities remain
in the reconstructed owners.

| Body, inclusive span | Coverage | Original ABI |
| --- | --- | --- |
| BE5E90..BE5EF5, 102 bytes | complete | ECX tree; iterator output/name stack; EAX output; RET8 |
| BE5FA0..BE6030, 145 bytes | complete | ECX FileStore; name/flags stack; EAX new stream; RET8 |
| BEF750..BEF83F, 240 bytes | complete consumer with required current stream methods | ECX source; EAX new stream; RET |
| BB8F60..BB8F87, 40 bytes | complete raw leaf; definition requested | ECX ignored; token stack; AL membership; RET4 |
| BEF540..BEF57C, 61 bytes | complete | ECX memory stream; low/high/origin stack; RET0C |

Names are descriptive hypotheses. The seek entry retains its native Win32
machine interface using a reserved fastcall EDX parameter. The other new C++
interfaces add explicit context and do not claim binary ABI compatibility,
native FH3 exception-object identity or incidental caller register equivalence.

## Producer evidence and actual lookup

BE7FA0 establishes FileStore identity D689E8, primary tree at provider+14,
head at provider+18 and an empty self-linked sentinel with byte+19 set.
BE6590 allocates a 1Ch node, writes left/parent/right at +0/+4/+8, then calls
BE6170 with node+C. BE6170 constructs the eight-byte string header and stores
and retains the supplied stream at its own +8, establishing node+14. The node
producer writes color+18 and zero sentinel byte+19. These existing layouts
are consumed directly; this packet does not implement tree population.

BE5E90 calls the existing BE54D0 lower bound before its null-tree validation.
It compares the selected node with the current head, then calls 443D00 with
the supplied header and node+C. Fallback rereads the current head. Both result
words are captured before publishing output owner then node, preserving output
aliases. The four current direct callers are open BE5FA0, remove BE7130, add
BE7760 and request BE7CD0; each supplies output and name on the stack.

BE5FA0 returns null for flags bit0 before reading the name or owner. Other flag
bits have no further effect. It searches provider+14 without copying or
normalizing the name, captures the head after lookup, validates the iterator
owner and returns null for that captured sentinel. On a hit, it reads name+4
for the diagnostic whose target 4254B0 is a single RET, validates the captured
iterator owner and current owner+4 again, then converts the current node+14.
BF6713 can return through its current registered CRT handler; the existing
explicit invalid_parameter callback preserves continuation when reached.

## Current type query and conversion

NativeStoredStreamConversionContext borrows the existing memory-owner context
and three actual type DWORDs at 0109DBA0..0109DBA8. CD8FC0 copies current
0109DB58/+4 to 0109DBA4/+4 and writes an ID obtained from the current counter
to 0109DBA0. This packet does not initialize a competing type counter or IDs.
BB8F60 compares the stacked token against those three current words in order,
returning on the first match. D642C0 table+C points to BB8F60. Table+1C points
to BEF540, which ignores the high offset and wraps the low DWORD addition from
backing data for origin0, current cursor for origin1, or end for every other
origin. It adds no bounds or malformed-storage checks.

BEF750 returns null before touching any context for a null source. Otherwise it
reads current 0109DBA0 and current source table+C. A true AL result reads current
source+8 and calls existing BEF6D0: a fresh cursor-zero wrapper retains the same
backing without retaining or changing the source wrapper.

The false branch selects each next method from the source's **current** table:

| Call site | Contract |
| --- | --- |
| BEF78F | table+C: ECX source, type token stack, AL result, RET4 |
| BEF7BC | table+1C: ECX source, three zero DWORDs for low/high/origin, RET0C |
| BEF7C5 | table+30: ECX source, no stack arguments, EDX:EAX length, RET |
| BEF80B | table+24: ECX source, data/low length/null actual-count stack, RET0C |

The low length DWORD is saved across allocation and construction; high EDX is
saved but never consumed. The routine allocates raw 10h, constructs it with
signed low length through 8D43C0, and performs exactly one read into backing+8.
It neither checks the read result nor trims the stored length, restores the
source cursor, releases the source or fills a short-read remainder. BEF6D0 then
creates the returned wrapper and the temporary backing reference is decremented
atomically, dispatching its current slot0 only at zero. Nonpositive low counts
retain the existing signed allocation versus unsigned request behavior; this
is not a general validated byte-copy API.

Numeric D642C0 methods dispatch through the borrowed original immutable table
and established current leaf identities. Other input streams require actual
callable original-ABI tables, with EDX holding the selected target. This is an
explicit external-method contract; arbitrary other original numeric profile
words are not executable bindings. Physical/MPKG/MSAR owners need their own
concrete dispatch before entering this path as reconstructed numeric owners.

## Cleanup and verification boundaries

Handler CC765B selects FuncInfo E02228, maxState1, map E02220, action CC7650.
The action bytes are `8b45ec50e853eff2ff59c3`: saved raw owner, scalar free,
POP ECX, RET. State0 protects backing construction only. State -1 is written
before the read; there is no rollback for a throwing read or wrapper creation.
The C++ guard preserves that scope. At inspection, CC7659..CC765A were outside
the saved cleanup body; the primary was sent the precise two-byte repair span.
Workers made no Ghidra mutation. BB8F60 likewise required a 40-byte definition.

The report records the strict Win32 build, existing checks, guarded live/PE
byte comparisons and the ignored original-instruction fixture results. The
fixture uses five unchanged complete original bodies, instrumented external
allocation/owner/CRT services, and actual linked bsp_core implementations.
It covers lookup aliases and hit/miss flags, shared-backing/cursor behavior,
the type predicate, unchecked seek and current-table changes between every
nonmemory operation, including zero length and a partial read. It compares
the initialized prefix only; the remaining backing bytes are intentionally
uninitialized. Its service tables are fixture instrumentation, not implemented
physical/archive owners. Native FH3 unwinding and real provider I/O are not
executed. No game frame or gameplay validation is claimed by this packet.
