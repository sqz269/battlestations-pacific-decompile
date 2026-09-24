# Native GUI layer pool storage (CC10)

Addresses: `00AC51A0` allocation wrapper, `00AC4E50` pool acquire,
`00AC4C40` return wrapper, `00AC47F0` pool return, `00AC4070` slab
initializer, `00AC4750` allocator-list trim, `00AC4D70` pool constructor,
`00AC4670` pool destructor, and process-static pool `00F8BF50`.
The listing-only CRT initializer is `00CD73D0..00CD73E5` inclusive;
Ghidra has no function there. The PE initializer table at `00CE3498`
points to it. `00CE0AD0` is the registered teardown thunk.

`00AC51A0` consists of `MOV ECX,00F8BF50; JMP 00AC4E50`. Its caller's
`ECX=124h` size is overwritten. The wrapper takes no size argument:
`00AA3BEE` and `00AA5929` both select the same static pool. The free
wrapper pushes the raw slot, selects `00F8BF50` in ECX, calls `00AC47F0`,
then returns. `00AA3901` reaches it only when the scalar deleting
destructor requests a free. This packet does not run either page destructor.

The 38h pool storage has the existing allocator-list element at `+00`,
critical section at `+0C`, recursion word at `+24`, slab table at `+28`,
slab count at `+2C`, capacity at `+30`, and first-free index at `+34`.
`00AC4070` initializes a `2544h` slab: 32 slots of `128h`, each carrying
a hidden slab index at `+124h`; the descending free-index stack starts
at `+2500h` and the WORD free count is at `+2540h`. The 124h payload is
untouched. Return reads the live hidden slab index under the same lock and
divides the slot's slab offset by `128h`, matching the `00AC47F0` listing.

`NativeGuiLayerPool` instantiates the repository's existing fixed-pool
shape templates in `native_material_pools.cpp`. It uses the same canonical
`00E188B4` allocator-list domain and singleton allocation/free service,
with its own `00F8BF50` storage, critical section, slab table and slabs.
`GameNativeGuiLayerPoolProcess` keeps this one storage and companion alive
through the registered exit callback. Explicit `initialize_once_00cd73d0`
performs the `00AC4D70` constructor then registers `00CE0AD0`; an
allocation before completed startup throws. The original constructor has
FH3 unwind state at `00CB87CE`, while the C++ source uses the existing
template's source cleanup and does not reproduce native SEH or ABI.

The MSVC Win32 build and existing CTests passed. A focused allocation
fixture initializes the process pool, allocates raw slots, checks the
slab-index/free-stack relationship, returns them through `00AC4C40`, and
checks that the same pool reissues a returned slot. It does not construct a
GUI page in the payload. No application startup call or page production is
bound here; the host must initialize this process pool before any raw page
allocator call and drain pages before the registered pool teardown.

`00AC4750` has a Ghidra listing gap after the CRT free call caused by a
suspect no-return interpretation. The existing fixed-pool trim template
supplies the source behavior; no Ghidra flow repair or annotation was made
by this worker. The code is a typed source counterpart, not a binary
replacement for the native class pool.
