// Producers behind the unit-side gunnery pass. See docs/GUNNERY_TABLES.md.
//
// Every constant table here is transcribed from the shipped image or from the
// shipped Lua data table named in the comment; every rule is a projection of one
// native body and cites the site it comes from.

#include "bsp/gunnery_tables.hpp"

#include <cstring>

namespace bsp {
namespace {

// 007327E4..00732991, the string literals in the order of the stores.
constexpr const char* kFunctionNames[kUnitGunneryCategoryCount] = {
    "PLANEGUN",             // 0, EBP (xor ebp,ebp at 007327EC)
    "AAMACHINEGUN",         // 1, 00732857
    "LIGHTARTILLERY",       // 2, EBX (mov ebx,2 at 00732806)
    "MEDIUMARTILLERY",      // 3, 00732895
    "HEAVYARTILLERY",       // 4, 007328B6
    "FLAK",                 // 5, 007328D7
    "LIGHTARTILLERYFLAK",   // 6, 007328F8
    "TORPEDO",              // 7, 00732919
    "DEPTHCHARGE",          // 8, 00732937
    "DEPTHCHARGELAUNCHER",  // 9, 00732955
    "BOMBPLATFORM",         // 0Ah, 00732973
    "CATAPULT",             // 0Bh, 00732991
};

constexpr GunneryPreferenceRow make_row(int length,
                                        std::array<std::uint8_t, kGunneryPreferenceRowCapacity> ids) {
    return GunneryPreferenceRow{length, ids};
}

}  // namespace

// Transcribed from battlestationspacific.exe, 00E092C8, twelve rows of 61h
// dwords on a 184h stride. Only the leading entries shown are non-zero.
const std::array<GunneryPreferenceRow, kUnitGunneryCategoryCount>
    kGunneryPreferenceLists = {{
        // 0 PLANEGUN, 00E092C8. The single id 61h is one past the class-id
        // space, so the row never matches a class: an inert placeholder.
        make_row(1, {0x61}),
        // 1 AAMACHINEGUN, 00E0944C. Planes first, then the soft surface classes.
        make_row(12, {0x17, 0x11, 0x12, 0x15, 0x16, 0x10, 0x13, 0x0E, 0x0C,
                      0x0B, 0x08, 0x41}),
        // 2 LIGHTARTILLERY, 00E095D0.
        make_row(14, {0x07, 0x0C, 0x0B, 0x0E, 0x0A, 0x08, 0x09, 0x0D, 0x1C,
                      0x1B, 0x45, 0x46, 0x19, 0x41}),
        // 3 MEDIUMARTILLERY, 00E09754.
        make_row(14, {0x07, 0x0A, 0x08, 0x09, 0x0C, 0x0B, 0x0E, 0x0D, 0x1C,
                      0x1B, 0x45, 0x46, 0x19, 0x41}),
        // 4 HEAVYARTILLERY, 00E098D8. Battleship first.
        make_row(14, {0x0D, 0x0A, 0x07, 0x09, 0x0C, 0x0B, 0x08, 0x0E, 0x1C,
                      0x1B, 0x45, 0x46, 0x19, 0x41}),
        // 5 FLAK, 00E09A5C. Air only, plus the torpedo boat.
        make_row(9, {0x17, 0x11, 0x12, 0x15, 0x16, 0x14, 0x10, 0x13, 0x0E}),
        // 6 LIGHTARTILLERYFLAK, 00E09BE0. The dual-purpose row: air first,
        // then the whole surface list.
        make_row(22, {0x17, 0x11, 0x12, 0x16, 0x15, 0x14, 0x10, 0x13, 0x0E,
                      0x07, 0x08, 0x0B, 0x0C, 0x09, 0x0A, 0x0D, 0x1C, 0x1B,
                      0x45, 0x46, 0x19, 0x41}),
        // 7 TORPEDO, 00E09D64. Mothership first.
        make_row(8, {0x09, 0x0D, 0x0A, 0x07, 0x0C, 0x0B, 0x08, 0x41}),
        // 8 DEPTHCHARGE, 00E09EE8. Submarine only.
        make_row(2, {0x08, 0x41}),
        // 9 DEPTHCHARGELAUNCHER, 00E0A06C. Submarine only.
        make_row(2, {0x08, 0x41}),
        // 0Ah BOMBPLATFORM, 00E0A1F0. Empty in the image.
        make_row(0, {}),
        // 0Bh CATAPULT, 00E0A374. Empty in the image.
        make_row(0, {}),
    }};

const char* gunnery_category_function_name(int category) noexcept {
    if (category < 0 || category >= kUnitGunneryCategoryCount) {
        return nullptr;
    }
    return kFunctionNames[category];
}

int gunnery_category_from_function_name(const char* function_name) noexcept {
    if (function_name == nullptr) {
        return -1;
    }
    for (int i = 0; i < kUnitGunneryCategoryCount; ++i) {
        if (std::strcmp(function_name, kFunctionNames[i]) == 0) {
            return i;
        }
    }
    return -1;
}

void build_rank_table_00727bd0(
    const std::array<GunneryPreferenceRow, kUnitGunneryCategoryCount>& lists,
    int* out) noexcept {
    if (out == nullptr) {
        return;
    }
    int base = 0;  // EBX, category * 61h
    for (int category = 0; category < kUnitGunneryCategoryCount; ++category) {
        // 00727BEC: REP STOSD 61h zeros over this category's row.
        int* row = out + static_cast<std::size_t>(category) * kUnitGunneryClassIdCount;
        for (int i = 0; i < kUnitGunneryClassIdCount; ++i) {
            row[i] = 0;
        }
        // 00727BF3: EDX, the one-based rank, restarts at 1 per category.
        int rank = 1;
        const GunneryPreferenceRow& list = lists[static_cast<std::size_t>(category)];
        for (int slot = 0; slot < kUnitGunneryClassIdCount; ++slot) {
            const int id = slot < list.length ? static_cast<int>(list.ids[static_cast<std::size_t>(slot)]) : 0;
            if (id == 0) {
                continue;  // 00727BF7
            }
            // 00727BFB: the write index is id + category*61h, not clamped. Row 0's
            // id 61h therefore lands on row 1 slot 0; the next iteration's zeroing
            // above erases it before row 1 is filled.
            const int index = id + base;
            const int limit = kUnitGunneryCategoryCount * kUnitGunneryClassIdCount;
            if (index >= 0 && index < limit) {
                out[index] = rank;
            }
            ++rank;
        }
        base += kUnitGunneryClassIdCount;  // 00727C13
    }
}

int gunnery_rank(const int* rank_table, int category, int class_id) noexcept {
    if (rank_table == nullptr || category < 0 ||
        category >= kUnitGunneryCategoryCount || class_id < 0 ||
        class_id >= kUnitGunneryClassIdCount) {
        return 0;
    }
    return rank_table[class_id + category * kUnitGunneryClassIdCount];
}

void gunnery_throttle_prime_00864c1d(GunneryThrottle& throttle,
                                     float think_time) noexcept {
    // 00864C1D..00864C28: FLD [GlobalConfig+88h], FSTP [this+6Ch].
    throttle.accumulator = think_time;
}

bool gunnery_throttle_step_00865014(GunneryThrottle& throttle,
                                    float dt,
                                    float think_time) noexcept {
    // 00865014: this+6Ch += dt. 0086501D..0086502D: compare and, on a pass,
    // zero the accumulator (0086506A).
    throttle.accumulator += dt;
    if (throttle.accumulator < think_time) {
        return false;
    }
    throttle.accumulator = 0.0f;
    return true;
}

bool entity_suppresses_gunnery_00862440(bool proxy_present,
                                        bool untouchable) noexcept {
    // 0086244D and 0086245D: both must hold.
    return proxy_present && untouchable;
}

bool weapon_function_is_artillery_00956dc9(int weapon_function) noexcept {
    // 00956E22, 00956E27, 00956E2C and 00956E31 (against ECX, `mov ecx, 6` at
    // 00956DB1, the only ECX write between the loop head and the compares).
    return weapon_function == static_cast<int>(GunneryCategory::kLightArtillery) ||
           weapon_function == static_cast<int>(GunneryCategory::kMediumArtillery) ||
           weapon_function == static_cast<int>(GunneryCategory::kHeavyArtillery) ||
           weapon_function == static_cast<int>(GunneryCategory::kLightArtilleryFlak);
}

float category_engagement_range_00956d63(
    const GunneryRebuildDevice* guns, int count, int category) noexcept {
    // 00956D63: the row is seeded with the float at 00CE38B8 before the walk.
    float range = kGunneryCategoryRangeSeed;
    if (guns == nullptr) {
        return range;
    }
    for (int i = 0; i < count; ++i) {
        const GunneryRebuildDevice& gun = guns[i];
        // 00956D9F: 00731020, the ammunition record's range.
        if (range < gun.max_range) {
            range = gun.max_range;  // 00956DF3
        }
        // 00956DB1..00956DD0: only a Function 6 gun sitting in category 6 gets
        // the alternate term; everything else contributes 0.0f.
        float alternate = 0.0f;
        if (gun.weapon_function == static_cast<int>(GunneryCategory::kLightArtilleryFlak) &&
            category == static_cast<int>(GunneryCategory::kLightArtilleryFlak)) {
            alternate = gun.flak_alternate_range;
        }
        if (range < alternate) {
            range = alternate;  // 00956E0D
        }
    }
    return range;
}

GunneryRebuildResult rebuild_weapon_category_index_00956c20(
    UnitWeaponCategoryIndexHost& host) {
    GunneryRebuildResult result{};

    // 00956C31.
    host.clear_torpedo_flag();
    // 00956C38: the all-guns list at unit+424h.
    host.clear_list_00955eb0(-1);
    // 00956C48..00956C55: the twelve records at unit+394h + i*0Ch.
    for (int category = 0; category < kUnitGunneryCategoryCount; ++category) {
        host.clear_list_00955eb0(category);
    }

    // 00956C57..00956D3E: the unit's direct device children.
    GunneryRebuildDevice devices[256]{};
    int device_total = 0;
    const int reported = host.device_count();
    for (int i = 0; i < reported && device_total < 256; ++i) {
        const GunneryRebuildDevice device = host.device_at(i);
        // 00956C62 and 00956C76: not dead, and vtable[5Ch](20h).
        if (device.dead || !device.is_gun) {
            continue;
        }
        // 00956C86: the torpedo flag is set from the class before the
        // operational test, so a destroyed torpedo mount still sets it.
        if (device.weapon_function == static_cast<int>(GunneryCategory::kTorpedo)) {
            host.set_torpedo_flag();  // 00956C8F
            result.has_torpedo_launcher = true;
        }
        // 00956C98: 00729F10.
        if (!device.operational) {
            continue;
        }
        const int category = device.weapon_function;
        if (category < 0 || category >= kUnitGunneryCategoryCount) {
            continue;
        }
        // 00956CB6..00956CF4: the category node, appended at the tail.
        host.append_to_category(category, device.device);
        int& count = result.category_counts[static_cast<std::size_t>(category)];
        if (count < 32) {
            result.category_guns[static_cast<std::size_t>(category)][static_cast<std::size_t>(count)] =
                device.device;
            ++count;
        }
        // 00956CF9..00956D30: the same node shape on the all-guns list.
        host.append_to_all_guns(device.device);
        if (result.all_gun_count < 256) {
            result.all_guns[static_cast<std::size_t>(result.all_gun_count)] = device.device;
            ++result.all_gun_count;
        }
        devices[device_total] = device;
        ++device_total;
    }

    // 00956D49 and 00956D51: both seeded from the same 00CE38B8 constant.
    result.artillery_max_range = kGunneryCategoryRangeSeed;
    result.any_weapon_max_range = kGunneryCategoryRangeSeed;

    // 00956D5D..00956EC0: the per-category pass.
    for (int category = 0; category < kUnitGunneryCategoryCount; ++category) {
        GunneryRebuildDevice row[32]{};
        const int count = result.category_counts[static_cast<std::size_t>(category)];
        int filled = 0;
        for (int i = 0; i < device_total && filled < count; ++i) {
            if (devices[i].weapon_function == category) {
                row[filled] = devices[i];
                ++filled;
            }
        }

        const float range = category_engagement_range_00956d63(row, filled, category);
        result.category_ranges[static_cast<std::size_t>(category)] = range;
        host.store_category_range(category, range);

        float blast_sum = 0.0f;  // 00956D6C seeds the row with 0.
        for (int i = 0; i < filled; ++i) {
            const GunneryRebuildDevice& gun = row[i];
            // 00956E35..00956E43.
            if (weapon_function_is_artillery_00956dc9(gun.weapon_function) &&
                result.artillery_max_range < gun.max_range) {
                result.artillery_max_range = gun.max_range;
            }
            // 00956E4B..00956E59: every Function contributes here.
            if (result.any_weapon_max_range < gun.max_range) {
                result.any_weapon_max_range = gun.max_range;
            }
            // 00956E61: only when [desc+78h] > 0.
            if (gun.has_blast) {
                blast_sum += static_cast<float>(
                    (static_cast<double>(gun.blast_inner) + static_cast<double>(gun.blast_outer)) *
                    kGunneryBlastSumScale);
            }
        }
        result.category_blast_sums[static_cast<std::size_t>(category)] = blast_sum;
        host.store_category_blast_sum(category, blast_sum);
    }

    host.store_artillery_max_range(result.artillery_max_range);
    host.store_any_weapon_max_range(result.any_weapon_max_range);
    return result;
}

}  // namespace bsp
