#pragma once

#include "bsp/dyn_engine_runtime.hpp"
#include "bsp/dyn_world_runtime.hpp"

namespace bsp {
// 00C420E0..00C421A3, retained native name Dyn_Engine_CreateWorld.
// ECX engine, one stack descriptor pointer, EAX world, RET4 at00C421A1.
// Allocates and fully constructs a 48Ch world, then appends its pointer to the
// actual engine vector at+0/+4/+8. Growth uses unsigned32 capacity*2+2 and
// byte extent capacity*4, preserving the native store/allocation/copy/free order.
//
// Engine storage must come from its actual constructor and remain valid. The
// complete world context borrows initialized engine/global dispatch owners;
// its existing memory service allocates both the world and engine pointer list.
// Native explicit null-world and null-destination branches are retained, but
// malformed vectors, wrapped allocation extents, failed OS initialization and
// native SEH/OOM unwind are outside the supported successful-allocation domain.
// This adds no owner destructor and is a new C++ interface, not a native ABI thunk.
DynWorldStorage* dyn_engine_create_world_00c420e0(DynEngineStorage& engine,
    const DynWorldDescriptor& descriptor, const DynWorldRuntimeContext& context);
} // namespace bsp
