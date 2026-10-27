# Raw native Line lifetime prerequisite

`native_gui_line_storage` supplies the real Line allocation, storage, constructor,
Points append/reader and terminal path. `native_gui_line_identity` registers one
reference over that same allocation's existing atomic `+04`. Neither creates a
page, `GuiWidgetOwner`, GUI manager or separate Model. This is a prerequisite for
an authoritative page/type producer, not native Line page delivery.

## Source evidence and ABI

Evidence is from the existing `bsp.gpr`, `/battlestationspacific.exe`, function
count 77629, and the saved listings/exports for the addresses below. Current
interfaces are explicit C++ boundaries, not drop-in x86 entry points.

| Original entry | Reached behavior |
| --- | --- |
| `00AA1530` | ECX optional source, EAX actual slot or null, RET. `00AC2380` takes a slot from the existing Line process pool; null source calls `00AC2D00`, nonnull calls `00AC2D60`. Constructor failure returns its captured failed slot after constructor cleanup. The two callable arms require only their reached inputs. |
| `00AC2D00` | ECX actual, EAX same, RET. Real `00AA9390(type4)`, then `D5C9E8`, `F0/F4/F8/FC=0`, `100=4`. **EC is unwritten.** |
| `00AC2D60` | ECX destination, stack source, EAX same, RET4. Real `00AA9520`, stamp Line, vector copy at EC. **EC, FC and 100 are unwritten by the derived copy constructor.** Vector failure runs its own cleanup, then base `00AA9730`. |
| `00AC28D0` | ECX actual, RET. Stamp Line, free current F0 with the existing CRT binding, zero F0/F4/F8, then real `00AA9730`. Preserve EC/FC/100. |
| `00AC2CA0` | ECX actual, stack flags, EAX saved identity, RET4. Same full destructor; inspect current flags low byte afterward; return the same Line pool slot iff bit0. Explicit scalar has no count decrement/test. |
| `00AC2920` | ECX actual, stack type, AL boolean, RET4. Compare current lineage `F8BF34/38/3C` in order. Upper EAX bits are not represented by the new bool interface. |
| `00AC2DC0` | ECX actual, two stack float DWORDs, RET8. Forward their address to vector at EC and `004222C0`. New pointer interface forwards the same pair without arithmetic. |
| `00AC2DE0` | ECX actual, stack reader, RET4. Base `00AAA710`, then optional `Points`, ascending integer keys from1 until missing, tag6 Vec2, two MOVSS stack spills, append, close. Full reader reuses the existing body; derived-only entry requires completed base properties. |

The Line pool traits already bind actual process `F8BEFC`, pool vtable
`D5C9D8`, stride108h, 32 slots and allocator `AC2380`. Its explicit process
startup must precede allocation. The object is104h; the pool-owned DWORD104 is
separate. `NativeGuiLineTailStorage` views the same EC..103 storage: opaque proxy
EC; pointer begin/end/capacity F0/F4/F8; committed geometry FC; GPU capacity100.
No replacement points vector or counter is retained in host metadata.

## Qualified stock vector binding

`NativeGuiLineVectorCalls` can bind a genuine raw implementation.
`NativeGuiLineStockVectorCalls` is also a concrete qualified stock substitution,
with **no game or vendor-library reconstruction credit**:

* `0079EE20` zeros destination F0/F4/F8, allocates exactly source size through
  `00413030 -> 00BF681B`, copies through `00793500`, and publishes end. Its
  constructor catch `0079EEE4 -> 0041F250` frees current first and clears all
  three pointer slots before rethrow/base cleanup. Handler tables `DBBBD4` and
  `DBBBC0/DBBBA0` identify that catch; EC remains untouched.
* `004222C0` with spare capacity copies one pair through `0041C760` then advances
  end. Full capacity reaches `00421810 -> 00420B60`: snapshot the pair with
  MOVSS, enforce max `1FFFFFFF`, choose `max(size+1, cap+floor(cap/2))` (overflow
  growth resets to0 first), allocate, copy prefix through `00419930`, fill one
  through `0041E2D0/0041C760`, free the old backing, publish F8 then F4 then F0.
* These reached routines neither read nor write EC. `00419930`, `00793500` and
  `0041C760` use **FLD/FSTP per float**, preserved with MSVC Win32 inline assembly;
  x87 state, exceptions and NaN quieting are not replaced by memcpy/SSE copies.
* Existing `singleton_lifetime_allocate` supplies the BF681B malloc/new-handler
  behavior; existing `singleton_lifetime_free` supplies matching CRT free. No
  second heap or guessed allocator/proxy is introduced.

The stock provider requires a valid actual vector header/backing from that same
allocator. Its admission rejects malformed ranges and distinct-copy violations
with host exceptions; it does not claim original invalid-iterator handler repair
semantics, foreign allocator compatibility, or structured-fault recovery. Native
allocation and x87 operation ordering are retained for admitted inputs. The two
Points readers require a real table visitor; the full reader additionally needs
the existing actual `GuiReaderHost` base/push transport.

## Identity, Model and page terminal contract

`NativeGuiLineReference` borrows the already-constructed positive actual `+04`
atomic, verifies the current `D5C9E8` zero/scalar slots, and registers that exact
reference in the renderer's actual-owner registry. It never stores a synthetic
count or adds a native retain. Registration failure leaves the completed raw
allocation with its caller; terminal cleanup must still use its real lifetime
context. Once registered, zero and explicit scalar retirement pass through this
reference. Zero follows `00BD30E0 -> current04(flags1)`. Terminal processing
unbinds the opaque raw key before any reuse; it performs no post-pool raw read.
Entered terminal failure retires the companion but preserves the actual slot and
failed provider frames for diagnosis; do not retry or reuse them. Destruction of
live companion metadata terminates instead of silently destroying native data.

`bind_same_model_00aa6720(owner)` checks the canonical type4 runtime owner and
same live Model, node runtime and actual-owner registry, then calls the existing
raw `00AA6720`. The future producer must invoke it at the **same original Model
binding point before parenting/current74**. It does not create, retain or clone
a Model. Raw base4C otherwise remains the constructor's null value.

The GPU sibling `GameNativeGuiLineApplication` consumes this reference, raw
identity, same `GuiWidgetOwner/layout`, same Model4C, and a real authority token.
The future page owner must already own that exact layout under
`GuiPageLifetimeAuthority`. After genuine scene/Model/queue retirement, release
the GPU material borrow guard (canonical raw count1, node/model null, completed
operations), release its adapter metadata, require zero page aliases, then take
the same sole page allocation for explicit terminal destruction. Keep raw
identity/profile/lifetime providers alive through the typed destructor and scalar
pool return. A token adds no native ref and does not make raw scalar safe while
native material references remain.

Current `GuiWidgetOwnerRuntime` still rejects Line child construction. No new
type factory or raw-base/logical-layout synchronization is invented here. The
remaining producer must establish the authoritative raw/base/layout association
and typed terminal dispatch; current legacy pages cannot opt in without a real
sole-owner transfer. This packet leaves the GUI facade, node application, Icon
adapter and frontend registry unchanged.

## Validation

Win32 Release compilation and the repository's existing checks pass through
`scripts/build.ps1`, with temporary local CMake source registration restored
before committing. No new test suite, game run, native differential proof or
page activation is claimed. Existing CTest checks pass 2/2 (11.89 seconds); they do not constitute
native differential execution of these new entry points.
