# Raw locale uppercase leaves (Source only)

Base e00b43c07, MSVC Win32. Descriptive names are hypotheses. Only two new ordinary C++ APIs are supplied; semantic LocaleTables APIs are unchanged.
Native 00A9EBA0 uses ECX manager, stack DWORD, AX result, RET4; 00A9EC30 uses ECX manager, stack raw8 header, RET4. These C++ interfaces do not reproduce that ABI/register/flag/return-address contract.
The first API has the zero-extended WORD domain actually supplied by EC30. Native EBA0 captures full EBX and pushes the full DWORD on miss while comparing BX; arbitrary incoming high DWORD bits are outside this Source API.

EBA0 EBB0–C6 loads lower+4024 and, only if nonnull, CURRENT end+4028; uint32 index compares unsigned against wrapped SUB then SAR1 bits.
EBC8–DA reuses the captured lower pointer and reloads end for its second guard. EBDF reloads CURRENT lower after the actual invalid-parameter call may return.
The first matching WORD wins. EBF0–EC08 validates same-index upper+4034/+4038, then EC0D reloads CURRENT upper regardless of whether the handler ran.
No upper-table failure becomes a CRT fallback. No vector::at, fabricated exception, noreturn, default result, bounds repair or hidden retain is introduced.

Both BF6713 sites use existing invoke_native_invalid_parameter_00bf6713 from native_invalid_parameter_noinfo_call.hpp/cpp.
That entry calls the actual selected SDK _invalid_parameter_noinfo and may return/propagate/terminate under its current handler policy. Original encoded-handler/debugger/Watson behavior is not established.
The miss-only C0391C call is represented by selected Source std::towupper from cwctype. Current SDK wint_t is asserted16-bit; current CRT locale/PTD/case-table behavior is qualified, not Original VS2005 equivalence.
No vendor-library body, PTD bridge, handler installation, semantic LocaleTables transport or fake raw provider is reconstructed.

EC30 EC39–3D reads only length before its zero-length return; data/manager/provider are not reached on that arm.
EC40 reloads data every iteration, EC43 reads WORD, EC47 captures destination, EC4D maps, EC52 increments uint32 index, EC55 stores AX to the saved destination, EC58 compares CURRENT length.
An invalid-parameter/provider callback may edit manager/header values; it must preserve every reached backing and especially the captured destination through its store. The store intentionally does not resample header+4.
Unsigned32 address arithmetic and explicit SAR1 bit propagation avoid C++ signed-overflow/pointer-difference assumptions. Byte-based memcpy avoids typed-layout/alignment casts. MSVC compiler barriers keep explicit reload/store epochs; they are not atomicity or thread-synchronization guarantees.

Caller must supply the genuine loaded 0x4040 manager and actual raw8 header/array extents, serialized accessible memory and transitive provider/handler lifetime. Zero-length only needs its reached header length storage.
This packet owns no manager, cache, array, allocator, registry, global, cleanup or publication. Invalid accessible-memory proofs are outside the supported Source domain; original fault behavior is not reproduced.
Manager construction/load/publication remains unbound, source_ready=false; startup and ac60 EMPTY admission stay OFF/held. Font48/Viper19 cannot be cleared or skipped with these leaves.

Evidence: shared exports/bsp/functions/00a9eba0 and00a9ec30 assembly; Original b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6. Correct Native spans are EBA0..EC2C(exclusive), EC30..EC63(exclusive).
Unreached EBAD..EBB0(exclusive) bytes8D4900 encode LEA ECX,[ECX]; initial EBAB JMP skips them. C0391C wrapper was inspected only to identify fallback, not reconstructed or credited.
No Ghidra mutation, ledger change, startup caller, test/probe/game or runtime/ABI/production claim. Root owns CMake registration and coupled Win32 validation.
