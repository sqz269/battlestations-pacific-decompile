#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene integer-set insertion requires MSVC Win32.
#endif

namespace bsp {

// Explicit backing for native flag DWORD, predecessor iterator,16-byte linker
// parameters and20-byte allocator parameters. Addresses/binding metadata remain
// stable and metadata is disjoint from all described cells/payload. Only
// reached backing is required; the flag's upper3 bytes retain caller values.
struct NativeSceneIntegerSetInsertFrame {
    void* descent_flag4;
    void* predecessor8;
    void* linker_parameters16;
    void* node_parameters20;
};

// Whole0046A150..0046A33B plus Source temporary unwind00C61C10.
// Native ECX tree;stack output,leftDWORD,parent,key-pointer;RET10h/EAX output.
// These16-byte parameters remain LIVE;20-byte allocator cells are separately
// published color,key,head,parent,head before real allocation/late live reads.
// Unsigned count>=3FFFFFFEh throws the existing owning D69260 length error.
// After allocation reread head/count,late low-left byte,relink/rebalance,
// blacken current root,then CURRENT output pointer and NODE before OWNER.
void* link_native_scene_integer_set_node_0046a150(void* actual_tree,
    void* actual_parameters16, void* actual_node_parameters20);

// Whole0046AC50..0046AD08. Native ECX tree;stack output,key-pointer;RET8.
// Input key POINTER is captured at entry. Signed search DWORD is captured
// once onlyif starting root is nonnil; final comparison rereads CURRENT key.
// Result0Ch publishes OWNER,NODE,insertedBYTE;final3 padding bytes untouched.
// Live8h parameters/private flag/predecessor/link/allocator backing retain
// alias-sensitive accesses,including complete returning current-CRT paths.
void* insert_native_scene_integer_set_key_0046ac50(void* actual_tree,
    void* actual_parameters8, const NativeSceneIntegerSetInsertFrame&);

// These are new ordinary Source interfaces. Actual cells/payload can alias
// when all reached accesses/releases remain valid; unsynchronized concurrency
// and implicit Source/native private-frame/argument-home aliases are excluded.
// Current CRT heap/handler and owning C++ throw transport are concrete; native
// RTTI/catch inheritance/FH3/SEH/register/byte identity/game validation is not.
} // namespace bsp
