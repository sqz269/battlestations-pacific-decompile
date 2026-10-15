# POD8 allocation and failure entries

This packet reconstructs four complete Native-consumed functions using actual raw storage and existing concrete children. The two derived functions were added to the lease with primary authorization after an exhaustive public Source search found no existing providers. Correct Native library/compiler names and prior evidence are retained. There are no Ghidra, shared CMake or packet-registry mutations.

| Complete Native span | Source entry | Original physical ABI |
| --- | --- | --- |
| `00413390..004133E4`, 85 bytes | `allocate_native_pod8_records_00413390` | ECX uint32 count; no consumed EDX/stack input; plain RET; EAX actual storage |
| `004203A0..00420408`, 105 bytes | `throw_native_pod8_vector_length_error_004203a0` | No consumed input or normal return |
| `004010E0..004010EA`, 11 bytes | `destroy_native_pod8_allocation_exception_004010e0` | ECX actual raw12 owner; no stack input; tail into base cleanup, no semantic result |
| `0040D810..0040D828`, 25 bytes | `copy_native_pod8_allocation_exception_0040d810` | ECX actual raw12 destination; one stack source; RET4/EAX captured destination |

Source fastcall declarations reserve an unused EDX placeholder where needed. It is not an Original parameter. Original C++ template/return identities and throw ABI are not inferred from these Source declarations.

## Allocation and direct overflow

The complete allocator retains the positive-count unsigned predicate `FFFFFFFFh / count < 8`. Count0 bypasses division but still invokes the actual allocation provider with zero bytes. `1FFFFFFFh` is admitted, requesting `FFFFFFF8h` bytes; larger unsigned counts throw before allocation. The optimized Source listing retains TEST/zero branch, unsigned DIV/CMP8/JB, and a genuine `singleton_lifetime_allocate` call for the admitted path, including zero. No signed guard, null early return or private allocator is added.

Admitted allocation requests exactly `count*8` native and host bytes through the established object-byte domain. `singleton_lifetime_allocate` calls current malloc, calls current `_callnewh` on null, retries when requested, and otherwise throws current `std::bad_alloc`. Returned storage pairs with `singleton_lifetime_free`. Original BF681B performs the analogous native malloc/new-handler retry but uses its own static CRT owner109DD68/flag109DD74 and E03CC0 failure transport. That static heap, handler and allocation-failure exception identity remain an explicit current-CRT boundary.

Direct multiplication overflow has a stronger preserved contract. `NativePod8AllocationOverflow` contains only the actual 0Ch owner: native profile DWORD, message pointer, ownership DWORD. Its constructor passes an **actual null pointer-slot address** to genuine BF6340, preserving the null path's DWORD RMW, and then publishes D6923C. It does not value-initialize or silently construct an SBO. Result fields are null message and ownership1. `what()` preserves Native BF646A's `Unknown exception` fallback. This direct-overflow payload differs from native heap failure's borrowed `bad allocation` message.

Copies call the complete 25-byte derived adapter, which captures the destination, calls actual BF63A6, publishes D6923C afterward and returns the captured destination. Destruction calls the complete 11-byte derived adapter, which publishes D6923C then tail-jumps actual BF6454. The base reads the full ownership word before D69370 publication and conditionally frees the late message, including null, without clearing message/control. No private duplicate of either derived schedule, fake throw callback or invented static owner is used.

Native ThrowInfo E03CC0 selects destructor4010E0 and catchable arrayE03CB4: bad_alloc E17510/size0Ch/copy40D810 and exception E174F0/size0Ch/copyBF63A6; PMDs `{0,-1,0}`. The new Source class is also 0Ch, but has its own Source catch type/RTTI/EH metadata. It does not claim Original `std::bad_alloc`, E03CC0 or BF6885 catch compatibility. Its emitted Source ThrowInfo points to the real class copy/destructor and records payload size0Ch.

## Length owner and temporary lifetime

203A0's complete 105-byte body matches accepted BD0590 after exactly its handler immediate and three CALL displacement normalizations; all 89 remaining bytes match and all three call targets match. The one-state FH3 maps also agree: C5E478 loads FuncInfo D84728; mapD84720 takes state0 to-1 through C5E470, whose whole body is `LEA ECX,[EBP-50h]; JMP4072D0`. There is no catch map. The provider's corresponding FuncInfo/map/action are DFF4BC/DFF4B4/CC5470.

The actual provider initializes only temporary SBO capacity15, length0 and first NUL, assigns 18 bytes from `vector<T> too long`, then arms cleanup after assignment succeeds. It constructs the actual 28h legacy owner through411700, publishes D69260 and throws owning `NativeSingletonVectorLengthError`. That transport copies through411940 and destroys through411780; temporary cleanup also runs on owner-constructor failure. A second outer cleanup guard or generic `std::length_error` substitution would change this established lifetime and is not introduced.

Native ThrowInfo D83F98 selects411780 and three catchable types: length_error E08000/28h/copy411940, logic_error E08020/28h/copy4118D0, exception E174F0/0Ch/copyBF63A6. The payload is base0Ch plus SBO1Ch at+0Ch. Native vtable words and RTTI metadata remain address data. Existing `NativeAliasListLengthError` has the same owner lifecycle but a distinct Source catch type; the established vector-family transport is reused here.

## Evidence, callers and checks

Every live batch used `bsp.py ghidra`, which verifies the existing `bsp` project, `/battlestationspacific.exe`, `x86:LE:32:default` and base00400000. Complete assembly, metadata, every discovered caller setup and the existing constructor/copy/cleanup Source contracts were inspected. No reimport was performed.

The physical census finds three allocator CALLs and two length CALLs. The 420F80 sites consume ECX chosen capacity/EDX0 at42104B, and no throw input at420FEA. Other genuine sites reside in currently uncreated raw functions: wrapper418700..41870D and constructor420AC0..420B06. Their complete argument setups confirm the same contracts; nearest-index caller names are not accepted as live ownership. Metadata references additionally bind4010E0 and40D810. Exact rows and the uncreated ranges are recorded in the JSON report.

MSVC x86 C++17 `/MD /O2 /W4 /WX /EHsc /fp:strict /permissive-` compilation passed without warnings. The two complete derived sections account for36 Native bytes: all28 nonrelocation bytes match captured live bytes and installed PE, and the two REL32 operands bind the actual existing base providers. The allocator/length entry use the qualified Source EH/service composition; no whole-entry byte identity is claimed for them.

One ignored combined probe linked the new object with the real current integrator archive. It passed actual zero/small allocation/free; direct-overflow raw12 fields, owning copy, what and post-destruction base profile/control; and full28h length catch, independent heap-message copy and member/base teardown. It uses `/MANIFEST:EMBED` and adds no test target/framework. Temporary code, exact captures, COFF parser and probe are under `local/goal15g_pod8/`. The primary owns final registered object/archive checks, Win32 build/existing suites, annotations, exports and main integration. Native throw execution, original RTTI/SEH/FH3/static-CRT identity and game validation remain unproved.
