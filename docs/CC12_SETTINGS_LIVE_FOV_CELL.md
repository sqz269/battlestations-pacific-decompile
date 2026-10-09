# CC12: retained native settings FOV cell

Primary integration: the normal MSVC Win32 build and all three existing checks
passed. The complete emitted accessor is 14 bytes/5 instructions: it obtains
the guarded process owner and returns its actual +34h address. Its exact core
library member is retained; the accessor is absent from the game map, so the
GlobalConfig application binding remains open. See
[primary review](../reports/cc12_settings_world_source_primary_review.json).

`GameNativeSettingsApplication::fov_divisor_00f889b4()` now returns the actual
`float` subobject at `+34h` in the canonical process-owned `BCh` settings object.
It is suitable for the existing `NativeGlobalConfigLoadContext` reference field.
The accessor does not read or copy the value: it borrows the live cell through
the application's retained `GameNativeSettingsProcess&` and its existing guarded
`settings()` accessor. This packet adds a Source composition interface only;
it gives no additional original-function reconstruction credit and does not
instantiate the still-separate application GlobalConfig context.

## Original evidence and ABI

- `008D7710`: native `ECX` is the settings owner; `EAX` returns the same owner;
  `RET` has no stack pop. `008D77D8` loads the bits at `00CE7D20` into `XMM0`;
  `008D7865` stores those bits to `[ESI+34h]`. The current Source constructor
  already reproduces that store through `memcpy` using captured video bits.
- The actual static owner starts at `00F88980`, so its `+34h` cell is
  `00F889B4`. No mapped-PE zero cell or copied presentation option is used.
- `0087D7B0`: native `ECX` is the GlobalConfig owner; no stack arguments, `RET`.
  `0087EC29` and `0087EC79` each execute `FDIV float ptr [00F889B4]`, after
  their respective Ship/Plane Lua numeric reads. The following `FSTP`s write
  GlobalConfig `+F8h` and `+FCh`.
- The unchanged Source loader's `fov(float,const volatile float&)` takes the
  address of that reference and issues a separate x87 memory `FDIV` for each
  call. This packet changes neither x87 arithmetic nor lookup/store ordering.

These sites were inspected in the existing `exports/bsp/functions/008d7710`
and `0087d7b0` export triples. Their metadata identifies project `bsp`, program
`/battlestationspacific.exe`, with export dates 2026-09-18. No new live Ghidra
query, re-export, annotation or GPR mutation was performed.

## C++ storage and lifetime

`NativeGameSettingsStorage` has opaque bytes `[00h,34h)`, a real `float` member
at `34h`, and opaque bytes `[38h,BCh)`. The float's lifetime begins with the
containing object; the accessor binds directly to that member with added
`const volatile` qualification. It does not fabricate a float reference from
the previous byte array, allocate another cell, or introduce a union lifetime.

Size, alignment, exact offsets, standard layout, trivial default construction,
trivial copy construction/assignment and trivial copyability are asserted in
the header. The adjacent ranges account for every byte of the `BCh` object.
No member initializer or user-defined constructor/copy/destructor was added.
Default-initialized ordinary storage therefore remains uninitialized until its
caller supplies preimages and invokes the native partial constructor. Existing
process `settings{}` still supplies the initial zero preimage; the constructor's
sparse writes and all destructor behavior remain unchanged.

The new const/nonconst `data()` accessors address the complete object's byte
representation, not the prefix array. Whole-owner `memcpy` snapshots/restores
remain supported by trivial copyability, including opaque bytes and exact
float representations. Ordinary generated copy operations remain available;
no deep-copy, refcount, initialization or replay policy was added. Native APIs
continue to accept the actual owner through their unchanged `void*` contracts.

The process instance and its `Impl` are retained for CRT lifetime, cannot be
copied, and own the cell directly. `settings()` rejects access until static
initialization reaches its existing `ready` phase. The new application accessor
uses that same guard; it adds no cached value or second phase guard. An already
borrowed cell remains process-owned even after application services retire.
`volatile` preserves the intended live reads; it adds no thread synchronization.

## Client census and bounded change

Tracked production/test/tool searches found the storage type only in the
native owner header/source, settings process header/source, and settings
application header/source. The only direct `.bytes` use was the application's
clan-string read at `+B4h`; it now uses `storage.data()+0xB4`. Its base-address
reads and every `&storage`/`&settings()` native call remain unchanged. There
were no tracked storage-type tests or other direct byte-array clients.

Only three Source files changed: `include/bsp/native_game_settings_owner.hpp`,
`include/bsp/game_native_settings_application.hpp`, and
`src/game_native_settings_application.cpp`. The process and loader need no
behavioral edits. Shared CMake, headers owned by other packets, and ledgers were
not changed.

## Validation boundary

The worker reviewed the complete Source diff and existing assembly sites;
`git diff --check` passed. The companion JSON report records the census,
contracts, checked source/export hashes and baseline commit. The assertions
are compile-time contracts awaiting the integrator's normal Win32 build.
Per packet ownership, the worker ran no build, executable probe, new test or
game. This is not runtime, game-startup, whole-loader, or binary-ABI validation.
