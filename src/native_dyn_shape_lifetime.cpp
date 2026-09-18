#include "bsp/native_dyn_shape_lifetime.hpp"

namespace bsp {
DynConvexShapeStorage* delete_native_dyn_convex_shape_004062c0(
    DynConvexShapeStorage& shape,std::uint32_t flags,DynConvexShapePoolStorage& pool) {
    static_assert(sizeof(void*)==4);
    auto* captured=&shape;
    *reinterpret_cast<volatile std::uint32_t*>(captured)=0x00d7a04c;
    if(flags&1)dyn_convex_shape_free_00408040(pool,*captured);
    return captured;
}
} // namespace bsp
