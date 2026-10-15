// cc13_x87c: three hard leftovers (packet cc13-x87c, docs/CC13_X87C.md).
// Names are hypotheses; see the header for the native layouts and hooks.

#include "bsp/cc13_x87c.hpp"

#include <cstring>

namespace bsp {

namespace {

// The eight-neighbour step shared by both jump tables (00422274 for the probe,
// 00422294 for the move): 0 = -y, then clockwise through +x, +y and -x.
void cc13_x87c_step(std::int32_t direction, std::int32_t& x, std::int32_t& y) {
    switch (direction) {
    case 0: y -= 1; break;
    case 1: x += 1; y -= 1; break;
    case 2: x += 1; break;
    case 3: x += 1; y += 1; break;
    case 4: y += 1; break;
    case 5: x -= 1; y += 1; break;
    case 6: x -= 1; break;
    case 7: x -= 1; y -= 1; break;
    default: break;  // JA past the table: no step
    }
}

}  // namespace

// 00421CE0 BSP_AvoidZone_TraceGridContours (hypothesis).
// ABI: __stdcall(owner, grid, step), RET 0Ch, returns EAX = path or null.
// Evidence: listing 00421CE0..0042226E, jump tables 00422274/00422294, the
// x87 block 00421FDE..0042205D read from the bytes (00422009 is DC C9,
// FMUL ST(1),ST(0); the Ghidra listing prints it as FMUL ST1).
// Simplifications: the x87 sums are written as float operations in the image's
// order, which is exact at the main thread's 24-bit precision control; at 53 or
// 64 bits the image keeps the intermediates wider. The 16-float world copy goes
// through FLD/FSTP in the image, which would quiet a signalling NaN; here it is
// a plain float copy. The owner pointer is stored without a null test, as in
// the image (operator new throws rather than returning null in practice).
Cc13X87cContourPathNative* cc13_x87c_avoid_zone_trace_grid_contour_00421ce0(
    Cc13X87cContourOwnerNative& owner, std::int8_t* const* grid, std::int32_t step,
    Cc13X87cContourHost& host) {
    // 00421D04..00421D43: the grid extents from the source's slots +10h/+14h,
    // reduced by the step with signed (IDIV) division.
    const std::int32_t width =
        (host.grid_source_extent_x_vslot10(owner.grid_source) - 1) / step + 1;   // [ESP+48h]
    const std::int32_t height =
        (host.grid_source_extent_y_vslot14(owner.grid_source) - 1) / step + 1;  // [ESP+38h]

    // 00421D3C..00421D7C: new(1Ch), 00421C10 under EH state 0 (the unwind
    // funclet 00C5E570 frees the block), then the owner word.
    void* const block = host.operator_new_00bf681b(0x1C);
    Cc13X87cContourPathNative* path = nullptr;
    if (block != nullptr) {
        try {
            path = host.construct_path_00421c10(block);
        } catch (...) {
            host.free_00bf65ac(block);
            throw;
        }
    }
    path->owner = &owner;  // 00421D7C

    // 00421D7E..00421DAE: the first set cell, y outer and x inner.
    std::int32_t x = 0;
    std::int32_t y = 0;
    bool found = false;
    for (y = 0; y < height && !found; ) {
        for (x = 0; x < width; ++x) {
            if (grid[x][y] != 0) {  // 00421D94 CMP byte, 0
                found = true;
                break;
            }
        }
        if (!found) ++y;
    }

    if (found) {
        const std::int32_t start_x = x;       // [ESP+44h]
        const std::int32_t start_y = y;       // [ESP+2Ch]
        std::int32_t direction = 1;           // [ESP+28h], ECX
        bool running = true;                  // [ESP+13h]
        std::int32_t points = 0;              // [ESP+18h]
        std::int32_t x_max = x;               // [ESP+14h]
        std::int32_t x_min = x;               // [ESP+20h]
        std::int32_t y_max = y;               // [ESP+1Ch]
        std::int32_t y_min = y;               // [ESP+34h]
        std::int8_t neighbour = 0;            // [ESP+12h]

        do {
            // 00421DE0..00421DFB: the cell mark, (direction + 8) * 8 in byte arithmetic.
            grid[x][y] = static_cast<std::int8_t>(
                static_cast<std::uint8_t>((direction + 8) * 8));
            // 00421DF5..00421E00: start the probe at direction + 5 (mod 8), then
            // advance before every probe (00421E10..00421E1A).
            std::int32_t probe = direction + 5;
            if (probe > 7) probe -= 8;
            std::int32_t tries = 0;  // [ESP+30h]
            for (;;) {
                if (probe > 6) {
                    probe -= 7;
                } else {
                    probe += 1;
                }
                std::int32_t nx = x;
                std::int32_t ny = y;
                neighbour = 0;  // 00421E24
                cc13_x87c_step(probe, nx, ny);
                if (nx >= 0 && ny >= 0 && nx < width && ny < height) {
                    neighbour = grid[nx][ny];        // 00421E75
                    if (neighbour > 0) break;        // 00421E7E JG
                }
                if (++tries >= 8) break;             // 00421E80..00421E8E
            }

            if (probe != direction) {  // 00421E94
                // 00421E9C..00421ECA: grid to world through 0087FC30.
                float xz_storage[2];
                const float* const xz = host.forward_local_grid_to_world_xz_0087fc30(
                    owner, xz_storage, static_cast<float>(x * step),
                    static_cast<float>(y * step));
                const float gx = xz[0];  // [ESP+3Ch]
                const float gz = xz[1];  // [ESP+40h]

                // 00421ECF..00421FD9: the inlined world-inverse refresh.
                if (owner.inverse_valid == 0) {
                    if (owner.world_valid == 0) {
                        if (owner.parent != nullptr) {
                            host.entity_pose_refresh_world_00414db0(owner.parent);  // 00421F05
                        }
                        float product[16];
                        const float* source = owner.local;  // 00421F26
                        if (owner.parent != nullptr) {      // 00421F0A re-reads +3Ch
                            source = host.matrix_multiply4x4_00413920(
                                owner.local, product,
                                reinterpret_cast<const float*>(
                                    static_cast<const std::uint8_t*>(owner.parent) + 0xCC));
                        }
                        for (int i = 0; i < 16; ++i) owner.world[i] = source[i];  // 00421F29..00421FB2
                        owner.world_valid = 1;    // 00421FB8
                        owner.inverse_valid = 0;  // 00421FBF
                    }
                    owner.inverse_valid = 1;      // 00421FD2
                    host.matrix_affine_inverse_00b63d50(owner.inverse, owner.world);  // 00421FD9
                }

                // 00421FDE..0042205D, operand order from the bytes.
                const float* const m = owner.inverse;
                float sum_x = m[4] * 0.0f;        // 00422009 DC C9: ST1 = m[4] * 0
                sum_x = sum_x + m[0] * gx;        // 0042200D FADDP, m[0]*gx from 00421FF5
                sum_x = sum_x + m[8] * gz;        // 0042201F
                sum_x = sum_x + m[12];            // 00422021, FSTP [ESP+5Ch]
                float sum_z = 0.0f * m[6];        // 00422039 FMULP ST3 (the FLDZ copy)
                sum_z = sum_z + gx * m[2];        // 0042203D FADDP ST2
                sum_z = sum_z + gz * m[10];       // 0042203F FMUL [+138h], 00422045 FADDP
                sum_z = sum_z + m[14];            // 00422047, FSTP [ESP+64h]
                const float point[3] = {sum_x, 0.0f, sum_z};  // y from XORPS at 00421FEC
                host.vector12_push_back_004215d0(&path->vector_proxy, point);  // 00422061
                ++points;                          // 00422066
                direction = probe;                 // 0042206B
            }

            // 00422071..004220AF: add the exit direction to the mark, then move.
            grid[x][y] = static_cast<std::int8_t>(
                static_cast<std::uint8_t>(grid[x][y]) + static_cast<std::uint8_t>(direction));
            cc13_x87c_step(direction, x, y);

            // 004220B2..004220CF: closed (back at the start through a marked
            // cell) or over 25000 points.
            if ((neighbour > 1 && x == start_x && y == start_y) || points > 25000) {
                running = false;
            }
            // 004220D4..00422100: the bounding box.
            if (x < x_min) x_min = x;
            if (x > x_max) x_max = x;
            if (y < y_min) y_min = y;
            if (y > y_max) y_max = y;
        } while (running);

        // 0042210F..004221E4: clear the traced region row by row. A marked cell
        // (40h and above) is entry direction ((cell >> 3) - 4) & 7 and exit
        // direction cell & 7; the row stops clearing at an exit-side cell.
        if (y_min <= y_max) {
            for (std::int32_t row = y_min;;) {
                bool changed = false;  // [ESP+13h]
                bool inside = false;   // [ESP+12h]
                if (x_min > x_max) break;  // 00422134
                for (std::int32_t column = x_min; column <= x_max; ++column) {
                    std::int8_t& cell = grid[column][row];
                    const std::int8_t value = cell;    // 0042214A DL
                    bool enter = false;                // BL
                    bool leave = false;                // AL
                    if (value >= 0x40) {               // 00422154 JL (signed)
                        const std::uint8_t in =
                            static_cast<std::uint8_t>((value >> 3) - 4) & 7;  // SAR, SUB, AND
                        const std::uint8_t out = static_cast<std::uint8_t>(value) & 7;
                        enter = in < 6 && (out == 7 || out < in);            // 00422167..00422178
                        if (in < 2) {
                            leave = !(out < 3 && out != 0);                  // 0042217A..00422187
                        } else {
                            leave = in >= 4 && out >= 3 && out < in;         // 0042218B..00422198
                        }
                    }
                    if (!inside) {                     // 004221A2
                        inside = enter;
                        if (!enter) continue;
                    }
                    if (value > 0) {                   // 004221B0 JLE
                        cell = 0;
                        changed = true;
                    }
                    if (inside && leave) inside = false;  // 004221BC..004221C4
                }
                if (!changed) break;                   // 004221D6
                if (++row > y_max) break;              // 004221DD..004221E4
            }
        }
    }

    // 004221EA..00422256: keep the path only with at least three points.
    if (path->first == nullptr ||
        static_cast<std::uint32_t>((path->last - path->first) / 3) < 3) {  // signed /0Ch, JNC
        if (path->first != nullptr) host.free_00bf65ac(path->first);       // 00422219
        path->first = nullptr;
        path->last = nullptr;
        path->end = nullptr;
        if (path->name_data != nullptr) {                                  // 0042222D
            const std::uint32_t size = path->name_length + 1;              // read before 00419CC0
            void* const pool = host.sized_pool_singleton_00419cc0();
            host.sized_pool_return_block_00bd1510(pool, path->name_data, size, 1);
        }
        host.free_00bf65ac(path);  // 00422248
        return nullptr;
    }
    return path;
}

namespace {

using cc13_seg30::ArchiveArg;

template <typename T>
T cc13_x87c_read(const void* base, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + offset, sizeof(T));
    return value;
}

ArchiveArg cc13_x87c_key(const char* name) {
    return ArchiveArg{0, reinterpret_cast<std::uintptr_t>(name)};
}
ArchiveArg cc13_x87c_int(std::uint32_t value) { return ArchiveArg{1, value}; }
ArchiveArg cc13_x87c_float(float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return ArchiveArg{2, bits};
}
// Kind 3: the image writes only the low byte of the value word (MOV byte);
// the other three bytes are whatever the outgoing argument slot held.
ArchiveArg cc13_x87c_bool(std::uint8_t value) { return ArchiveArg{3, value}; }
ArchiveArg cc13_x87c_object(std::uint32_t value) { return ArchiveArg{4, value}; }

// The 00CE37FC lock guard: entered inline, left inline on the normal path and
// by 00411F00 on unwind (the same two operations).
class Cc13X87cLockGuard {
public:
    Cc13X87cLockGuard(Cc13X87cAirOpsDeclareHost& host, void* lock) : host_(host), lock_(lock) {
        if (lock_ != nullptr) host_.lock_enter(lock_);
    }
    ~Cc13X87cLockGuard() {
        if (lock_ != nullptr) host_.lock_leave(lock_);
    }
    Cc13X87cLockGuard(const Cc13X87cLockGuard&) = delete;
    Cc13X87cLockGuard& operator=(const Cc13X87cLockGuard&) = delete;

private:
    Cc13X87cAirOpsDeclareHost& host_;
    void* lock_;
};

// Property names, read from the image.
constexpr const char* kCc13X87cReadySlots = "readySlots";                    // 00CF87B4
constexpr const char* kCc13X87cLandingSquadrons = "landingSquadrons";        // 00CF87A0
constexpr const char* kCc13X87cSquadronId = "squadronID";                    // 00CF8794
constexpr const char* kCc13X87cWaitingPos = "waitingPos";                    // 00CF8788
constexpr const char* kCc13X87cLandingPlanes = "landingPlanes";              // 00CF8778
constexpr const char* kCc13X87cPlaneId = "planeID";                          // 00CF8770
constexpr const char* kCc13X87cDistance = "distance";                        // 00CF8764
constexpr const char* kCc13X87cSpeedmul = "speedmul";                        // 00CF8758
constexpr const char* kCc13X87cLeftside = "leftside";                        // 00CF874C
constexpr const char* kCc13X87cAbort = "abort";                              // 00CF8744
constexpr const char* kCc13X87cState = "state";                              // 00CE7324
constexpr const char* kCc13X87cLaunchingSquadronId = "launchingSquadronID";  // 00CF8730
constexpr const char* kCc13X87cMaxInAirPlanes = "maxInAirPlanes";            // 00CF8720
constexpr const char* kCc13X87cRunwayFailure = "runwayFailure";              // 00CF8710
constexpr const char* kCc13X87cHangarFailure = "hangarFailure";              // 00CF8700
constexpr const char* kCc13X87cSuppPlaneTypes = "suppplanetypes";            // 00CF86F0
constexpr const char* kCc13X87cSlots = "slots";                              // 00CF86E8
constexpr const char* kCc13X87cClassId = "classID";                          // 00CF86E0
constexpr const char* kCc13X87cCount = "count";                              // 00CF86D8
constexpr const char* kCc13X87cEquipment = "equipment";                      // 00CF86CC
constexpr const char* kCc13X87cTargetId = "targetID";                        // 00CF86C0
constexpr const char* kCc13X87cStock = "stock";                              // 00CF86B8
constexpr const char* kCc13X87cSquadLimit = "squadLimit";                    // 00CF86AC
constexpr const char* kCc13X87cTraffic = "traffic";                          // 00CE5890

}  // namespace

// 006C19D0 BSP_AirOpsBlock_DeclareLuaProperties (hypothesis; the name says
// "declare" but every entry is a +0Ch write of the current value).
// ABI: __thiscall(ECX = block, archive), RET 4. Evidence: listing
// 006C19D0..006C23BF; strings read from the PE; guard destructor 00411F00.
// Block words (offsets only, no layout recovered beyond them): +4h/+8h/+0Ch
// locks; +10h readySlots std::list (head +14h, count +18h, node value +8h);
// +1Ch/+1Dh runwayFailure/hangarFailure bytes; +20h suppplanetypes; +38h
// launchingSquadronID; +3Ch traffic object; +40h stock std::list (head +44h,
// count +48h; node +8h class, +0Ch count, +10h squadLimit); +4Ch/+50h slots
// array of 58h records and count; +58h maxInAirPlanes; +98h/+9Ch
// landingSquadrons array of 14h records and count; +A8h/+ACh landingPlanes
// vector of 14h records.
// Simplifications: kind-3 values carry a zero-extended byte (the image leaves
// the upper three bytes stale); the trivially true checked-iterator container
// compare (CMP EBP,EBP / CMP EBX,EBX) is not reproduced.
void cc13_x87c_air_ops_declare_properties_006c19d0(void* block, void* archive,
                                                   Cc13X87cAirOpsDeclareHost& host) {
    // 006C19F2..006C1A9E: readySlots, no lock.
    if (cc13_x87c_read<std::uint32_t>(block, 0x18) != 0) {
        host.archive_open_04(archive, cc13_x87c_key(kCc13X87cReadySlots));
        std::uint32_t index = 0;  // [ESP+40h]
        auto* node = cc13_x87c_read<std::uint8_t*>(
            cc13_x87c_read<std::uint8_t*>(block, 0x14), 0);  // head->next
        for (;;) {
            auto* const head = cc13_x87c_read<std::uint8_t*>(block, 0x14);  // re-read, 006C1A32
            if (node == head) break;
            if (node == cc13_x87c_read<std::uint8_t*>(block, 0x14)) host.invalid_parameter_00bf6713();
            host.archive_write_0c(archive, cc13_x87c_int(index),
                                  cc13_x87c_int(cc13_x87c_read<std::uint32_t>(node, 0x8)));  // 006C1A82
            if (node == cc13_x87c_read<std::uint8_t*>(block, 0x14)) host.invalid_parameter_00bf6713();
            node = cc13_x87c_read<std::uint8_t*>(node, 0);
            ++index;
        }
        host.archive_close_08(archive);  // 006C1A9E
    }

    // 006C1AA0..006C1BC9: landingSquadrons under lock +4h.
    {
        Cc13X87cLockGuard guard(host, cc13_x87c_read<void*>(block, 0x4));
        if (cc13_x87c_read<std::uint32_t>(block, 0x9C) != 0) {
            host.archive_open_04(archive, cc13_x87c_key(kCc13X87cLandingSquadrons));
            auto* record = cc13_x87c_read<std::uint8_t*>(block, 0x98);
            std::uint32_t index = 0;
            auto end = [&] {
                return cc13_x87c_read<std::uint8_t*>(block, 0x98) +
                       cc13_x87c_read<std::uint32_t>(block, 0x9C) * 0x14;
            };
            for (; record != end(); record += 0x14, ++index) {  // end re-read, 006C1B93
                host.archive_open_04(archive, cc13_x87c_int(index));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cSquadronId),
                                      cc13_x87c_object(cc13_x87c_read<std::uint32_t>(record, 0x0)));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cWaitingPos),
                                      cc13_x87c_int(cc13_x87c_read<std::uint32_t>(record, 0x8)));
                host.archive_close_08(archive);
            }
            host.archive_close_08(archive);  // 006C1BBA
        }
    }

    // 006C1BCF..006C1F24: landingPlanes under lock +8h.
    {
        Cc13X87cLockGuard guard(host, cc13_x87c_read<void*>(block, 0x8));
        auto first = [&] { return cc13_x87c_read<std::uint8_t*>(block, 0xA8); };
        auto size = [&]() -> std::uint32_t {  // signed (last - first) / 14h
            return static_cast<std::uint32_t>(
                (cc13_x87c_read<std::uint8_t*>(block, 0xAC) - first()) / 0x14);
        };
        if (first() != nullptr && size() != 0) {
            host.archive_open_04(archive, cc13_x87c_key(kCc13X87cLandingPlanes));
            // The checked operator[] before every field read.
            auto at = [&](std::uint32_t i) -> std::uint8_t* {
                if (first() == nullptr || i >= size()) host.invalid_parameter_00bf6713();
                return first() + i * 0x14;
            };
            for (std::uint32_t i = 0; first() != nullptr && i < size(); ++i) {  // 006C1C50..006C1C79
                host.archive_open_04(archive, cc13_x87c_int(i));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cPlaneId),
                                      cc13_x87c_object(cc13_x87c_read<std::uint32_t>(at(i), 0x0)));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cDistance),
                                      cc13_x87c_float(cc13_x87c_read<float>(at(i), 0x4)));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cSpeedmul),
                                      cc13_x87c_float(cc13_x87c_read<float>(at(i), 0x8)));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cLeftside),
                                      cc13_x87c_bool(cc13_x87c_read<std::uint8_t>(at(i), 0xD)));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cAbort),
                                      cc13_x87c_bool(cc13_x87c_read<std::uint8_t>(at(i), 0xC)));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cState),
                                      cc13_x87c_int(cc13_x87c_read<std::uint32_t>(at(i), 0x10)));
                host.archive_close_08(archive);  // 006C1EFB
            }
            host.archive_close_08(archive);  // 006C1F0F
        }
    }

    // 006C1F2A..006C203C: the scalar properties, no lock.
    host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cLaunchingSquadronId),
                          cc13_x87c_object(cc13_x87c_read<std::uint32_t>(block, 0x38)));
    host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cMaxInAirPlanes),
                          cc13_x87c_int(cc13_x87c_read<std::uint32_t>(block, 0x58)));
    {
        // 006C1F85..006C1FCA: written only when it differs from {3, 0}.
        const ArchiveArg value = cc13_x87c_bool(cc13_x87c_read<std::uint8_t>(block, 0x1C));
        const ArchiveArg fallback = cc13_x87c_bool(0);
        if (!host.tagged_value_equals_00bd5680(value, fallback)) {
            host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cRunwayFailure), value);
        }
    }
    {
        // 006C1FCC..006C200C, the same for hangarFailure.
        const ArchiveArg value = cc13_x87c_bool(cc13_x87c_read<std::uint8_t>(block, 0x1D));
        const ArchiveArg fallback = cc13_x87c_bool(0);
        if (!host.tagged_value_equals_00bd5680(value, fallback)) {
            host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cHangarFailure), value);
        }
    }
    host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cSuppPlaneTypes),
                          cc13_x87c_int(cc13_x87c_read<std::uint32_t>(block, 0x20)));  // 006C203C

    // 006C203E..006C2227: slots under lock +0Ch.
    {
        Cc13X87cLockGuard guard(host, cc13_x87c_read<void*>(block, 0xC));
        if (cc13_x87c_read<std::uint32_t>(block, 0x50) != 0) {
            host.archive_open_04(archive, cc13_x87c_key(kCc13X87cSlots));
            auto end = [&] {
                return cc13_x87c_read<std::uint8_t*>(block, 0x4C) +
                       cc13_x87c_read<std::uint32_t>(block, 0x50) * 0x58;
            };
            auto* record = cc13_x87c_read<std::uint8_t*>(block, 0x4C);
            for (std::uint32_t index = 0; record != end(); record += 0x58, ++index) {
                host.archive_open_04(archive, cc13_x87c_int(index));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cClassId),
                    cc13_x87c_int(host.vehicle_class_registry_id_0095b9c0(
                        cc13_x87c_read<void*>(record, 0x4))));                      // 006C20DE
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cCount),
                                      cc13_x87c_int(cc13_x87c_read<std::uint32_t>(record, 0x8)));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cEquipment),
                                      cc13_x87c_int(cc13_x87c_read<std::uint32_t>(record, 0x10)));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cSquadronId),
                                      cc13_x87c_object(cc13_x87c_read<std::uint32_t>(record, 0x28)));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cTargetId),
                                      cc13_x87c_object(cc13_x87c_read<std::uint32_t>(record, 0x4C)));
                host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cState),
                                      cc13_x87c_int(cc13_x87c_read<std::uint32_t>(record, 0x2C)));
                host.archive_close_08(archive);  // 006C21F2
            }
            host.archive_close_08(archive);  // 006C2212
        }
    }

    // 006C222D..006C2383: stock, no lock.
    if (cc13_x87c_read<std::uint32_t>(block, 0x48) != 0) {
        host.archive_open_04(archive, cc13_x87c_key(kCc13X87cStock));
        auto head = [&] { return cc13_x87c_read<std::uint8_t*>(block, 0x44); };
        auto* node = cc13_x87c_read<std::uint8_t*>(head(), 0);
        for (std::uint32_t index = 0; node != head(); ++index) {  // end re-read, 006C2268
            host.archive_open_04(archive, cc13_x87c_int(index));
            if (node == head()) host.invalid_parameter_00bf6713();
            host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cClassId),
                cc13_x87c_int(host.vehicle_class_registry_id_0095b9c0(
                    cc13_x87c_read<void*>(node, 0x8))));                            // 006C22AC
            if (node == head()) host.invalid_parameter_00bf6713();
            host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cCount),
                                  cc13_x87c_int(cc13_x87c_read<std::uint32_t>(node, 0xC)));
            if (node == head()) host.invalid_parameter_00bf6713();
            host.archive_write_0c(archive, cc13_x87c_key(kCc13X87cSquadLimit),
                                  cc13_x87c_int(cc13_x87c_read<std::uint32_t>(node, 0x10)));
            host.archive_close_08(archive);  // 006C2365
            if (node == head()) host.invalid_parameter_00bf6713();
            node = cc13_x87c_read<std::uint8_t*>(node, 0);
        }
        host.archive_close_08(archive);  // 006C2383
    }

    // 006C2385..006C23AB: traffic, delegated to the object at +3Ch (no null test).
    void* const traffic = cc13_x87c_read<void*>(block, 0x3C);  // read before the open, 006C2387
    host.archive_open_04(archive, cc13_x87c_key(kCc13X87cTraffic));
    host.traffic_visit_vslot10(traffic, archive);
    host.archive_close_08(archive);
}

namespace {

std::uint8_t* cc13_x87c_bytes(void* base) { return static_cast<std::uint8_t*>(base); }

template <typename T>
void cc13_x87c_write(void* base, std::size_t offset, T value) {
    std::memcpy(cc13_x87c_bytes(base) + offset, &value, sizeof(T));
}

// The checked operator[] of a vector<int>/vector<void*> whose first/last words
// sit at base+offset / base+offset+4h (size = (last - first) >> 2, unsigned).
std::uint32_t cc13_x87c_vector4_size(const void* base, std::size_t offset) {
    const auto first = cc13_x87c_read<std::uint32_t>(base, offset);
    if (first == 0) return 0;
    return (cc13_x87c_read<std::uint32_t>(base, offset + 4) - first) >> 2;
}

// The intrusive release: InterlockedDecrement(object+4h), slot 0 when it hits 0.
void cc13_x87c_release(Cc13X87cShipDamageGfxHost& host, void* object) {
    if (host.interlocked_decrement_00ce2220(reinterpret_cast<long*>(cc13_x87c_bytes(object) + 4)) ==
        0) {
        host.refcounted_destroy_vslot00(object);
    }
}

// The inlined world refresh at 0081A0DB / 0081A1E3 (no inverse; +10Ch cleared).
void cc13_x87c_refresh_unit_world(void* unit, Cc13X87cShipDamageGfxHost& host) {
    if (cc13_x87c_read<std::uint8_t>(unit, 0xC8) != 0) return;
    if (cc13_x87c_read<void*>(unit, 0x3C) != nullptr) {
        host.entity_pose_refresh_world_00414db0(cc13_x87c_read<void*>(unit, 0x3C));
    }
    float product[16];
    const float* source = reinterpret_cast<const float*>(cc13_x87c_bytes(unit) + 0x74);
    if (void* const parent = cc13_x87c_read<void*>(unit, 0x3C); parent != nullptr) {
        source = host.matrix_multiply4x4_00413920(
            source, product, reinterpret_cast<const float*>(cc13_x87c_bytes(parent) + 0xCC));
    }
    for (int i = 0; i < 16; ++i) cc13_x87c_write<float>(unit, 0xCC + 4 * i, source[i]);  // FLD/FSTP
    cc13_x87c_write<std::uint8_t>(unit, 0xC8, 1);
    cc13_x87c_write<std::uint8_t>(unit, 0x10C, 0);
}

constexpr float kCc13X87cOne = 1.0f;  // 00D7A24C, float 3F800000h

void cc13_x87c_identity(float matrix[16]) {
    for (int i = 0; i < 16; ++i) matrix[i] = (i % 5 == 0) ? kCc13X87cOne : 0.0f;
}

}  // namespace

// 00819CC0 BSP_ShipUnit_SetDamagedGfxLevel (hypothesis).
// ABI: __thiscall(ECX = unit, level), RET 4. Evidence: listing
// 00819CC0..0081A4EC; vtable slot +1B4h census by byte scan of the image
// (nine .rdata hits, no call xref); the base 00877B00 occupies the same slot of
// 00CF8C08. Unit words: +358h current level, +48h part list (next +44h, part
// +38Ch group id), +4A4h parent scene node, +538h class, +664h/+668h vector of
// 44h records (node at +0h), +A14h/+A18h effect slots and count, pose block
// +3Ch/+74h/+C8h/+CCh/+FCh/+10Ch. Class words: +684h anchor points (0Ch
// stride), +688h anchored effect count, +690h announcement descriptors, +694h
// descriptor period, +7A8h/+7B8h/+7C8h id vectors, +7F8h/+7FCh effect handles.
// Simplifications: the `(-(p != 0)) & 00E186ECh` / `& 00E17BF8h` tests are
// written as null tests (the masks are non-zero constants); the effect
// reference of the anchored loop is released on unwind through a catch, as
// the EH state 0 funclet does.
void cc13_x87c_ship_set_damaged_gfx_level_00819cc0(void* unit, std::int32_t level,
                                                   Cc13X87cShipDamageGfxHost& host) {
    if (cc13_x87c_read<std::int32_t>(unit, 0x358) == level) return;  // 00819CF0
    host.set_damaged_gfx_level_base_00877b00(unit, level);           // 00819CF7
    if (level <= 0) return;                                           // 00819CFE JLE

    auto unit_class = [&] { return cc13_x87c_read<void*>(unit, 0x538); };

    // 00819D10..00819DC8: hide and detach every part whose group id is listed
    // in class+7A8h.
    for (std::uint32_t i = 0; i < cc13_x87c_vector4_size(unit_class(), 0x7A8); ++i) {
        if (i >= cc13_x87c_vector4_size(unit_class(), 0x7A8)) host.invalid_parameter_00bf6713();
        const auto id = cc13_x87c_read<std::int32_t>(
            reinterpret_cast<void*>(cc13_x87c_read<std::uintptr_t>(unit_class(), 0x7A8)), i * 4);
        for (void* part = cc13_x87c_read<void*>(unit, 0x48); part != nullptr;
             part = cc13_x87c_read<void*>(part, 0x44)) {
            if (!host.part_vslot5c(part, 0x1E)) continue;                   // 00819D69
            if (cc13_x87c_read<std::int32_t>(part, 0x38C) != id) continue;  // 00819D6F
            host.part_vslot70(part, 1);                                     // 00819D80
            if (host.part_vslot20(part) == nullptr) continue;               // 00819D89
            void* const node = host.part_vslot20(part);                     // 00819D96
            host.scene_node_set_visibility_factor_00b6da70(
                cc13_x87c_read<void*>(cc13_x87c_read<void*>(node, 0x160), 0xC), 0.0f,
                false);                                                     // 00819DA9
            host.unit_part_detach_spatial_node_00710b80(host.part_vslot20(part));  // 00819DB9
        }
    }

    // 00819DCD..00819E80: stop and release the effect slots listed in class+7B8h.
    for (std::uint32_t i = 0; i < cc13_x87c_vector4_size(unit_class(), 0x7B8); ++i) {
        if (i >= cc13_x87c_vector4_size(unit_class(), 0x7B8)) host.invalid_parameter_00bf6713();
        const auto index = cc13_x87c_read<std::int32_t>(
            reinterpret_cast<void*>(cc13_x87c_read<std::uintptr_t>(unit_class(), 0x7B8)), i * 4);
        if (cc13_x87c_read<std::int32_t>(unit, 0xA18) <= index) continue;  // 00819E18 JLE
        auto slot = [&] {
            return cc13_x87c_bytes(cc13_x87c_read<void*>(unit, 0xA14)) + index * 4;
        };
        void* const effect = cc13_x87c_read<void*>(slot(), 0);
        if (effect == nullptr) continue;                                    // 00819E37
        host.point_effect_stop_children_00867b10(effect);                  // 00819E43
        cc13_x87c_write<std::uint8_t>(effect, 0x9, 1);                      // 00819E48
        std::uint8_t* const reloaded = slot();                              // 00819E4C
        if (void* const held = cc13_x87c_read<void*>(reloaded, 0); held != nullptr) {
            cc13_x87c_release(host, held);                                  // 00819E5E
            cc13_x87c_write<void*>(reloaded, 0, nullptr);
        }
        cc13_x87c_write<void*>(reloaded, 0, nullptr);                       // 00819E77
    }

    // 00819E85..00819F20: release the scene nodes of the 44h records listed in class+7C8h.
    for (std::uint32_t i = 0; i < cc13_x87c_vector4_size(unit_class(), 0x7C8); ++i) {
        if (i >= cc13_x87c_vector4_size(unit_class(), 0x7C8)) host.invalid_parameter_00bf6713();
        const auto index = cc13_x87c_read<std::uint32_t>(
            reinterpret_cast<void*>(cc13_x87c_read<std::uintptr_t>(unit_class(), 0x7C8)), i * 4);
        const auto first = cc13_x87c_read<std::uintptr_t>(unit, 0x664);
        if (first == 0 ||
            index >= static_cast<std::uint32_t>(
                         static_cast<std::int32_t>(cc13_x87c_read<std::uintptr_t>(unit, 0x668) - first) /
                         0x44)) {
            host.invalid_parameter_00bf6713();                              // 00819EFB
        }
        void* const record =
            reinterpret_cast<void*>(cc13_x87c_read<std::uintptr_t>(unit, 0x664) + index * 0x44);
        if (void* const node = cc13_x87c_read<void*>(record, 0); node != nullptr) {
            host.node_unlink_release_00b6dfa0(node);                        // 00819F16
            cc13_x87c_write<void*>(record, 0, nullptr);
        }
    }

    void* const cls = unit_class();  // EDI from the last loop test, 00819F25
    float matrix[16];
    if (cc13_x87c_read<std::int32_t>(cls, 0x694) <= 0) {                    // 00819F27 JLE
        // 0081A365..0081A4D2: one random effect from class+7F8h, if that handle is set.
        const std::uint32_t count = cc13_x87c_vector4_size(cls, 0x7F8);
        const std::uint32_t pick = host.random_next_u32_00bd2fc0(nullptr) % count;  // DIV, no zero test
        void* const c = unit_class();
        if (pick >= cc13_x87c_vector4_size(c, 0x7F8)) host.invalid_parameter_00bf6713();
        auto handle_at = [&] {
            return cc13_x87c_read<void*>(
                reinterpret_cast<void*>(cc13_x87c_read<std::uintptr_t>(c, 0x7F8)), pick * 4);
        };
        if (handle_at() == nullptr) return;                                 // 0081A3C9
        void* const parent_node = cc13_x87c_read<void*>(unit, 0x4A4);
        cc13_x87c_identity(matrix);
        if (pick >= cc13_x87c_vector4_size(c, 0x7F8)) host.invalid_parameter_00bf6713();  // 0081A46A
        void* const handle = handle_at();
        if (handle != nullptr) {
            host.interlocked_increment_00ce221c(reinterpret_cast<long*>(cc13_x87c_bytes(handle) + 4));
        }
        void* effect = nullptr;
        host.point_effect_create_with_parent_matrix_008687c0(&effect, parent_node, handle, matrix,
                                                             1, 0, 0);  // 0081A499
        if (effect != nullptr) cc13_x87c_write<std::uint8_t>(effect, 0x9, 1);  // 0081A4B0
        if (effect != nullptr) cc13_x87c_release(host, effect);                 // 0081A4C2
        return;
    }

    // 00819F33..0081A360: the anchored effects and their announcements.
    const auto anchored = cc13_x87c_read<std::int32_t>(cls, 0x688);  // [ESP+10h]
    const auto period = cc13_x87c_read<std::int32_t>(cls, 0x694);    // [ESP+68h]
    std::int32_t descriptor_index = 0;                               // [ESP+1Ch]
    std::uint32_t anchor_offset = 0;                                 // [ESP+20h]
    for (std::int32_t i = 0; i == 0 || i < anchored; ++i, anchor_offset += 0xC) {
        void* const c = unit_class();
        void* const parent_node = cc13_x87c_read<void*>(unit, 0x4A4);
        cc13_x87c_identity(matrix);
        const std::uint32_t count = cc13_x87c_vector4_size(c, 0x7F8);
        const std::uint32_t pick = host.random_next_u32_00bd2fc0(nullptr) % count;  // 0081A000
        if (pick >= cc13_x87c_vector4_size(c, 0x7F8)) host.invalid_parameter_00bf6713();
        void* const handle = cc13_x87c_read<void*>(
            reinterpret_cast<void*>(cc13_x87c_read<std::uintptr_t>(c, 0x7F8)), pick * 4);
        if (handle != nullptr) {
            host.interlocked_increment_00ce221c(reinterpret_cast<long*>(cc13_x87c_bytes(handle) + 4));
        }
        void* effect = nullptr;
        host.point_effect_create_with_parent_matrix_008687c0(&effect, parent_node, handle, matrix,
                                                             1, 0, 0);  // 0081A057
        try {
            if (effect != nullptr) {
                if (i < anchored) {
                    host.land_fort_set_effect_anchor_00745130(
                        effect, reinterpret_cast<const float*>(
                                    cc13_x87c_read<std::uintptr_t>(unit_class(), 0x684) +
                                    anchor_offset));                        // 0081A094
                }
                cc13_x87c_write<std::uint8_t>(effect, 0x9, 1);              // 0081A099
            }
            void* const global = host.game_global_00e188a8();
            if (global != nullptr && cc13_x87c_read<void*>(global, 0x21D0) != nullptr) {
                const auto descriptor = cc13_x87c_read<std::int32_t>(
                    reinterpret_cast<void*>(cc13_x87c_read<std::uintptr_t>(unit_class(), 0x690)),
                    descriptor_index * 4);                                  // 0081A0D2
                float position[3];
                if (i < anchored) {
                    cc13_x87c_refresh_unit_world(unit, host);               // 0081A0DB
                    float transformed[3];
                    host.transform_affine_point_004142e0(
                        reinterpret_cast<const float*>(
                            cc13_x87c_read<std::uintptr_t>(unit_class(), 0x684) + anchor_offset),
                        transformed,
                        reinterpret_cast<const float*>(cc13_x87c_bytes(unit) + 0xCC));  // 0081A1A8
                    position[0] = transformed[0];
                    position[1] = transformed[1];
                    position[2] = transformed[2];
                } else {
                    cc13_x87c_refresh_unit_world(unit, host);               // 0081A1E3
                    position[0] = cc13_x87c_read<float>(unit, 0xFC);        // 0081A2B9
                    position[1] = cc13_x87c_read<float>(unit, 0x100);
                    position[2] = cc13_x87c_read<float>(unit, 0x104);
                }
                float velocity_storage[3];
                const float* const velocity = host.unit_vslot34(unit, velocity_storage);  // 0081A2F2
                host.point_announcement_create_0049c940(
                    cc13_x87c_read<void*>(host.game_global_00e188a8(), 0x21D0), position,
                    velocity, descriptor);                                  // 0081A309
            }
        } catch (...) {
            if (effect != nullptr) cc13_x87c_release(host, effect);  // EH state 0 funclet
            throw;
        }
        if (++descriptor_index >= period) descriptor_index = 0;      // 0081A312..0081A323
        if (effect != nullptr) cc13_x87c_release(host, effect);      // 0081A336
    }
}

}  // namespace bsp
