#pragma once
// cc13_dyn1: the in-house mitengine "Dyn" block [00C30930,00C5DF60), the functions earlier
// waves left without a reconstruction record. docs/CC13_DYN1.md carries the per-function
// ABI, evidence and uncertainty. Nothing here is wired.
//
// Everything works on native storage by byte offset, like the native_dyn_* files: the
// pointers are the image's own objects (body, motion, shape, world) and every offset in a
// name or comment is a native one. Names are descriptive hypotheses, not recovered symbols.
// Image calls that leave this file go through Cc13Dyn1Calls, one named hook per native call
// site kind, invoked at the point the binary makes the call.

#include <cstddef>
#include <cstdint>

namespace bsp {
struct DynProfileScopeContext;
struct AvoidZoneDynHullMemory;
struct NativeDynNarrowPhaseCalls;
}

namespace bsp::cc13_dyn1 {

// oct18b-nC: polygon clipping helpers in the in-house code band. Original
// ECX=&output_count, ESI=output_xy; stack input_xy,count,bound, RET 0Ch.
// Input is a closed polygon represented by n>=1 XY pairs; output must have
// capacity for 2*n pairs and be disjoint from input. No capacity check exists.
// x87 comparisons include unordered values on the inside branch (JBE).
void clip_y_at_most_00c309d0(std::int32_t& output_count, float* output_xy,
    const float* input_xy, std::int32_t count, float bound);
void clip_y_at_least_00c30d10(std::int32_t& output_count, float* output_xy,
    const float* input_xy, std::int32_t count, float bound);
void clip_x_at_most_00c31060(std::int32_t& output_count, float* output_xy,
    const float* input_xy, std::int32_t count, float bound);
void clip_x_at_least_00c313a0(std::int32_t& output_count, float* output_xy,
    const float* input_xy, std::int32_t count, float bound);
// 00C34CF0: EBX=count, stack output-matrices,node-pointer-array, RET 8.
// Each node's +08h twelve floats become four rows of XYZ,0/0/0/1.
void expand_node_matrices_00c34cf0(float* output, const void* const* nodes, std::int32_t count);
// 00C30900: EAX=word pointer, MOV [EAX],0; RET; EAX retained by native body.
void zero_word_00c30900(std::uint32_t* word) noexcept;

struct Raw {
    std::uint8_t* p;
    template <class T>
    T& at(std::size_t off) const noexcept { return *reinterpret_cast<T*>(p + off); }
    void*& ptr(std::size_t off) const noexcept { return at<void*>(off); }
    std::uint8_t* field(std::size_t off) const noexcept { return p + off; }
    Raw deref(std::size_t off) const noexcept { return Raw{static_cast<std::uint8_t*>(ptr(off))}; }
    explicit operator bool() const noexcept { return p != nullptr; }
};
inline Raw raw(const void* p) noexcept {
    return Raw{static_cast<std::uint8_t*>(const_cast<void*>(p))};
}

// The image's 1.0f at 00D7A24C (bytes 00 00 80 3F).
inline constexpr float kOne_00d7a24c = 1.0f;
// The Dyn world lock at 0109ECD8 (a CRITICAL_SECTION in .data) every shape mask setter takes.
inline constexpr std::uint32_t kDynWorldLock_0109ecd8 = 0x0109ecd8u;

struct Cc13Dyn1Calls {
    virtual ~Cc13Dyn1Calls() = default;
    // EnterCriticalSection / LeaveCriticalSection through the IAT (00CE2218 / 00CE2210) on
    // the lock whose image address is given.
    virtual void enter_critical_section(std::uint32_t lock_address) = 0;
    virtual void leave_critical_section(std::uint32_t lock_address) = 0;
    // 00C43AA0, stack body, RET 4: clear_native_dyn_body_manifolds_00c43aa0 in
    // src/native_game_physics_lifetime.cpp is the reconstruction the host forwards to.
    virtual void clear_body_manifolds_00c43aa0(void* body) = 0;
    // The native virtual call [[dispatcher]+0] with ECX = dispatcher, stack (hit_out, shape,
    // start, end), callee-cleaned; AL is the hit flag. hit_out is 24 bytes: point, normal.
    virtual bool shape_ray_test_slot0(void* dispatcher, float* hit_out, void* shape,
                                      const float* start, const float* end) = 0;
    // The native virtual call [[object]+0] with ECX = object and no stack arguments.
    virtual void object_virtual_slot0(void* object) = 0;
    // The native tail JMP [[manager]+8] with ECX = manager and one stack argument (the
    // proxy), RET 4 in the callee: the broad-phase manager's proxy update (vtable slot 2).
    virtual void broadphase_update_proxy_slot2(void* manager, void* proxy) = 0;
    // 00BF55BE operator new (cdecl, one size) and 00BF6989 free (cdecl, one pointer).
    virtual void* allocate_00bf55be(std::uint32_t size) = 0;
    virtual void free_00bf6989(void* block) = 0;
    // 00BF65AC _free (cdecl, one pointer), a second free entry the image uses.
    virtual void free_00bf65ac(void* block) = 0;
    // DeleteCriticalSection through the IAT (00CE2214).
    virtual void delete_critical_section(void* section) = 0;
};

// ---- body accessors (B = Dyn body, M = [B+4h] its motion record) ----
// 00C31DC0 __fastcall(ECX = B), RET: EAX = [B+70h], the first shape of the body chain
// (shapes link through shape+208h).
void* body_first_shape_00c31dc0(void* body) noexcept;
// 00C31EA0 __fastcall(ECX = B), RET: EAX = [B+6Ch], the owner (user) pointer.
void* body_owner_00c31ea0(void* body) noexcept;
// 00C31F20 __thiscall(ECX = B, out), RET 4: out = M+0Ch..14h, the angular velocity.
void body_angular_velocity_00c31f20(void* body, float out[3]) noexcept;
// 00C31F40 __thiscall(ECX = B, out), RET 4: out = M+00h..08h, the linear velocity.
void body_linear_velocity_00c31f40(void* body, float out[3]) noexcept;
// 00C31F60 __thiscall(ECX = B, lo, hi), RET 8: B+38h..40h = lo, B+44h..4Ch = hi.
void body_set_box_00c31f60(void* body, const float lo[3], const float hi[3]) noexcept;
// 00C31F90 __thiscall(ECX = B, lo_out, hi_out), RET 8: the reverse copy of 00C31F60.
void body_box_00c31f90(const void* body, float lo_out[3], float hi_out[3]) noexcept;
// 00C35300 __fastcall(ECX = B), RET: EAX = B+2Ch, the position.
float* body_position_00c35300(void* body) noexcept;
// 00C37F10 __thiscall(ECX = B, out), RET 4: out[i] = 1.0 / M+54h[i], the inertia.
void body_inertia_00c37f10(const void* body, float out[3]) noexcept;
// 00C37F60 __thiscall(ECX = B, force, point), RET 8: M+38h += force,
// M+44h += (point - B+2Ch) x force, B+50h &= ~12h (wake).
void body_add_force_at_point_00c37f60(void* body, const float force[3],
                                      const float point[3]) noexcept;

// 00C56CF0 __thiscall(ECX = B, const float xf[12]), RET 4 (or the callee's RET 4 after
// the tail JMP): B+08h..34h = xf (REP MOVSD). When B+50h has bits 3 and 0 set, the world
// box of the local box B+38h..4Ch under xf goes to the proxy [B+60h]+04h..18h and the
// broad-phase manager [[[B]+444h]+ACh] gets its slot-2 proxy update; otherwise xf is also
// copied to [B+4h]+84h.
void body_set_transform_00c56cf0(void* body, const float xf[12], Cc13Dyn1Calls& calls);

// ---- primitive shapes (vtables: Dyn::Shape 00D7A04C, BoxShape 00D7A0C8, CylinderShape
// 00D7A098, SphereShape 00D7A058) ----
inline constexpr std::uint32_t kDynShapeVtable_00d7a04c = 0x00d7a04cu;
inline constexpr std::uint32_t kDynBoxShapeVtable_00d7a0c8 = 0x00d7a0c8u;
inline constexpr std::uint32_t kDynCylinderShapeVtable_00d7a098 = 0x00d7a098u;
inline constexpr std::uint32_t kDynSphereShapeVtable_00d7a058 = 0x00d7a058u;
// Slot 0 of each class, __thiscall(ECX = S), RET: the shape's box in body space at
// S+0Ch..20h from its extents grown by the 0.02f at 00D7A2F8 and its local transform
// S+34h..60h, then 00C55FC0 with ESI = [S+4h] (dyn_body_recompute_bounds_00c55fc0).
// Box extents S+210h..218h; cylinder half height S+210h and radius S+214h (y is the
// axis); sphere radius S+210h about the translation S+58h.
void box_shape_bounds_00c57880(void* shape);
void cylinder_shape_bounds_00c57ff0(void* shape);
void sphere_shape_bounds_00c58520(void* shape);
// 00C58690 Dyn::TerrainShape slot 0, __thiscall(ECX = S), RET: the heightfield's box in
// body space: x and z span 0..(count-1)*spacing (S+214h/S+21Ch, S+218h/S+220h), y the
// min/max over the S+214h*S+218h samples at [S+210h] (type S+22Ch: 0 float, 1 uint16
// scaled by S+224h plus S+228h with FFFFh = -1000.0f, otherwise 0.0); then 00C55FC0 with
// ESI = [S+4h]. No epsilon growth, unlike the primitive shapes.
void terrain_shape_bounds_00c58690(void* shape);
// 00C57B90 __thiscall(ECX = body, shape, desc), RET 8, EAX = shape. Inlined Dyn::Shape
// base constructor, BoxShape vtable, transform desc+14h..43h to S+34h, extents
// desc+44h..4Fh to S+210h, then a direct call of 00C57880. The SEH frame (handler
// 00CC8473, state 0 after the base) is not reproduced.
void* box_shape_construct_00c57b90(void* body, void* shape, const void* desc,
                                   std::uint32_t base_vtable = kDynShapeVtable_00d7a04c,
                                   std::uint32_t box_vtable = kDynBoxShapeVtable_00d7a0c8);
// Support mappings (GJK): slot 2 in float, slot 3 in double. __thiscall(ECX = S, out,
// dir, xf), RET 0Ch; xf is the shape's world 3x4 (rows then translation, floats).
// The box picks, per row k of xf, the sign of dir . row_k (zero or unordered gives -1)
// and returns translation + sum_k sign_k * extent_k * row_k. The sphere returns
// translation + radius * dir (dir is not normalised here).
void box_shape_support_00c47aa0(const void* shape, float out[3], const float dir[3],
                                const float xf[12]) noexcept;
void box_shape_support_00c38470(const void* shape, double out[3], const double dir[3],
                                const float xf[12]) noexcept;
void sphere_shape_support_00c48080(const void* shape, float out[3], const float dir[3],
                                   const float xf[12]) noexcept;
void sphere_shape_support_00c38970(const void* shape, double out[3], const double dir[3],
                                   const float xf[12]) noexcept;
// The cylinder (axis = row 1): the radial part is dir projected on rows 0 and 2,
// normalised when its squared length exceeds 1e-6f (float, 00D7A288) or 1e-7 (double,
// 00D7A298) and zero otherwise, scaled by S+214h; the axial part is sign(dir . row1) *
// S+210h along row 1. out = (axial + translation) + radial.
void cylinder_shape_support_00c47ca0(const void* shape, float out[3], const float dir[3],
                                     const float xf[12]) noexcept;
void cylinder_shape_support_00c38810(const void* shape, double out[3], const double dir[3],
                                     const float xf[12]) noexcept;
// 00C582E0, register ABI EDI = desc, stack (shape, body), RET 8, EAX = shape. Inlined base
// constructor, CylinderShape vtable; row 1 = the normalised axis desc+20h..28h, row 0 =
// row1 x (0,0,1) normalised, row 2 = row0 x row1 (two 004011D0 square roots), translation
// desc+14h..1Ch, radius desc+30h to S+214h, half height desc+2Ch to S+210h, then a direct
// call of 00C57FF0. SEH frame (handler 00CC8497) not reproduced.
void* cylinder_shape_construct_00c582e0(const void* desc, void* shape, void* body,
                                        std::uint32_t base_vtable = kDynShapeVtable_00d7a04c,
                                        std::uint32_t cylinder_vtable = kDynCylinderShapeVtable_00d7a098);
// 00C585B0 __thiscall(ECX = body, shape, desc), RET 8, EAX = shape. Identity rows,
// translation desc+18h..20h, radius desc+14h, then a direct call of 00C58520. SEH frame
// (handler 00CC84A9) not reproduced.
void* sphere_shape_construct_00c585b0(void* body, void* shape, const void* desc,
                                      std::uint32_t base_vtable = kDynShapeVtable_00d7a04c,
                                      std::uint32_t sphere_vtable = kDynSphereShapeVtable_00d7a058);

// ---- shape accessors (S = Dyn shape, 218h-byte convex record) ----
// 00C32430 __fastcall(ECX = S), RET: EAX = S+34h, the shape's local 3x4 transform.
float* shape_local_transform_00c32430(void* shape) noexcept;
// 00C32450 __fastcall(ECX = S), RET: EAX = [S+2Ch], the group bits.
std::uint32_t shape_group_00c32450(const void* shape) noexcept;
// 00C32490 __fastcall(ECX = S), RET: EAX = [S+4h], the owning body.
void* shape_body_00c32490(const void* shape) noexcept;
// 00C47F60 / 00C47F90 / 00C47FC0 / 00C47FF0 / 00C48020 / 00C48050, __thiscall(ECX = S,
// value), RET 4 through a tail JMP to LeaveCriticalSection: update S+30h
// (collide mask) or S+2Ch (group), then under the world lock clear the owning body's
// manifolds through 00C43AA0.
void shape_collide_mask_clear_00c47f60(void* shape, std::uint32_t bits, Cc13Dyn1Calls& calls);
void shape_collide_mask_set_00c47f90(void* shape, std::uint32_t bits, Cc13Dyn1Calls& calls);
void shape_group_clear_00c47fc0(void* shape, std::uint32_t bits, Cc13Dyn1Calls& calls);
void shape_group_set_00c47ff0(void* shape, std::uint32_t bits, Cc13Dyn1Calls& calls);
void shape_collide_mask_assign_00c48020(void* shape, std::uint32_t value, Cc13Dyn1Calls& calls);
void shape_group_assign_00c48050(void* shape, std::uint32_t value, Cc13Dyn1Calls& calls);

// ---- transforms ----
// 00C33650 __thiscall(ECX = src 3x4, dst 4x4), RET 4: rows of three become rows of four,
// +0Ch/+1Ch/+2Ch = 0, +3Ch = the 1.0f at 00D7A24C. FLD/FSTP DWORD copies, which quiet a signalling NaN; the reverse copies the same way.
void transform_expand_3x4_to_4x4_00c33650(const float src[12], float dst[16]) noexcept;
// 00C336C0 __thiscall(ECX = dst 3x4, src 4x4), RET 4: the reverse, dropping column 3.
void transform_from_4x4_00c336c0(float dst[12], const float src[16]) noexcept;

// ---- convex hull data ----
// 00C32CE0 __thiscall(ECX = &hull_data_ptr, out), RET 4: H = [ECX]; for i < [H+4]:
// out[3i..3i+2] = the first three words of the 16-byte vertex [H]+10h*i.
void hull_copy_vertices_00c32ce0(void* const* hull_data, float* out) noexcept;
// 00C35810, register ABI ESI = destination vector {data, size, capacity} of uint16,
// EDI = source record (indices at [EDI+0Ch], count [EDI+10h]), RET: assign.
void u16_vector_assign_00c35810(void* dest_vector, const void* source,
                                Cc13Dyn1Calls& calls);
// 00C37430 __thiscall(ECX = &hull_data_ptr, dest_vector), RET 4: 00C35810 with
// EDI = [ECX], ESI = dest_vector.
void hull_copy_indices_00c37430(void* const* hull_data, void* dest_vector,
                                Cc13Dyn1Calls& calls);

// 00C35630 __thiscall(ECX = convex mesh shape, dest_vector), RET 4: the body of 00C35810
// inlined with EDI = [S+210h] (the hull record): copy its uint16 index array.
void convex_shape_copy_indices_00c35630(const void* shape, void* dest_vector,
                                        Cc13Dyn1Calls& calls);
// 00C356D0 __thiscall(ECX = convex mesh shape, dest_vector), RET 4: H = [S+210h]; make the
// {data, size, capacity} vector of 12-byte points hold H's [H+4] vertices (16-byte stride
// at [H]). Grows only when both the size and the capacity are below the count.
void convex_shape_copy_vertices_00c356d0(const void* shape, void* dest_vector,
                                         Cc13Dyn1Calls& calls);

// ---- EH-cleanup and small helpers ----
// 00C30930, register ABI EAX = CRITICAL_SECTION*, RET: DeleteCriticalSection (IAT 00CE2214).
void delete_critical_section_00c30930(void* section, Cc13Dyn1Calls& calls);
// 00C31CC0 and its byte-identical twin 00C31D70, register ABI ESI = owner, RET: free
// [ESI+EA74h] through 00BF65AC and then [ESI+EA6Ch] through 00BF6989, each when non-null.
// Called only from the unwind funclets 00CC8910 / 00CC8930.
void free_owner_buffers_00c31cc0(void* owner, Cc13Dyn1Calls& calls);
void free_owner_buffers_00c31d70(void* owner, Cc13Dyn1Calls& calls);
// 00C32420 __fastcall(ECX = S), RET: [S] = Dyn::Shape vtable 00D7A04C (the base
// destructor's vtable restore, called from unwind funclet 00CC84B3).
void shape_restore_base_vtable_00c32420(void* shape,
                                        std::uint32_t base_vtable = kDynShapeVtable_00d7a04c) noexcept;
// 00C32D50 __fastcall(ECX = P), RET, EAX = P: two zero dwords.
void* zero_pair_00c32d50(void* pair) noexcept;

// 00C38040 __fastcall(ECX = profile scope), RET: the standalone profile-scope leave the
// unwind funclets call (16 sites, 00CC8422..00CC855C): RDTSC, node [scope+10h] gets
// last = now - scope+0/+4 at +30h/+34h, sum += last at +38h/+3Ch, count +40h += 1, then
// the current node pointer [[0109E9F8]+4] = [[[0109E9F8]+4]]. The same instruction
// sequence as the inline fragment 00C5BCD7, so this forwards to
// dyn_profile_scope_leave_00c5bcd7.
void profile_scope_leave_00c38040(const void* scope, const DynProfileScopeContext& context);

// ---- unreachable small bodies (docs/CC13_DYN1.md, "Reachability") ----
// The Dyn::SAPBroadPhaseManager (vtable 00D7A13C) and Dyn::SAPRadixBroadPhaseManager
// (vtable 00D7A118) are built only by 00C37030 and the Ghidra-undefined 00C37210, which
// nothing calls or references, so their slots never run in this image. Small ones only.
// 00C32BB0 SAP slot 6 __thiscall(ECX = M, pair), RET 4: next = [pair+0Ch]; next unless it
// is the list sentinel M+48h, then 0.
void* sap_next_pair_00c32bb0(void* manager, const void* pair) noexcept;
// 00C32BD0 SAP slot 5 __fastcall(ECX = M), RET: [M+58h] ? [M+44h] : 0.
void* sap_first_pair_00c32bd0(const void* manager) noexcept;
// 00C32BE0 SAP slot 4 __fastcall(ECX = M), RET: [M+58h].
std::uint32_t sap_pair_count_00c32be0(const void* manager) noexcept;
// 00C32C20 SAPRadix slot 6 __thiscall(ECX = M unused, pair), RET 4: pair + 8.
void* radix_next_pair_00c32c20(const void* manager, void* pair) noexcept;
// 00C32C30 SAPRadix slot 4 __fastcall(ECX = M), RET: [M+20h].
std::uint32_t radix_pair_count_00c32c30(const void* manager) noexcept;
// 00C40D70 SAPRadix slot 5 __fastcall(ECX = M), RET: [M+20h] ? [M+1Ch] : 0.
void* radix_first_pair_00c40d70(const void* manager) noexcept;
// 00C327F0, register ABI ESI = O, RET, EAX = 0: free and clear [O+8h] then [O+14h]
// (00BF6989), each when non-null. Unreferenced.
std::uint32_t free_and_clear_pair_00c327f0(void* object, Cc13Dyn1Calls& calls);
// 00C32830, register ABI ESI = O, RET: free [O+10h] when non-null (00BF6989), then zero
// O+0, +4, +0Ch, +10h. Unreferenced.
void free_and_reset_00c32830(void* object, Cc13Dyn1Calls& calls);
// 00C32A60, register ABI EAX = &block, RET: free [EAX] when non-null (00BF6989).
// Unreferenced.
void free_if_set_00c32a60(void* const* block, Cc13Dyn1Calls& calls);

// 00C36C00, register ABI EDI = manifold, ECX = first body, EAX = second body, RET:
// manifold+C8h = 0, +CCh = first, +D0h = second, then 00C35260 (ESI = body, stack
// manifold) for the first body and for [manifold+D0h]. Unreferenced; forwards to
// append_native_dyn_manifold_00c35260.
void manifold_bind_bodies_00c36c00(void* manifold, void* first_body, void* second_body,
                                   const AvoidZoneDynHullMemory& memory,
                                   NativeDynNarrowPhaseCalls& narrow_calls);

// ---- misc object ----
// 00C357B0 __thiscall(ECX = O, v), RET 4: O+58h..60h = v, then virtual slot 0 of O.
void object_set_vec58_and_notify_00c357b0(void* object, const float v[3], Cc13Dyn1Calls& calls);
// 00C357D0 __fastcall(ECX = O), RET: EAX = O+58h.
float* object_vec58_00c357d0(void* object) noexcept;

// ---- geometry ----
// 00C377F0 stdcall(x0, y0, x1, y1, x2, y2), RET 18h: EAX = 1 when the 2D cross
// (y2-y0)*(x1-x0) - (y1-y0)*(x2-x0) is above 0 (ordered), else 0.
bool ccw_2d_00c377f0(float x0, float y0, float x1, float y1, float x2, float y2) noexcept;
// 00C37860, register ABI EAX = u_out, ECX = v_out, EDX = w_out; stack (x0, y0, x1, y1,
// x2, y2, px, py), RET 20h: the 2D barycentric coordinates of p in the triangle
// (0, 1, 2): u weights vertex 1, v vertex 2, w = 1 - u - v vertex 0.
void barycentric_2d_00c37860(float* u_out, float* v_out, float* w_out, float x0, float y0,
                             float x1, float y1, float x2, float y2, float px,
                             float py) noexcept;
// 00C412D0, register ABI EBX = segment start, EDX = box {min[3], max[3]}, stack
// (entry_out, &end), RET 8: slab test of the segment against the box. AL = 1 and
// entry_out = start + (end - start) * t_enter when the segment meets the box.
bool segment_box_entry_00c412d0(const float start[3], const float box[6], float entry_out[3],
                                const float end[3]) noexcept;
// 00C445B0, register ABI ECX = segment start, EAX = segment end; stack (world, point_out,
// normal_out, shape_out, group_mask), RET 14h. Walks the two body lists of [world+A8h]
// (+E0h/+E4h then +204h/+208h, next body+84h); for each body whose broad-phase box
// ([body+60h]+4) the current segment meets, tests every shape of its chain whose group
// [S+2Ch] meets group_mask through the world's per-type dispatcher [world+90h+4*[S+8h]].
// Each hit shortens the segment to its point. AL = any hit.
bool world_ray_cast_00c445b0(const float start[3], const float end[3], void* world,
                             float point_out[3], float normal_out[3], void** shape_out,
                             std::uint32_t group_mask, Cc13Dyn1Calls& calls);

}  // namespace bsp::cc13_dyn1
