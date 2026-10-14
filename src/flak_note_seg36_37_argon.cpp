#include "bsp/flak_note_seg36_37_argon.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>

namespace bsp::flak_note_seg36_37_argon {

namespace {

template <typename T>
inline T& field_at(void* base, std::ptrdiff_t offset) {
    return *reinterpret_cast<T*>( static_cast<std::uint8_t*>(base) + offset );
}

template <typename T>
inline const T& field_at(const void* base, std::ptrdiff_t offset) {
    return *reinterpret_cast<const T*>( static_cast<const std::uint8_t*>(base) + offset );
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

inline void release_refcounted_obj(void* obj) {
    if (obj == nullptr) {
        return;
    }
    if (interlocked_dec_00ce2220(&field_at<volatile std::int32_t>(obj, 0x04)) == 0) {
        using DtorFn = void(__thiscall*)(void*);
        vslot_at<DtorFn>(obj, 0x00)(obj);
    }
}

struct LegacySboString28 {
    std::uint32_t alloc_cookie{0};
    union {
        char buf[16];
        char* ptr;
    } box{};
    std::uint32_t size{0};
    std::uint32_t capacity{15};
};

inline const char* sbo_c_str(const void* str_obj) {
    const std::uint32_t cap = field_at<std::uint32_t>(str_obj, 0x18);
    if (cap < 16u) {
        return reinterpret_cast<const char*>(static_cast<const std::uint8_t*>(str_obj) + 0x04);
    }
    return field_at<const char*>(str_obj, 0x04);
}

inline void sbo_tidy(void* str_obj) {
    if (field_at<std::uint32_t>(str_obj, 0x18) >= 16u) {
        fn_at<void(__cdecl*)(void*)>(0x00bf65ac)(field_at<void*>(str_obj, 0x04));
    }
    field_at<std::uint32_t>(str_obj, 0x18) = 15u;
    field_at<std::uint32_t>(str_obj, 0x14) = 0u;
    field_at<std::uint8_t>(str_obj, 0x04) = 0u;
}

inline void refresh_entity_world_matrix_inline(void* entity) {
    if (field_at<std::uint8_t>(entity, 0xc8) != 0u) {
        return;
    }
    void* parent = field_at<void*>(entity, 0x3c);
    if (parent != nullptr) {
        fn_at<void(__thiscall*)(void*)>(0x00414db0)(parent);
    }
    parent = field_at<void*>(entity, 0x3c);
    float tmp_mat[16]{};
    const float* src_mat = nullptr;
    if (parent != nullptr) {
        src_mat = fn_at<const float*(__thiscall*)(const void*, float*, const void*)>(0x00413920)(
            static_cast<const std::uint8_t*>(entity) + 0x74,
            tmp_mat,
            static_cast<const std::uint8_t*>(parent) + 0xcc
        );
    } else {
        src_mat = reinterpret_cast<const float*>(static_cast<const std::uint8_t*>(entity) + 0x74);
    }
    float* dst_mat = reinterpret_cast<float*>(static_cast<std::uint8_t*>(entity) + 0xcc);
    for (int i = 0; i < 16; ++i) {
        dst_mat[i] = src_mat[i];
    }
    field_at<std::uint8_t>(entity, 0xc8) = 1u;
    field_at<std::uint8_t>(entity, 0x10c) = 0u;
}

}  // namespace

// [0x0070bc30] BSP_FlagModel_ScalarDelete
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = flags), RET 4.
void* FlagModel_ScalarDelete_0070bc30(void* self, std::uint8_t flags) {
    fn_at<void(__thiscall*)(void*)>(0x0070b2c0)(self);
    if ((flags & 1u) != 0u) {
        fn_at<void(__thiscall*)(void*, void*)>(0x0070acb0)(
            reinterpret_cast<void*>(0x00e19b14),
            self
        );
    }
    return self;
}

// [0x0070bc50] BSP_FlagParams_Assign
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = src), RET 4.
void* FlagParams_Assign_0070bc50(void* self, const void* src) {
    field_at<float>(self, 0x00) = field_at<float>(src, 0x00);
    field_at<float>(self, 0x04) = field_at<float>(src, 0x04);
    void* dst_arr = static_cast<std::uint8_t*>(self) + 0x08;
    const void* src_arr = static_cast<const std::uint8_t*>(src) + 0x08;
    if (dst_arr != src_arr) {
        const std::uint32_t count = field_at<std::uint32_t>(src_arr, 0x00);
        fn_at<void(__thiscall*)(void*, std::uint32_t, std::uint32_t)>(0x0041dd40)(dst_arr, count, 1u);
        if (field_at<std::uint32_t>(src_arr, 0x00) != 0u) {
            fn_at<void*(__cdecl*)(void*, const void*, std::size_t)>(0x00bf7680)(
                field_at<void*>(dst_arr, 0x04),
                field_at<const void*>(src_arr, 0x04),
                field_at<std::uint32_t>(dst_arr, 0x00)
            );
        }
    }
    field_at<std::uint32_t>(self, 0x10) = field_at<std::uint32_t>(src, 0x10);
    return self;
}

// [0x0070bcb0] BSP_FlagModel_BuildGeometry
// Original ABI: __thiscall (ECX = self), RET.
void FlagModel_BuildGeometry_0070bcb0(void* self) {
    void* raw = fn_at<void*(__fastcall*)(std::uint32_t)>(0x00b73b60)(0xbcu);
    void* geom = (raw != nullptr) ? fn_at<void*(__thiscall*)(void*)>(0x00b73d70)(raw) : nullptr;
    field_at<void*>(self, 0x198) = geom;
    const float lod_dist = field_at<float>(reinterpret_cast<const void*>(0x00d7a260), 0x00);
    fn_at<void(__thiscall*)(void*, std::uint32_t, void*, float, float)>(0x00b75170)(
        self,
        0u,
        geom,
        lod_dist,
        lod_dist
    );
    release_refcounted_obj(field_at<void*>(self, 0x198));
    fn_at<void(__thiscall*)(void*, void*, void*)>(0x0070b9d0)(
        field_at<void*>(self, 0x194),
        field_at<void*>(self, 0x198),
        static_cast<std::uint8_t*>(self) + 0x18c
    );
    void* mgr = *reinterpret_cast<void**>(0x00e188a8);
    fn_at<void(__thiscall*)(void*, void*)>(0x00b6d890)(
        self,
        field_at<void*>(mgr, 0x19ec)
    );
}

// [0x0070bee0] BSP_FlagModel_Setup
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = params), RET 4.
void FlagModel_Setup_0070bee0(void* self, const void* params) {
    FlagParams_Assign_0070bc50(static_cast<std::uint8_t*>(self) + 0x184, params);
    FlagModel_BuildGeometry_0070bcb0(self);
    float center_and_radius[4]{
        *reinterpret_cast<const float*>(0x00f87574),
        *reinterpret_cast<const float*>(0x00f87578),
        *reinterpret_cast<const float*>(0x00f8757c),
        0.0f
    };
    const void* sub = field_at<const void*>(self, 0x194);
    const float w = field_at<float>(sub, 0x44);
    const float h = field_at<float>(sub, 0x48);
    const float max_dim = (w > h) ? w : h;
    center_and_radius[3] = static_cast<float>(
        static_cast<double>(max_dim) * (*reinterpret_cast<const double*>(0x00d7a328))
    );
    void* node0 = fn_at<void*(__thiscall*)(void*, std::uint32_t)>(0x00b74640)(self, 0u);
    fn_at<void(__thiscall*)(void*, const float*)>(0x00b72650)(node0, center_and_radius);
    void* node1 = fn_at<void*(__thiscall*)(void*, std::uint32_t)>(0x00b74640)(self, 0u);
    void* sub_node = fn_at<void*(__thiscall*)(void*, std::uint32_t)>(0x00b732c0)(node1, 0u);
    fn_at<void(__thiscall*)(void*, const float*)>(0x00b855b0)(sub_node, center_and_radius);
    fn_at<void(__thiscall*)(void*, const float*)>(0x00b74390)(self, center_and_radius);
}

// [0x0070bfc0] BSP_FlagModel_Construct
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = arg0, [ESP+8] = params), RET 8.
void* FlagModel_Construct_0070bfc0(void* self, void* arg0, const void* params) {
    fn_at<void(__thiscall*)(void*, void*)>(0x00b75030)(self, arg0);
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfd488u;
    fn_at<void(__thiscall*)(void*)>(0x0070bba0)(static_cast<std::uint8_t*>(self) + 0x184);
    FlagModel_Setup_0070bee0(self, params);
    return self;
}

// [0x0070c030] BSP_FlakBulletClass_ReadLuaFields
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = lua_node), RET 4.
void FlakBulletClass_ReadLuaFields_0070c030(void* self, void* lua_node) {
    fn_at<void(__thiscall*)(void*, void*)>(0x006e8770)(self, lua_node);
    field_at<std::uint32_t>(self, 0xd4) = 0u;
    std::uint8_t temp_lua_val[20]{};
    void* val = fn_at<void*(__thiscall*)(void*, void*, const char*)>(0x00b67800)(
        lua_node,
        temp_lua_val,
        reinterpret_cast<const char*>(0x00cfd510)
    );
    const std::int32_t int_val = fn_at<std::int32_t(__thiscall*)(void*)>(0x00b66290)(val);
    field_at<float>(self, 0x58) = static_cast<float>(int_val);
    fn_at<void(__thiscall*)(void*)>(0x00b67700)(temp_lua_val);
}

// [0x0070c0b0] BSP_FlakBulletClass_DeriveEngagementRange
// Original ABI: __thiscall (ECX = self), RET.
void FlakBulletClass_DeriveEngagementRange_0070c0b0(void* self) {
    const std::uint8_t ok = fn_at<std::uint8_t(__thiscall*)(void*)>(0x006e9890)(self);
    if (ok == 1u) {
        field_at<float>(self, 0xd8) = field_at<float>(self, 0x58) / field_at<float>(self, 0x50);
    }
}

// [0x0070c0f0] BSP_FlakBullet_WriteState
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = stream), RET 4.
void FlakBullet_WriteState_0070c0f0(void* self, void* stream) {
    fn_at<void(__thiscall*)(void*, void*)>(0x006e71b0)(self, stream);
    using BeginGroupFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t);
    using WriteFieldFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, std::uint32_t);
    using EndGroupFn = void(__thiscall*)(void*);
    vslot_at<BeginGroupFn>(stream, 0x04)(stream, 0u, 0x00cfd548u);
    vslot_at<WriteFieldFn>(stream, 0x0c)(
        stream,
        0u,
        0x00cfd53cu,
        2u,
        field_at<std::uint32_t>(self, 0x284)
    );
    vslot_at<WriteFieldFn>(stream, 0x0c)(
        stream,
        0u,
        0x00cfd530u,
        3u,
        field_at<std::uint8_t>(self, 0x288)
    );
    vslot_at<WriteFieldFn>(stream, 0x0c)(
        stream,
        0u,
        0x00cfd524u,
        2u,
        field_at<std::uint32_t>(self, 0x28c)
    );
    vslot_at<WriteFieldFn>(stream, 0x0c)(
        stream,
        0u,
        0x00cfd51cu,
        2u,
        field_at<std::uint32_t>(self, 0x290)
    );
    vslot_at<EndGroupFn>(stream, 0x08)(stream);
}

// [0x0070c1f0] BSP_Entity_AsKind05
// Original ABI: __thiscall (ECX = self), RET.
void* Entity_AsKind05_0070c1f0(void* self) {
    if (self != nullptr) {
        using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
        if (vslot_at<IsKindFn>(self, 0x5c)(self, 5u) != 0u) {
            return self;
        }
    }
    return nullptr;
}

// [0x0070c370] BSP_FlakProjectile_TickAdvance
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = dt), RET 4.
// x87 DC-form note: 0070c4e1 executes `FMUL ST1, ST0` (`dc c9`) with 0.5 (`[0x00d7a280]`) in ST0
// above `pos.x + prev.x` in ST1, followed by `FXCH; FSTP mid.x`, keeping 0.5 in ST0 to scale
// `mid.y` (0070c4ed), `mid.z` (0070c4fd), and `0.5 * segLen + 2 * blastRadius` (0070c51e).
void FlakProjectile_TickAdvance_0070c370(void* self, float dt) {
    if (field_at<std::uint8_t>(self, 0xc8) == 0u) {
        fn_at<void(__thiscall*)(void*)>(0x00414db0)(self);
    }
    field_at<float>(self, 0x1dc) = field_at<float>(self, 0xfc);
    field_at<float>(self, 0x1e0) = field_at<float>(self, 0x100);
    field_at<float>(self, 0x1e4) = field_at<float>(self, 0x104);

    if (field_at<std::uint8_t>(self, 0xc8) == 0u) {
        fn_at<void(__thiscall*)(void*)>(0x00414db0)(self);
    }
    float pos[3]{
        field_at<float>(self, 0x178) * dt + field_at<float>(self, 0xfc),
        field_at<float>(self, 0x17c) * dt + field_at<float>(self, 0x100),
        field_at<float>(self, 0x180) * dt + field_at<float>(self, 0x104)
    };
    fn_at<void(__thiscall*)(void*, const float*)>(0x004151e0)(self, pos);
    field_at<float>(self, 0x1c8) = field_at<float>(self, 0x1c8) + dt;
    fn_at<void(__thiscall*)(void*)>(0x006e79d0)(self);

    if (field_at<std::uint8_t>(self, 0x1cc) != 0u) {
        return;
    }

    const void* bullet_cls = field_at<const void*>(self, 0x174);
    const float max_tof = field_at<float>(bullet_cls, 0xd8);
    if (max_tof < field_at<float>(self, 0x1c8)) {
        field_at<std::uint8_t>(self, 0x1cc) = 1u;
        fn_at<void(__thiscall*)(void*)>(0x006e7450)(self);
        return;
    }

    if (field_at<std::uint8_t>(self, 0xc8) == 0u) {
        fn_at<void(__thiscall*)(void*)>(0x00414db0)(self);
    }
    const float dx = field_at<float>(self, 0xfc) - field_at<float>(self, 0x1dc);
    const float dy = field_at<float>(self, 0x100) - field_at<float>(self, 0x1e0);
    const float dz = field_at<float>(self, 0x104) - field_at<float>(self, 0x1e4);
    const float len_sq = dx * dx + dy * dy + dz * dz;
    float seg_len = 0.0f;
    if (static_cast<double>(len_sq) > (*reinterpret_cast<const double*>(0x00ce3820))) {
        seg_len = static_cast<float>(fn_at<double(__cdecl*)(double)>(0x00bf7030)(len_sq));
    }

    if (field_at<std::uint8_t>(self, 0xc8) == 0u) {
        fn_at<void(__thiscall*)(void*)>(0x00414db0)(self);
    }
    // 0070c4cf..0070c526: 0.5 retained in ST0 via DC C9 (`FMUL ST1, ST0`) at 0070c4e1
    const double half = *reinterpret_cast<const double*>(0x00d7a280);
    const float mid[3]{
        static_cast<float>((field_at<float>(self, 0xfc) + field_at<float>(self, 0x1dc)) * half),
        static_cast<float>((field_at<float>(self, 0x100) + field_at<float>(self, 0x1e0)) * half),
        static_cast<float>((field_at<float>(self, 0x104) + field_at<float>(self, 0x1e4)) * half)
    };
    const float blast_r = field_at<float>(bullet_cls, 0x58);
    const float query_r = static_cast<float>((blast_r + blast_r) + seg_len * half);

    void* spatial_grid = *reinterpret_cast<void**>(0x00e17b08);
    using GridBeginFn = void*(__thiscall*)(void*, void*, const float*, float, std::uint32_t);
    using GridAtEndFn = std::uint8_t(__thiscall*)(void*);
    using GridNextFn = void(__thiscall*)(void*);
    using GridDerefFn = void*(__thiscall*)(void*);

    std::uint8_t iter_buf[28]{};
    void* it = vslot_at<GridBeginFn>(spatial_grid, 0x18)(spatial_grid, iter_buf, mid, query_r, 8u);
    std::int32_t hit_found = 0;

    if (vslot_at<GridAtEndFn>(it, 0x0c)(it) == 0u) {
        do {
            void* cand = vslot_at<GridDerefFn>(it, 0x08)(it);
            void* unit = Entity_AsKind05_0070c1f0(cand);
            if (unit != nullptr &&
                field_at<std::uint8_t>(unit, 0x35d) != 0u &&
                (field_at<std::uint32_t>(unit, 0x2cc) & 1u) == 0u &&
                field_at<std::int32_t>(unit, 0x348) == 1 &&
                field_at<std::int32_t>(unit, 0x2fc) < 2) {
                void* owner = fn_at<void*(__thiscall*)(void*)>(0x006e72a0)(self);
                if (unit != owner) {
                    const std::uint8_t allied = fn_at<std::uint8_t(__thiscall*)(void*, void*)>(0x00682550)(
                        reinterpret_cast<void*>(0x00df77f0),
                        unit
                    );
                    if (allied == 0u) {
                        if (field_at<std::uint8_t>(unit, 0xc8) == 0u) {
                            fn_at<void(__thiscall*)(void*)>(0x00414db0)(unit);
                        }
                        if (field_at<std::uint8_t>(self, 0xc8) == 0u) {
                            fn_at<void(__thiscall*)(void*)>(0x00414db0)(self);
                        }
                        const float vx = field_at<float>(self, 0xfc) - field_at<float>(self, 0x1dc);
                        const float vy = field_at<float>(self, 0x100) - field_at<float>(self, 0x1e0);
                        const float vz = field_at<float>(self, 0x104) - field_at<float>(self, 0x1e4);

                        const float wx = field_at<float>(unit, 0xfc) - field_at<float>(self, 0x1dc);
                        const float wy = field_at<float>(unit, 0x100) - field_at<float>(self, 0x1e0);
                        const float wz = field_at<float>(unit, 0x104) - field_at<float>(self, 0x1e4);

                        const float c1 = wx * vx + wy * vy + wz * vz;
                        float dist_to_seg = 0.0f;
                        if (c1 > 0.0f) {
                            const float c2 = vx * vx + vy * vy + vz * vz;
                            if (c2 > c1) {
                                const float t = c1 / c2;
                                const float px = field_at<float>(self, 0x1dc) + vx * t;
                                const float py = field_at<float>(self, 0x1e0) + vy * t;
                                const float pz = field_at<float>(self, 0x1e4) + vz * t;
                                const float ex = field_at<float>(unit, 0xfc) - px;
                                const float ey = field_at<float>(unit, 0x100) - py;
                                const float ez = field_at<float>(unit, 0x104) - pz;
                                const float d2 = ex * ex + ey * ey + ez * ez;
                                if (static_cast<double>(d2) > (*reinterpret_cast<const double*>(0x00ce3820))) {
                                    dist_to_seg = static_cast<float>(fn_at<double(__cdecl*)(double)>(0x00bf7030)(d2));
                                }
                            } else {
                                const float ex = field_at<float>(unit, 0xfc) - field_at<float>(self, 0xfc);
                                const float ey = field_at<float>(unit, 0x100) - field_at<float>(self, 0x100);
                                const float ez = field_at<float>(unit, 0x104) - field_at<float>(self, 0x104);
                                const float d2 = ex * ex + ey * ey + ez * ez;
                                if (static_cast<double>(d2) > (*reinterpret_cast<const double*>(0x00ce3820))) {
                                    dist_to_seg = static_cast<float>(fn_at<double(__cdecl*)(double)>(0x00bf7030)(d2));
                                }
                            }
                        } else {
                            const float d2 = wx * wx + wy * wy + wz * wz;
                            if (static_cast<double>(d2) > (*reinterpret_cast<const double*>(0x00ce3820))) {
                                dist_to_seg = static_cast<float>(fn_at<double(__cdecl*)(double)>(0x00bf7030)(d2));
                            }
                        }

                        if (dist_to_seg < field_at<float>(bullet_cls, 0x58)) {
                            hit_found = 1;
                            if (dist_to_seg < field_at<float>(self, 0x284)) {
                                field_at<float>(self, 0x284) = dist_to_seg;
                            }
                        }
                    }
                }
            }
            vslot_at<GridNextFn>(it, 0x04)(it);
        } while (vslot_at<GridAtEndFn>(it, 0x0c)(it) == 0u);
    }

    if (field_at<std::uint8_t>(self, 0x288) != 0u) {
        if (field_at<std::int32_t>(self, 0x294) != 0) {
            const float step_back = dt - (field_at<float>(self, 0x284) - field_at<float>(self, 0x28c));
            if (field_at<std::uint8_t>(self, 0xc8) == 0u) {
                fn_at<void(__thiscall*)(void*)>(0x00414db0)(self);
            }
            const float burst_pos[3]{
                field_at<float>(self, 0xfc) - field_at<float>(self, 0x178) * step_back,
                field_at<float>(self, 0x100) - field_at<float>(self, 0x17c) * step_back,
                field_at<float>(self, 0x104) - field_at<float>(self, 0x180) * step_back
            };
            fn_at<void(__thiscall*)(void*, const float*)>(0x004151e0)(self, burst_pos);
            field_at<std::uint8_t>(self, 0x1cc) = 1u;
            fn_at<void(__thiscall*)(void*)>(0x006e7450)(self);
            fn_at<void(__thiscall*)(void*)>(0x007b5d00)(it);
            return;
        }
        const float elapsed = field_at<float>(self, 0x28c) + dt;
        field_at<float>(self, 0x28c) = elapsed;
        if (elapsed > field_at<float>(self, 0x284)) {
            field_at<std::int32_t>(self, 0x294) = 1;
            fn_at<void(__thiscall*)(void*)>(0x007b5d00)(it);
            return;
        }
    } else if (hit_found != 0) {
        field_at<std::uint8_t>(self, 0x288) = 1u;
        const int r = fn_at<int(__cdecl*)()>(0x00bf65ff)();
        field_at<float>(self, 0x284) = static_cast<float>(
            static_cast<double>(r) * (*reinterpret_cast<const double*>(0x00d7a288)) *
            (*reinterpret_cast<const double*>(0x00d7a290))
        );
        field_at<float>(self, 0x28c) = 0.0f;
    }

    fn_at<void(__thiscall*)(void*)>(0x007b5d00)(it);
}

// [0x0070cae0] BSP_FlakProjectile_Construct
// Original ABI: __thiscall (ECX = self), RET.
void* FlakProjectile_Construct_0070cae0(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x006e7b00)(self);
    field_at<std::uint8_t>(self, 0x288) = 0u;
    field_at<std::uint32_t>(self, 0x294) = 0u;
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfd5d0u;
    field_at<std::uintptr_t>(self, 0x10) = 0x00cfd5b8u;
    field_at<std::uintptr_t>(self, 0x24) = 0x00cfd5b0u;
    field_at<std::uintptr_t>(self, 0x170) = 0x00cfd570u;
    field_at<std::uintptr_t>(self, 0x244) = 0x00cfd554u;
    field_at<std::uint32_t>(self, 0xc4) = 0x32u;
    field_at<float>(self, 0x284) = *reinterpret_cast<const float*>(0x00d7a248);
    return self;
}

// [0x0070cb80] BSP_MFlakBullet_IsKindOf
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = kind_id), RET 4.
std::uint32_t MFlakBullet_IsKindOf_0070cb80(const void* self, std::uint32_t kind_id) {
    if (kind_id == 0x32u || kind_id == 0x29u || kind_id == 0u ||
        kind_id == field_at<std::uint32_t>(self, 0xc4)) {
        return 1u;
    }
    return 0u;
}

// [0x0070cc30] BSP_FlakBulletClass_CreateProjectile
// Original ABI: __thiscall (ECX = self, 7 stack args), RET 1Ch.
void* FlakBulletClass_CreateProjectile_0070cc30(
    void* self,
    void* out_ref,
    std::uint32_t owner_id,
    const float* start_pos,
    const float* direction,
    std::uint8_t activate_flag,
    std::uint32_t /*unused_arg*/,
    const float* speed_scale_pair
) {
    void* raw = fn_at<void*(__cdecl*)(std::size_t)>(0x00bf55be)(0x298u);
    fn_at<void*(__cdecl*)(void*, int, std::size_t)>(0x00bf79f0)(raw, 0, 0x298u);
    void* proj = (raw != nullptr) ? FlakProjectile_Construct_0070cae0(raw) : nullptr;
    field_at<void*>(proj, 0x174) = self;

    const float one = *reinterpret_cast<const float*>(0x00d7a24c);
    float ident_mat[16]{
        one, 0.0f, 0.0f, 0.0f,
        0.0f, one, 0.0f, 0.0f,
        0.0f, 0.0f, one, 0.0f,
        0.0f, 0.0f, 0.0f, one
    };
    void* mgr = *reinterpret_cast<void**>(0x00e188a8);
    fn_at<void(__thiscall*)(void*, std::uint32_t, void*, const float*)>(0x00923870)(
        proj,
        0u,
        field_at<void*>(mgr, 0x19cc),
        ident_mat
    );

    field_at<std::uint32_t>(proj, 0x54) = owner_id;
    void* proj_iface = static_cast<std::uint8_t*>(proj) + 0x170;
    field_at<float>(proj, 0xa4) = start_pos[0];
    field_at<float>(proj, 0xa8) = start_pos[1];
    field_at<float>(proj, 0xac) = start_pos[2];
    field_at<std::uint8_t>(proj, 0xc8) = 1u;
    field_at<std::uint8_t>(proj, 0x10c) = 0u;
    field_at<float>(proj, 0xfc) = start_pos[0];
    field_at<float>(proj, 0x100) = start_pos[1];
    field_at<float>(proj, 0x104) = start_pos[2];
    field_at<float>(proj_iface, 0x60) = start_pos[0];
    field_at<float>(proj_iface, 0x64) = start_pos[1];
    field_at<float>(proj_iface, 0x68) = start_pos[2];
    field_at<float>(proj_iface, 0x6c) = start_pos[0];
    field_at<float>(proj_iface, 0x70) = start_pos[1];
    field_at<float>(proj_iface, 0x74) = start_pos[2];

    const float speed = field_at<float>(field_at<const void*>(proj, 0x174), 0x50);
    field_at<float>(proj, 0x178) = direction[0] * speed;
    field_at<float>(proj, 0x17c) = direction[1] * speed;
    field_at<float>(proj, 0x180) = direction[2] * speed;

    void* rot_mat = static_cast<std::uint8_t*>(proj) + 0x74;
    field_at<float>(rot_mat, 0x20) = direction[0];
    field_at<float>(rot_mat, 0x28) = direction[2];
    field_at<float>(rot_mat, 0x24) = 0.0f;
    field_at<float>(rot_mat, 0x10) = 0.0f;
    field_at<float>(rot_mat, 0x14) = one;
    field_at<float>(rot_mat, 0x18) = 0.0f;
    fn_at<void(__thiscall*)(void*)>(0x0085dc80)(rot_mat);

    field_at<std::uint8_t>(proj, 0xc8) = 0u;
    field_at<std::uint8_t>(proj, 0x10c) = 0u;
    for (void* child = field_at<void*>(proj, 0x48); child != nullptr; child = field_at<void*>(child, 0x44)) {
        fn_at<void(__thiscall*)(void*)>(0x0042ed50)(child);
    }

    using Slot24Fn = void(__thiscall*)(void*, std::uint32_t);
    vslot_at<Slot24Fn>(proj_iface, 0x24)(proj_iface, 0u);

    field_at<float>(proj, 0x1c8) = 0.0f;
    field_at<float>(proj, 0x290) = field_at<float>(self, 0x70) * speed_scale_pair[1];
    if (activate_flag != 0u) {
        using Slot3CFn = void(__thiscall*)(void*);
        vslot_at<Slot3CFn>(proj_iface, 0x3c)(proj_iface);
    }
    (void)out_ref;
    return proj_iface;
}

// [0x0070cea0] BSP_FlakBullet_Activate
// Original ABI: __thiscall (ECX = self), RET.
void FlakBullet_Activate_0070cea0(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x006e7e80)(self);
    void* net_ctx = field_at<void*>(self, 0xc0);
    if (net_ctx == nullptr || field_at<std::int32_t>(net_ctx, 0x04) != 3) {
        return;
    }
    std::uint8_t pkt_arg[20]{};
    fn_at<void(__thiscall*)(void*, void*)>(0x009238a0)(self, pkt_arg);
    std::uint8_t writer[24]{};
    fn_at<void(__thiscall*)(void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t)>(
        0x004425c0
    )(
        writer,
        *reinterpret_cast<std::uint32_t*>(pkt_arg + 0x00),
        *reinterpret_cast<std::uint32_t*>(pkt_arg + 0x04),
        *reinterpret_cast<std::uint32_t*>(pkt_arg + 0x08),
        *reinterpret_cast<std::uint32_t*>(pkt_arg + 0x0c),
        *reinterpret_cast<std::uint32_t*>(pkt_arg + 0x10)
    );
    fn_at<void(__thiscall*)(void*, std::uint32_t, std::uintptr_t)>(0x00bd8e20)(writer, 0u, 0x00cfd548u);
    fn_at<void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, void*)>(0x00bd6830)(
        writer,
        0u,
        0x00cfd53cu,
        2u,
        static_cast<std::uint8_t*>(self) + 0x284
    );
    fn_at<void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, void*)>(0x00bd6830)(
        writer,
        0u,
        0x00cfd530u,
        3u,
        static_cast<std::uint8_t*>(self) + 0x288
    );
    fn_at<void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, void*)>(0x00bd6830)(
        writer,
        0u,
        0x00cfd524u,
        2u,
        static_cast<std::uint8_t*>(self) + 0x28c
    );
    fn_at<void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, void*)>(0x00bd6830)(
        writer,
        0u,
        0x00cfd51cu,
        2u,
        static_cast<std::uint8_t*>(self) + 0x290
    );
    fn_at<void(__thiscall*)(void*)>(0x00bd7a20)(writer);
    fn_at<void(__thiscall*)(void*)>(0x00441a20)(writer);
}

// [0x0070d1b0] BSP_Unit_ClassMaxSpeed
// Original ABI: __thiscall (ECX = self), RET (returns float in ST0).
float Unit_ClassMaxSpeed_0070d1b0(void* self) {
    using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
    if (vslot_at<IsKindFn>(self, 0x5c)(self, 6u) != 0u) {
        const void* cls = field_at<const void*>(self, 0x538);
        return field_at<float>(cls, 0x500);
    }
    const void* sub = field_at<const void*>(self, 0x3d0);
    const void* cls = field_at<const void*>(sub, 0x538);
    return field_at<float>(cls, 0x188);
}

// [0x0070d7b0] BSP_UnitGroup_InitWithLeader
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = leader), RET 4.
void UnitGroup_InitWithLeader_0070d7b0(void* self, void* leader) {
    field_at<void*>(self, 0x14) = leader;
    field_at<void*>(leader, 0x284) = self;
    field_at<void*>(self, 0x18) = field_at<void*>(self, 0x14);
    field_at<float>(self, 0x1c) = *reinterpret_cast<const float*>(0x00f87574);
    field_at<float>(self, 0x20) = *reinterpret_cast<const float*>(0x00f87578);
    field_at<float>(self, 0x24) = *reinterpret_cast<const float*>(0x00f8757c);
    field_at<float>(self, 0x48) = *reinterpret_cast<const float*>(0x00cf4888);
    field_at<float>(self, 0x28) = 0.0f;
    field_at<float>(self, 0x38) = 0.0f;
    field_at<float>(self, 0x2c) = 0.0f;
    field_at<float>(self, 0x3c) = 0.0f;
    field_at<float>(self, 0x30) = 0.0f;
    field_at<float>(self, 0x40) = 0.0f;
    field_at<float>(self, 0x34) = 0.0f;
    field_at<float>(self, 0x44) = 0.0f;
    field_at<std::int32_t>(self, 0x4f8) = 1;
    fn_at<void(__fastcall*)(void*, void*)>(0x00694a60)(leader, self);
    using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
    if (vslot_at<IsKindFn>(leader, 0x5c)(leader, 6u) != 0u) {
        field_at<std::int32_t>(self, 0x4fc) = 6;
        const void* cls = field_at<const void*>(leader, 0x538);
        field_at<float>(self, 0x504) = field_at<float>(cls, 0x500);
    }
}

// [0x0070da00] BSP_UnitGroup_RefreshSpeedCeiling
// Original ABI: __thiscall (ECX = self), RET.
void UnitGroup_RefreshSpeedCeiling_0070da00(void* self) {
    field_at<float>(self, 0x504) = *reinterpret_cast<const float*>(0x00cfd6f4);
    const std::int32_t count = field_at<std::int32_t>(self, 0x4f8);
    if (count <= 0) {
        return;
    }
    std::uint8_t* slot = static_cast<std::uint8_t*>(self) + 0x18;
    for (std::int32_t i = 0; i < count; ++i, slot += 0x34) {
        void* member = field_at<void*>(slot, 0x00);
        if (member != nullptr) {
            using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
            float spd = 0.0f;
            if (vslot_at<IsKindFn>(member, 0x5c)(member, 6u) != 0u) {
                spd = field_at<float>(field_at<const void*>(member, 0x538), 0x500);
            } else {
                const void* sub = field_at<const void*>(member, 0x3d0);
                spd = field_at<float>(field_at<const void*>(sub, 0x538), 0x188);
            }
            if (field_at<float>(self, 0x504) > spd) {
                field_at<float>(self, 0x504) = spd;
            }
        } else {
            field_at<float>(slot, 0x30) = *reinterpret_cast<const float*>(0x00cf4888);
        }
    }
}

// [0x0070db20] BSP_UnitGroup_CreateForLeader
// Original ABI: __fastcall (ECX = leader), RET.
void* UnitGroup_CreateForLeader_0070db20(void* leader) {
    void* raw = fn_at<void*(__cdecl*)(std::size_t)>(0x00bf681b)(0x508u);
    void* grp = (raw != nullptr) ? fn_at<void*(__thiscall*)(void*)>(0x0070dab0)(raw) : nullptr;
    UnitGroup_InitWithLeader_0070d7b0(grp, leader);
    return grp;
}

// [0x0070db60] BSP_UnitGroup_SwapSlotsByDistance
// Original ABI: __thiscall (ECX = self), RET.
void UnitGroup_SwapSlotsByDistance_0070db60(void* self) {
    if (field_at<std::int32_t>(self, 0x4fc) == 0x18) {
        return;
    }
    const std::int32_t count = field_at<std::int32_t>(self, 0x4f8);
    if (count <= 0) {
        return;
    }
    void* excluded_unit = *reinterpret_cast<void**>(0x00e188d8);
    std::uint8_t* scan_slot = static_cast<std::uint8_t*>(self) + 0x18;
    for (std::int32_t i = 0; i < count; ++i, scan_slot += 0x34) {
        if (field_at<void*>(scan_slot, 0x00) == excluded_unit) {
            return;
        }
    }

    std::uint8_t* slot_i = static_cast<std::uint8_t*>(self) + 0x18;
    std::int32_t row_offset_i = 0;
    for (std::int32_t next_i = 1; (next_i - 1) < field_at<std::int32_t>(self, 0x4f8);
         ++next_i, slot_i += 0x34, row_offset_i += 0x0d) {
        if (field_at<void*>(slot_i, 0x00) == nullptr || next_i >= field_at<std::int32_t>(self, 0x4f8)) {
            continue;
        }
        std::uint8_t* slot_j = slot_i + 0x34;
        std::int32_t row_offset_j = row_offset_i + 0x0d;
        for (std::int32_t j = next_i; j < field_at<std::int32_t>(self, 0x4f8);
             ++j, slot_j += 0x34, row_offset_j += 0x0d) {
            void* unit_j = field_at<void*>(slot_j, 0x00);
            void* unit_i = field_at<void*>(slot_i, 0x00);
            if (unit_j == nullptr ||
                field_at<void*>(unit_i, 0x538) != field_at<void*>(unit_j, 0x538)) {
                continue;
            }
            void* leader = field_at<void*>(self, 0x14);
            refresh_entity_world_matrix_inline(leader);
            refresh_entity_world_matrix_inline(unit_i);
            leader = field_at<void*>(self, 0x14);
            refresh_entity_world_matrix_inline(leader);
            refresh_entity_world_matrix_inline(unit_j);
            leader = field_at<void*>(self, 0x14);
            refresh_entity_world_matrix_inline(unit_i);

            float rel_i_x = 0.0f;
            float rel_i_z = 0.0f;
            fn_at<void(__thiscall*)(void*, const void*, float*, float*)>(0x00811180)(
                leader,
                static_cast<const std::uint8_t*>(unit_i) + 0xfc,
                &rel_i_x,
                &rel_i_z
            );

            refresh_entity_world_matrix_inline(unit_j);
            float rel_j_x = 0.0f;
            float rel_j_z = 0.0f;
            fn_at<void(__thiscall*)(void*, const void*, float*, float*)>(0x00811180)(
                leader,
                static_cast<const std::uint8_t*>(unit_j) + 0xfc,
                &rel_j_x,
                &rel_j_z
            );

            const std::int32_t mode = field_at<std::int32_t>(self, 0x500);
            const std::int32_t idx_i = row_offset_i + mode;
            const std::int32_t idx_j = row_offset_j + mode;
            const float* base_f = reinterpret_cast<const float*>(self);
            const float slot_i_x = base_f[idx_i + 10];
            const float slot_i_z = base_f[idx_i + 14];
            const float slot_j_x = base_f[idx_j + 10];
            const float slot_j_z = base_f[idx_j + 14];

            auto dist2d = [](float dx, float dz) -> float {
                const float sq = dx * dx + dz * dz;
                if (static_cast<double>(sq) > (*reinterpret_cast<const double*>(0x00ce3820))) {
                    return static_cast<float>(fn_at<double(__cdecl*)(double)>(0x00bf7030)(sq));
                }
                return 0.0f;
            };

            const float d_ii = dist2d(rel_i_x - slot_i_x, rel_i_z - slot_i_z);
            const float d_ij = dist2d(rel_i_x - slot_j_x, rel_i_z - slot_j_z);
            const float d_ji = dist2d(rel_j_x - slot_i_x, rel_j_z - slot_i_z);
            const float d_jj = dist2d(rel_j_x - slot_j_x, rel_j_z - slot_j_z);

            if ((d_ii + d_jj) > (d_ji + d_ij)) {
                field_at<void*>(slot_i, 0x00) = unit_j;
                field_at<void*>(slot_j, 0x00) = unit_i;
                const float w_i = field_at<float>(slot_i, 0x30);
                field_at<float>(slot_i, 0x30) = field_at<float>(slot_j, 0x30);
                field_at<float>(slot_j, 0x30) = w_i;
            }
        }
    }
}

// [0x0070e3a0] BSP_Entity_AsKind06
// Original ABI: __thiscall (ECX = self), RET.
void* Entity_AsKind06_0070e3a0(void* self) {
    if (self != nullptr) {
        using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
        if (vslot_at<IsKindFn>(self, 0x5c)(self, 6u) != 0u) {
            return self;
        }
    }
    return nullptr;
}

// [0x0070e3c0] BSP_UnitGroup_GetMaxFollowerTurnRadius
// Original ABI: __thiscall (ECX = self), RET (returns float in ST0).
float UnitGroup_GetMaxFollowerTurnRadius_0070e3c0(void* self) {
    float max_r = *reinterpret_cast<const float*>(0x00ce3d08);
    const std::int32_t count = field_at<std::int32_t>(self, 0x4f8);
    const std::uint8_t* slot = static_cast<const std::uint8_t*>(self) + 0x18;
    for (std::int32_t i = 0; i < count; ++i, slot += 0x34) {
        void* member = field_at<void*>(slot, 0x00);
        if (member != nullptr) {
            using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
            if (vslot_at<IsKindFn>(member, 0x5c)(member, 6u) != 0u &&
                member != field_at<void*>(self, 0x14)) {
                const float r = fn_at<float(__thiscall*)(void*)>(0x0082e850)(
                    field_at<void*>(member, 0x538)
                );
                if (!(max_r > r)) {
                    max_r = r;
                }
            }
        }
    }
    return max_r;
}

// [0x0070e450] BSP_ShipGroup_ControllerAreaKey
// Original ABI: __thiscall (ECX = self), RET.
std::int32_t ShipGroup_ControllerAreaKey_0070e450(void* self) {
    std::int32_t best = 0;
    const std::int32_t count = field_at<std::int32_t>(self, 0x4f8);
    const std::uint8_t* slot = static_cast<const std::uint8_t*>(self) + 0x18;
    for (std::int32_t i = 0; i < count; ++i, slot += 0x34) {
        void* member = field_at<void*>(slot, 0x00);
        if (member != nullptr) {
            using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
            if (vslot_at<IsKindFn>(member, 0x5c)(member, 6u) != 0u) {
                using AreaKeyFn = std::int32_t(__thiscall*)(void*);
                const std::int32_t k = vslot_at<AreaKeyFn>(member, 0x214)(member);
                if (best <= k) {
                    best = k;
                }
            }
        }
    }
    return best;
}

// [0x0070e4c0] BSP_UnitGroup_DetachMember
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = member, [ESP+8] = arg1), RET 8.
void UnitGroup_DetachMember_0070e4c0(void* self, void* member, std::uint32_t /*arg1*/) {
    if (member == field_at<void*>(self, 0x14) && field_at<std::int32_t>(self, 0x4f8) > 1) {
        void* next_leader = fn_at<void*(__thiscall*)(void*, void*)>(0x0070d8d0)(self, member);
        if (next_leader == nullptr) {
            const std::int32_t count = field_at<std::int32_t>(self, 0x4f8);
            const std::uint8_t* slot = static_cast<const std::uint8_t*>(self) + 0x18;
            for (std::int32_t i = 0; i < count; ++i, slot += 0x34) {
                void* m = field_at<void*>(slot, 0x00);
                if (m != nullptr && m != member) {
                    next_leader = m;
                    break;
                }
            }
        }
        void* old_ship = Entity_AsKind06_0070e3a0(member);
        void* new_ship = Entity_AsKind06_0070e3a0(next_leader);
        if (new_ship != old_ship) {
            if (old_ship != nullptr) {
                fn_at<void(__thiscall*)(void*, void*)>(0x00815e20)(new_ship, old_ship);
            }
            field_at<void*>(self, 0x14) = new_ship;
        }
    }

    field_at<void*>(member, 0x284) = nullptr;
    fn_at<void(__fastcall*)(void*, void*)>(0x006952a0)(member, self);

    bool found = false;
    std::uint8_t* slot_vec = static_cast<std::uint8_t*>(self) + 0x20;
    for (std::int32_t i = 0; i < field_at<std::int32_t>(self, 0x4f8); ++i, slot_vec += 0x34) {
        if (!found) {
            if (field_at<void*>(slot_vec, -0x08) != member) {
                continue;
            }
            field_at<std::int32_t>(self, 0x4f8) -= 1;
            found = true;
        }
        if (i < field_at<std::int32_t>(self, 0x4f8)) {
            field_at<void*>(slot_vec, -0x08) = field_at<void*>(slot_vec, 0x2c);
            for (std::ptrdiff_t off = -0x04; off <= 0x28; off += 4) {
                field_at<float>(slot_vec, off) = field_at<float>(slot_vec, off + 0x34);
            }
        }
    }

    if (field_at<std::int32_t>(self, 0x4f8) > 0) {
        UnitGroup_RefreshSpeedCeiling_0070da00(self);
    } else {
        using ScalarDeleteFn = void(__thiscall*)(void*, std::uint32_t);
        vslot_at<ScalarDeleteFn>(self, 0x00)(self, 1u);
    }
}

// [0x0070e620] BSP_UnitGroup_ReadSceneFields
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = reader), RET 4.
void UnitGroup_ReadSceneFields_0070e620(void* self, void* reader) {
    using ReadRefFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, void*);
    using ReadValFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, void*);
    using BeginGroupFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t);
    using HasElemFn = std::uint8_t(__thiscall*)(void*, std::uint32_t, std::int32_t);
    using EndGroupFn = void(__thiscall*)(void*);

    void* leader_out = nullptr;
    vslot_at<ReadRefFn>(reader, 0x10)(reader, 0u, 0x00cfd760u, 4u, &leader_out);
    field_at<void*>(self, 0x14) = leader_out;

    vslot_at<ReadValFn>(reader, 0x10)(reader, 0u, 0x00ce4ad0u, 1u, static_cast<std::uint8_t*>(self) + 0x4fc);
    vslot_at<ReadValFn>(reader, 0x10)(reader, 0u, 0x00cfd758u, 1u, static_cast<std::uint8_t*>(self) + 0x500);
    vslot_at<ReadValFn>(reader, 0x10)(reader, 0u, 0x00cfd74cu, 2u, static_cast<std::uint8_t*>(self) + 0x504);

    vslot_at<BeginGroupFn>(reader, 0x04)(reader, 0u, 0x00cfd740u);
    field_at<std::int32_t>(self, 0x4f8) = 0;

    for (std::int32_t idx = 0; vslot_at<HasElemFn>(reader, 0x14)(reader, 1u, idx) != 0u; ++idx) {
        const std::int32_t slot_idx = field_at<std::int32_t>(self, 0x4f8);
        std::uint8_t* slot = static_cast<std::uint8_t*>(self) + 0x18 + slot_idx * 0x34;
        vslot_at<BeginGroupFn>(reader, 0x04)(reader, 1u, static_cast<std::uintptr_t>(idx));
        void* member = nullptr;
        vslot_at<ReadRefFn>(reader, 0x10)(reader, 0u, 0x00cfd738u, 4u, &member);
        field_at<void*>(slot, 0x00) = member;
        if (member != nullptr) {
            field_at<void*>(member, 0x284) = self;
            fn_at<void(__fastcall*)(void*, void*)>(0x00694a60)(member, self);
        }
        vslot_at<ReadValFn>(reader, 0x10)(reader, 0u, 0x00cfd724u, 5u, slot + 0x04);
        vslot_at<BeginGroupFn>(reader, 0x04)(reader, 0u, 0x00cfd71cu);
        std::uint8_t* sub_f = slot + 0x20;
        for (std::int32_t k = 0; k < 4; ++k, sub_f += 4) {
            vslot_at<BeginGroupFn>(reader, 0x04)(reader, 1u, static_cast<std::uintptr_t>(k));
            vslot_at<ReadValFn>(reader, 0x10)(reader, 0u, 0x00ceb488u, 2u, sub_f - 0x10);
            vslot_at<ReadValFn>(reader, 0x10)(reader, 0u, 0x00cfd718u, 2u, sub_f);
            vslot_at<EndGroupFn>(reader, 0x08)(reader);
        }
        field_at<std::int32_t>(self, 0x4f8) += 1;
        vslot_at<EndGroupFn>(reader, 0x08)(reader);
        vslot_at<EndGroupFn>(reader, 0x08)(reader);
    }
    vslot_at<EndGroupFn>(reader, 0x08)(reader);
}

// [0x0070e890] BSP_ObserverMember_WriteRef_0070e890
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = writer), RET 4.
void ObserverMember_WriteRef_0070e890(void* self, void* writer) {
    using WriteRefFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, void*);
    using WriteValFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, std::uint32_t);
    using WritePtrFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t, std::uint32_t, const void*);
    using BeginGroupFn = void(__thiscall*)(void*, std::uint32_t, std::uintptr_t);
    using EndGroupFn = void(__thiscall*)(void*);

    vslot_at<WriteRefFn>(writer, 0x0c)(writer, 0u, 0x00cfd760u, 4u, field_at<void*>(self, 0x14));
    vslot_at<WriteValFn>(writer, 0x0c)(writer, 0u, 0x00ce4ad0u, 1u, field_at<std::uint32_t>(self, 0x4fc));
    vslot_at<WriteValFn>(writer, 0x0c)(writer, 0u, 0x00cfd758u, 1u, field_at<std::uint32_t>(self, 0x500));
    vslot_at<WriteValFn>(writer, 0x0c)(writer, 0u, 0x00cfd74cu, 2u, field_at<std::uint32_t>(self, 0x504));

    vslot_at<BeginGroupFn>(writer, 0x04)(writer, 0u, 0x00cfd740u);
    const std::uint8_t* slot_sub = static_cast<const std::uint8_t*>(self) + 0x38;
    for (std::int32_t idx = 0; idx < field_at<std::int32_t>(self, 0x4f8); ++idx, slot_sub += 0x34) {
        vslot_at<BeginGroupFn>(writer, 0x04)(writer, 1u, static_cast<std::uintptr_t>(idx));
        vslot_at<WriteRefFn>(writer, 0x0c)(writer, 0u, 0x00cfd738u, 4u, field_at<void*>(slot_sub, -0x20));
        vslot_at<WritePtrFn>(writer, 0x0c)(writer, 0u, 0x00cfd724u, 5u, slot_sub - 0x1c);
        vslot_at<BeginGroupFn>(writer, 0x04)(writer, 0u, 0x00cfd71cu);
        const std::uint8_t* sub_f = slot_sub;
        for (std::int32_t k = 0; k < 4; ++k, sub_f += 4) {
            vslot_at<BeginGroupFn>(writer, 0x04)(writer, 1u, static_cast<std::uintptr_t>(k));
            vslot_at<WriteValFn>(writer, 0x0c)(writer, 0u, 0x00ceb488u, 2u, field_at<std::uint32_t>(sub_f, -0x10));
            vslot_at<WriteValFn>(writer, 0x0c)(writer, 0u, 0x00cfd718u, 2u, field_at<std::uint32_t>(sub_f, 0x00));
            vslot_at<EndGroupFn>(writer, 0x08)(writer);
        }
        vslot_at<EndGroupFn>(writer, 0x08)(writer);
        vslot_at<EndGroupFn>(writer, 0x08)(writer);
    }
    vslot_at<EndGroupFn>(writer, 0x08)(writer);
}

// [0x0070eb00] BSP_ObserverMember_TargetWithinRange
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = max_dist), RET 4.
bool ObserverMember_TargetWithinRange_0070eb00(void* self, float max_dist) {
    const float max_dist_sq = max_dist * max_dist;
    void* leader = field_at<void*>(self, 0x14);
    if (leader == nullptr) {
        return false;
    }
    using IsKindFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
    if (vslot_at<IsKindFn>(leader, 0x5c)(leader, 6u) == 0u) {
        return false;
    }

    const std::uint8_t* slot = static_cast<const std::uint8_t*>(self) + 0x18;
    for (std::int32_t i = 0; i < field_at<std::int32_t>(self, 0x4f8); ++i, slot += 0x34) {
        void* member = field_at<void*>(slot, 0x00);
        if (member == nullptr || member == field_at<void*>(self, 0x14)) {
            continue;
        }
        const std::int32_t mode = field_at<std::int32_t>(self, 0x500);
        const float angle = field_at<float>(slot, 0x20 + mode * 4);
        float base_pos[3]{};
        float dir_vec[3]{};
        float unused_out = 0.0f;
        fn_at<void(__thiscall*)(void*, float, float*, float*, float*)>(0x00811150)(
            leader,
            angle,
            base_pos,
            dir_vec,
            &unused_out
        );
        const float orig_dx = dir_vec[0];
        dir_vec[0] = dir_vec[2];
        dir_vec[1] = 0.0f;
        dir_vec[2] = (*reinterpret_cast<const float*>(0x00d7a208)) - orig_dx;
        fn_at<void(__thiscall*)(float*)>(0x0042b260)(dir_vec);

        const float dist_offset = field_at<float>(slot, 0x10 + field_at<std::int32_t>(self, 0x500) * 4);
        const float target_pos[3]{
            base_pos[0] + dir_vec[0] * dist_offset,
            base_pos[1] + dir_vec[1] * dist_offset,
            base_pos[2] + dir_vec[2] * dist_offset
        };
        if (field_at<std::uint8_t>(member, 0xc8) == 0u) {
            fn_at<void(__thiscall*)(void*)>(0x00414db0)(member);
        }
        const float ex = target_pos[0] - field_at<float>(member, 0xfc);
        const float ey = target_pos[1] - field_at<float>(member, 0x100);
        const float ez = target_pos[2] - field_at<float>(member, 0x104);
        const float d2 = ex * ex + ey * ey + ez * ez;
        if (d2 > max_dist_sq) {
            return false;
        }
    }
    return true;
}

// [0x0070eca0] BSP_UnitGroup_RemoveEntity
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = entity), RET 4.
void UnitGroup_RemoveEntity_0070eca0(void* self, void* entity) {
    if (entity == nullptr) {
        return;
    }
    const std::int32_t count = field_at<std::int32_t>(self, 0x4f8);
    const std::uint8_t* slot = static_cast<const std::uint8_t*>(self) + 0x18;
    for (std::int32_t i = 0; i < count; ++i, slot += 0x34) {
        if (field_at<void*>(slot, 0x00) == entity) {
            UnitGroup_DetachMember_0070e4c0(self, field_at<void*>(slot, 0x00), 1u);
            return;
        }
    }
}

// [0x0070ecf0] BSP_UnitGroup_CreateFromScene
// Original ABI: __fastcall (ECX = reader), RET.
void* UnitGroup_CreateFromScene_0070ecf0(void* reader) {
    void* raw = fn_at<void*(__cdecl*)(std::size_t)>(0x00bf681b)(0x508u);
    void* grp = (raw != nullptr) ? fn_at<void*(__thiscall*)(void*)>(0x0070dab0)(raw) : nullptr;
    UnitGroup_ReadSceneFields_0070e620(grp, reader);
    return grp;
}

// [0x0070f090] BSP_UnitPartNode_SphereOverlapForward
// Original ABI: __thiscall (ECX = self, stack [ESP+4..10h] = center, radius, filter, out_list), RET 10h.
bool UnitPartNode_SphereOverlapForward_0070f090(
    void* self,
    const void* center,
    float radius,
    const void* filter,
    void* out_list
) {
    const std::uint8_t r = fn_at<std::uint8_t(__thiscall*)(void*, const void*, float, const void*, void*)>(
        0x0098aae0
    )(self, center, radius, filter, out_list);
    return r != 0u;
}

// [0x0070f3d0] STL_PartGroupRows_Allocate_0070f3d0
// Original ABI: __fastcall (ECX = count), RET.
void* STL_PartGroupRows_Allocate_0070f3d0(std::uint32_t count) {
    if (count > 0u && (0xffffffffu / count) < 0x10u) {
        std::uint8_t exc[12]{};
        const char* msg = nullptr;
        fn_at<void(__thiscall*)(void*, const char**)>(0x00bf6340)(exc, &msg);
        *reinterpret_cast<std::uintptr_t*>(exc) = 0x00d6923cu;
        fn_at<void(__stdcall*)(void*, std::uintptr_t)>(0x00bf6885)(exc, 0x00e03cc0u);
    }
    const std::size_t bytes = (count > 0u) ? (static_cast<std::size_t>(count) << 4u) : 0u;
    return fn_at<void*(__cdecl*)(std::size_t)>(0x00bf681b)(bytes);
}

// [0x0070f6b0] BSP_UnitPartCollisionShape_ConstructWithMesh
// Original ABI: __thiscall (ECX = self, stack [ESP+4..0Ch] = part_node, mesh_shape, user_word), RET 0Ch.
void* UnitPartCollisionShape_ConstructWithMesh_0070f6b0(
    void* self,
    void* part_node,
    void* mesh_shape,
    std::uint32_t user_word
) {
    field_at<std::uintptr_t>(self, 0x00) = 0x00ce89dcu;
    field_at<std::uint32_t>(self, 0x1c) = user_word;
    const float x = *reinterpret_cast<const float*>(0x00f87574);
    const float y = *reinterpret_cast<const float*>(0x00f87578);
    const float z = *reinterpret_cast<const float*>(0x00f8757c);
    field_at<float>(self, 0x10) = x;
    field_at<float>(self, 0x14) = y;
    field_at<float>(self, 0x18) = z;
    field_at<float>(self, 0x04) = x;
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfd768u;
    field_at<float>(self, 0x08) = y;
    field_at<void*>(self, 0x20) = part_node;
    field_at<void*>(self, 0x24) = mesh_shape;
    field_at<float>(self, 0x0c) = z;
    return self;
}

// [0x0070f710] BSP_CollisionShapeD768_SegmentSlot
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = segment_query), RET 4 (tail-jumps to 00723840).
std::uint32_t CollisionShapeD768_SegmentSlot_0070f710(void* self, void* segment_query) {
    void* mesh_shape = field_at<void*>(self, 0x24);
    if (mesh_shape != nullptr) {
        return fn_at<std::uint32_t(__thiscall*)(void*, void*)>(0x00723840)(mesh_shape, segment_query);
    }
    return 0u;
}

// [0x0070f720] BSP_UnitPartCollisionShape_TestSphere
// Original ABI: __thiscall (ECX = self, stack [ESP+4..0Ch] = center, radius, out_result), RET 0Ch.
bool UnitPartCollisionShape_TestSphere_0070f720(
    void* self,
    const void* center,
    float radius,
    void* out_result
) {
    void* part_node = field_at<void*>(self, 0x20);
    if ((field_at<std::uint8_t>(part_node, 0x5c) & 2u) == 0u) {
        fn_at<void(__thiscall*)(void*)>(0x00b6db70)(part_node);
    }
    void* xform_res = fn_at<void*(__thiscall*)(void*, const void*, float, void*)>(0x00b6e0d0)(
        field_at<void*>(self, 0x20),
        center,
        radius,
        out_result
    );
    const std::uint8_t hit = fn_at<std::uint8_t(__thiscall*)(void*, const void*, void*)>(0x00723f80)(
        field_at<void*>(self, 0x24),
        static_cast<const std::uint8_t*>(part_node) + 0xf0,
        xform_res
    );
    return hit != 0u;
}

// [0x0070f900] STL_LegacySboString_FindFirstOf_0070f900
// Original ABI: __thiscall (ECX = self, stack [ESP+4..0Ch] = needle, start_pos, needle_len), RET 0Ch.
std::int32_t STL_LegacySboString_FindFirstOf_0070f900(
    const void* self,
    const char* needle,
    std::uint32_t start_pos,
    std::uint32_t needle_len
) {
    const std::uint32_t str_len = field_at<std::uint32_t>(self, 0x14);
    if (needle_len > 0u && start_pos < str_len) {
        const char* base = sbo_c_str(self);
        const char* end = base + str_len;
        for (const char* p = base + start_pos; p < end; ++p) {
            const void* match = fn_at<const void*(__cdecl*)(const void*, int, std::size_t)>(0x00bfa700)(
                needle,
                static_cast<int>(*p),
                needle_len
            );
            if (match != nullptr) {
                return static_cast<std::int32_t>(p - sbo_c_str(self));
            }
        }
    }
    return -1;
}

// [0x0070fdb0] BSP_LogicalVertexStream_ReadNormal
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = out_xyz), RET 4.
// x87 DC-form note: 0070ff59 executes `FDIV ST1, ST0` (`dc f9`) with divisor 32767.0 (`[0x00ce5f98]`,
// case 10) or 65535.0 (`[0x00ce5f90]`, case 12) in ST0 and `nx` in ST1, followed by `FXCH; FSTP nx`
// while keeping the divisor in ST0 to divide `ny` (0070ff65) and `nz` (0070ff6b `FDIVR`).
// The 6 `dc ff` byte pairs reported at 0071005c..0071008c are jump-table data words (`0x0070ffdc`)
// in the `.text` switch table at `0x00710058`.
bool LogicalVertexStream_ReadNormal_0070fdb0(void* self, float* out_xyz) {
    const std::uint32_t fmt = field_at<std::uint32_t>(self, 0x0c);
    if (fmt > 0x0fu) {
        return false;
    }
    const std::uint8_t* ptr = field_at<const std::uint8_t*>(self, 0x14);
    float nx = 0.0f;
    float ny = 0.0f;
    float nz = 0.0f;
    switch (fmt) {
        case 0u: {
            const float* f = reinterpret_cast<const float*>(ptr);
            nx = f[0];
            ny = 0.0f;
            nz = 0.0f;
            ptr += 4;
            break;
        }
        case 1u: {
            const float* f = reinterpret_cast<const float*>(ptr);
            nx = f[0];
            ny = f[1];
            nz = 0.0f;
            ptr += 8;
            break;
        }
        case 2u: {
            const float* f = reinterpret_cast<const float*>(ptr);
            nx = f[0];
            ny = f[1];
            nz = f[2];
            ptr += 12;
            break;
        }
        case 3u: {
            const float* f = reinterpret_cast<const float*>(ptr);
            nx = f[0];
            ny = f[1];
            nz = f[2];
            ptr += 16;
            break;
        }
        case 4u:
        case 5u: {
            const std::uint32_t packed = *reinterpret_cast<const std::uint32_t*>(ptr);
            const double inv255 = *reinterpret_cast<const double*>(0x00ce5f70);
            nx = static_cast<float>(static_cast<double>((packed >> 16u) & 0xffu) * inv255);
            ny = static_cast<float>(static_cast<double>((packed >> 8u) & 0xffu) * inv255);
            nz = static_cast<float>(static_cast<double>(packed & 0xffu) * inv255);
            ptr += 4;
            break;
        }
        case 6u: {
            const std::int16_t* s = reinterpret_cast<const std::int16_t*>(ptr);
            nx = static_cast<float>(s[0]);
            ny = static_cast<float>(s[1]);
            nz = 0.0f;
            ptr += 4;
            break;
        }
        case 7u: {
            const std::int16_t* s = reinterpret_cast<const std::int16_t*>(ptr);
            nx = static_cast<float>(s[0]);
            ny = static_cast<float>(s[1]);
            nz = static_cast<float>(s[2]);
            ptr += 8;
            break;
        }
        case 8u: {
            const double inv255 = *reinterpret_cast<const double*>(0x00ce5f70);
            nx = static_cast<float>(static_cast<double>(ptr[0]) * inv255);
            ny = static_cast<float>(static_cast<double>(ptr[1]) * inv255);
            nz = static_cast<float>(static_cast<double>(ptr[2]) * inv255);
            ptr += 4;
            break;
        }
        case 9u: {
            const std::int16_t* s = reinterpret_cast<const std::int16_t*>(ptr);
            const double denom = *reinterpret_cast<const double*>(0x00ce5f98);
            nx = static_cast<float>(static_cast<double>(s[0]) / denom);
            ny = static_cast<float>(static_cast<double>(s[1]) / denom);
            nz = 0.0f;
            ptr += 4;
            break;
        }
        case 10u: {
            const std::int16_t* s = reinterpret_cast<const std::int16_t*>(ptr);
            const double denom = *reinterpret_cast<const double*>(0x00ce5f98);
            nx = static_cast<float>(static_cast<double>(s[0]) / denom);
            ny = static_cast<float>(static_cast<double>(s[1]) / denom);
            nz = static_cast<float>(static_cast<double>(s[2]) / denom);
            ptr += 8;
            break;
        }
        case 11u: {
            const std::uint16_t* u = reinterpret_cast<const std::uint16_t*>(ptr);
            const double denom = *reinterpret_cast<const double*>(0x00ce5f90);
            nx = static_cast<float>(static_cast<double>(u[0]) / denom);
            ny = static_cast<float>(static_cast<double>(u[1]) / denom);
            nz = 0.0f;
            ptr += 4;
            break;
        }
        case 12u: {
            const std::uint16_t* u = reinterpret_cast<const std::uint16_t*>(ptr);
            const double denom = *reinterpret_cast<const double*>(0x00ce5f90);
            nx = static_cast<float>(static_cast<double>(u[0]) / denom);
            ny = static_cast<float>(static_cast<double>(u[1]) / denom);
            nz = static_cast<float>(static_cast<double>(u[2]) / denom);
            ptr += 8;
            break;
        }
        default:
            return false;
    }
    field_at<const std::uint8_t*>(self, 0x14) = ptr;
    out_xyz[0] = nx;
    out_xyz[1] = ny;
    out_xyz[2] = nz;
    return true;
}

// [0x007100a0] BSP_LogicalVertexStream_ReadUV
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = out_uv), RET 4.
// x87 DC-form note: 00710184 executes `FDIV ST1, ST0` (`dc f9`) with divisor 32767.0 (`[0x00ce5f98]`,
// case 9) or 65535.0 (`[0x00ce5f90]`, case 11) in ST0 and `u` in ST1, retaining the divisor in ST0
// to divide `v` (00710190) and the unused third component (00710196 `FDIVR`).
bool LogicalVertexStream_ReadUV_007100a0(void* self, float* out_uv) {
    const std::uint32_t fmt = field_at<std::uint32_t>(self, 0x0c);
    if (fmt > 0x0fu) {
        return false;
    }
    const std::uint8_t* ptr = field_at<const std::uint8_t*>(self, 0x14);
    float u_val = 0.0f;
    float v_val = 0.0f;
    switch (fmt) {
        case 0u: {
            const float* f = reinterpret_cast<const float*>(ptr);
            u_val = f[0];
            v_val = 0.0f;
            ptr += 4;
            break;
        }
        case 1u: {
            const float* f = reinterpret_cast<const float*>(ptr);
            u_val = f[0];
            v_val = f[1];
            ptr += 8;
            break;
        }
        case 2u: {
            const float* f = reinterpret_cast<const float*>(ptr);
            u_val = f[0];
            v_val = f[1];
            ptr += 12;
            break;
        }
        case 3u: {
            const float* f = reinterpret_cast<const float*>(ptr);
            u_val = f[0];
            v_val = f[1];
            ptr += 16;
            break;
        }
        case 4u:
        case 5u: {
            const std::uint32_t packed = *reinterpret_cast<const std::uint32_t*>(ptr);
            const double inv255 = *reinterpret_cast<const double*>(0x00ce5f70);
            u_val = static_cast<float>(static_cast<double>((packed >> 16u) & 0xffu) * inv255);
            v_val = static_cast<float>(static_cast<double>((packed >> 8u) & 0xffu) * inv255);
            ptr += 4;
            break;
        }
        case 6u: {
            const std::int16_t* s = reinterpret_cast<const std::int16_t*>(ptr);
            u_val = static_cast<float>(s[0]);
            v_val = static_cast<float>(s[1]);
            ptr += 4;
            break;
        }
        case 7u: {
            const std::int16_t* s = reinterpret_cast<const std::int16_t*>(ptr);
            u_val = static_cast<float>(s[0]);
            v_val = static_cast<float>(s[1]);
            ptr += 8;
            break;
        }
        case 8u: {
            const double inv255 = *reinterpret_cast<const double*>(0x00ce5f70);
            u_val = static_cast<float>(static_cast<double>(ptr[0]) * inv255);
            v_val = static_cast<float>(static_cast<double>(ptr[1]) * inv255);
            ptr += 4;
            break;
        }
        case 9u: {
            const std::int16_t* s = reinterpret_cast<const std::int16_t*>(ptr);
            const double denom = *reinterpret_cast<const double*>(0x00ce5f98);
            u_val = static_cast<float>(static_cast<double>(s[0]) / denom);
            v_val = static_cast<float>(static_cast<double>(s[1]) / denom);
            ptr += 4;
            break;
        }
        case 10u: {
            const std::int16_t* s = reinterpret_cast<const std::int16_t*>(ptr);
            const double denom = *reinterpret_cast<const double*>(0x00ce5f98);
            u_val = static_cast<float>(static_cast<double>(s[0]) / denom);
            v_val = static_cast<float>(static_cast<double>(s[1]) / denom);
            ptr += 8;
            break;
        }
        case 11u: {
            const std::uint16_t* u = reinterpret_cast<const std::uint16_t*>(ptr);
            const double denom = *reinterpret_cast<const double*>(0x00ce5f90);
            u_val = static_cast<float>(static_cast<double>(u[0]) / denom);
            v_val = static_cast<float>(static_cast<double>(u[1]) / denom);
            ptr += 4;
            break;
        }
        case 12u: {
            const std::uint16_t* u = reinterpret_cast<const std::uint16_t*>(ptr);
            const double denom = *reinterpret_cast<const double*>(0x00ce5f90);
            u_val = static_cast<float>(static_cast<double>(u[0]) / denom);
            v_val = static_cast<float>(static_cast<double>(u[1]) / denom);
            ptr += 8;
            break;
        }
        default:
            return false;
    }
    field_at<const std::uint8_t*>(self, 0x14) = ptr;
    out_uv[0] = u_val;
    out_uv[1] = v_val;
    return true;
}

// [0x00710360] BSP_UnitPartEffectPointers_PushBack_00710360
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = elem_ptr), RET 4.
void UnitPartEffectPointers_PushBack_00710360(void* self, const void* elem_ptr) {
    const std::int32_t cap = field_at<std::int32_t>(self, 0x08);
    if (field_at<std::int32_t>(self, 0x04) == cap) {
        const std::int32_t new_cap = (cap * 2 > 1) ? (cap * 2) : 1;
        fn_at<void(__thiscall*)(void*, std::int32_t)>(0x007102a0)(self, new_cap);
    }
    std::uint32_t* slot = field_at<std::uint32_t*>(self, 0x00) + field_at<std::int32_t>(self, 0x04);
    if (slot != nullptr) {
        *slot = *static_cast<const std::uint32_t*>(elem_ptr);
    }
    field_at<std::int32_t>(self, 0x04) += 1;
}

// [0x007103e0] BSP_UnitPartEntryPointers_PushBack_007103e0
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = elem_ptr), RET 4.
void UnitPartEntryPointers_PushBack_007103e0(void* self, const void* elem_ptr) {
    const std::int32_t cap = field_at<std::int32_t>(self, 0x08);
    if (field_at<std::int32_t>(self, 0x04) == cap) {
        const std::int32_t new_cap = (cap * 2 > 1) ? (cap * 2) : 1;
        fn_at<void(__thiscall*)(void*, std::int32_t)>(0x0070faa0)(self, new_cap);
    }
    std::uint32_t* slot = field_at<std::uint32_t*>(self, 0x00) + field_at<std::int32_t>(self, 0x04);
    if (slot != nullptr) {
        *slot = *static_cast<const std::uint32_t*>(elem_ptr);
    }
    field_at<std::int32_t>(self, 0x04) += 1;
}

// [0x007104e0] STL_List_Clear_007104e0
// Original ABI: __thiscall (ECX = self), RET.
void STL_List_Clear_007104e0(void* self) {
    void* head = field_at<void*>(self, 0x04);
    void* node = field_at<void*>(head, 0x00);
    field_at<void*>(head, 0x00) = head;
    head = field_at<void*>(self, 0x04);
    field_at<void*>(head, 0x04) = head;
    field_at<std::uint32_t>(self, 0x08) = 0u;
    while (node != field_at<void*>(self, 0x04)) {
        void* next = field_at<void*>(node, 0x00);
        fn_at<void(__cdecl*)(void*)>(0x00bf65ac)(node);
        node = next;
    }
}

// [0x00710530] STL_List_Clear_00710530
// Original ABI: __thiscall (ECX = self), RET.
void STL_List_Clear_00710530(void* self) {
    STL_List_Clear_007104e0(self);
}

// [0x00710630] BSP_RefPtr_Release_00710630
// Original ABI: __thiscall (ECX = self), RET.
void* RefPtr_Release_00710630(void* self) {
    void* obj = field_at<void*>(self, 0x00);
    if (obj != nullptr) {
        release_refcounted_obj(obj);
        field_at<void*>(self, 0x00) = nullptr;
    }
    return self;
}

// [0x007106b0] BSP_RefPtr_Release_007106b0
// Original ABI: __thiscall (ECX = self), RET.
void* RefPtr_Release_007106b0(void* self) {
    return RefPtr_Release_00710630(self);
}

// [0x00710700] STL_MoveDwords_00710700
// Original ABI: __thiscall (ECX = self, stack [ESP+4..0Ch] = first, last, dst), RET 0Ch.
void* STL_MoveDwords_00710700(void* /*self*/, const void* first, const void* last, void* dst) {
    const std::ptrdiff_t count = (static_cast<const std::uint8_t*>(last) - static_cast<const std::uint8_t*>(first)) >> 2;
    const std::size_t bytes = static_cast<std::size_t>(count) * 4u;
    std::uint8_t* end_dst = static_cast<std::uint8_t*>(dst) + bytes;
    if (bytes != 0u) {
        fn_at<void(__cdecl*)(void*, std::size_t, const void*, std::size_t)>(0x00bf67a7)(
            dst,
            bytes,
            first,
            bytes
        );
    }
    return end_dst;
}

// [0x007107e0] BSP_UnitPartEffectPointers_PushBack_007107e0
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = elem_ptr), RET 4.
void UnitPartEffectPointers_PushBack_007107e0(void* self, const void* elem_ptr) {
    UnitPartEffectPointers_PushBack_00710360(self, elem_ptr);
}

// [0x00710820] BSP_UnitPartEntryPointers_PushBack_00710820
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = elem_ptr), RET 4.
void UnitPartEntryPointers_PushBack_00710820(void* self, const void* elem_ptr) {
    UnitPartEntryPointers_PushBack_007103e0(self, elem_ptr);
}

// [0x007109a0] BSP_RefPtr_Assign_007109a0
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = new_ptr), RET 4.
void* RefPtr_Assign_007109a0(void* self, void* new_ptr) {
    void* old_obj = field_at<void*>(self, 0x00);
    if (old_obj != nullptr) {
        release_refcounted_obj(old_obj);
        field_at<void*>(self, 0x00) = nullptr;
    }
    field_at<void*>(self, 0x00) = new_ptr;
    return self;
}

// [0x007109e0] STL_FindDword_007109e0
// Original ABI: __fastcall (ECX = out_iter, EDX = key_ptr, stack [ESP+4..10h] = container_tag, first, unused_tag, last), RET 10h.
void* STL_FindDword_007109e0(
    void* out_iter,
    const std::uint32_t* key_ptr,
    std::uint32_t /*container_tag*/,
    std::uint32_t* first,
    std::uint32_t /*unused_tag*/,
    std::uint32_t* last
) {
    std::uint32_t* cur = first;
    if (cur != last) {
        const std::uint32_t key = *key_ptr;
        while (*cur != key) {
            ++cur;
            if (cur == last) {
                break;
            }
        }
    }
    field_at<std::uint32_t*>(out_iter, 0x04) = cur;
    field_at<std::uint32_t*>(out_iter, 0x00) = first;
    return out_iter;
}

// [0x00710a60] BSP_RefPtr_Release_00710a60
// Original ABI: __thiscall (ECX = self), RET.
void RefPtr_Release_00710a60(void* self) {
    (void)RefPtr_Release_00710630(self);
}

// [0x00710e10] BSP_UnitPartEffect_AttachAtPart
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = part_ref_by_val), RET 4.
void UnitPartEffect_AttachAtPart_00710e10(void* self, void* part_ref) {
    const float one = *reinterpret_cast<const float*>(0x00d7a24c);
    float ident_mat[16]{
        one, 0.0f, 0.0f, 0.0f,
        0.0f, one, 0.0f, 0.0f,
        0.0f, 0.0f, one, 0.0f,
        0.0f, 0.0f, 0.0f, one
    };
    void* owner = field_at<void*>(self, 0x00);
    if (part_ref != nullptr) {
        interlocked_inc_00ce221c(&field_at<volatile std::int32_t>(part_ref, 0x04));
    }
    void* out_handle = nullptr;
    void** ret_ptr = fn_at<void**(__fastcall*)(void*, void*, void*, float*, std::uint32_t, std::uint32_t, std::uint32_t)>(
        0x008687c0
    )(&out_handle, owner, part_ref, ident_mat, 1u, 0u, 0u);
    void* attached = *ret_ptr;
    release_refcounted_obj(out_handle);

    void* vec = static_cast<std::uint8_t*>(self) + 0x04;
    UnitPartEffectPointers_PushBack_00710360(vec, &attached);
    release_refcounted_obj(part_ref);
}

// [0x00710f70] STL_List_Construct_UnitParts_00710f70
// Original ABI: __thiscall (ECX = self), RET.
void* STL_List_Construct_UnitParts_00710f70(void* self) {
    void* head = fn_at<void*(__cdecl*)()>(0x007103a0)();
    field_at<void*>(self, 0x04) = head;
    field_at<std::uint32_t>(self, 0x08) = 0u;
    return self;
}

// [0x00710fa0] STL_List_Construct_UnitPartShapes_00710fa0
// Original ABI: __thiscall (ECX = self), RET.
void* STL_List_Construct_UnitPartShapes_00710fa0(void* self) {
    void* head = fn_at<void*(__cdecl*)()>(0x007103c0)();
    field_at<void*>(self, 0x04) = head;
    field_at<std::uint32_t>(self, 0x08) = 0u;
    return self;
}

// [0x00711210] BSP_UnitPartEffect_StartByIndex
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = index, [ESP+8] = effect_arg), RET 8.
void UnitPartEffect_StartByIndex_00711210(void* self, std::int32_t index, void* effect_arg) {
    if (index < 0 || index >= field_at<std::int32_t>(self, 0x1a4)) {
        return;
    }
    void* eff_ref = nullptr;
    fn_at<void(__fastcall*)(void*, void*, std::uint32_t)>(0x00871ba0)(&eff_ref, effect_arg, 0u);
    if (eff_ref != nullptr) {
        interlocked_inc_00ce221c(&field_at<volatile std::int32_t>(eff_ref, 0x04));
        void** slots = field_at<void**>(self, 0x1a0);
        UnitPartEffect_AttachAtPart_00710e10(slots[index], eff_ref);
    }
    release_refcounted_obj(eff_ref);
}

// [0x007114a0] STL_Vector_Tidy_007114a0
// Original ABI: __thiscall (ECX = self), RET.
void STL_Vector_Tidy_007114a0(void* self) {
    void* buf = field_at<void*>(self, 0x04);
    if (buf != nullptr) {
        fn_at<void(__cdecl*)(void*)>(0x00bf65ac)(buf);
    }
    field_at<void*>(self, 0x04) = nullptr;
    field_at<void*>(self, 0x08) = nullptr;
    field_at<void*>(self, 0x0c) = nullptr;
}

// [0x00712100] STL_Vector_Assign_00712100
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = rhs), RET 4.
void* STL_Vector_Assign_00712100(void* self, const void* rhs) {
    if (self == rhs) {
        return self;
    }
    const std::uint8_t* rhs_first = field_at<const std::uint8_t*>(rhs, 0x04);
    const std::uint8_t* rhs_last = field_at<const std::uint8_t*>(rhs, 0x08);
    const std::uint32_t rhs_len = (rhs_first != nullptr)
        ? static_cast<std::uint32_t>((rhs_last - rhs_first) >> 2)
        : 0u;
    if (rhs_len == 0u) {
        fn_at<void(__thiscall*)(void*)>(0x004fdca0)(self);
        return self;
    }

    std::uint8_t* lhs_first = field_at<std::uint8_t*>(self, 0x04);
    const std::uint32_t lhs_len = (lhs_first != nullptr)
        ? static_cast<std::uint32_t>((field_at<std::uint8_t*>(self, 0x08) - lhs_first) >> 2)
        : 0u;
    if (rhs_len <= lhs_len) {
        fn_at<void(__cdecl*)(const void*, const void*, void*)>(0x004fc490)(rhs_first, rhs_last, lhs_first);
        const std::uint8_t* rf = field_at<const std::uint8_t*>(rhs, 0x04);
        const std::uint32_t n = (rf != nullptr)
            ? static_cast<std::uint32_t>((field_at<const std::uint8_t*>(rhs, 0x08) - rf) >> 2)
            : 0u;
        field_at<std::uint8_t*>(self, 0x08) = field_at<std::uint8_t*>(self, 0x04) + n * 4u;
        return self;
    }

    const std::uint32_t lhs_cap = (lhs_first != nullptr)
        ? static_cast<std::uint32_t>((field_at<std::uint8_t*>(self, 0x0c) - lhs_first) >> 2)
        : 0u;
    if (rhs_len <= lhs_cap) {
        const std::uint8_t* mid = rhs_first + lhs_len * 4u;
        fn_at<void(__cdecl*)(const void*, const void*, void*)>(0x004fc490)(rhs_first, mid, lhs_first);
        field_at<void*>(self, 0x08) = STL_MoveDwords_00710700(
            self,
            mid,
            field_at<const void*>(rhs, 0x08),
            field_at<void*>(self, 0x08)
        );
        return self;
    }

    if (lhs_first != nullptr) {
        fn_at<void(__cdecl*)(void*)>(0x00bf65ac)(lhs_first);
    }
    const std::uint8_t* rf = field_at<const std::uint8_t*>(rhs, 0x04);
    const std::uint32_t alloc_n = (rf != nullptr)
        ? static_cast<std::uint32_t>((field_at<const std::uint8_t*>(rhs, 0x08) - rf) >> 2)
        : 0u;
    if (fn_at<std::uint8_t(__thiscall*)(void*, std::uint32_t)>(0x00505750)(self, alloc_n) != 0u) {
        field_at<void*>(self, 0x08) = STL_MoveDwords_00710700(
            self,
            field_at<const void*>(rhs, 0x04),
            field_at<const void*>(rhs, 0x08),
            field_at<void*>(self, 0x04)
        );
    }
    return self;
}

// [0x00712270] STL_ConstructDwordVector_00712270
// Original ABI: __fastcall (ECX = dst, EDX = src), RET.
void STL_ConstructDwordVector_00712270(void* dst, const void* src) {
    if (dst != nullptr) {
        fn_at<void(__thiscall*)(void*, const void*)>(0x00712040)(dst, src);
    }
}

// [0x00712330] STL_PartGroupRows_DestroyRange_Thunk_00712330
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = first, [ESP+8] = last), RET 8.
void STL_PartGroupRows_DestroyRange_Thunk_00712330(void* self, void* first, void* last) {
    fn_at<void(__fastcall*)(void*, void*, void*, void*)>(0x00711f70)(first, last, self, last);
}

// [0x007123c0] STL_ConstructDwordVector_007123c0
// Original ABI: __stdcall (stack [ESP+4] = dst, [ESP+8] = src), RET 8.
void STL_ConstructDwordVector_007123c0(void* dst, const void* src) {
    if (dst != nullptr) {
        fn_at<void(__thiscall*)(void*, const void*)>(0x00712040)(dst, src);
    }
}

// [0x00712900] STL_PartGroupRows_AssignForward_00712900
// Original ABI: __cdecl (stack [ESP+4] = first, [ESP+8] = last, [ESP+0Ch] = dst), RET.
void* STL_PartGroupRows_AssignForward_00712900(void* first, void* last, void* dst) {
    std::uint8_t* src_cur = static_cast<std::uint8_t*>(first);
    std::uint8_t* src_end = static_cast<std::uint8_t*>(last);
    std::uint8_t* dst_cur = static_cast<std::uint8_t*>(dst);
    std::uint8_t* dst_end = dst_cur + (((src_end - src_cur) >> 4) << 4);
    while (src_cur != src_end) {
        STL_Vector_Assign_00712100(dst_cur, src_cur);
        src_cur += 0x10;
        dst_cur += 0x10;
    }
    return dst_end;
}

// [0x00712940] STL_PartGroupRows_AssignRepeated_00712940
// Original ABI: __cdecl (stack [ESP+4] = first, [ESP+8] = last, [ESP+0Ch] = value), RET.
void STL_PartGroupRows_AssignRepeated_00712940(void* first, void* last, const void* value) {
    std::uint8_t* cur = static_cast<std::uint8_t*>(first);
    std::uint8_t* end = static_cast<std::uint8_t*>(last);
    while (cur != end) {
        STL_Vector_Assign_00712100(cur, value);
        cur += 0x10;
    }
}

// [0x00712970] STL_PartGroupRows_UninitializedFill_00712970
// Original ABI: __fastcall (ECX = dst, EDX = count, stack [ESP+4..10h] = value, alloc_ctx, unused0, unused1), RET 10h.
// Includes EH catch-all funclet Catch_All@007129cb (007129db CALL 0x00711c00, 007129eb CALL 0x00bf6885).
void STL_PartGroupRows_UninitializedFill_00712970(
    void* dst,
    std::uint32_t count,
    const void* value,
    void* /*alloc_ctx*/,
    std::uint32_t /*unused0*/,
    std::uint32_t /*unused1*/
) {
    std::uint8_t* cur = static_cast<std::uint8_t*>(dst);
    while (count > 0u) {
        if (cur != nullptr) {
            fn_at<void(__thiscall*)(void*, const void*)>(0x00712040)(cur, value);
        }
        --count;
        cur += 0x10;
    }
}

// [0x00712a10] STL_PartGroupRows_TransferConstruct_00712a10
// Original ABI: __fastcall (ECX = first, EDX = last, stack [ESP+4..10h] = dst, alloc_ctx, unused0, unused1), RET 10h.
// Includes EH catch-all funclet Catch_All@00712a9e (00712ab3 CALL 0x00711c00, 00712ac3 CALL 0x00bf6885).
void* STL_PartGroupRows_TransferConstruct_00712a10(
    void* first,
    void* last,
    void* dst,
    void* /*alloc_ctx*/,
    std::uint32_t /*unused0*/,
    std::uint32_t /*unused1*/
) {
    std::uint32_t empty_vec[4]{0u, 0u, 0u, 0u};
    std::uint8_t* src_cur = static_cast<std::uint8_t*>(first);
    std::uint8_t* src_end = static_cast<std::uint8_t*>(last);
    std::uint8_t* dst_cur = static_cast<std::uint8_t*>(dst);
    while (src_cur != src_end) {
        if (dst_cur != nullptr) {
            fn_at<void(__thiscall*)(void*, const void*)>(0x00712040)(dst_cur, empty_vec);
        }
        for (std::ptrdiff_t off = 0x04; off <= 0x0c; off += 0x04) {
            void* tmp = field_at<void*>(src_cur, off);
            field_at<void*>(src_cur, off) = field_at<void*>(dst_cur, off);
            field_at<void*>(dst_cur, off) = tmp;
        }
        dst_cur += 0x10;
        src_cur += 0x10;
    }
    if (empty_vec[1] != 0u) {
        fn_at<void(__cdecl*)(void*)>(0x00bf65ac)(reinterpret_cast<void*>(empty_vec[1]));
    }
    return dst_cur;
}

// [0x00712af0] STL_SwapBackward_Vector16_00712af0
// Original ABI: __cdecl (stack [ESP+4] = first, [ESP+8] = last, [ESP+0Ch] = dst_last), RET.
void STL_SwapBackward_Vector16_00712af0(void* first, void* last, void* dst_last) {
    std::uint8_t* src_cur = static_cast<std::uint8_t*>(last);
    std::uint8_t* dst_cur = static_cast<std::uint8_t*>(dst_last);
    std::uint8_t* src_begin = static_cast<std::uint8_t*>(first);
    while (src_cur != src_begin) {
        src_cur -= 0x10;
        dst_cur -= 0x10;
        for (std::ptrdiff_t off = 0x04; off <= 0x0c; off += 0x04) {
            void* tmp = field_at<void*>(src_cur, off);
            field_at<void*>(src_cur, off) = field_at<void*>(dst_cur, off);
            field_at<void*>(dst_cur, off) = tmp;
        }
    }
}

// [0x00712f20] STL_PartGroupRows_UninitializedFillAdapter_00712f20
// Original ABI: __thiscall (ECX = self, stack [ESP+4..0Ch] = dst, count, value), RET 0Ch.
void* STL_PartGroupRows_UninitializedFillAdapter_00712f20(
    void* self,
    void* dst,
    std::uint32_t count,
    const void* value
) {
    STL_PartGroupRows_UninitializedFill_00712970(dst, count, value, self, 0u, 0u);
    return static_cast<std::uint8_t*>(dst) + (count << 4u);
}

// [0x00712f60] STL_PartGroupRows_TransferAdapter_00712f60
// Original ABI: __thiscall (ECX = self, stack [ESP+4..0Ch] = first, last, dst), RET 0Ch.
void* STL_PartGroupRows_TransferAdapter_00712f60(
    void* self,
    void* first,
    void* last,
    void* dst
) {
    return STL_PartGroupRows_TransferConstruct_00712a10(first, last, dst, self, 0u, 0u);
}

// [0x00712f90] STL_PartGroupRows_BackwardTransferAdapter_00712f90
// Original ABI: __cdecl (stack [ESP+4..10h] = first, last, dst_last, self), RET.
void* STL_PartGroupRows_BackwardTransferAdapter_00712f90(
    void* /*self*/,
    void* first,
    void* last,
    void* dst_last
) {
    STL_SwapBackward_Vector16_00712af0(first, last, dst_last);
    const std::ptrdiff_t bytes = ((static_cast<std::uint8_t*>(last) - static_cast<std::uint8_t*>(first)) >> 4) << 4;
    return static_cast<std::uint8_t*>(dst_last) - bytes;
}

// [0x00712ff0] STL_PartGroupRows_InsertCount_00712ff0
// Original ABI: __thiscall (ECX = self, stack [ESP+4..10h] = pos_owner, pos_ptr, count, value), RET 10h.
// Includes EH catch-all funclets Catch_All@0071315e and Catch_All@007131e7.
void STL_PartGroupRows_InsertCount_00712ff0(
    void* self,
    std::uint32_t /*pos_owner*/,
    void* pos_ptr,
    std::uint32_t count,
    const void* value
) {
    std::uint32_t local_val[4]{};
    fn_at<void(__thiscall*)(void*, const void*)>(0x00712040)(local_val, value);

    std::uint8_t* first = field_at<std::uint8_t*>(self, 0x04);
    std::uint32_t cap = (first != nullptr)
        ? static_cast<std::uint32_t>((field_at<std::uint8_t*>(self, 0x0c) - first) >> 4)
        : 0u;
    if (count != 0u) {
        const std::uint32_t len = (first != nullptr)
            ? static_cast<std::uint32_t>((field_at<std::uint8_t*>(self, 0x08) - first) >> 4)
            : 0u;
        if (0x0fffffffu - len < count) {
            fn_at<void(__cdecl*)()>(0x00711e60)();
        }
        if (cap < len + count) {
            cap = (0x0fffffffu - (cap >> 1u) < cap) ? 0u : (cap + (cap >> 1u));
            if (cap < len + count) {
                cap = len + count;
            }
            std::uint8_t* new_buf = static_cast<std::uint8_t*>(STL_PartGroupRows_Allocate_0070f3d0(cap));
            void* mid = STL_PartGroupRows_TransferConstruct_00712a10(first, pos_ptr, new_buf, self, 0u, 0u);
            mid = STL_PartGroupRows_UninitializedFillAdapter_00712f20(self, mid, count, local_val);
            void* end_new = STL_PartGroupRows_TransferConstruct_00712a10(
                pos_ptr,
                field_at<void*>(self, 0x08),
                mid,
                self,
                0u,
                0u
            );
            (void)end_new;
            if (first != nullptr) {
                fn_at<void(__fastcall*)(void*, void*, void*, void*)>(0x00711f70)(
                    first,
                    field_at<void*>(self, 0x08),
                    self,
                    mid
                );
                fn_at<void(__cdecl*)(void*)>(0x00bf65ac)(first);
            }
            field_at<std::uint8_t*>(self, 0x0c) = new_buf + (cap << 4u);
            field_at<std::uint8_t*>(self, 0x08) = new_buf + ((len + count) << 4u);
            field_at<std::uint8_t*>(self, 0x04) = new_buf;
        } else {
            std::uint8_t* last = field_at<std::uint8_t*>(self, 0x08);
            std::uint8_t* pos = static_cast<std::uint8_t*>(pos_ptr);
            const std::uint32_t tail_elems = static_cast<std::uint32_t>((last - pos) >> 4);
            if (tail_elems < count) {
                const std::uint32_t byte_shift = count << 4u;
                STL_PartGroupRows_TransferAdapter_00712f60(self, pos, last, pos + byte_shift);
                STL_PartGroupRows_UninitializedFillAdapter_00712f20(
                    self,
                    field_at<void*>(self, 0x08),
                    count - tail_elems,
                    local_val
                );
                field_at<std::uint8_t*>(self, 0x08) += byte_shift;
                STL_PartGroupRows_AssignRepeated_00712940(
                    pos,
                    field_at<std::uint8_t*>(self, 0x08) - byte_shift,
                    local_val
                );
            } else {
                const std::uint32_t byte_shift = count << 4u;
                std::uint8_t* split = last - byte_shift;
                field_at<void*>(self, 0x08) = STL_PartGroupRows_TransferAdapter_00712f60(self, split, last, last);
                STL_PartGroupRows_BackwardTransferAdapter_00712f90(self, pos, split, last);
                STL_PartGroupRows_AssignRepeated_00712940(pos, pos + byte_shift, local_val);
            }
        }
    }
    if (local_val[1] != 0u) {
        fn_at<void(__cdecl*)(void*)>(0x00bf65ac)(reinterpret_cast<void*>(local_val[1]));
    }
}

// [0x00713270] STL_PartGroupRows_Resize_00713270
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = new_size, [ESP+8..14h] = 16-byte value vector), RET 14h.
void STL_PartGroupRows_Resize_00713270(
    void* self,
    std::uint32_t new_size,
    std::uint32_t val_w0,
    void* val_first,
    void* val_last,
    void* val_end
) {
    const void* val_words[4]{
        reinterpret_cast<const void*>(static_cast<std::uintptr_t>(val_w0)),
        val_first,
        val_last,
        val_end
    };
    std::uint8_t* first = field_at<std::uint8_t*>(self, 0x04);
    const std::uint32_t cur_size = (first != nullptr)
        ? static_cast<std::uint32_t>((field_at<std::uint8_t*>(self, 0x08) - first) >> 4)
        : 0u;
    if (cur_size < new_size) {
        STL_PartGroupRows_InsertCount_00712ff0(
            self,
            reinterpret_cast<std::uintptr_t>(self),
            field_at<void*>(self, 0x08),
            new_size - cur_size,
            val_words
        );
    } else if (first != nullptr && new_size < cur_size) {
        std::uint8_t* erase_first = first + (new_size << 4u);
        std::uint8_t* erase_last = field_at<std::uint8_t*>(self, 0x08);
        std::uint32_t out_iter[2]{};
        fn_at<void*(__thiscall*)(void*, void*, void*, void*, void*, void*)>(0x00712b80)(
            self,
            out_iter,
            self,
            erase_first,
            self,
            erase_last
        );
    }
    if (val_first != nullptr) {
        fn_at<void(__cdecl*)(void*)>(0x00bf65ac)(val_first);
    }
}

// [0x00713750] BSP_ResourceItemNote_GetRuntimeTypeToken
std::uint32_t ResourceItemNote_GetRuntimeTypeToken_00713750() {
    return *reinterpret_cast<const std::uint32_t*>(0x00e19b54);
}

// [0x00713760] BSP_ResourceItemNote_GetRuntimeTypeNameAddress
std::uint32_t ResourceItemNote_GetRuntimeTypeNameAddress_00713760() {
    return *reinterpret_cast<const std::uint32_t*>(0x00e19b60);
}

// [0x00713790] BSP_ResourceItemAux_GetRuntimeTypeToken
std::uint32_t ResourceItemAux_GetRuntimeTypeToken_00713790() {
    return *reinterpret_cast<const std::uint32_t*>(0x00e19b64);
}

// [0x007137a0] BSP_ResourceItemAux_GetRuntimeTypeNameAddress
std::uint32_t ResourceItemAux_GetRuntimeTypeNameAddress_007137a0() {
    return *reinterpret_cast<const std::uint32_t*>(0x00e19b70);
}

// [0x007137d0] BSP_ResourceItemZoneDesc_GetRuntimeTypeToken
std::uint32_t ResourceItemZoneDesc_GetRuntimeTypeToken_007137d0() {
    return *reinterpret_cast<const std::uint32_t*>(0x00e19b74);
}

// [0x007137e0] BSP_ResourceItemZoneDesc_GetRuntimeTypeNameAddress
std::uint32_t ResourceItemZoneDesc_GetRuntimeTypeNameAddress_007137e0() {
    return *reinterpret_cast<const std::uint32_t*>(0x00e19b80);
}

// [0x00714030] BSP_GameResource_AsType44_00714030
void* GameResource_AsType44_00714030(void* self) {
    if (self != nullptr) {
        const std::uint32_t tok = fn_at<std::uint32_t(__cdecl*)()>(0x00721c90)();
        using AcceptsFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
        if (vslot_at<AcceptsFn>(self, 0x0c)(self, tok) != 0u) {
            return self;
        }
    }
    return nullptr;
}

// [0x00714060] BSP_GameResource_AsType54_00714060
void* GameResource_AsType54_00714060(void* self) {
    if (self != nullptr) {
        const std::uint32_t tok = fn_at<std::uint32_t(__cdecl*)()>(0x006f9a80)();
        using AcceptsFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
        if (vslot_at<AcceptsFn>(self, 0x0c)(self, tok) != 0u) {
            return self;
        }
    }
    return nullptr;
}

// [0x00714090] BSP_ResourceItemZoneDesc_Cast
void* ResourceItemZoneDesc_Cast_00714090(void* self) {
    if (self != nullptr) {
        const std::uint32_t tok = *reinterpret_cast<const std::uint32_t*>(0x00e19b74);
        using AcceptsFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
        if (vslot_at<AcceptsFn>(self, 0x0c)(self, tok) != 0u) {
            return self;
        }
    }
    return nullptr;
}

// [0x007140c0] BSP_ResourceItemAux_Cast
void* ResourceItemAux_Cast_007140c0(void* self) {
    if (self != nullptr) {
        const std::uint32_t tok = *reinterpret_cast<const std::uint32_t*>(0x00e19b64);
        using AcceptsFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
        if (vslot_at<AcceptsFn>(self, 0x0c)(self, tok) != 0u) {
            return self;
        }
    }
    return nullptr;
}

// [0x007140f0] BSP_ResourceItemNote_Cast
void* ResourceItemNote_Cast_007140f0(void* self) {
    if (self != nullptr) {
        const std::uint32_t tok = *reinterpret_cast<const std::uint32_t*>(0x00e19b54);
        using AcceptsFn = std::uint8_t(__thiscall*)(void*, std::uint32_t);
        if (vslot_at<AcceptsFn>(self, 0x0c)(self, tok) != 0u) {
            return self;
        }
    }
    return nullptr;
}

// [0x007149b0] BSP_Matrix4f_InvertInto_007149b0
void* Matrix4f_InvertInto_007149b0(void* self) {
    float tmp[16]{};
    void* inv = fn_at<void*(__fastcall*)(float*, const void*)>(0x00b632d0)(tmp, self);
    fn_at<void(__thiscall*)(void*, const void*)>(0x004134f0)(self, inv);
    return self;
}

// [0x00715af0] STL_UninitCopy8_00715af0
void* STL_UninitCopy8_00715af0(
    const void* first,
    const void* last,
    void* dst,
    void* /*alloc_ctx*/,
    std::uint32_t /*unused0*/,
    std::uint32_t /*unused1*/
) {
    const std::uint32_t* s = static_cast<const std::uint32_t*>(first);
    const std::uint32_t* e = static_cast<const std::uint32_t*>(last);
    std::uint32_t* d = static_cast<std::uint32_t*>(dst);
    while (s != e) {
        if (d != nullptr) {
            d[0] = s[0];
            d[1] = s[1];
        }
        s += 2;
        d += 2;
    }
    return d;
}

// [0x00715b20] STL_UninitCopy8_00715b20
void* STL_UninitCopy8_00715b20(
    const void* first,
    const void* last,
    void* dst,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
) {
    return STL_UninitCopy8_00715af0(first, last, dst, alloc_ctx, unused0, unused1);
}

// [0x00715b50] STL_UninitCopy8_00715b50
void* STL_UninitCopy8_00715b50(
    const void* first,
    const void* last,
    void* dst,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
) {
    return STL_UninitCopy8_00715af0(first, last, dst, alloc_ctx, unused0, unused1);
}

// [0x00715b80] STL_UninitCopy8_00715b80
void* STL_UninitCopy8_00715b80(
    const void* first,
    const void* last,
    void* dst,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
) {
    return STL_UninitCopy8_00715af0(first, last, dst, alloc_ctx, unused0, unused1);
}

// [0x00715bb0] STL_UninitCopy8_00715bb0
void* STL_UninitCopy8_00715bb0(
    const void* first,
    const void* last,
    void* dst,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
) {
    return STL_UninitCopy8_00715af0(first, last, dst, alloc_ctx, unused0, unused1);
}

// [0x00716960] STL_Fill8_00716960
void STL_Fill8_00716960(void* first, void* last, const void* value) {
    std::uint32_t* cur = static_cast<std::uint32_t*>(first);
    std::uint32_t* end = static_cast<std::uint32_t*>(last);
    const std::uint32_t* v = static_cast<const std::uint32_t*>(value);
    while (cur != end) {
        cur[0] = v[0];
        cur[1] = v[1];
        cur += 2;
    }
}

// [0x00716990] STL_Fill8_00716990
void STL_Fill8_00716990(void* first, void* last, const void* value) {
    STL_Fill8_00716960(first, last, value);
}

// [0x007169c0] STL_Fill8_007169c0
void STL_Fill8_007169c0(void* first, void* last, const void* value) {
    STL_Fill8_00716960(first, last, value);
}

// [0x007169f0] STL_Fill8_007169f0
void STL_Fill8_007169f0(void* first, void* last, const void* value) {
    STL_Fill8_00716960(first, last, value);
}

// [0x00716a20] STL_Fill8_00716a20
void STL_Fill8_00716a20(void* first, void* last, const void* value) {
    STL_Fill8_00716960(first, last, value);
}

// [0x00716b50] STL_UninitFillN8_00716b50
void STL_UninitFillN8_00716b50(
    void* dst,
    std::uint32_t count,
    const void* value,
    void* /*alloc_ctx*/,
    std::uint32_t /*unused0*/,
    std::uint32_t /*unused1*/
) {
    std::uint32_t* d = static_cast<std::uint32_t*>(dst);
    const std::uint32_t* v = static_cast<const std::uint32_t*>(value);
    while (count > 0u) {
        if (d != nullptr) {
            d[0] = v[0];
            d[1] = v[1];
        }
        --count;
        d += 2;
    }
}

// [0x00716b80] STL_UninitFillN8_00716b80
void STL_UninitFillN8_00716b80(
    void* dst,
    std::uint32_t count,
    const void* value,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
) {
    STL_UninitFillN8_00716b50(dst, count, value, alloc_ctx, unused0, unused1);
}

// [0x00716bb0] STL_UninitFillN8_00716bb0
void STL_UninitFillN8_00716bb0(
    void* dst,
    std::uint32_t count,
    const void* value,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
) {
    STL_UninitFillN8_00716b50(dst, count, value, alloc_ctx, unused0, unused1);
}

// [0x00716be0] STL_UninitFillN8_00716be0
void STL_UninitFillN8_00716be0(
    void* dst,
    std::uint32_t count,
    const void* value,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
) {
    STL_UninitFillN8_00716b50(dst, count, value, alloc_ctx, unused0, unused1);
}

// [0x00716c10] STL_UninitFillN8_00716c10
void STL_UninitFillN8_00716c10(
    void* dst,
    std::uint32_t count,
    const void* value,
    void* alloc_ctx,
    std::uint32_t unused0,
    std::uint32_t unused1
) {
    STL_UninitFillN8_00716b50(dst, count, value, alloc_ctx, unused0, unused1);
}

// [0x00717510] STL_Vector_Tidy_00717510
void STL_Vector_Tidy_00717510(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00717570] STL_Vector_Tidy_00717570
void STL_Vector_Tidy_00717570(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x007175a0] STL_Vector_Tidy_007175a0
void STL_Vector_Tidy_007175a0(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00717880] STL_CopyBackward8_00717880
void* STL_CopyBackward8_00717880(const void* first, const void* last, void* dst_last) {
    const std::uint8_t* f = static_cast<const std::uint8_t*>(first);
    const std::uint8_t* cur = static_cast<const std::uint8_t*>(last);
    std::uint8_t* d = static_cast<std::uint8_t*>(dst_last);
    std::uint8_t* ret = d - (((cur - f) >> 3) * 8);
    if (f != cur) {
        const std::ptrdiff_t delta = d - cur;
        do {
            cur -= 8;
            *reinterpret_cast<std::uint32_t*>( const_cast<std::uint8_t*>(cur) + delta ) =
                *reinterpret_cast<const std::uint32_t*>(cur);
            *reinterpret_cast<std::uint32_t*>( const_cast<std::uint8_t*>(cur) + delta + 4 ) =
                *reinterpret_cast<const std::uint32_t*>(cur + 4);
        } while (cur != f);
    }
    return ret;
}

// [0x007178c0] STL_CopyBackward8_007178c0
void* STL_CopyBackward8_007178c0(const void* first, const void* last, void* dst_last) {
    return STL_CopyBackward8_00717880(first, last, dst_last);
}

// [0x00717900] STL_CopyBackward8_00717900
void* STL_CopyBackward8_00717900(const void* first, const void* last, void* dst_last) {
    return STL_CopyBackward8_00717880(first, last, dst_last);
}

// [0x00717940] STL_CopyBackward8_00717940
void* STL_CopyBackward8_00717940(const void* first, const void* last, void* dst_last) {
    return STL_CopyBackward8_00717880(first, last, dst_last);
}

// [0x00717980] STL_CopyBackward8_00717980
void* STL_CopyBackward8_00717980(const void* first, const void* last, void* dst_last) {
    return STL_CopyBackward8_00717880(first, last, dst_last);
}

// [0x00717c70] BSP_ResourceItemNote_Construct
void* ResourceItemNote_Construct_00717c70(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x00b868b0)(self);
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfd860u;
    field_at<std::uint32_t>(self, 0x1c) = 0u;
    field_at<std::uint32_t>(self, 0x20) = 15u;
    field_at<std::uint8_t>(self, 0x0c) = 0u;
    return self;
}

// [0x00717ca0] BSP_ResourceItemNote_AcceptsRuntimeTypeToken
bool ResourceItemNote_AcceptsRuntimeTypeToken_00717ca0(const void* /*self*/, std::uint32_t token) {
    const std::uint32_t* p = reinterpret_cast<const std::uint32_t*>(0x00e19b54);
    const std::uint32_t* end = reinterpret_cast<const std::uint32_t*>(0x00e19b60);
    do {
        if (*p == token) {
            return true;
        }
        ++p;
    } while (p != end);
    return false;
}

// [0x00717cd0] BSP_ResourceItemNote_Destruct
void ResourceItemNote_Destruct_00717cd0(void* self) {
    sbo_tidy(static_cast<std::uint8_t*>(self) + 0x08);
    fn_at<void(__thiscall*)(void*)>(0x00b86890)(self);
}

// [0x00717d50] STL_String_Tidy_00717d50
void STL_String_Tidy_00717d50(void* self) {
    sbo_tidy(static_cast<std::uint8_t*>(self) + 0x20);
    sbo_tidy(self);
}

// [0x00717da0] BSP_ResourceItemZoneDesc_Construct
void* ResourceItemZoneDesc_Construct_00717da0(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x00b868b0)(self);
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfd888u;
    field_at<std::uint32_t>(self, 0x2c) = 0u;
    field_at<std::uint32_t>(self, 0x30) = 15u;
    field_at<std::uint8_t>(self, 0x1c) = 0u;
    return self;
}

// [0x00717dd0] BSP_ResourceItemZoneDesc_AcceptsRuntimeTypeToken
bool ResourceItemZoneDesc_AcceptsRuntimeTypeToken_00717dd0(const void* /*self*/, std::uint32_t token) {
    const std::uint32_t* p = reinterpret_cast<const std::uint32_t*>(0x00e19b74);
    const std::uint32_t* end = reinterpret_cast<const std::uint32_t*>(0x00e19b80);
    do {
        if (*p == token) {
            return true;
        }
        ++p;
    } while (p != end);
    return false;
}

// [0x00717e00] BSP_ResourceItemZoneDesc_Destruct
void ResourceItemZoneDesc_Destruct_00717e00(void* self) {
    sbo_tidy(static_cast<std::uint8_t*>(self) + 0x18);
    fn_at<void(__thiscall*)(void*)>(0x00b86890)(self);
}

// [0x00717ef0] STL_String_Tidy_00717ef0
void STL_String_Tidy_00717ef0(void* self) {
    sbo_tidy(self);
}

// [0x007180e0] STL_Vector_Tidy_007180e0
void STL_Vector_Tidy_007180e0(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00718110] STL_Vector_Tidy_00718110
void STL_Vector_Tidy_00718110(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00718140] STL_Vector_Tidy_00718140
void STL_Vector_Tidy_00718140(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x007185f0] STL_Vector_Tidy_007185f0
void STL_Vector_Tidy_007185f0(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00718620] STL_Vector_Tidy_00718620
void STL_Vector_Tidy_00718620(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00718650] STL_Vector_Tidy_00718650
void STL_Vector_Tidy_00718650(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00718680] STL_Vector_Tidy_00718680
void STL_Vector_Tidy_00718680(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x007186b0] STL_Vector_Tidy_007186b0
void STL_Vector_Tidy_007186b0(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00718710] CG_scalar_deleting_dtor_00718710
void* ResourceItemNote_ScalarDelete_00718710(void* self, std::uint8_t flags) {
    sbo_tidy(static_cast<std::uint8_t*>(self) + 0x08);
    fn_at<void(__thiscall*)(void*)>(0x00b86890)(self);
    if ((flags & 1u) != 0u) {
        fn_at<void(__cdecl*)(void*)>(0x00bf65ac)(self);
    }
    return self;
}

// [0x00718a40] STL_Vector_Tidy_00718a40
void STL_Vector_Tidy_00718a40(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00718a70] STL_Vector_Tidy_00718a70
void STL_Vector_Tidy_00718a70(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00718aa0] STL_Vector_Tidy_00718aa0
void STL_Vector_Tidy_00718aa0(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00718ad0] STL_Vector_Tidy_00718ad0
void STL_Vector_Tidy_00718ad0(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00718b00] STL_Vector_Tidy_00718b00
void STL_Vector_Tidy_00718b00(void* self) {
    STL_Vector_Tidy_007114a0(self);
}

// [0x00718c70] BSP_ModelNoteNameEquals_Predicate
// Original ABI: __thiscall (ECX = predicate sbo string, stack [ESP+4] = element_pair), RET 4.
bool ModelNoteNameEquals_Predicate_00718c70(const void* self, const void* element_pair) {
    LegacySboString28 tmp{};
    void* item = field_at<void*>(element_pair, 0x00);
    void* copied = fn_at<void*(__thiscall*)(void*, void*)>(0x00711c30)(item, &tmp);
    const char* key_ptr = sbo_c_str(self);
    const std::uint32_t key_len = field_at<std::uint32_t>(self, 0x14);
    const std::uint32_t item_len = field_at<std::uint32_t>(copied, 0x14);
    const std::int32_t cmp = fn_at<std::int32_t(__thiscall*)(const void*, std::uint32_t, std::uint32_t, const char*, std::uint32_t)>(
        0x004beb60
    )(copied, 0u, item_len, key_ptr, key_len);
    sbo_tidy(&tmp);
    return cmp == 0;
}

// [0x00718f50] BSP_StructuredNode_ReadCharacterString
// Original ABI: __fastcall (ECX = node_handle, EDX = dst_string), RET.
void* StructuredNode_ReadCharacterString_00718f50(void* node_handle, void* dst_string) {
    struct NativePooledString {
        std::uint32_t len{0};
        const char* data{nullptr};
    } temp_str{};
    NativePooledString* res = fn_at<NativePooledString*(__thiscall*)(void*, NativePooledString*)>(0x00bea010)(
        node_handle,
        &temp_str
    );
    const char* s = (res->data != nullptr) ? res->data : reinterpret_cast<const char*>(0x00e19b94);
    const std::size_t n = std::strlen(s);
    fn_at<void(__thiscall*)(void*, const char*, std::size_t)>(0x00408720)(dst_string, s, n);
    if (temp_str.data != nullptr) {
        const std::uint32_t block_size = temp_str.len + 1u;
        void* pool = fn_at<void*(__cdecl*)()>(0x00419cc0)();
        fn_at<void(__thiscall*)(void*, const char*, std::uint32_t, std::uint32_t)>(0x00bd1510)(
            pool,
            temp_str.data,
            block_size,
            1u
        );
    }
    return node_handle;
}

// [0x00719000] BSP_NoteResourceParser_Parse
// Original ABI: __thiscall (ECX = self unused, stack [ESP+4] = node_handle), RET 4.
void* NoteResourceParser_Parse_00719000(void* /*self*/, void* node_handle) {
    void* raw = fn_at<void*(__cdecl*)(std::size_t)>(0x00bf681b)(0x28u);
    void* item = nullptr;
    if (raw != nullptr) {
        fn_at<void(__thiscall*)(void*)>(0x00b868b0)(raw);
        field_at<std::uintptr_t>(raw, 0x00) = 0x00cfd860u;
        field_at<std::uint32_t>(raw, 0x20) = 15u;
        field_at<std::uint32_t>(raw, 0x1c) = 0u;
        field_at<std::uint8_t>(raw, 0x0c) = 0u;
        item = raw;
    }
    StructuredNode_ReadCharacterString_00718f50(
        node_handle,
        static_cast<std::uint8_t*>(item) + 0x08
    );
    return item;
}

// [0x00719080] BSP_ZoneDescResourceParser_ReadItem
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = node_handle), RET 4.
void ZoneDescResourceParser_ReadItem_00719080(void* self, void* node_handle) {
    while (fn_at<std::uint8_t(__thiscall*)(void*)>(0x00715bf0)(node_handle) != 0u) {
        void* child = nullptr;
        fn_at<void(__thiscall*)(void*, void**)>(0x00bea680)(node_handle, &child);
        const char* tag = (child != nullptr) ? field_at<const char*>(child, 0x14) : nullptr;
        if (tag != nullptr && fn_at<int(__cdecl*)(const char*, const char*)>(0x00bf7fbf)(tag, "Name") == 0) {
            void* name_str = static_cast<std::uint8_t*>(self) + 0x18;
            StructuredNode_ReadCharacterString_00718f50(&child, name_str);
            const char* cstr = sbo_c_str(name_str);
            const char* const* table = reinterpret_cast<const char* const*>(0x00e08138);
            std::int32_t idx = -1;
            for (std::int32_t i = 0; table[i] != nullptr; ++i) {
                if (fn_at<int(__fastcall*)(const char*, const char*)>(0x00438e10)(cstr, table[i]) == 0) {
                    idx = i;
                    break;
                }
            }
            field_at<std::int32_t>(self, 0x34) = idx;
        } else if (tag != nullptr && fn_at<int(__cdecl*)(const char*, const char*)>(0x00bf7fbf)(tag, "Index") == 0) {
            fn_at<void*(__thiscall*)(void*, void*)>(0x00be9a80)(&child, static_cast<std::uint8_t*>(self) + 0x38);
        } else if (tag != nullptr && fn_at<int(__cdecl*)(const char*, const char*)>(0x00bf7fbf)(tag, "Armor") == 0) {
            fn_at<void*(__thiscall*)(void*, void*)>(0x00be9a80)(&child, static_cast<std::uint8_t*>(self) + 0x3c);
        } else if (tag != nullptr && fn_at<int(__cdecl*)(const char*, const char*)>(0x00bf7fbf)(tag, "Sphere") == 0) {
            void* h0 = fn_at<void*(__thiscall*)(void*, void*)>(0x00be9a60)(&child, static_cast<std::uint8_t*>(self) + 0x08);
            void* h1 = fn_at<void*(__thiscall*)(void*, void*)>(0x00be9a60)(h0, static_cast<std::uint8_t*>(self) + 0x0c);
            fn_at<void*(__thiscall*)(void*, void*)>(0x00be9a60)(h1, static_cast<std::uint8_t*>(self) + 0x10);
            fn_at<void*(__thiscall*)(void*, void*)>(0x00be9a60)(&child, static_cast<std::uint8_t*>(self) + 0x14);
        } else {
            fn_at<void(__thiscall*)(void*)>(0x00be9c40)(&child);
        }
        fn_at<void(__thiscall*)(void*)>(0x00be9ed0)(&child);
    }
}

// [0x00719240] BSP_ZoneDescResourceParser_CreateItem
// Original ABI: __thiscall (ECX = self unused, stack [ESP+4] = node_handle), RET 4.
void* ZoneDescResourceParser_CreateItem_00719240(void* /*self*/, void* node_handle) {
    void* raw = fn_at<void*(__cdecl*)(std::size_t)>(0x00bf681b)(0x40u);
    void* item = nullptr;
    if (raw != nullptr) {
        fn_at<void(__thiscall*)(void*)>(0x00b868b0)(raw);
        field_at<std::uintptr_t>(raw, 0x00) = 0x00cfd888u;
        field_at<std::uint32_t>(raw, 0x30) = 15u;
        field_at<std::uint32_t>(raw, 0x2c) = 0u;
        field_at<std::uint8_t>(raw, 0x1c) = 0u;
        item = raw;
    }
    ZoneDescResourceParser_ReadItem_00719080(item, node_handle);
    return item;
}

namespace {

inline void vector_insert_n8_impl(
    void* self,
    void* pos_ptr,
    std::uint32_t count,
    const void* value,
    std::uintptr_t xlen_addr,
    std::uintptr_t alloc_addr,
    std::uintptr_t uninit_copy_addr,
    std::uintptr_t fill_n_addr,
    std::uintptr_t copy_tail_addr,
    std::uintptr_t copy_back_addr,
    std::uintptr_t fill_range_addr
) {
    const std::uint32_t* val_words = static_cast<const std::uint32_t*>(value);
    const std::uint32_t local_val[2]{val_words[0], val_words[1]};

    std::uint8_t* first = field_at<std::uint8_t*>(self, 0x04);
    std::uint32_t cap = (first != nullptr)
        ? static_cast<std::uint32_t>((field_at<std::uint8_t*>(self, 0x0c) - first) >> 3)
        : 0u;
    if (count == 0u) {
        return;
    }
    const std::uint32_t len = (first != nullptr)
        ? static_cast<std::uint32_t>((field_at<std::uint8_t*>(self, 0x08) - first) >> 3)
        : 0u;
    if (0x1fffffffu - len < count) {
        fn_at<void(__cdecl*)()>(xlen_addr)();
    }
    if (cap < len + count) {
        cap = (0x1fffffffu - (cap >> 1u) < cap) ? 0u : (cap + (cap >> 1u));
        if (cap < len + count) {
            cap = len + count;
        }
        std::uint8_t* new_buf = static_cast<std::uint8_t*>(
            fn_at<void*(__fastcall*)(std::uint32_t, std::uint32_t)>(alloc_addr)(cap, 0u)
        );
        void* mid = fn_at<void*(__fastcall*)(const void*, const void*, void*, void*, std::uint32_t, std::uint32_t)>(
            uninit_copy_addr
        )(first, pos_ptr, new_buf, self, count, 0u);
        mid = fn_at<void*(__thiscall*)(void*, void*, std::uint32_t, const void*)>(fill_n_addr)(
            self,
            mid,
            count,
            local_val
        );
        fn_at<void*(__fastcall*)(const void*, const void*, void*, void*, std::uint32_t, std::uint32_t)>(
            uninit_copy_addr
        )(pos_ptr, field_at<void*>(self, 0x08), mid, self, count, 0u);
        if (first != nullptr) {
            fn_at<void(__cdecl*)(void*)>(0x00bf65ac)(first);
        }
        field_at<std::uint8_t*>(self, 0x0c) = new_buf + cap * 8u;
        field_at<std::uint8_t*>(self, 0x08) = new_buf + (len + count) * 8u;
        field_at<std::uint8_t*>(self, 0x04) = new_buf;
        return;
    }

    std::uint8_t* last = field_at<std::uint8_t*>(self, 0x08);
    std::uint8_t* pos = static_cast<std::uint8_t*>(pos_ptr);
    const std::uint32_t tail_elems = static_cast<std::uint32_t>((last - pos) >> 3);
    if (tail_elems < count) {
        const std::uint32_t byte_shift = count * 8u;
        fn_at<void*(__thiscall*)(void*, const void*, const void*, void*)>(copy_tail_addr)(
            self,
            pos,
            last,
            pos + byte_shift
        );
        fn_at<void*(__thiscall*)(void*, void*, std::uint32_t, const void*)>(fill_n_addr)(
            self,
            field_at<void*>(self, 0x08),
            count - tail_elems,
            local_val
        );
        field_at<std::uint8_t*>(self, 0x08) += byte_shift;
        fn_at<void(__cdecl*)(void*, void*, const void*)>(fill_range_addr)(
            pos,
            field_at<std::uint8_t*>(self, 0x08) - byte_shift,
            local_val
        );
    } else {
        const std::uint32_t byte_shift = count * 8u;
        std::uint8_t* split = last - byte_shift;
        field_at<void*>(self, 0x08) = fn_at<void*(__thiscall*)(void*, const void*, const void*, void*)>(
            copy_tail_addr
        )(self, split, last, last);
        fn_at<void*(__cdecl*)(const void*, const void*, void*)>(copy_back_addr)(pos, split, last);
        fn_at<void(__cdecl*)(void*, void*, const void*)>(fill_range_addr)(
            pos,
            pos + byte_shift,
            local_val
        );
    }
}

}  // namespace

// [0x007192c0] STL_Vector_InsertN8_007192c0 (includes Catch_All@00719427)
void STL_Vector_InsertN8_007192c0(
    void* self,
    std::uint32_t /*pos_owner*/,
    void* pos_ptr,
    std::uint32_t count,
    const void* value
) {
    vector_insert_n8_impl(
        self, pos_ptr, count, value,
        0x00718d20u, 0x00714cb0u, 0x00715af0u, 0x00718180u,
        0x00718950u, 0x00717880u, 0x00716960u
    );
}

// [0x00719510] STL_Vector_InsertN8_00719510 (includes Catch_All@00719677)
void STL_Vector_InsertN8_00719510(
    void* self,
    std::uint32_t /*pos_owner*/,
    void* pos_ptr,
    std::uint32_t count,
    const void* value
) {
    vector_insert_n8_impl(
        self, pos_ptr, count, value,
        0x00718d90u, 0x00714d10u, 0x00715b20u, 0x007181c0u,
        0x00718980u, 0x007178c0u, 0x00716990u
    );
}

// [0x00719760] STL_Vector_InsertN8_00719760 (includes Catch_All@007198c7)
void STL_Vector_InsertN8_00719760(
    void* self,
    std::uint32_t /*pos_owner*/,
    void* pos_ptr,
    std::uint32_t count,
    const void* value
) {
    vector_insert_n8_impl(
        self, pos_ptr, count, value,
        0x00718e00u, 0x00714d70u, 0x00715b50u, 0x00718200u,
        0x007189b0u, 0x00717900u, 0x007169c0u
    );
}

// [0x007199b0] STL_Vector_InsertN8_007199b0 (includes Catch_All@00719b17)
void STL_Vector_InsertN8_007199b0(
    void* self,
    std::uint32_t /*pos_owner*/,
    void* pos_ptr,
    std::uint32_t count,
    const void* value
) {
    vector_insert_n8_impl(
        self, pos_ptr, count, value,
        0x00718e70u, 0x00714dd0u, 0x00715b80u, 0x00718240u,
        0x007189e0u, 0x00717940u, 0x007169f0u
    );
}

// [0x00719c00] STL_Vector_InsertN8_00719c00 (includes Catch_All@00719d67)
void STL_Vector_InsertN8_00719c00(
    void* self,
    std::uint32_t /*pos_owner*/,
    void* pos_ptr,
    std::uint32_t count,
    const void* value
) {
    vector_insert_n8_impl(
        self, pos_ptr, count, value,
        0x00718ee0u, 0x00714e30u, 0x00715bb0u, 0x00718280u,
        0x00718a10u, 0x00717980u, 0x00716a20u
    );
}

// [0x00719fa0] BSP_ModelNoteList_FindIfNameEquals
// Original ABI: __fastcall (ECX = first, EDX = last, stack [ESP+4..1Ch] = 28-byte LegacySboString28 by value), RET 1Ch.
void* ModelNoteList_FindIfNameEquals_00719fa0(
    void* first,
    void* last,
    void* predicate_sbo_string_by_value
) {
    std::uint8_t* cur = static_cast<std::uint8_t*>(first);
    std::uint8_t* end = static_cast<std::uint8_t*>(last);
    while (cur != end && !ModelNoteNameEquals_Predicate_00718c70(predicate_sbo_string_by_value, cur)) {
        cur += 8;
    }
    sbo_tidy(predicate_sbo_string_by_value);
    return cur;
}

// [0x0071aae0] BSP_ModelNoteList_FindByName
// Original ABI: __thiscall (ECX = out_iter, stack [ESP+4..10h] = first_owner, first_ptr, last_owner, last_ptr,
// [ESP+14h..2Ch] = 28-byte LegacySboString28 by value), RET 2Ch.
void* ModelNoteList_FindByName_0071aae0(
    void* out_iter,
    void* first_owner,
    void* first_ptr,
    void* /*last_owner*/,
    void* last_ptr,
    const void* key_string
) {
    LegacySboString28 pred_copy{};
    fn_at<void(__thiscall*)(void*, const void*, std::uint32_t, std::uint32_t)>(0x00408120)(
        &pred_copy,
        key_string,
        0u,
        0xffffffffu
    );
    void* found = ModelNoteList_FindIfNameEquals_00719fa0(first_ptr, last_ptr, &pred_copy);
    field_at<void*>(out_iter, 0x00) = first_owner;
    field_at<void*>(out_iter, 0x04) = found;
    sbo_tidy(const_cast<void*>(key_string));
    return out_iter;
}

// [0x0071abb0] BSP_ResourceItemAux_Construct
void* ResourceItemAux_Construct_0071abb0(void* self) {
    fn_at<void(__thiscall*)(void*)>(0x00b868b0)(self);
    field_at<std::uintptr_t>(self, 0x00) = 0x00cfd8fcu;
    field_at<std::uint32_t>(self, 0x20) = 15u;
    field_at<std::uint32_t>(self, 0x1c) = 0u;
    field_at<std::uint8_t>(self, 0x0c) = 0u;
    field_at<std::uint32_t>(self, 0x24) = 0xffffffffu;
    field_at<std::uint32_t>(self, 0x40) = 15u;
    field_at<std::uint32_t>(self, 0x3c) = 0u;
    field_at<std::uint8_t>(self, 0x2c) = 0u;
    field_at<void*>(self, 0x48) = nullptr;
    field_at<void*>(self, 0x4c) = nullptr;
    field_at<void*>(self, 0x50) = nullptr;
    return self;
}

// [0x0071ac10] BSP_ResourceItemAux_AcceptsRuntimeTypeToken
bool ResourceItemAux_AcceptsRuntimeTypeToken_0071ac10(const void* /*self*/, std::uint32_t token) {
    const std::uint32_t* p = reinterpret_cast<const std::uint32_t*>(0x00e19b64);
    const std::uint32_t* end = reinterpret_cast<const std::uint32_t*>(0x00e19b70);
    do {
        if (*p == token) {
            return true;
        }
        ++p;
    } while (p != end);
    return false;
}

// [0x0071ac40] BSP_ResourceItemAux_Destruct
void ResourceItemAux_Destruct_0071ac40(void* self) {
    void* pts = field_at<void*>(self, 0x48);
    if (pts != nullptr) {
        fn_at<void(__cdecl*)(void*)>(0x00bf65ac)(pts);
    }
    field_at<void*>(self, 0x48) = nullptr;
    field_at<void*>(self, 0x4c) = nullptr;
    field_at<void*>(self, 0x50) = nullptr;
    sbo_tidy(static_cast<std::uint8_t*>(self) + 0x28);
    sbo_tidy(static_cast<std::uint8_t*>(self) + 0x08);
    fn_at<void(__thiscall*)(void*)>(0x00b86890)(self);
}

// [0x0071ad50] BSP_Model_FindNoteNode
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = key), RET 4.
void* Model_FindNoteNode_0071ad50(void* self, const char* key) {
    LegacySboString28 key_sbo{};
    fn_at<void(__thiscall*)(void*, const char*, std::size_t)>(0x00408720)(
        &key_sbo,
        key,
        std::strlen(key)
    );
    void* vec = static_cast<std::uint8_t*>(self) + 0x7c;
    std::uint8_t* first = field_at<std::uint8_t*>(vec, 0x04);
    std::uint8_t* last = field_at<std::uint8_t*>(vec, 0x08);
    if (first > last) {
        fn_at<void(__cdecl*)()>(0x00bf6713)();
    }
    void* found_iter[2]{};
    ModelNoteList_FindByName_0071aae0(found_iter, vec, first, vec, last, &key_sbo);
    last = field_at<std::uint8_t*>(vec, 0x08);
    if (field_at<std::uint8_t*>(vec, 0x04) > last) {
        fn_at<void(__cdecl*)()>(0x00bf6713)();
    }
    if (found_iter[0] == nullptr || found_iter[0] != vec) {
        fn_at<void(__cdecl*)()>(0x00bf6713)();
    }
    std::uint8_t* match = static_cast<std::uint8_t*>(found_iter[1]);
    if (match == last) {
        return nullptr;
    }
    if (match >= field_at<std::uint8_t*>(found_iter[0], 0x08)) {
        fn_at<void(__cdecl*)()>(0x00bf6713)();
    }
    return field_at<void*>(match, 0x04);
}

// [0x0071b3e0] BSP_GameResource_ReadPointsItem
// Original ABI: __thiscall (ECX = self, stack [ESP+4] = node_handle), RET 4.
void GameResource_ReadPointsItem_0071b3e0(void* self, void* node_handle) {
    while (fn_at<std::uint8_t(__thiscall*)(void*)>(0x00715bf0)(node_handle) != 0u) {
        void* child = nullptr;
        fn_at<void(__thiscall*)(void*, void**)>(0x00bea680)(node_handle, &child);
        const char* tag = (child != nullptr) ? field_at<const char*>(child, 0x14) : nullptr;
        if (tag != nullptr && fn_at<int(__cdecl*)(const char*, const char*)>(0x00bf7fbf)(tag, "Identifier") == 0) {
            void* next_h = StructuredNode_ReadCharacterString_00718f50(
                &child,
                static_cast<std::uint8_t*>(self) + 0x08
            );
            fn_at<void*(__thiscall*)(void*, void*)>(0x00be9a80)(
                next_h,
                static_cast<std::uint8_t*>(self) + 0x24
            );
        } else if (tag != nullptr && fn_at<int(__cdecl*)(const char*, const char*)>(0x00bf7fbf)(tag, "Category") == 0) {
            StructuredNode_ReadCharacterString_00718f50(
                &child,
                static_cast<std::uint8_t*>(self) + 0x28
            );
        } else if (tag != nullptr && fn_at<int(__cdecl*)(const char*, const char*)>(0x00bf7fbf)(tag, "Points") == 0) {
            while (child != nullptr && field_at<std::int32_t>(child, 0x20) != 0) {
                float pt[3]{};
                void* h0 = fn_at<void*(__thiscall*)(void*, float*)>(0x00be9a60)(&child, &pt[0]);
                void* h1 = fn_at<void*(__thiscall*)(void*, float*)>(0x00be9a60)(h0, &pt[1]);
                fn_at<void*(__thiscall*)(void*, float*)>(0x00be9a60)(h1, &pt[2]);
                fn_at<void(__thiscall*)(void*, const float*)>(0x004215d0)(
                    static_cast<std::uint8_t*>(self) + 0x44,
                    pt
                );
            }
        } else {
            fn_at<void(__thiscall*)(void*)>(0x00be9c40)(&child);
        }
        fn_at<void(__thiscall*)(void*)>(0x00be9ed0)(&child);
    }
}

}  // namespace bsp::flak_note_seg36_37_argon
