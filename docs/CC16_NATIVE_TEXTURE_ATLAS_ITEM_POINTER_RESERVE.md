# Actual atlas item pointer reserve 735F90

The ignored candidate supplies `reserve_native_texture_atlas_item_pointers_00735f90`
over the actual12-byte data/count/capacity header. Original ABI is ECX header,
one signed capacity DWORD/RET4. The new C++ interface is not the original ABI.
It does not construct or reinterpret a semantic `DwordVector`.

All38 assembly rows were inspected through `bsp.py show`. The full 735F90 and
735FF0 listings have the same instructions after a +60h relocation, including
internal branch destinations and the exact external BF55BE/BF6989 calls.
Identity was established from the actual listings, not from source resemblance.
The separate address/function is retained and receives its own raw Source body.

Signed request clamps to1; signed current capacity is tested before table/count
access. Allocation receives wrapped request*4 bytes through the established
concrete physical singleton allocation service at its accepted source-CRT
boundary. The copy loop rechecks CURRENT signed count and CURRENT source table
at their actual instruction epochs, preserves null destination tests and DWORD
address wrap, then frees CURRENT old table through the existing physical free
service. Replacement table publishes before capacity; count is untouched.
Inactive slots, pointees and their reference counts are untouched.

Borrow valid initialized actual backing and native allocation/free/alias/CPU/
lifetime domains. No new bounds/overflow/null repair, semantic copy, default,
rollback, retain, release or cleanup occurs. C++ failure and partial allocation
or copy debt propagate to the owning retained graph. This ordinary NORMAL body
does not claim native exception/heap identity, FH3/SEH, fault, ABI or game parity.

The proposal was authored from frozen 2a094149a200654ea7ddf63226483c2a72bcba35
under explicit Root approval of the smallest missing parser dependency.
`reports/cc16_native_texture_atlas_item_pointer_reserve.json` pins it. No tracked
Source, CMake, ledger, Ghidra, build, fixture, Game or installation was changed.
Source inspection only; compilation/runtime and independent peer credit are zero.
