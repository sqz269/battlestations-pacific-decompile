#include "bsp/dummy_depth_seg34_argon.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace bsp::dummy_depth_seg34_argon {

namespace {

template <typename T>
inline T& field_at(void* base, std::ptrdiff_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

template <typename T>
inline const T& field_at(const void* base, std::ptrdiff_t offset) {
    return *reinterpret_cast<const T*>(static_cast<const std::uint8_t*>(base) + offset);
}

template <typename Fn>
inline Fn fn_at(std::uintptr_t addr) {
    return reinterpret_cast<Fn>(addr);
}

template <typename Fn>
inline Fn vslot_at(void* obj, std::ptrdiff_t slot_byte_offset) {
    void* vtable = *reinterpret_cast<void**>(obj);
    return *reinterpret_cast<Fn*>(static_cast<std::uint8_t*>(vtable) + slot_byte_offset);
}

inline void interlocked_inc_00ce221c(volatile std::int32_t* p) {
    using Fn = std::int32_t(__stdcall*)(volatile std::int32_t*);
    Fn f = *reinterpret_cast<Fn*>(0x00ce221c);
    if (f != nullptr) {
        f(p);
    } else {
        ++(*p);
    }
}

inline std::int32_t interlocked_dec_00ce2220(volatile std::int32_t* p) {
    using Fn = std::int32_t(__stdcall*)(volatile std::int32_t*);
    Fn f = *reinterpret_cast<Fn*>(0x00ce2220);
    if (f != nullptr) {
        return f(p);
    }
    return --(*p);
}

struct ByValPair8 {
    std::uint32_t w0;
    std::uint32_t w1;
};

struct ByValLuaObj20 {
    std::uint32_t w[5];
};

inline void detach_from_intrusive_list(void* list_head_base, std::ptrdiff_t head_ptr_offset, void* entity) {
    void* node = field_at<void*>(list_head_base, head_ptr_offset);
    while (node != nullptr) {
        if (field_at<void*>(node, 0x08) == entity) {
            fn_at<void(__thiscall*)(void*, void*)>(0x004837d0)(list_head_base, node);
            break;
        }
        node = field_at<void*>(node, 0x04);
    }
}

inline void* climb_owner_chain_to_kind5(void* self) {
    using GetOwnerFn = void*(__thiscall*)(void*, std::uint32_t);
    GetOwnerFn get_owner = fn_at<GetOwnerFn>(0x00923810);
    if (get_owner(self, 1u) == nullptr) {
        return nullptr;
    }
    void* cur = get_owner(self, 1u);
    while (get_owner(cur, 1u) != nullptr) {
        using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
        if (vslot_at<IsKindFn>(cur, 0x5c)(cur, 5u) != 0u) {
            break;
        }
        cur = get_owner(cur, 1u);
    }
    return cur;
}

}  // namespace

// ============================================================================
// 1. DynWorld / ConvexObject / ConvexHull Debug & Transform (0x006fa540..0x006fb030)
// ============================================================================

// [0x006fa540] BSP_DynWorld_ForEachBodyDebugDraw
// Original ABI: __thiscall (ECX = self), RET 0.
void DynWorld_ForEachBodyDebugDraw_006fa540(void* self) {
    void* body = fn_at<void*(__thiscall*)(void*)>(0x00c31dc0)(self);
    while (body != nullptr) {
        const float* world_m = fn_at<const float*(__thiscall*)(void*)>(0x00c32000)(self);
        const float* body_m = fn_at<const float*(__thiscall*)(void*)>(0x00c32430)(body);

        float combined_3x4[12]{};
        combined_3x4[0] = static_cast<float>(
            static_cast<double>(body_m[1]) * static_cast<double>(world_m[3]) +
            static_cast<double>(body_m[0]) * static_cast<double>(world_m[0]) +
            static_cast<double>(body_m[2]) * static_cast<double>(world_m[6])
        );
        combined_3x4[1] = static_cast<float>(
            static_cast<double>(body_m[0]) * static_cast<double>(world_m[1]) +
            static_cast<double>(body_m[1]) * static_cast<double>(world_m[4]) +
            static_cast<double>(body_m[2]) * static_cast<double>(world_m[7])
        );
        combined_3x4[2] = static_cast<float>(
            static_cast<double>(body_m[0]) * static_cast<double>(world_m[2]) +
            static_cast<double>(world_m[5]) * static_cast<double>(body_m[1]) +
            static_cast<double>(world_m[8]) * static_cast<double>(body_m[2])
        );
        combined_3x4[3] = static_cast<float>(
            static_cast<double>(body_m[4]) * static_cast<double>(world_m[3]) +
            static_cast<double>(world_m[0]) * static_cast<double>(body_m[3]) +
            static_cast<double>(world_m[6]) * static_cast<double>(body_m[5])
        );
        combined_3x4[4] = static_cast<float>(
            static_cast<double>(body_m[4]) * static_cast<double>(world_m[4]) +
            static_cast<double>(body_m[3]) * static_cast<double>(world_m[1]) +
            static_cast<double>(body_m[5]) * static_cast<double>(world_m[7])
        );
        combined_3x4[5] = static_cast<float>(
            static_cast<double>(body_m[3]) * static_cast<double>(world_m[2]) +
            static_cast<double>(world_m[5]) * static_cast<double>(body_m[4]) +
            static_cast<double>(world_m[8]) * static_cast<double>(body_m[5])
        );
        combined_3x4[6] = static_cast<float>(
            static_cast<double>(world_m[0]) * static_cast<double>(body_m[6]) +
            static_cast<double>(body_m[7]) * static_cast<double>(world_m[3]) +
            static_cast<double>(body_m[8]) * static_cast<double>(world_m[6])
        );
        combined_3x4[7] = static_cast<float>(
            static_cast<double>(world_m[1]) * static_cast<double>(body_m[6]) +
            static_cast<double>(body_m[7]) * static_cast<double>(world_m[4]) +
            static_cast<double>(body_m[8]) * static_cast<double>(world_m[7])
        );
        combined_3x4[8] = static_cast<float>(
            static_cast<double>(body_m[7]) * static_cast<double>(world_m[5]) +
            static_cast<double>(body_m[6]) * static_cast<double>(world_m[2]) +
            static_cast<double>(body_m[8]) * static_cast<double>(world_m[8])
        );
        combined_3x4[9] = static_cast<float>(
            static_cast<double>(body_m[10]) * static_cast<double>(world_m[3]) +
            static_cast<double>(world_m[0]) * static_cast<double>(body_m[9]) +
            static_cast<double>(world_m[6]) * static_cast<double>(body_m[11]) +
            static_cast<double>(world_m[9])
        );
        combined_3x4[10] = static_cast<float>(
            static_cast<double>(body_m[10]) * static_cast<double>(world_m[4]) +
            static_cast<double>(body_m[9]) * static_cast<double>(world_m[1]) +
            static_cast<double>(body_m[11]) * static_cast<double>(world_m[7]) +
            static_cast<double>(world_m[10])
        );
        combined_3x4[11] = static_cast<float>(
            static_cast<double>(body_m[9]) * static_cast<double>(world_m[2]) +
            static_cast<double>(world_m[5]) * static_cast<double>(body_m[10]) +
            static_cast<double>(world_m[8]) * static_cast<double>(body_m[11]) +
            static_cast<double>(world_m[11])
        );

        float expanded_4x4[16]{};
        fn_at<void(__thiscall*)(const float*, float*)>(0x00c33650)(combined_3x4, expanded_4x4);
        fn_at<void(__fastcall*)(void*, const float*)>(0x006fa320)(body, expanded_4x4);
        body = field_at<void*>(body, 0x208);
    }
}

// [0x006fa6d0] BSP_DynWorld_ForEachBodyTransform
// Original ABI: __fastcall (ECX = self, EDX = rhs_mat4x4), RET 0.
void DynWorld_ForEachBodyTransform_006fa6d0(void* self, const float* rhs_mat4x4) {
    void* body = fn_at<void*(__thiscall*)(void*)>(0x00c31dc0)(self);
    while (body != nullptr) {
        float local_mat[16]{};
        float tmp_mat[16]{};
        void* m3x4 = fn_at<void*(__thiscall*)(void*, float*)>(0x00c32430)(body, local_mat);
        fn_at<void(__thiscall*)(void*, float*)>(0x00c33650)(m3x4, local_mat);
        const float* prod = fn_at<const float*(__thiscall*)(const float*, float*, const float*)>(0x00413920)(
            local_mat,
            tmp_mat,
            rhs_mat4x4
        );
        for (int i = 0; i < 16; ++i) {
            local_mat[i] = prod[i];
        }
        fn_at<void(__fastcall*)(void*, const float*)>(0x006fa320)(body, local_mat);
        body = field_at<void*>(body, 0x208);
    }
}

// [0x006faf00] BSP_ConvexObjectResourceParser_Create
// Original ABI: __stdcall (stack [ESP+4] = payload_stream), RET 4.
void* ConvexObjectResourceParser_Create_006faf00(void* payload_stream) {
    void* mem = fn_at<void*(__cdecl*)(std::size_t)>(0x00bf681b)(0x2cu);
    void* obj = nullptr;
    if (mem != nullptr) {
        obj = fn_at<void*(__thiscall*)(void*)>(0x006f9cd0)(mem);
    }
    fn_at<void(__thiscall*)(void*, void*)>(0x006fad70)(obj, payload_stream);
    return obj;
}

// [0x006fb030] BSP_ConvexHull_DebugDrawEdges
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = world_mat), RET 4.
void ConvexHull_DebugDrawEdges_006fb030(void* self, const void* world_mat) {
    void* hull = static_cast<std::uint8_t*>(self) + 0x0c;
    const std::uint32_t vert_count = fn_at<std::uint32_t(__thiscall*)(void*)>(0x00c32d20)(hull);

    struct Vec12Buffer {
        float* begin{nullptr};
        float* end{nullptr};
        float* cap{nullptr};
    } verts{};

    fn_at<void(__thiscall*)(void*, std::uint32_t, float, float, float)>(0x00421ae0)(
        &verts,
        vert_count,
        0.0f,
        0.0f,
        0.0f
    );
    if (verts.begin == nullptr || ((verts.end - verts.begin) / 3) == 0) {
        fn_at<void(__cdecl*)()>(0x00bf6713)();
    }
    fn_at<void(__thiscall*)(void*, float*)>(0x00c32ce0)(hull, verts.begin);

    struct ShortBuffer {
        std::uint16_t* begin{nullptr};
        std::uint16_t* end{nullptr};
        std::uint16_t* cap{nullptr};
    } adj{};
    fn_at<void(__thiscall*)(void*, void*)>(0x00c37430)(hull, &adj);

    std::uint32_t cursor = 0u;
    for (std::uint32_t i = 0u; i < vert_count; ++i) {
        const std::uint16_t degree = adj.begin[cursor & 0xffffu];
        ++cursor;
        const std::uint32_t cur_len =
            (verts.begin != nullptr) ? static_cast<std::uint32_t>((verts.end - verts.begin) / 3) : 0u;
        if (verts.begin == nullptr || i >= cur_len) {
            fn_at<void(__cdecl*)()>(0x00bf6713)();
        }
        float v0_local[3] = {
            verts.begin[i * 3u + 0u],
            verts.begin[i * 3u + 1u],
            verts.begin[i * 3u + 2u]
        };
        float tmp0[3]{};
        const float* p0_src = fn_at<const float*(__thiscall*)(const float*, float*, const void*)>(0x004142e0)(
            v0_local,
            tmp0,
            world_mat
        );
        float p0[3] = {p0_src[0], p0_src[1], p0_src[2]};

        for (std::uint32_t k = 0u; k < degree; ++k, ++cursor) {
            const std::uint32_t nbr = adj.begin[cursor & 0xffffu];
            if (nbr >= i) {
                const std::uint32_t nbr_cap =
                    (verts.begin != nullptr) ? static_cast<std::uint32_t>((verts.end - verts.begin) / 3) : 0u;
                if (verts.begin == nullptr || nbr >= nbr_cap) {
                    fn_at<void(__cdecl*)()>(0x00bf6713)();
                }
                float v1_local[3] = {
                    verts.begin[nbr * 3u + 0u],
                    verts.begin[nbr * 3u + 1u],
                    verts.begin[nbr * 3u + 2u]
                };
                float tmp1[3]{};
                const float* p1_src = fn_at<const float*(__thiscall*)(const float*, float*, const void*)>(0x004142e0)(
                    v1_local,
                    tmp1,
                    world_mat
                );
                float p1[3] = {p1_src[0], p1_src[1], p1_src[2]};
                const float c = *reinterpret_cast<const float*>(0x00d7a24c);
                fn_at<void(__fastcall*)(const float*, const float*, float, float, float, float)>(0x0085fb30)(
                    p0,
                    p1,
                    c,
                    c,
                    c,
                    c
                );
            }
        }
    }

    if (adj.begin != nullptr) {
        fn_at<void(__cdecl*)(void*)>(0x00bf6989)(adj.begin);
    }
    if (verts.begin != nullptr) {
        fn_at<void(__cdecl*)(void*)>(0x00bf65ac)(verts.begin);
    }
}

// ============================================================================
// 2. UnitDebugInspector & Entity Kind04 Casts (0x006fb680..0x006fbbc0)
// ============================================================================

// [0x006fb680] BSP_UnitDebugInspector_OnTargetGone
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = notifier), RET 4.
void UnitDebugInspector_OnTargetGone_006fb680(void* self, void* notifier) {
    if (field_at<void*>(self, 0x1c) == notifier) {
        field_at<void*>(self, 0x1c) = nullptr;
    }
}

// [0x006fb6a0] BSP_UnitDebugInspector_SetTarget
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = target), RET 4.
void UnitDebugInspector_SetTarget_006fb6a0(void* self, void* target) {
    void* old_target = field_at<void*>(self, 0x1c);
    if (old_target != nullptr) {
        fn_at<void(__fastcall*)(void*, void*)>(0x006952a0)(old_target, self);
    }
    field_at<void*>(self, 0x1c) = target;
    if (target != nullptr) {
        fn_at<void(__fastcall*)(void*, void*)>(0x00694a60)(target, self);
    }
}

// [0x006fb750] BSP_UnitDebugInspector_PrintFlooded
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = unit), RET 4.
void UnitDebugInspector_PrintFlooded_006fb750(void* self, void* unit) {
    const float flooded = fn_at<float(__thiscall*)(void*)>(0x0074fa80)(
        static_cast<std::uint8_t*>(unit) + 0x10d4
    );
    const float y = static_cast<float>(
        static_cast<double>(field_at<float>(self, 0x00)) + *reinterpret_cast<const double*>(0x00ce3938)
    );
    const float x = static_cast<float>(
        static_cast<double>(field_at<float>(self, 0x04)) + *reinterpret_cast<const double*>(0x00cfb980)
    );
    fn_at<void(__cdecl*)(void*, float, float, const char*, double)>(0x004c7070)(
        *reinterpret_cast<void**>(0x00e188a8),
        x,
        y,
        reinterpret_cast<const char*>(0x00cfb990),
        static_cast<double>(flooded)
    );
    field_at<float>(self, 0x00) = static_cast<float>(
        static_cast<double>(field_at<float>(self, 0x00)) + *reinterpret_cast<const double*>(0x00cfb988)
    );
}

// [0x006fb7c0] BSP_UnitDebugInspector_PrintLine
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = text), RET 4.
void UnitDebugInspector_PrintLine_006fb7c0(void* self, const char* text) {
    const float y_step = static_cast<float>(
        static_cast<double>(field_at<float>(self, 0x00)) + *reinterpret_cast<const double*>(0x00d7a308)
    );
    field_at<float>(self, 0x00) = y_step;
    const float y = static_cast<float>(
        static_cast<double>(y_step) + *reinterpret_cast<const double*>(0x00ce3938)
    );
    const float x0 = static_cast<float>(
        static_cast<double>(field_at<float>(self, 0x04)) + *reinterpret_cast<const double*>(0x00cfb980)
    );
    const float x = static_cast<float>(
        static_cast<double>(x0) - *reinterpret_cast<const double*>(0x00ce3dc0)
    );
    fn_at<void(__cdecl*)(void*, float, float, const char*, const char*)>(0x004c7070)(
        *reinterpret_cast<void**>(0x00e188a8),
        x,
        y,
        reinterpret_cast<const char*>(0x00ceb484),
        text
    );
    field_at<float>(self, 0x00) = static_cast<float>(
        static_cast<double>(field_at<float>(self, 0x00)) + *reinterpret_cast<const double*>(0x00cfb988)
    );
}

// [0x006fb8f0] BSP_UnitDebugInspector_ResetOnUnwind
// Original ABI: __thiscall (ECX = self), RET 0.
void UnitDebugInspector_ResetOnUnwind_006fb8f0(void* self) {
    *reinterpret_cast<void**>(0x00e19aac) = nullptr;
    field_at<std::uintptr_t>(self, 0x00) = 0x00ce3818u;
}

// [0x006fb920] BSP_UnitDebugInspector_Construct
// Original ABI: __thiscall (ECX = self), RET 0, returns self.
void* UnitDebugInspector_Construct_006fb920(void* self) {
    field_at<std::uint32_t>(self, 0x04) = 0u;
    field_at<std::uint32_t>(self, 0x08) = 0u;
    field_at<std::uint32_t>(self, 0x0c) = 0u;
    field_at<std::uint8_t>(self, 0x10) = 0u;
    field_at<std::uintptr_t>(self, 0x14) = 0x00cfb9a8u;
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfb9b0u;
    field_at<std::uintptr_t>(self, 0x14) = 0x00cfb9acu;
    field_at<std::uint8_t>(self, 0x18) = 0u;
    field_at<void*>(self, 0x1c) = nullptr;

    void* cb = fn_at<void*(__cdecl*)(std::size_t)>(0x00bf681b)(4u);
    if (cb != nullptr) {
        field_at<std::uintptr_t>(cb, 0x00) = 0x00cfb9a0u;
    }
    field_at<void*>(self, 0x20) = cb;
    fn_at<void(__thiscall*)(void*, void*)>(0x004d63d0)(*reinterpret_cast<void**>(0x00e188a8), cb);
    return self;
}

// [0x006fb9f0] BSP_UnitDebugInspector_Destruct
// Original ABI: __thiscall (ECX = self), RET 0.
void UnitDebugInspector_Destruct_006fb9f0(void* self) {
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfb9b0u;
    field_at<std::uintptr_t>(self, 0x14) = 0x00cfb9acu;
    void* cb = field_at<void*>(self, 0x20);
    if (cb != nullptr) {
        using DtorFn = void(__thiscall*)(void*, std::uint32_t);
        vslot_at<DtorFn>(cb, 0x00)(cb, 1u);
    }
    *reinterpret_cast<void**>(0x00e19aac) = nullptr;
    field_at<std::uintptr_t>(self, 0x14) = 0x00ce3818u;
    fn_at<void(__thiscall*)(void*)>(0x00695870)(self);
}

// [0x006fba70] BSP_Entity_AsKind04_006fba70
// Original ABI: __thiscall (ECX = self), RET 0.
void* Entity_AsKind04_006fba70(void* self) {
    if (self != nullptr) {
        using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
        if (vslot_at<IsKindFn>(self, 0x5c)(self, 4u) != 0u) {
            return self;
        }
    }
    return nullptr;
}

// [0x006fba90] BSP_Entity_AsKind04_006fba90
// Original ABI: __thiscall (ECX = self), RET 0.
void* Entity_AsKind04_006fba90(void* self) {
    if (self != nullptr) {
        using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
        if (vslot_at<IsKindFn>(self, 0x5c)(self, 4u) != 0u) {
            return self;
        }
    }
    return nullptr;
}

// [0x006fbad0] BSP_UnitDebugInspector_PrintUnitPanel
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = unit), RET 4.
void UnitDebugInspector_PrintUnitPanel_006fbad0(void* self, void* unit) {
    using GetNameFn = const char*(__thiscall*)(void*);
    const char* name = vslot_at<GetNameFn>(unit, 0x10)(unit);
    UnitDebugInspector_PrintLine_006fb7c0(self, name);

    using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
    if (vslot_at<IsKindFn>(unit, 0x5c)(unit, 6u) != 0u) {
        UnitDebugInspector_PrintFlooded_006fb750(self, unit);
    }

    std::uint32_t idx = 0u;
    while (true) {
        const std::uintptr_t begin = field_at<std::uintptr_t>(unit, 0x348);
        if (begin == 0u) {
            break;
        }
        const std::uint32_t count = static_cast<std::uint32_t>((field_at<std::uintptr_t>(unit, 0x34c) - begin) >> 2);
        if (idx >= count) {
            break;
        }
        if (begin == 0u || idx >= count) {
            fn_at<void(__cdecl*)()>(0x00bf6713)();
        }
        ++idx;
    }

    field_at<float>(self, 0x04) = static_cast<float>(
        static_cast<double>(field_at<float>(self, 0x04)) + *reinterpret_cast<const double*>(0x00ce3d88)
    );
    void* child = field_at<void*>(unit, 0x48);
    while (child != nullptr) {
        if (vslot_at<IsKindFn>(child, 0x5c)(child, 4u) != 0u) {
            UnitDebugInspector_PrintUnitPanel_006fbad0(self, child);
        }
        child = field_at<void*>(child, 0x44);
    }
    field_at<float>(self, 0x04) = static_cast<float>(
        static_cast<double>(field_at<float>(self, 0x04)) - *reinterpret_cast<const double*>(0x00ce3d88)
    );
}

// [0x006fbb80] BSP_UnitDebugInspector_Draw
// Original ABI: __thiscall (ECX = self), RET 0.
void UnitDebugInspector_Draw_006fbb80(void* self) {
    if (field_at<std::uint8_t>(self, 0x18) == 0u) {
        return;
    }
    void* target = field_at<void*>(self, 0x1c);
    if (target == nullptr) {
        return;
    }
    using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
    if (vslot_at<IsKindFn>(target, 0x5c)(target, 4u) == 0u) {
        return;
    }
    float cursor[2] = {0.0f, 0.0f};
    UnitDebugInspector_PrintUnitPanel_006fbad0(cursor, target);
}

// [0x006fbbc0] BSP_UnitDebugInspector_FrameCallback
// Original ABI: __thiscall (ECX = self), RET 0.
void UnitDebugInspector_FrameCallback_006fbbc0(void* /*self*/) {
    void* inspector = fn_at<void*(__cdecl*)()>(0x0051f5e0)();
    if (field_at<std::uint8_t>(inspector, 0x18) == 0u) {
        return;
    }
    void* target = field_at<void*>(inspector, 0x1c);
    if (target == nullptr) {
        return;
    }
    using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
    if (vslot_at<IsKindFn>(target, 0x5c)(target, 4u) == 0u) {
        return;
    }
    float cursor[2] = {0.0f, 0.0f};
    UnitDebugInspector_PrintUnitPanel_006fbad0(cursor, target);
}

// ============================================================================
// 3. ElevatorPlatform & OwnedPtrWithDefault (0x006fbc20..0x006fc2e0)
// ============================================================================

// [0x006fbc20] BSP_OwnedPtrWithDefault_Assign_006fbc20
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = new_ptr), RET 4.
void OwnedPtrWithDefault_Assign_006fbc20(void* self, void* new_ptr) {
    void* cur = field_at<void*>(self, 0x08);
    if (cur == new_ptr) {
        return;
    }
    void* embedded_default = static_cast<std::uint8_t*>(self) + 0x04;
    if (cur != embedded_default && cur != nullptr) {
        using DtorFn = void(__thiscall*)(void*, std::uint32_t);
        vslot_at<DtorFn>(cur, 0x00)(cur, 1u);
    }
    field_at<void*>(self, 0x08) = (new_ptr != nullptr) ? new_ptr : embedded_default;
}

// [0x006fbce0] BSP_ElevatorPlatform_StartTravel
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = downward_flag, [ESP+8] = elapsed_time), RET 8.
void ElevatorPlatform_StartTravel_006fbce0(void* self, std::uint8_t downward_flag, float elapsed_time) {
    // cc13-fixture5: the main thread runs this at 24-bit precision, so every x87
    // operation rounds to float (previously a double chain, and the constants were
    // read through their image addresses).
    constexpr float kMinTimeFraction_00ce65d0 = 0.4f;   // double 0.4000000059604645
    constexpr float kMinRate_00d7a358 = 0.01f;          // double 0.009999999776482582
    constexpr float kMinRateValue_00d7a238 = 0.01f;
    const float margin_1c = field_at<float>(self, 0x1c);
    const float depth_48 = field_at<float>(self, 0x48);
    const float speed_44 = field_at<float>(self, 0x44);
    // 006FBCFA-006FBD13: ((m + m) + d) / s.
    const float total_time = ((margin_1c + margin_1c) + depth_48) / speed_44;

    // 006FBD16-006FBD30: 0 > e takes 0 (a NaN is kept).
    const float clamped_elapsed = (0.0f > elapsed_time) ? 0.0f : elapsed_time;
    const float rem_time = total_time - clamped_elapsed;                    // 006FBD3B
    const float min_time = total_time * kMinTimeFraction_00ce65d0;          // 006FBD43, exact product
    const float denom_time = (min_time > rem_time) ? min_time : rem_time;   // 006FBD55

    // 006FBD69-006FBD85: down m + h, up (m + d) - h.
    const float height_18 = field_at<float>(self, 0x18);
    const float travel_dist = downward_flag != 0u ? margin_1c + height_18
                                                  : (margin_1c + depth_48) - height_18;

    const float nonneg_dist = (0.0f > travel_dist) ? 0.0f : travel_dist;   // 006FBD93 JA
    const float rate = (nonneg_dist / denom_time) / speed_44;              // 006FBDA9-006FBDAC
    // 006FBDB6: the double 0.01f > rate (FCOMIP / JBE, so a NaN rate is stored).
    field_at<float>(self, 0x4c) = kMinRate_00d7a358 > rate ? kMinRateValue_00d7a238 : rate;
}

// [0x006fbf00] BSP_ElevatorPlatform_ReadState
// Original ABI: __thiscall (ECX = self, stack 12-byte ArchiveReaderRef [w0, w1, reader_obj]), RET 0Ch.
void ElevatorPlatform_ReadState_006fbf00(
    void* self,
    std::uint32_t reader_w0,
    std::uint32_t reader_w1,
    void* reader_obj
) {
    struct ArchiveReaderRef12 {
        std::uint32_t w0;
        std::uint32_t w1;
        void* obj;
    } ref{reader_w0, reader_w1, reader_obj};

    // 006FBF26: ECX = LEA of the 12-byte by-value reader itself (its w0 is a vptr), not of ref.obj.
    ByValPair8 cargo_key{0u, 0x00cfb9e8u};
    fn_at<void(__fastcall*)(void*, void*, ByValPair8)>(0x006cf0a0)(
        &ref,
        static_cast<std::uint8_t*>(self) + 0x20,
        cargo_key
    );

    using ReadPropFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, void*);
    vslot_at<ReadPropFn>(ref.obj, 0x10)(
        ref.obj,
        0u,
        0x00cfb9dcu,
        5u,
        static_cast<std::uint8_t*>(self) + 0x38
    );
    vslot_at<ReadPropFn>(ref.obj, 0x10)(
        ref.obj,
        0u,
        0x00cfb9d0u,
        2u,
        static_cast<std::uint8_t*>(self) + 0x18
    );

    // 006FBF8E..006FBF9A: the "state" read is a vcall on the by-value reader itself (vtable = its w0,
    // ECX = its address), unlike the two reads above, which go through ref.obj.
    std::uint32_t state_val = 0u;
    using ReadTaggedFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, ByValPair8);
    vslot_at<ReadTaggedFn>(&ref, 0x10)(
        &ref,
        0u,
        0x00ce7324u,
        ByValPair8{1u, reinterpret_cast<std::uintptr_t>(&state_val)}
    );
    field_at<std::uint32_t>(self, 0x50) = state_val;
    fn_at<void(__thiscall*)(void*)>(0x00410630)(&ref);
}

// [0x006fbfd0] BSP_ElevatorPlatform_WriteState
// Original ABI: __thiscall (ECX = self, stack 12-byte ArchiveWriterRef [w0, w1, writer_obj]), RET 0Ch.
void ElevatorPlatform_WriteState_006fbfd0(
    void* self,
    std::uint32_t writer_w0,
    std::uint32_t writer_w1,
    void* writer_obj
) {
    struct ArchiveWriterRef12 {
        std::uint32_t w0;
        std::uint32_t w1;
        void* obj;
    } ref{writer_w0, writer_w1, writer_obj};

    struct ObserverCopy24 {
        std::uint32_t w[6];
    } obs_copy{};
    fn_at<void*(__thiscall*)(void*, const void*)>(0x006ce8a0)(
        &obs_copy,
        static_cast<const std::uint8_t*>(self) + 0x20
    );
    obs_copy.w[0] = 0x00cf89b4u;
    fn_at<void(__thiscall*)(void*, ObserverCopy24, ByValPair8)>(0x006cead0)(
        &ref.obj,
        obs_copy,
        ByValPair8{0u, 0x00cfb9e8u}
    );

    using WritePtrPropFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, const void*);
    vslot_at<WritePtrPropFn>(ref.obj, 0x0c)(
        ref.obj,
        0u,
        0x00cfb9dcu,
        5u,
        static_cast<const std::uint8_t*>(self) + 0x38
    );

    using WriteFloatPropFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, float);
    vslot_at<WriteFloatPropFn>(ref.obj, 0x0c)(
        ref.obj,
        0u,
        0x00cfb9d0u,
        2u,
        field_at<float>(self, 0x18)
    );

    using WriteTaggedFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, ByValPair8);
    vslot_at<WriteTaggedFn>(ref.obj, 0x0c)(
        ref.obj,
        0u,
        0x00ce7324u,
        ByValPair8{1u, field_at<std::uint32_t>(self, 0x50)}
    );
    fn_at<void(__thiscall*)(void*)>(0x004107b0)(&ref);
}

// [0x006fc2e0] BSP_ElevatorPlatform_Destruct
// Original ABI: __thiscall (ECX = self), RET 0.
void ElevatorPlatform_Destruct_006fc2e0(void* self) {
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfb9f4u;
    void* owned = field_at<void*>(self, 0x08);
    void* embedded_default = static_cast<std::uint8_t*>(self) + 0x04;
    if (owned != embedded_default && owned != nullptr) {
        using DtorFn = void(__thiscall*)(void*, std::uint32_t);
        vslot_at<DtorFn>(owned, 0x00)(owned, 1u);
    }
    void* ref_obj = field_at<void*>(self, 0x54);
    if (ref_obj != nullptr) {
        if (interlocked_dec_00ce2220(&field_at<volatile std::int32_t>(ref_obj, 0x04)) == 0) {
            using ReleaseDtorFn = void(__thiscall*)(void*);
            vslot_at<ReleaseDtorFn>(ref_obj, 0x00)(ref_obj);
        }
        field_at<void*>(self, 0x54) = nullptr;
    }
    fn_at<void(__thiscall*)(void*)>(0x006cdd70)(static_cast<std::uint8_t*>(self) + 0x20);
    field_at<std::uintptr_t>(embedded_default, 0x00) = 0x00cf88f8u;
}

// ============================================================================
// 4. DepthCharge / DEPTHCHARGE_CREATE & TurningGun Constructors (0x006fc960..0x006fdfc0)
// ============================================================================

// [0x006fc960] BSP_DepthChargeProjectile_Construct
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4, returns self.
void* DepthChargeProjectile_Construct_006fc960(void* self, void* arg0) {
    fn_at<void*(__thiscall*)(void*, void*)>(0x006e2670)(self, arg0);
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfba80u;
    field_at<std::uintptr_t>(self, 0x10) = 0x00cfba64u;
    field_at<std::uintptr_t>(self, 0x24) = 0x00cfba5cu;
    field_at<std::uintptr_t>(self, 0x170) = 0x00cfba58u;
    field_at<std::uintptr_t>(self, 0x1e4) = 0x00cfba50u;
    field_at<std::uintptr_t>(self, 0x310) = 0x00cfba10u;
    field_at<std::uintptr_t>(self, 0x3e4) = 0x00cfb9f8u;
    field_at<std::uint32_t>(self, 0xc4) = 0x2cu;
    return self;
}

// [0x006fc9d0] BSP_DepthCharge_VSlot198_ForwardTo194
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4.
void DepthCharge_VSlot198_ForwardTo194_006fc9d0(void* self, float arg0) {
    using VSlot194Fn = void(__thiscall*)(void*, float);
    vslot_at<VSlot194Fn>(self, 0x194)(self, arg0);
}

// [0x006fc9f0] BSP_SessionMessage_DEPTHCHARGE_CREATE_ApplyFields
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = entity), RET 4.
void SessionMessage_DEPTHCHARGE_CREATE_ApplyFields_006fc9f0(const void* self, void* entity) {
    field_at<float>(entity, 0x468) = field_at<float>(self, 0x120);
    field_at<float>(entity, 0x470) = field_at<float>(self, 0x124);
}

// [0x006fca50] BSP_Random_UniformBetweenB4andB8_006fca50
// Original ABI: __thiscall (ECX = self), RET 0, returns ST0.
double Random_UniformBetweenB4andB8_006fca50(const void* self) {
    const float lo = field_at<float>(self, 0xb4);
    const float hi = field_at<float>(self, 0xb8);
    return fn_at<double(__thiscall*)(std::uintptr_t, float, float)>(0x00bd2f10)(1u, lo, hi);
}

// [0x006fcb60] BSP_SessionMessage_DEPTHCHARGE_CREATE_Write
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = bit_stream), RET 4.
void SessionMessage_DEPTHCHARGE_CREATE_Write_006fcb60(void* self, void* bit_stream) {
    fn_at<void(__thiscall*)(void*, void*)>(0x006e0690)(self, bit_stream);
    const float scale = *reinterpret_cast<const float*>(0x00d7a248);
    using WriteFloatFn = void(__thiscall*)(void*, float, std::uint32_t, std::uint32_t, float, std::uint32_t);
    WriteFloatFn write_float = fn_at<WriteFloatFn>(0x004295c0);
    write_float(bit_stream, field_at<float>(self, 0x120), 0u, 1u, scale, 0x20u);
    write_float(bit_stream, field_at<float>(self, 0x124), 0u, 1u, scale, 0x20u);
}

// [0x006fd0d0] BSP_DepthCharge_SendCreateMessage
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = peer_arg), RET 4.
void DepthCharge_SendCreateMessage_006fd0d0(void* self, std::uint32_t peer_arg) {
    std::uint8_t msg[0x128]{};
    void* cls = field_at<void*>(self, 0x314);
    const std::uint32_t cls_id = field_at<std::uint32_t>(cls, 0x0c);
    fn_at<void*(__thiscall*)(void*, std::uint32_t, std::uint32_t, void*)>(0x006e0d20)(
        msg,
        0xb7u,
        cls_id,
        self
    );
    field_at<float>(msg, 0x120) = field_at<float>(self, 0x468);
    field_at<std::uintptr_t>(msg, 0x00) = 0x00cfbc28u;
    field_at<float>(msg, 0x124) = field_at<float>(self, 0x470);
    fn_at<void(__thiscall*)(void*, std::uint32_t, void*)>(0x00779fc0)(self, peer_arg, msg);
    field_at<std::uintptr_t>(msg, 0x00) = 0x00cf9324u;
    fn_at<void(__thiscall*)(void*)>(0x007668e0)(msg);
}

// [0x006fd190] BSP_DepthCharge_RegisterInWorldLists
// Original ABI: __thiscall (ECX = self), RET 0.
void DepthCharge_RegisterInWorldLists_006fd190(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x00928560)(self);
    using PushBackFn = void(__thiscall*)(void*, void*);
    PushBackFn push_back = fn_at<PushBackFn>(0x00484540);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x30, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x210, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x228, self);
}

// [0x006fd1d0] BSP_DepthCharge_DetachFromWorldLists
// Original ABI: __thiscall (ECX = self), RET 0.
void DepthCharge_DetachFromWorldLists_006fd1d0(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x006e29c0)(self);
    void* parent = field_at<void*>(self, 0x30);
    detach_from_intrusive_list(static_cast<std::uint8_t*>(parent) + 0x228, 0x04, self);
}

// [0x006fded0] BSP_RapidTurningGun_Construct
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4, returns self.
void* RapidTurningGun_Construct_006fded0(void* self, void* arg0) {
    fn_at<void*(__thiscall*)(void*, void*)>(0x006fdda0)(self, arg0);
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfbf58u;
    field_at<std::uintptr_t>(self, 0x10) = 0x00cfbf3cu;
    field_at<std::uintptr_t>(self, 0x24) = 0x00cfbf34u;
    field_at<std::uintptr_t>(self, 0x170) = 0x00cfbf30u;
    field_at<std::uintptr_t>(self, 0x1e4) = 0x00cfbf28u;
    field_at<std::uintptr_t>(self, 0x310) = 0x00cfbf10u;
    field_at<std::uint32_t>(self, 0xc4) = 0x24u;
    return self;
}

// [0x006fdfc0] BSP_SingleTurningGun_Construct
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4, returns self.
void* SingleTurningGun_Construct_006fdfc0(void* self, void* arg0) {
    RapidTurningGun_Construct_006fded0(self, arg0);
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfc190u;
    field_at<std::uintptr_t>(self, 0x10) = 0x00cfc178u;
    field_at<std::uintptr_t>(self, 0x24) = 0x00cfc170u;
    field_at<std::uintptr_t>(self, 0x170) = 0x00cfc16cu;
    field_at<std::uintptr_t>(self, 0x1e4) = 0x00cfc164u;
    field_at<std::uintptr_t>(self, 0x310) = 0x00cfc14cu;
    field_at<std::uint8_t>(self, 0x4d8) = 0u;
    field_at<std::uint32_t>(self, 0xc4) = 0x27u;
    return self;
}

// ============================================================================
// 5. DummyKamikazePlane & Dummy2D (0x006fe730..0x006ff570)
// ============================================================================

// [0x006fe730] BSP_DummyKamikazePlaneClass_ResolveLinkedClass
// Original ABI: __thiscall (ECX = self), RET 0 (tail-jumps to linked_class vslot +14h).
void DummyKamikazePlaneClass_ResolveLinkedClass_006fe730(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x006e97f0)(self);
    void* linked = field_at<void*>(self, 0xdc);
    using VSlot10Fn = void(__thiscall*)(void*, std::uint32_t);
    vslot_at<VSlot10Fn>(linked, 0x10)(linked, 0u);
    using VSlot14Fn = void(__thiscall*)(void*);
    vslot_at<VSlot14Fn>(linked, 0x14)(linked);
}

// [0x006fe780] BSP_LuaTable_ReadNumberOrDefault
// Original ABI: __fastcall (ECX = key_name, EDX = out_val, stack 20B LuaObject + 4B default_val), RET 18h.
void LuaTable_ReadNumberOrDefault_006fe780(
    const char* key_name,
    float* out_val,
    void* lua_table_by_val,
    float default_val
) {
    ByValLuaObj20 sub_obj{};
    fn_at<void*(__thiscall*)(void*, void*, const char*)>(0x00b67800)(
        lua_table_by_val,
        &sub_obj,
        key_name
    );
    float result = default_val;
    if (fn_at<std::uint8_t(__thiscall*)(void*)>(0x00b65fb0)(&sub_obj) == 0u) {
        result = static_cast<float>(fn_at<double(__thiscall*)(void*)>(0x00b66270)(&sub_obj));
    }
    *out_val = result;
    fn_at<void(__thiscall*)(void*)>(0x00b67700)(&sub_obj);
    fn_at<void(__thiscall*)(void*)>(0x00b67700)(lua_table_by_val);
}

// [0x006fe820] BSP_DummyKamikazePlane_InitAll
// Original ABI: __thiscall (ECX = self), RET 0.
void DummyKamikazePlane_InitAll_006fe820(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x0077fad0)(self);
    fn_at<void(__thiscall*)(void*, std::uint32_t)>(0x006e0b40)(self, 1u);
    void* owner = climb_owner_chain_to_kind5(self);
    if (owner != nullptr) {
        field_at<std::uint32_t>(self, 0x54) = field_at<std::uint32_t>(owner, 0x54);
        using VSlot12CFn = std::uint32_t(__thiscall*)(void*);
        field_at<std::uint32_t>(self, 0x4a0) = vslot_at<VSlot12CFn>(owner, 0x12c)(owner);
        field_at<std::uint32_t>(self, 0x180) = field_at<std::uint32_t>(owner, 0x180);
    }
}

// [0x006fe8b0] BSP_Dummy2D_Destruct
// Original ABI: __thiscall (ECX = self), RET 0.
void Dummy2D_Destruct_006fe8b0(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x0042bca0)(static_cast<std::uint8_t*>(self) + 0x488);
    fn_at<void(__thiscall*)(void*)>(0x006e0860)(static_cast<std::uint8_t*>(self) + 0x468);
    fn_at<void(__thiscall*)(void*)>(0x006e2a20)(self);
}

// [0x006fe950] BSP_DummyKamikazePlane_Construct
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4, returns self.
void* DummyKamikazePlane_Construct_006fe950(void* self, void* arg0) {
    Dummy2D_Construct_006ff3f0(self, arg0);
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfc698u;
    field_at<std::uintptr_t>(self, 0x10) = 0x00cfc67cu;
    field_at<std::uintptr_t>(self, 0x24) = 0x00cfc674u;
    field_at<std::uintptr_t>(self, 0x170) = 0x00cfc670u;
    field_at<std::uintptr_t>(self, 0x1e4) = 0x00cfc668u;
    field_at<std::uintptr_t>(self, 0x310) = 0x00cfc628u;
    field_at<std::uintptr_t>(self, 0x3e4) = 0x00cfc610u;
    field_at<std::uint32_t>(self, 0xc4) = 0x2fu;
    return self;
}

// [0x006fea80] BSP_Dummy2D_RegisterInWorldLists
// Original ABI: __thiscall (ECX = self), RET 0.
void Dummy2D_RegisterInWorldLists_006fea80(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x00928560)(self);
    using PushBackFn = void(__thiscall*)(void*, void*);
    PushBackFn push_back = fn_at<PushBackFn>(0x00484540);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x30, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x210, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x234, self);
}

// [0x006feac0] BSP_Dummy2D_DetachFromWorldLists
// Original ABI: __thiscall (ECX = self), RET 0.
void Dummy2D_DetachFromWorldLists_006feac0(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x006e29c0)(self);
    void* parent = field_at<void*>(self, 0x30);
    detach_from_intrusive_list(static_cast<std::uint8_t*>(parent) + 0x234, 0x04, self);
}

// [0x006feb00] BSP_Dummy2D_GetName
// Original ABI: __thiscall (ECX = self), RET 0.
const char* Dummy2D_GetName_006feb00(const void* self) {
    const char* name = nullptr;
    if (field_at<std::uint32_t>(self, 0x15c) != 0u) {
        name = field_at<const char*>(self, 0x160);
    } else {
        const void* cls = field_at<const void*>(self, 0x314);
        name = field_at<const char*>(cls, 0x14);
    }
    return (name != nullptr) ? name : reinterpret_cast<const char*>(0x00e19ab4);
}

// [0x006feb30] BSP_DummyKamikazePlane_RegisterInWorldLists
// Original ABI: __thiscall (ECX = self), RET 0.
void DummyKamikazePlane_RegisterInWorldLists_006feb30(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x00928560)(self);
    using PushBackFn = void(__thiscall*)(void*, void*);
    PushBackFn push_back = fn_at<PushBackFn>(0x00484540);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x30, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x210, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x234, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x24c, self);
}

// [0x006feb80] BSP_DummyKamikazePlane_DetachFromWorldLists
// Original ABI: __thiscall (ECX = self), RET 0.
void DummyKamikazePlane_DetachFromWorldLists_006feb80(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x006e29c0)(self);
    void* parent = field_at<void*>(self, 0x30);
    detach_from_intrusive_list(static_cast<std::uint8_t*>(parent) + 0x234, 0x04, self);
    parent = field_at<void*>(self, 0x30);
    detach_from_intrusive_list(static_cast<std::uint8_t*>(parent) + 0x24c, 0x04, self);
}

// [0x006ff100] BSP_DummyKamikazePlaneClass_CreateEntity
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4.
void* DummyKamikazePlaneClass_CreateEntity_006ff100(void* self, void* arg0) {
    void* mem = fn_at<void*(__cdecl*)(std::size_t)>(0x00bf55be)(0x4a4u);
    fn_at<void*(__cdecl*)(void*, int, std::size_t)>(0x00bf79f0)(mem, 0, 0x4a4u);
    void* entity = nullptr;
    if (mem != nullptr) {
        entity = DummyKamikazePlane_Construct_006fe950(mem, arg0);
    }
    field_at<void*>(entity, 0x314) = self;
    return entity;
}

// [0x006ff2f0] BSP_DummyShot_BindOwnerCommand
// Original ABI: __thiscall (ECX = self [shot subobject at entity + 0x310], stack [ESP+4] = arg0, [ESP+8] = arg1), RET 8.
void DummyShot_BindOwnerCommand_006ff2f0(void* self, std::uint32_t arg0, std::uint32_t arg1) {
    void* entity = static_cast<std::uint8_t*>(self) - 0x310;
    void* owner = climb_owner_chain_to_kind5(entity);
    if (owner != nullptr) {
        using GetMasterFn = void*(__thiscall*)(void*);
        if (vslot_at<GetMasterFn>(owner, 0x140)(owner) != nullptr) {
            void* master = vslot_at<GetMasterFn>(owner, 0x140)(owner);
            using GetCmdFn = void*(__thiscall*)(void*);
            void* cmd = vslot_at<GetCmdFn>(master, 0x114)(master);
            if (cmd != nullptr) {
                using GetCmdTargetFn = void*(__thiscall*)(void*);
                void* cmd_target = vslot_at<GetCmdTargetFn>(cmd, 0x18)(cmd);
                fn_at<void*(__thiscall*)(void*, void*)>(0x006e1820)(
                    static_cast<std::uint8_t*>(self) + 0x158,
                    cmd_target
                );
            }
        }
        const float one = *reinterpret_cast<const float*>(0x00d7a24c);
        const std::uint8_t global_override = *reinterpret_cast<const std::uint8_t*>(0x00f87152);
        float scale_a = one;
        if (global_override == 0u && field_at<std::uint8_t>(owner, 0x2f0) == 0u) {
            scale_a = field_at<float>(owner, 0x2f4);
        }
        field_at<float>(self, -0x1c) = scale_a;

        float scale_b = one;
        if (global_override == 0u && field_at<std::uint8_t>(owner, 0x2f0) == 0u) {
            scale_b = field_at<float>(owner, 0x2f8);
        }
        field_at<float>(self, -0x18) = scale_b;
    }
    fn_at<void(__thiscall*)(void*, std::uint32_t, std::uint32_t)>(0x006e1f00)(self, arg0, arg1);
}

// [0x006ff3f0] BSP_Dummy2D_Construct
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4, returns self.
void* Dummy2D_Construct_006ff3f0(void* self, void* arg0) {
    fn_at<void*(__thiscall*)(void*, void*)>(0x006e2670)(self, arg0);
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfc910u;
    field_at<std::uintptr_t>(self, 0x10) = 0x00cfc8f4u;
    field_at<std::uintptr_t>(self, 0x24) = 0x00cfc8ecu;
    field_at<std::uintptr_t>(self, 0x170) = 0x00cfc8e8u;
    field_at<std::uintptr_t>(self, 0x1e4) = 0x00cfc8e0u;
    field_at<std::uintptr_t>(self, 0x310) = 0x00cfc8a0u;
    field_at<std::uintptr_t>(self, 0x3e4) = 0x00cfc888u;
    field_at<std::uint32_t>(self, 0x46c) = 0u;
    field_at<std::uint32_t>(self, 0x470) = 0u;
    field_at<std::uint32_t>(self, 0x474) = 0u;
    field_at<std::uint32_t>(self, 0x47c) = 0u;
    field_at<std::uint8_t>(self, 0x478) = 1u;
    field_at<std::uintptr_t>(self, 0x468) = 0x00cf938cu;

    void* obs = static_cast<std::uint8_t*>(self) + 0x488;
    field_at<std::uint32_t>(obs, 0x04) = 0u;
    field_at<std::uint32_t>(obs, 0x08) = 0u;
    field_at<std::uint32_t>(obs, 0x0c) = 0u;
    field_at<void*>(obs, 0x14) = nullptr;
    field_at<std::uint8_t>(obs, 0x10) = 1u;
    field_at<std::uintptr_t>(obs, 0x00) = 0x00ce3cf0u;
    field_at<std::uint32_t>(self, 0xc4) = 0x2du;

    void* cur_obs_target = field_at<void*>(obs, 0x14);
    if (cur_obs_target != nullptr) {
        fn_at<void(__fastcall*)(void*, void*)>(0x006952a0)(cur_obs_target, obs);
        field_at<void*>(obs, 0x14) = nullptr;
    }

    std::int32_t& serial_counter = *reinterpret_cast<std::int32_t*>(0x00e19abc);
    field_at<std::int32_t>(self, 0x484) = serial_counter;
    if (serial_counter >= 0xfa) {
        serial_counter = 0;
    } else {
        serial_counter += 1;
    }
    return self;
}

// [0x006ff570] BSP_Dummy2D_Activate
// Original ABI: __thiscall (ECX = self), RET 0.
void Dummy2D_Activate_006ff570(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x006e2e10)(self);
    field_at<float>(self, 0x480) = *reinterpret_cast<const float*>(0x00d7a260);
    void* obs = static_cast<std::uint8_t*>(self) + 0x488;
    void* old_target = field_at<void*>(obs, 0x14);
    if (old_target != nullptr) {
        fn_at<void(__fastcall*)(void*, void*)>(0x006952a0)(old_target, obs);
        field_at<void*>(obs, 0x14) = nullptr;
    }

    struct NativeStr8 {
        std::uint32_t size{0};
        char* data{nullptr};
    } key_str{};
    fn_at<void(__thiscall*)(void*, std::uint32_t, std::uint32_t)>(0x0041dd40)(&key_str, 7u, 1u);
    if (key_str.data != nullptr) {
        fn_at<void*(__cdecl*)(void*, const void*, std::size_t)>(0x00bf7680)(
            key_str.data,
            reinterpret_cast<const void*>(0x00cfcab8),
            key_str.size + 1u
        );
    }

    ByValLuaObj20 lua_obj{};
    void* lua_tbl = fn_at<void*(__thiscall*)(void*, void*)>(0x00927b40)(self, &lua_obj);
    fn_at<void(__thiscall*)(void*, void*, std::uint32_t)>(0x00b67460)(
        lua_tbl,
        &key_str,
        field_at<std::uint32_t>(self, 0x484)
    );
    fn_at<void(__thiscall*)(void*)>(0x00b67700)(&lua_obj);

    if (key_str.data != nullptr) {
        // (data, size + 1, 1) are pushed first; 00419CC0 takes no arguments and returns the pool
        // singleton, which becomes ECX for 00BD1510 ReturnBlock(ptr, size, align), RET 0Ch
        // (006FF639..006FF647 and 0070131C..0070132A).
        void* pool = fn_at<void*(__cdecl*)()>(0x00419cc0)();
        fn_at<void(__thiscall*)(void*, void*, std::uint32_t, std::uint32_t)>(0x00bd1510)(
            pool, key_str.data, key_str.size + 1u, 1u);
    }
}

// ============================================================================
// 6. DummySubmarine (0x006ff6a0..0x006fffe0)
// ============================================================================

// [0x006ff6a0] BSP_DummySubmarineClass_VSlot28_Finalise
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4.
void DummySubmarineClass_VSlot28_Finalise_006ff6a0(void* self, std::uint32_t arg0) {
    fn_at<void(__thiscall*)(void*)>(0x006e97f0)(self);
    void* linked = field_at<void*>(self, 0xdc);
    using VSlot10Fn = void(__thiscall*)(void*, std::uint32_t);
    vslot_at<VSlot10Fn>(linked, 0x10)(linked, arg0);
    using VSlot14Fn = void(__thiscall*)(void*);
    vslot_at<VSlot14Fn>(linked, 0x14)(linked);
}

// [0x006ff700] BSP_DummySubmarine_VSlot198_ForwardTo194
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4.
void DummySubmarine_VSlot198_ForwardTo194_006ff700(void* self, float arg0) {
    using VSlot194Fn = void(__thiscall*)(void*, float);
    vslot_at<VSlot194Fn>(self, 0x194)(self, arg0);
}

// [0x006ff730] BSP_DummySubmarine_InitAll
// Original ABI: __thiscall (ECX = self), RET 0.
void DummySubmarine_InitAll_006ff730(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x0077fad0)(self);
    fn_at<void(__thiscall*)(void*, std::uint32_t)>(0x006e0b40)(self, 1u);
    void* owner = climb_owner_chain_to_kind5(self);
    if (owner != nullptr) {
        field_at<std::uint32_t>(self, 0x54) = field_at<std::uint32_t>(owner, 0x54);
        using VSlot12CFn = std::uint32_t(__thiscall*)(void*);
        field_at<std::uint32_t>(self, 0x4b8) = vslot_at<VSlot12CFn>(owner, 0x12c)(owner);
        field_at<std::uint32_t>(self, 0x180) = field_at<std::uint32_t>(owner, 0x180);
    }
}

// [0x006ff7c0] BSP_ObserverMember_SetTarget_006ff7c0
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = target), RET 4, returns self.
void* ObserverMember_SetTarget_006ff7c0(void* self, void* target) {
    void* cur = field_at<void*>(self, 0x14);
    if (cur != target) {
        if (cur != nullptr) {
            fn_at<void(__fastcall*)(void*, void*)>(0x006952a0)(cur, self);
        }
        field_at<void*>(self, 0x14) = target;
        if (target != nullptr) {
            fn_at<void(__fastcall*)(void*, void*)>(0x00694a60)(target, self);
        }
    }
    return self;
}

// [0x006ff800] BSP_DummySubmarine_Construct
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4, returns self.
void* DummySubmarine_Construct_006ff800(void* self, void* arg0) {
    Dummy2D_Construct_006ff3f0(self, arg0);
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfcb48u;
    field_at<std::uintptr_t>(self, 0x10) = 0x00cfcb2cu;
    field_at<std::uintptr_t>(self, 0x24) = 0x00cfcb24u;
    field_at<std::uintptr_t>(self, 0x170) = 0x00cfcb20u;
    field_at<std::uintptr_t>(self, 0x1e4) = 0x00cfcb18u;
    field_at<std::uintptr_t>(self, 0x310) = 0x00cfcad8u;
    field_at<std::uintptr_t>(self, 0x3e4) = 0x00cfcac0u;
    field_at<std::uint32_t>(self, 0x4a4) = 0u;
    field_at<std::uint32_t>(self, 0x4a8) = 0u;
    field_at<std::uint32_t>(self, 0x4ac) = 0u;
    field_at<std::uint32_t>(self, 0x4b4) = 0u;
    field_at<std::uint8_t>(self, 0x4b0) = 1u;
    field_at<std::uintptr_t>(self, 0x4a0) = 0x00ce3cf0u;
    field_at<std::uint32_t>(self, 0xc4) = 0x30u;
    return self;
}

// [0x006ff890] BSP_DummySubmarineShot_IsInWater_True
// Original ABI: __thiscall (ECX = self), RET 0.
std::uint32_t DummySubmarineShot_IsInWater_True_006ff890(const void* /*self*/) {
    return 1u;
}

// [0x006ff920] BSP_DummySubmarine_DestructBody
// Original ABI: __thiscall (ECX = self), RET 0.
void DummySubmarine_DestructBody_006ff920(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x0042bca0)(static_cast<std::uint8_t*>(self) + 0x4a0);
    Dummy2D_Destruct_006fe8b0(self);
}

// [0x006ff970] BSP_DummySubmarine_RegisterInWorldLists
// Original ABI: __thiscall (ECX = self), RET 0.
void DummySubmarine_RegisterInWorldLists_006ff970(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x00928560)(self);
    using PushBackFn = void(__thiscall*)(void*, void*);
    PushBackFn push_back = fn_at<PushBackFn>(0x00484540);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x30, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x210, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x234, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x258, self);
}

// [0x006ff9c0] BSP_DummySubmarine_DetachFromWorldLists
// Original ABI: __thiscall (ECX = self), RET 0.
void DummySubmarine_DetachFromWorldLists_006ff9c0(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x006e29c0)(self);
    void* parent = field_at<void*>(self, 0x30);
    detach_from_intrusive_list(static_cast<std::uint8_t*>(parent) + 0x234, 0x04, self);
    parent = field_at<void*>(self, 0x30);
    detach_from_intrusive_list(static_cast<std::uint8_t*>(parent) + 0x258, 0x04, self);
}

// [0x006fffe0] BSP_DummySubmarineClass_CreateEntity
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4.
void* DummySubmarineClass_CreateEntity_006fffe0(void* self, void* arg0) {
    void* mem = fn_at<void*(__cdecl*)(std::size_t)>(0x00bf55be)(0x4bcu);
    fn_at<void*(__cdecl*)(void*, int, std::size_t)>(0x00bf79f0)(mem, 0, 0x4bcu);
    void* entity = nullptr;
    if (mem != nullptr) {
        entity = DummySubmarine_Construct_006ff800(mem, arg0);
    }
    field_at<void*>(entity, 0x314) = self;
    return entity;
}

// ============================================================================
// 7. DummyTarget & DummyTargetVehicle (0x00700150..0x007016c0)
// ============================================================================

// [0x00700150] BSP_DummyTargetBulletClass_ResolveLinkedClass
// Original ABI: __thiscall (ECX = self), RET 0 (tail-jumps to linked_class vslot +14h).
void DummyTargetBulletClass_ResolveLinkedClass_00700150(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x006e97f0)(self);
    void* linked = field_at<void*>(self, 0xe0);
    using VSlot10Fn = void(__thiscall*)(void*, std::uint32_t);
    vslot_at<VSlot10Fn>(linked, 0x10)(linked, 0u);
    using VSlot14Fn = void(__thiscall*)(void*);
    vslot_at<VSlot14Fn>(linked, 0x14)(linked);
}

// [0x00700200] BSP_DummyTargetVehicleClass_ReconCategory13
// Original ABI: __thiscall (ECX = self), RET 0.
std::uint32_t DummyTargetVehicleClass_ReconCategory13_00700200(const void* /*self*/) {
    return 0x13u;
}

// [0x00700250] BSP_DummyTargetVehicleClass_ReconMarkDirty
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = recon_slot, [ESP+8] = arg1, [ESP+0Ch] = arg2), RET 0Ch.
void DummyTargetVehicleClass_ReconMarkDirty_00700250(
    void* /*self*/,
    void* recon_slot,
    std::uint32_t /*arg1*/,
    std::uint32_t /*arg2*/
) {
    fn_at<void(__thiscall*)(void*)>(0x00803ba0)(recon_slot);
}

// [0x00700260] BSP_DummyTargetVehicle_Activate
// Original ABI: __thiscall (ECX = self), RET 0.
void DummyTargetVehicle_Activate_00700260(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x00955420)(self);
    void* anim_ctrl = field_at<void*>(self, 0x4a4);
    using VSlot24Fn = void(__thiscall*)(void*, std::uint32_t, float, std::uint32_t);
    vslot_at<VSlot24Fn>(anim_ctrl, 0x24)(anim_ctrl, 0u, 0.0f, 1u);
}

// [0x00700290] BSP_LuaObject_ReadNumberFieldOr_00700290
// Original ABI: __fastcall (ECX = key_name, EDX = out_val, stack 20B LuaObject + 4B default_val), RET 18h.
void LuaObject_ReadNumberFieldOr_00700290(
    const char* key_name,
    float* out_val,
    void* lua_table_by_val,
    float default_val
) {
    ByValLuaObj20 sub_obj{};
    fn_at<void*(__thiscall*)(void*, void*, const char*)>(0x00b67800)(
        lua_table_by_val,
        &sub_obj,
        key_name
    );
    float result = default_val;
    if (fn_at<std::uint8_t(__thiscall*)(void*)>(0x00b65fb0)(&sub_obj) == 0u) {
        result = static_cast<float>(fn_at<double(__thiscall*)(void*)>(0x00b66270)(&sub_obj));
    }
    *out_val = result;
    fn_at<void(__thiscall*)(void*)>(0x00b67700)(&sub_obj);
    fn_at<void(__thiscall*)(void*)>(0x00b67700)(lua_table_by_val);
}

// [0x00700330] BSP_LuaObject_ReadNumberField_00700330
// Original ABI: __fastcall (ECX = key_name, EDX = out_val, stack 20B LuaObject), RET 14h.
void LuaObject_ReadNumberField_00700330(
    const char* key_name,
    float* out_val,
    void* lua_table_by_val
) {
    ByValLuaObj20 sub_obj{};
    void* obj_ptr = fn_at<void*(__thiscall*)(void*, void*, const char*)>(0x00b67800)(
        lua_table_by_val,
        &sub_obj,
        key_name
    );
    *out_val = static_cast<float>(fn_at<double(__thiscall*)(void*)>(0x00b66270)(obj_ptr));
    fn_at<void(__thiscall*)(void*)>(0x00b67700)(&sub_obj);
    fn_at<void(__thiscall*)(void*)>(0x00b67700)(lua_table_by_val);
}

// [0x007003b0] BSP_LuaObject_ReadNumberField_007003b0
// Original ABI: __fastcall (ECX = key_name, EDX = out_val, stack 20B LuaObject), RET 14h.
void LuaObject_ReadNumberField_007003b0(
    const char* key_name,
    float* out_val,
    void* lua_table_by_val
) {
    ByValLuaObj20 sub_obj{};
    void* obj_ptr = fn_at<void*(__thiscall*)(void*, void*, const char*)>(0x00b67800)(
        lua_table_by_val,
        &sub_obj,
        key_name
    );
    *out_val = static_cast<float>(fn_at<double(__thiscall*)(void*)>(0x00b66270)(obj_ptr));
    fn_at<void(__thiscall*)(void*)>(0x00b67700)(&sub_obj);
    fn_at<void(__thiscall*)(void*)>(0x00b67700)(lua_table_by_val);
}

// [0x00700520] BSP_DummyTarget_Activate
// Original ABI: __thiscall (ECX = self), RET 0.
void DummyTarget_Activate_00700520(void* self) {
    Dummy2D_Activate_006ff570(self);
    fn_at<void(__thiscall*)(void*, std::uint32_t)>(0x006e0b40)(self, 0u);
    void* owner = climb_owner_chain_to_kind5(self);
    if (owner != nullptr) {
        field_at<std::uint32_t>(self, 0x54) = field_at<std::uint32_t>(owner, 0x54);
    }
}

// [0x00700590] BSP_DummyTargetVehicle_Construct
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4, returns self.
void* DummyTargetVehicle_Construct_00700590(void* self, void* arg0) {
    fn_at<void*(__thiscall*)(void*, void*, std::uint32_t)>(0x0095cc90)(self, arg0, 0u);
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfcd60u;
    field_at<std::uintptr_t>(self, 0x10) = 0x00cfcd48u;
    field_at<std::uintptr_t>(self, 0x24) = 0x00cfcd40u;
    field_at<std::uintptr_t>(self, 0x170) = 0x00cfcd3cu;
    field_at<std::uintptr_t>(self, 0x1e4) = 0x00cfcd34u;
    field_at<std::uintptr_t>(self, 0x310) = 0x00cfcd1cu;
    field_at<std::uintptr_t>(self, 0x38c) = 0x00cfcd18u;
    field_at<std::uint32_t>(self, 0xc4) = 0x35u;
    field_at<float>(self, 0x730) = 0.0f;
    return self;
}

// [0x00700840] BSP_DummyTargetVehicle_GetName
// Original ABI: __thiscall (ECX = self), RET 0.
const char* DummyTargetVehicle_GetName_00700840(const void* self) {
    if (field_at<std::uint32_t>(self, 0x15c) != 0u) {
        const char* name = field_at<const char*>(self, 0x160);
        return (name != nullptr) ? name : reinterpret_cast<const char*>(0x00e19ac2);
    }
    const void* cls = field_at<const void*>(self, 0x538);
    return field_at<const char*>(cls, 0x54);
}

// [0x00700870] BSP_DummyTargetVehicle_Serialize
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = archive_writer), RET 4.
void DummyTargetVehicle_Serialize_00700870(void* self, void* archive_writer) {
    fn_at<void(__thiscall*)(void*, void*)>(0x0095bb10)(self, archive_writer);
    using BeginSectionFn = void(__thiscall*)(void*, ByValPair8);
    vslot_at<BeginSectionFn>(archive_writer, 0x04)(archive_writer, ByValPair8{0u, 0x00cfcf7cu});

    ByValPair8 default_open{2u, *reinterpret_cast<const std::uint32_t*>(0x00d7a24c)};
    ByValPair8 cur_open{2u, field_at<std::uint32_t>(self, 0x730)};
    if (fn_at<std::uint8_t(__thiscall*)(const void*, const void*)>(0x00bd5680)(&cur_open, &default_open) == 0u) {
        using WriteScalarFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, std::uint32_t);
        vslot_at<WriteScalarFn>(archive_writer, 0x0c)(
            archive_writer,
            0u,
            0x00cfcf70u,
            cur_open.w0,
            cur_open.w1
        );
    }

    using WritePairFn = void(__thiscall*)(void*, ByValPair8, ByValPair8);
    vslot_at<WritePairFn>(archive_writer, 0x0c)(
        archive_writer,
        ByValPair8{0u, 0x00cfcf64u},
        ByValPair8{5u, reinterpret_cast<std::uintptr_t>(static_cast<std::uint8_t*>(self) + 0x734)}
    );

    using EndSectionFn = void(__thiscall*)(void*);
    vslot_at<EndSectionFn>(archive_writer, 0x08)(archive_writer);
}

// [0x00700940] BSP_ObserverMember_SetTarget_00700940
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = target), RET 4, returns self.
void* ObserverMember_SetTarget_00700940(void* self, void* target) {
    void* cur = field_at<void*>(self, 0x14);
    if (cur != target) {
        if (cur != nullptr) {
            fn_at<void(__fastcall*)(void*, void*)>(0x006952a0)(cur, self);
        }
        field_at<void*>(self, 0x14) = target;
        if (target != nullptr) {
            fn_at<void(__fastcall*)(void*, void*)>(0x00694a60)(target, self);
        }
    }
    return self;
}

// [0x00700980] BSP_DummyTargetVehicle_RegisterInWorldLists
// Original ABI: __thiscall (ECX = self), RET 0.
void DummyTargetVehicle_RegisterInWorldLists_00700980(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x00928560)(self);
    using PushBackFn = void(__thiscall*)(void*, void*);
    PushBackFn push_back = fn_at<PushBackFn>(0x00484540);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x30, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x48, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x54, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x294, self);
}

// [0x007009c0] BSP_DummyTargetVehicle_DetachFromWorldLists
// Original ABI: __thiscall (ECX = self), RET 0.
void DummyTargetVehicle_DetachFromWorldLists_007009c0(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x006d3620)(self);
    void* parent = field_at<void*>(self, 0x30);
    detach_from_intrusive_list(static_cast<std::uint8_t*>(parent) + 0x54, 0x04, self);
    parent = field_at<void*>(self, 0x30);
    detach_from_intrusive_list(static_cast<std::uint8_t*>(parent) + 0x294, 0x04, self);
}

// [0x00700a20] BSP_DummyTarget_Construct
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4, returns self.
void* DummyTarget_Construct_00700a20(void* self, void* arg0) {
    Dummy2D_Construct_006ff3f0(self, arg0);
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfd018u;
    field_at<std::uintptr_t>(self, 0x10) = 0x00cfcffcu;
    field_at<std::uintptr_t>(self, 0x24) = 0x00cfcff4u;
    field_at<std::uintptr_t>(self, 0x170) = 0x00cfcfe0u + 0x10u;
    field_at<std::uintptr_t>(self, 0x1e4) = 0x00cfcfe8u;
    field_at<std::uintptr_t>(self, 0x310) = 0x00cfcfa8u;
    field_at<std::uintptr_t>(self, 0x3e4) = 0x00cfcf90u;
    field_at<std::uint32_t>(self, 0xc4) = 0x2eu;
    return self;
}

// [0x00700b10] BSP_DummyTarget_RegisterInWorldLists
// Original ABI: __thiscall (ECX = self), RET 0.
void DummyTarget_RegisterInWorldLists_00700b10(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x00928560)(self);
    using PushBackFn = void(__thiscall*)(void*, void*);
    PushBackFn push_back = fn_at<PushBackFn>(0x00484540);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x30, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x210, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x234, self);
    push_back(static_cast<std::uint8_t*>(field_at<void*>(self, 0x30)) + 0x240, self);
}

// [0x00700b60] BSP_DummyTarget_DetachFromWorldLists
// Original ABI: __thiscall (ECX = self), RET 0.
void DummyTarget_DetachFromWorldLists_00700b60(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x006e29c0)(self);
    void* parent = field_at<void*>(self, 0x30);
    detach_from_intrusive_list(static_cast<std::uint8_t*>(parent) + 0x234, 0x04, self);
    parent = field_at<void*>(self, 0x30);
    detach_from_intrusive_list(static_cast<std::uint8_t*>(parent) + 0x240, 0x04, self);
}

// [0x00700dd0] BSP_DummyTargetBulletClass_CreateEntity
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0), RET 4.
void* DummyTargetBulletClass_CreateEntity_00700dd0(void* self, void* arg0) {
    void* mem = fn_at<void*(__cdecl*)(std::size_t)>(0x00bf55be)(0x4a0u);
    fn_at<void*(__cdecl*)(void*, int, std::size_t)>(0x00bf79f0)(mem, 0, 0x4a0u);
    void* entity = nullptr;
    if (mem != nullptr) {
        entity = DummyTarget_Construct_00700a20(mem, arg0);
    }
    field_at<void*>(entity, 0x314) = self;
    return entity;
}

// [0x00701170] BSP_DummyTargetVehicle_Create
// Original ABI: __thiscall (ECX = vehicle_class, stack [arg0, name_str, pos_arg0, pos_arg1, orient_arg]), RET 14h.
void* DummyTargetVehicle_Create_00701170(
    void* vehicle_class,
    void* arg0,
    const char* name_str,
    std::uint32_t pos_arg0,
    std::uint32_t pos_arg1,
    std::uint32_t orient_arg
) {
    void* mem = fn_at<void*(__cdecl*)(std::size_t)>(0x00bf55be)(0x740u);
    fn_at<void*(__cdecl*)(void*, int, std::size_t)>(0x00bf79f0)(mem, 0, 0x740u);
    void* entity = nullptr;
    if (mem != nullptr) {
        entity = DummyTargetVehicle_Construct_00700590(mem, arg0);
    }
    fn_at<void(__fastcall*)(void*, std::uint32_t, std::uint32_t, std::uint32_t)>(0x004f04c0)(
        entity,
        orient_arg,
        pos_arg0,
        pos_arg1
    );

    void* dst_name = static_cast<std::uint8_t*>(entity) + 0x154;
    std::uint32_t len = 0u;
    if (name_str != nullptr) {
        const char* p = name_str;
        while (*p != '\0') {
            ++p;
        }
        len = static_cast<std::uint32_t>(p - name_str);
    }
    fn_at<void(__thiscall*)(void*, std::uint32_t, std::uint32_t)>(0x0041dd40)(dst_name, len, 0u);
    void* buf = field_at<void*>(dst_name, 0x04);
    if (buf != nullptr) {
        fn_at<void*(__cdecl*)(void*, const void*, std::size_t)>(0x00bf7680)(
            buf,
            name_str,
            field_at<std::uint32_t>(dst_name, 0x00)
        );
    }
    fn_at<void(__thiscall*)(void*, void*)>(0x009553d0)(entity, vehicle_class);
    field_at<void*>(entity, 0x354) = vehicle_class;
    return entity;
}

// [0x00701250] BSP_DummyTargetVehicle_HitOnTheFly
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = hit_arg), RET 4, returns AL = 0.
bool DummyTargetVehicle_HitOnTheFly_00701250(void* self, void* /*hit_arg*/) {
    void* lock_owner = fn_at<void*(__cdecl*)()>(0x004c1570)();
    void* cs = field_at<void*>(lock_owner, 0x04);
    if (cs != nullptr) {
        using EnterCsFn = void(__stdcall*)(void*);
        (*reinterpret_cast<EnterCsFn*>(0x00ce2218))(cs);
        field_at<std::int32_t>(cs, 0x18) += 1;
    }

    struct NativeStr8 {
        std::uint32_t size{0};
        char* data{nullptr};
    } key_str{};
    fn_at<void(__thiscall*)(void*, std::uint32_t, std::uint32_t)>(0x0041dd40)(&key_str, 0x0bu, 1u);
    if (key_str.data != nullptr) {
        fn_at<void*(__cdecl*)(void*, const void*, std::size_t)>(0x00bf7680)(
            key_str.data,
            reinterpret_cast<const void*>(0x00cfd208),
            key_str.size + 1u
        );
    }

    ByValLuaObj20 lua_obj{};
    void* lua_tbl = fn_at<void*(__thiscall*)(void*, void*)>(0x00927b40)(self, &lua_obj);
    fn_at<void(__thiscall*)(void*, void*, std::uint32_t)>(0x00b673a0)(lua_tbl, &key_str, 1u);
    fn_at<void(__thiscall*)(void*)>(0x00b67700)(&lua_obj);

    if (key_str.data != nullptr) {
        // (data, size + 1, 1) are pushed first; 00419CC0 takes no arguments and returns the pool
        // singleton, which becomes ECX for 00BD1510 ReturnBlock(ptr, size, align), RET 0Ch
        // (006FF639..006FF647 and 0070131C..0070132A).
        void* pool = fn_at<void*(__cdecl*)()>(0x00419cc0)();
        fn_at<void(__thiscall*)(void*, void*, std::uint32_t, std::uint32_t)>(0x00bd1510)(
            pool, key_str.data, key_str.size + 1u, 1u);
    }

    if (field_at<std::uint8_t>(self, 0xc8) == 0u) {
        fn_at<void(__thiscall*)(void*)>(0x00414db0)(self);
    }

    const float one = *reinterpret_cast<const float*>(0x00d7a24c);
    float effect_mat[16] = {
        one, 0.0f, 0.0f, 0.0f,
        0.0f, one, 0.0f, 0.0f,
        0.0f, 0.0f, one, 0.0f,
        field_at<float>(self, 0xfc),
        field_at<float>(self, 0x100),
        field_at<float>(self, 0x104),
        one
    };

    void* game_root = *reinterpret_cast<void**>(0x00e188a8);
    void* fx_mgr = field_at<void*>(game_root, 0x19ec);
    void* cls = field_at<void*>(self, 0x538);
    void* fx_desc = field_at<void*>(cls, 0x140);
    if (fx_desc != nullptr) {
        interlocked_inc_00ce221c(&field_at<volatile std::int32_t>(fx_desc, 0x04));
    }

    void* spawned_fx_ref = nullptr;
    void** spawned_fx_ptr = fn_at<void**(__fastcall*)(void*, void*, void*, const float*, std::uint32_t, std::uint32_t)>(
        0x00868420
    )(&spawned_fx_ref, fx_mgr, fx_desc, effect_mat, 0u, 0u);
    void* spawned_fx = *spawned_fx_ptr;
    if (spawned_fx_ref != nullptr) {
        if (interlocked_dec_00ce2220(&field_at<volatile std::int32_t>(spawned_fx_ref, 0x04)) == 0) {
            using ReleaseDtorFn = void(__thiscall*)(void*);
            vslot_at<ReleaseDtorFn>(spawned_fx_ref, 0x00)(spawned_fx_ref);
        }
    }
    field_at<std::uint8_t>(spawned_fx, 0x09) = 1u;
    fn_at<void(__thiscall*)(void*, std::uint32_t)>(0x00926d90)(self, 2u);

    if (cs != nullptr) {
        field_at<std::int32_t>(cs, 0x18) -= 1;
        using LeaveCsFn = void(__stdcall*)(void*);
        (*reinterpret_cast<LeaveCsFn*>(0x00ce2210))(cs);
    }
    return false;
}

// [0x007016c0] BSP_DummyTargetVehicle_InitAll
// Original ABI: __thiscall (ECX = self), RET 0.
void DummyTargetVehicle_InitAll_007016c0(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x0095e5b0)(self);
    void* init_bag = field_at<void*>(self, 0xc0);
    if (init_bag != nullptr && field_at<std::uint32_t>(init_bag, 0x04) == 1u) {
        void* prop_bag = field_at<void*>(init_bag, 0x08);
        void* entry = fn_at<void*(__thiscall*)(void*, std::uintptr_t)>(0x008f2260)(
            prop_bag,
            0x00cfc85cu
        );
        if (entry != nullptr && field_at<std::uint32_t>(entry, 0x04) == 7u) {
            field_at<float>(self, 0x734) = field_at<float>(entry, 0x0c);
            field_at<float>(self, 0x738) = field_at<float>(entry, 0x10);
            field_at<float>(self, 0x73c) = field_at<float>(entry, 0x14);
        }
    }

    std::uint8_t* recon_slot = static_cast<std::uint8_t*>(self) + 0x1e8;
    for (int i = 0; i < 3; ++i) {
        fn_at<void(__thiscall*)(void*, std::uint32_t)>(0x00805cf0)(recon_slot, 2u);
        recon_slot += 0x34;
    }

    init_bag = field_at<void*>(self, 0xc0);
    if (init_bag != nullptr && field_at<std::uint32_t>(init_bag, 0x04) == 3u) {
        ByValLuaObj20 saved_lua{};
        fn_at<void*(__thiscall*)(void*, void*)>(0x009238a0)(self, &saved_lua);
        struct LuaReader24 {
            std::uintptr_t vtable{0};
            std::uint32_t vec_data[5]{};
        } reader{};
        fn_at<void*(__thiscall*)(void*, ByValLuaObj20)>(0x004425c0)(&reader, saved_lua);
        fn_at<void(__thiscall*)(void*, ByValPair8)>(0x00bd8e20)(
            &reader,
            ByValPair8{0u, 0x00cfcf7cu}
        );
        fn_at<void(__thiscall*)(void*, ByValPair8, ByValPair8, ByValPair8)>(0x00bd68d0)(
            &reader,
            ByValPair8{0u, 0x00cfcf70u},
            ByValPair8{2u, reinterpret_cast<std::uintptr_t>(static_cast<std::uint8_t*>(self) + 0x730)},
            ByValPair8{2u, *reinterpret_cast<const std::uint32_t*>(0x00d7a24c)}
        );
        fn_at<void(__thiscall*)(void*, ByValPair8, ByValPair8)>(0x00bd6830)(
            &reader,
            ByValPair8{0u, 0x00cfcf64u},
            ByValPair8{5u, reinterpret_cast<std::uintptr_t>(static_cast<std::uint8_t*>(self) + 0x734)}
        );
        fn_at<void(__thiscall*)(void*)>(0x00bd7a20)(&reader);
        reader.vtable = 0x00ce44fcu;
        fn_at<void(__thiscall*)(void*)>(0x00441210)(&reader.vec_data[0]);
    }
}

// oct18b-nB-low additions; exact listings and family evidence are in
// docs/OCT18B_NB_LOW.md. These entries retain ECX and original stack cleanup.

// 006FCBC0..006FCC11: ECX=message, stack stream, RET 4. The base reader
// receives the stream object; only the two numeric reads use its cursor at +4.
void __fastcall SessionMessage_DEPTHCHARGE_CREATE_Read_006fcbc0(
    void* self, void*, void* bit_stream) {
    fn_at<void(__thiscall*)(void*, void*)>(0x006e06f0)(self, bit_stream);
    auto* cursor = static_cast<std::uint8_t*>(bit_stream) + 4;
    using ReadFloat = void(__thiscall*)(void*, float*, std::uint32_t,
                                       std::uint32_t, float, std::uint32_t);
    auto read = fn_at<ReadFloat>(0x004293f0);
    read(cursor, &field_at<float>(self, 0x120), 0, 1,
         *reinterpret_cast<const volatile float*>(0x00d7a248), 0x20);
    read(cursor, &field_at<float>(self, 0x124), 0, 1,
         *reinterpret_cast<const volatile float*>(0x00d7a248), 0x20);
}

// 006FF6D0: ECX=self, stack float dt; FLD/FSTP forwards dt to 006E1060;
// CALL's callee pops its float, then RET 4 pops this wrapper's original float.
void __fastcall DummySubmarine_PlaceStepPose_006ff6d0(void* self, void*, float dt) {
    __asm { fld dt }
    __asm { fstp dt }
    fn_at<void(__thiscall*)(void*, float)>(0x006e1060)(self, dt);
}

// Receiver is unused. 00700210 returns EAX=0 / RET; the following two entries
// are literal RET 4 no-ops, with no meaningful return register.
std::uint32_t __fastcall DummyTargetVehicleClass_VSlot04_Zero_00700210(const void*) { return 0; }
void __fastcall DummyTargetVehicle_VSlot17C_NoOp_00700220(void*, void*, std::uint32_t) {}
void __fastcall DummyTargetVehicle_VSlot180_NoOp_00700230(void*, void*, std::uint32_t) {}

// 007004F0: ECX=self, stack float[3] out, EAX=out, RET 4. Keep the original
// sequential FLD/FSTP copies (including their x87 behavior and alias ordering).
float* __fastcall DummyTarget_CopyVector734_007004f0(const void* self, void*, float* destination) {
    __asm {
        mov ecx, self
        mov eax, destination
        fld dword ptr [ecx + 734h]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 738h]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 73ch]
        fstp dword ptr [eax + 8]
    }
    return destination;
}

}  // namespace bsp::dummy_depth_seg34_argon
