# World constant-byte Source (CC12)

**IMPLEMENTED_UNBUILT:** the genuine Native `009035D0` constant-byte leaf now
has a dedicated ordinary C++ Source interface. It returns `true` and does not
read its receiver. It creates no World table or owner and changes no broader
World readiness. The name is provisional; the semantic meaning of the slot
remains unknown.

## Exact Native boundary

The existing method-table audit established Native table CE7784+8 ->009035D0.
A fresh target-verified read returns `B0 01 C3` at `[009035D0,009035D3)`, followed
by thirteen CC padding bytes through009035DF. The next catalogued function is
`009035E0`; the preceding catalogued entry is `009035A0`. The entire leaf is
two instructions: `MOV AL,1` at009035D0 and plain `RET` at009035D2. It has no
branch, callee, argument/storage read, allocation or ownership effect.

Native AL becomes1. The upper24 EAX bits, EBX/ECX/EDX/ESI/EDI/EBP and flags are
unchanged by those instructions. RET consumes only the return address, advancing
entry ESP by4; there are no stacked arguments to remove.
In particular, this is not proof that Native EAX becomes the 32-bit integer1,
nor that World active/ready fields are consulted. Ghidra's existing missing
function definition at this table target is not changed here.

## Source interface and qualification

[`native_world_constant_byte.hpp`](../include/bsp/native_world_constant_byte.hpp)
declares:

```cpp
bool __fastcall native_world_constant_byte_009035d0(
    void* unused_ecx_receiver, std::uint32_t unused_edx) noexcept;
```

The MSVC Win32 guard fixes the intended calling-convention family. Both
explicit formals fit the fastcall registers ECX/EDX, leaving zero stacked
arguments. The one-byte `bool` result models the true value in AL. The
[`Source body`](../src/native_world_constant_byte.cpp) contains only
`return true;`, with a compile-time check for four-byte pointers and one-byte
bool. Both arguments are ignored; no function call or inline assembly is added.

This ordinary Source contract does **not** promise the native upper-EAX,
flags or unused volatile-register preservation. It also does not claim a
native binary replacement, original symbol identity, complete class ABI or
exact emitted instruction bytes. Root must review the whole emitted body at
the next normal build. A matching compiler emission would be new physical
evidence, not a conclusion drawn solely from this declaration.

The real constant result is the entire observed Native behavior; it is not a
stand-in for a missing lifetime operation. No table entry is fabricated or
published. The three other directly evidenced World table methods remain
separate dependencies, and this helper does not make full World construction,
normal destruction, startup or gameplay ready. No registration or function
credit is recorded by this worker.

## Evidence and handoff

The [report](../reports/cc12_world_constant_byte_source.json) retains the exact
three Native bytes, the sixteen-byte leaf/padding span, live/disk comparisons,
source hashes and the interface limits. Each live CLI request verified project
`bsp` and `/battlestationspacific.exe` against the configured
`C:/Users/sqz269/bsp.gpr` target.

Exactly the new header/source and this document/report changed. No CMake,
config, ledger, Ghidra definition/annotation, build, test or probe occurred.
Root owns integration, optional saved-analysis publication after independent
evidence, and build/emitted-body acceptance.
