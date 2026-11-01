#pragma once
#include "bsp/native_gui_icon_state_append_wrapper_storage.hpp"
#include "bsp/native_gui_icon_native_size_storage.hpp"
#include "bsp/native_gui_icon_placeholder_storage.hpp"

namespace bsp {
struct NativePlatformFocusOwnersContext;
struct NativeGuiTextureResolverStorageContext;
struct NativeGuiIconPlaceholderStorageContext;

// Retained ordinary operand/subobject backing. Empty user-provided construction
// leaves Native homes, state3E/3F and inherited scratch preimages unseeded.
// temporary_2c is one genuine individual40h state, not a state-array family.
// local_0c is SAME backing for size2, captured pivot2/native-size output2,
// and the resolver's separate UV4. No implicit Native destructor is attached.
struct NativeGuiIconStateConstructionStorageFrame {
    NativeGuiIconStateConstructionStorageFrame() {}
    NativeGuiIconStateConstructionStorageFrame(const NativeGuiIconStateConstructionStorageFrame&) = delete;
    NativeGuiIconStateConstructionStorageFrame& operator=(const NativeGuiIconStateConstructionStorageFrame&) = delete;
    NativeGuiIconStateConstructionStorageFrame(NativeGuiIconStateConstructionStorageFrame&&) = delete;
    NativeGuiIconStateConstructionStorageFrame& operator=(NativeGuiIconStateConstructionStorageFrame&&) = delete;
    std::uint32_t local_0c[8];
    NativeGuiIconStateStorage temporary_2c;
    std::uint32_t state_74, preserve_home;
    std::uint32_t resolver_homes[4], uv_homes[4], native_size_uv_home;
    NativeGuiIconNativeSizeScratch native_size;
    NativeGuiIconPlaceholderStorageFrame placeholder;
    std::uint32_t prototype_home;
    NativeGuiIconStateAppendWrapperStorageFrame append;
};

struct NativeGuiIconStateConstructionStorageContext {
    NativePlatformFocusOwnersContext& gui;
    NativeGuiTextureResolverStorageContext& resolver;
    NativeGuiIconPlaceholderStorageContext& placeholder;
    NativeGuiIconStateGrowthStorageContext& growth;
    const volatile float& actual_one_00d7a24c;
};

// Whole645B/168row AB66C0 ordinary data/call schedule. Original ECX Icon138,
// four actual pointer DWORD homes [name8,UV4,pivot2-or-null,size2-or-null],
// EAX full DWORD index and RET10h become this four-argument Source interface.
// Capture Icon once. Capture pivot HOME2 before size HOME3 at their reached
// epochs, retaining the exact paired MOVSS/store order and Icon18/20 fallbacks.
// These are Native null branches, not a new input/profile/count/domain filter.
// Name HOME0 is captured after temporary initialization; name copy uses exact
// 41DD40(actual preserveHOME1) and BF7680 including reached count0. UV HOME1
// is read only after that copy and the CURRENT Icon101 BYTE comparison.
//
// Nondelayed path calls genuine4C12B0 before FLD1/FSTP scale staging, then real
// rawAA2660 with ORIGINAL captured name header and SAME local UV4/size2. Store
// returned texture amid the reached UV-word captures, then realAB1680. Ordered
// zero/NaN tests select realAB17B0 using SAME local pivot2 as output, SAME temp
// resolvedUV4, actual caller-retained scratch and genuine captured-dimension
// consumer. Delayed path uses whole rawAB3B30, not a semantic placeholder.
// RealAB63A0 inserts SAME temp. Read current vector first/last and capture the
// FULL wrapped DWORD size-minus1 BEFORE state-1/normal temp destruction.
//
// state0 precedes name construction. An ordinary Source C++ catch destroys
// SAME initialized temp once on a later C++ escape and barethrows only after
// AB3280 returns; its existing name-unwind/secondary-terminate policy is
// inherited. The normal state-1/destructor footer is outside that catch. No
// Native Origin/FH3/SEH, original exception/secondary identity or hardware-fault
// permission follows from state/entry. No retry, rollback, new terminate,
// owner, callback, countcap, no-op, NoThrow or automatic frame cleanup exists.
//
// Caller supplies genuine Icon138/state/vector/subobject/backing/atomic and
// provider lifetimes. gui, resolver, placeholder and growth borrow the SAME
// actual process/renderer/rawpool/feature/RO-one domains; their existing
// context/acquired/observed-release/current-generation obligations survive.
// In particular each reached cache/observed-release record is genuinely fresh
// and retained. A renderer profile/target/count is inert data, not proof of the
// Source-created D5F0A8/B319B0 class needed by either miss/load edge. Arbitrary
// OTHER64 transport and full actual Page/currentA4 production remain OPEN.
// No semantic Icon/State/NativeString projection or raw-to-owner cast occurs.
//
// Declared operand homes and payload aliases remain live where valid; Frame's
// actual subobjects/arrays are separate as declared. Genuine callbacks may
// mutate reached operand homes. Private Source controls/carriers/spills and
// callee private scratch stay disjoint from payload/provider-write extents.
// Each active invocation needs distinct retained Frame and acquired backing;
// preimages read by inherited helpers must already be genuine initialized
// bits. Keep partial/completed/current graph, numeric history and independent
// provider debt through failure and actual disposition; state is no authority.
// Existing lower copy/dtor/SDK/FP/custody policies remain qualified, including
// their successful-domain/private-alias restrictions. No T[n] birth, Original
// ABI/private-stack/FLAGS/fault/unmaskedFP/runtime/activation or extra body /
// vendor/helper/CG/duplicate credit is claimed. No C++ FP or FP reset is added.
std::uint32_t add_native_gui_icon_state_storage_00ab66c0(void* actual_icon138,
    volatile std::uint32_t (&actual_argument_homes)[4],
    NativeGuiIconStateConstructionStorageFrame&,
    NativeGuiIconStateConstructionStorageContext&);
} // namespace bsp
