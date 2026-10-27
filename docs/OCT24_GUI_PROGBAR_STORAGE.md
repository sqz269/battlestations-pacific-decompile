# Progbar raw lifetime boundary

This audit does not add a production raw Progbar factory, identity or scalar
adapter. The original allocation and member contracts alias native allocator
metadata. A normal flags1 lifetime cannot be admitted honestly without a further
genuine native custody/return contract. The original source bodies remain
available; their reconstruction/build status is not proof of such admission.

## Exact allocation and alias

AA14A0 receives its optional source in ECX and returns the object or null in EAX,
RET. It loads ECX=140h at AA14B9 and calls AB2B60 at AA14BE. That allocator stub
discards the requested size: MOV ECX,F8BDA0; JMP AB2930. This is the same actual
Icon pool with138h payload,13Ch stride and64 slots per slab.

AB1340 initializes this pool's slab trailers. AB1354 takes slab+138; AB136A
stores the actual slab index there; AB1372 advances by13Ch,64 times. AB2150
later reads object+138 at AB2167 and indexes the pool+28 slab table with that
DWORD at AB2170. This is an actual index, not an optional cache.

AC77C0 calls the SAME Icon AB5C60 constructor, reloads live D7A24C, stamps
D5CC18, stores float one into object+138 at AC77D6, and writes type60=5. The
native DefaultValue+13C is left unwritten by this constructor. AC7840 instead
calls full Icon AB5D30, stamps D5CC18 and copies source+138 through x87
FLD/FSTP to destination+138. It does not preserve the allocator trailer.

The reader AC7A90 first calls the full Icon reader AB3310. It passes the actual
address object+13C to reader current0C for `DefaultValue`, tagFloat2, default
live D7A24C. AC7AE6..7AE9 then copies that CURRENT field through x87 into+138.
The+13C address is exactly the next13Ch slot's first DWORD. For slot63 it is
slab+4F00, the beginning of the slab's free-index stack. Thus both stores reach
shared native pool bytes, not private Progbar-only fields.

## Destruction and selected calls

AC7830 stamps D5CC18 and tail-jumps to the SAME Icon AB5E70. AC7900 likewise
stamps D5CC18 and calls AB5E70, then tests flags bit0 at AC790E. Its flags1 path
calls AB24A0, the SAME F8BDA0 return thunk to AB2150. It has no alternate return
index or destructor-side restoration of+138 in these bodies. Neither the scalar
nor the inherited base destruction supplies a Progbar-specific count gate.

With the default value still present and no later external callback restoring
that word, AB2150 consumes3F800000 as its slab index. This is not the original
slot's genuine allocator index. The read/overwrite/index relationship is exact
source/listing evidence; a crash or gameplay consequence was not observed here.
The packet does not infer that every original page reaches that flags1 path.

AC78E0's loaded hook first calls AA7170 (current60(0), raw bounds), then freshly
selects current88 with arguments0,0,1. Live D5CC18+88 is AB1710, the Icon state
selection body, whose reached current80/GPU providers would still be mandatory.
The accessor AC7820 reads the same float+138. These methods cannot make a
matching normal pool return valid by themselves.

Flags0 skips physical pool return natively. It is not the ordinary already-zero
BD30E0 -> deleting04(1) route and does not establish a canonical reusable pool
lifetime. A metadata admission that merely promises flags0 would leave that
mandatory flags1 route unsupported. No such production reference is added.

## Required evidence before production admission

A future provider would need a source-backed native route explaining complete
custody of the aliased trailer, adjacent slot/free-stack bytes and actual return
index through every reached callback and destruction phase. A private saved
trailer/index, rewritten value, larger slot, alternate pool, spill object or
logical Progbar projection would change the original contract. None is used.
The existing Icon pool configuration and all138/13C/140 constants are unchanged.

This packet therefore closes the precise blocker audit rather than constructing
an object with a fabricated lifetime. It does not activate Progbar, a Page,
manager or retained type factory. Evidence was checked against the configured
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, original assembly at
AA14A0/AB2B60/AB1340/AB2150/AC77C0/AC7840/AC7830/AC7900/AC78E0/AC7A90 and
the live D5CC18 current88 word. No Ghidra mutation, game or new test was run.
