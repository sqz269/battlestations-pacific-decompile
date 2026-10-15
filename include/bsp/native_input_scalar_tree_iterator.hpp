#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scalar-tree iterator entries require MSVC Win32.
#endif

namespace bsp {
// Whole005531F0..00553252 (99B/38 instructions). Native ECX actual8B iterator
// (owner0,node4), incoming EDX unconsumed, no stacked inputs, plain RET.
// No semantic EAX result. Null-owner validation may return, then CURRENT node
// is reread; nil current node restores ESI and tails actual invalid service.
// Right-subtree minimum or parent climb; only iterator.node4 is written.
// Exact99B equality to accepted869A20 except two proved CRT rel32 fields.
void __fastcall increment_native_input_scalar_tree_iterator_005531f0(
    void* iterator, void* unused_edx);

// Whole00556DC0..00557066 (679B/231 instructions): native ECX actualtree,
// incoming EDX unconsumed; stack(output,input_owner,input_node), RET0Ch,
// EAXoutput. No extra owner==tree check. Increment mutates the actual by-value
// argument iterator; original node is separately captured, transplanted/freed.
// Complete0/1/2child/extrema/rotation/color-fix, CURRENT unsignedcount decrement
// onlyifnonzero, then current argument owner/node captured/stored to output.
// No payload release. Actual18h node: links0/4/8, unowned key/valueC/10,
// color14,nil15,padding16/17; header opaque0/head4/count8.
//
// Nil input constructs actual no-init SBO(cap15,len0,NUL), assigns27 counted
// message bytes BEFORE arming cleanup, constructs owning28h D6926C/D863A8
// profile, then throws the accepted NativeHardwareLayoutInvalidIterator.
// Actual411700/copy441760/destroy4412B0/temp4072D0 services are retained.
// Complete native679B matches accepted86E8A0 after13 CALLrel32 fields and
// its own handler immediate; full helper bodies and EH maps/actions also match.
void* __fastcall erase_native_input_scalar_tree_iterator_00556dc0(
    void* tree, void* unused_edx, void* output, void* input_owner, void* input_node);

// Direct accepted ordinary services, no callback/provider/algorithm substitute.
// Reached actual storage must support native accesses, including continuation
// after returning diagnostics. Actual current SDK/CRT and owning C++ exception
// transport apply. Source/private argument frames, volatile-register residue,
// original FH3/unwind spill aliases, native RTTI/catch/heap/ABI identity,
// hardware faults/SEH and game execution remain unverified. Descriptive names
// are hypotheses; historical556DC0 xlen_throw label/prototypes are incorrect.
// Whole5572F0 partial erasure is not implemented or promoted by these entries.
} // namespace bsp
