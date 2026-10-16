# GOAL16C: state-definition publication reader

`00B400F0..00B400F5` is the whole six-byte `MOV EAX,[0108FE90]; RET`.
It has no consumed register or stack inputs, reads the current pointer bits
without dereferencing, and returns them in EAX with no argument cleanup.
The installed PE and fresh target-verified Ghidra bytes agree. The image cell
is four loader-zero writable data bytes, not an observed live game value.

## Existing production ownership

The production renderer already holds a volatile
`NativeShaderStateDefinitionsStorage* definitions` at offset72. Its real
constructor/destructor contexts and descriptor contexts borrow that same
publication. The existing Source base constructor publishes its actual owner
before manager registration; the destructor unregisters the current publication
and clears it. These existing provider implementations and their own Native
ABI/EH qualifications are unchanged.

The change moves the live pointer value to canonical zero-initialized static
Source storage and replaces the old four-byte member slot with a reference to
that storage. The existing production routes still receive that same reference.
There is no duplicate production value, substitute registry, new lifecycle
implementation or getter call to a lazily allocating owner. A new Source
cell-address interface binds the real owner; it is not another recovered
Native function. Generic lower-level contexts retain their explicit borrowed
publication domains.

Before/after full class-layout compiles use the actual registered application
command and compare every member row. Renderer `Impl` remains9952 bytes and
all57 offsets agree, including the publication at72. The exact three Source
preimages, full logs/commands/objects and `class_layout_proof.json` are retained.
Binding the reference itself does not initialize/reset the cell; later real
Native-style lifecycle operations retain their existing publication semantics.

## Complete code, storage and actual bindings

Source snapshot `1e7f0a3d2` passes the registered Win32 build and all three
existing checks. The strict C++17 `/MD /O2 /Gy /W4 /WX /fp:strict` object and
actual registered object each contain all six whole leaf functions in the
extended global-reader module: three readers and three new Source cell-address
interfaces, totaling36 executable bytes. The new reader/accessor pair contributes
12 bytes; only the reader's six Original bytes count toward reconstruction.

All twelve mutable data bytes are accounted: four initialized bytes for the
existing register limit and eight `.bss` bytes for the two actual publication
cells. Actual symbol offsets distinguish all three nonoverlapping cells. All
six DIR32 operands bind to their specific genuine Source storage; there are no
undefined providers, extra executable sections or dynamic initializer/guard
sections. The complete registered object equals its unique actual archive
member. All six complete fixture contributions and their real cell operands
are checked. The real registered renderer constructor contains a call to the
new cell accessor, and the registered production game executable links.

One focused fixture links the real registered archive. Before production owners
exist, it checks the new null read, publication of actual existing 28h Source
storage, stable cell address, and clear. It also retains the earlier register
limit/registry initial-value, mutation and clear checks. This fixture does not
construct the production shader registry or renderer, invoke their lifetime
providers, or execute captured Native code. No new registered test is added.

Evidence is under `local/goal16c_shader_definitions_reader/`, including frozen
actual archive/registered objects/game executable, source preimages, all-layout
logs, strict code, fixture/map, build output, image/live bytes and `audit.json`.
The report records hashes and independent review when closed.

## Limits

This is one complete Source reader under canonical production Source storage
binding. Names and raw pointer interpretations remain provisional. Original
global placement, Native address-fault identity, binary adoption, full owner
lifecycle execution, downstream callers and game validation are not proved.
Normal register/flag effects follow the whole MOV/RET body; the absolute Source
data address differs from Original and is explicitly bound, not arbitrarily
masked. The full inclusive qualification/review/integration time is unmeasured;
no throughput improvement is claimed from this single small closure.
