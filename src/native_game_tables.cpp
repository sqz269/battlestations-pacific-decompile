#include "bsp/native_game_tables.hpp"
namespace bsp {
namespace {
using Word=std::uint32_t;
static_assert(sizeof(void*)==4&&sizeof(NativeUnitConversionDefinition)==16);
Word address(const void* p) noexcept {return reinterpret_cast<Word>(p);}
void put(void* p,Word offset,Word value) noexcept {
    *reinterpret_cast<volatile Word*>(address(p)+offset)=value;
}
Word read(const void* p,Word offset) noexcept {
    return *reinterpret_cast<const volatile Word*>(address(p)+offset);
}
const NativeUnitConversionDefinitions definitions{{
    {"ounce","g",0x41e26666u,0u},
    {"ounce","kg",0x3ce7d567u,0u},
    {"g","kg",0x3a83126fu,0u},
    {"t","kg",0x447a0000u,0u},
    {"m/s","kmh",0x40666666u,0u},
    {"m/s","km/h",0x40666666u,0u},
    {"mph","m/s",0x3ee4d5d8u,0u},
    {"nmph","m/s",0x3f03b257u,0u},
    {"kts","m/s",0x3f03b257u,0u},
    {"feet","m",0x3e9c0ebfu,0u},
    {"mile","m",0x44c92000u,0u},
    {"nmile","m",0x44e78000u,0u},
    {"mm","m",0x3a83126fu,0u},
    {"m","km",0x3a83126fu,0u},
    {"rpm","1/sec",0x42700000u,1u},
    {"KW","W",0x447a0000u,0u},
    {"Hp","W",0x443a82c1u,0u},
    {"DEG","RAD",0x3c8efa35u,0u},
    {"min","s",0x42700000u,0u},
    {"min","sec",0x42700000u,0u},
    {"h","min",0x42700000u,0u},
    {"h","s",0x45610000u,0u},
    {"h","sec",0x45610000u,0u},
}};
const std::array<Word,12*97> preferences{{
    97u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
    23u,17u,18u,21u,22u,16u,19u,14u,12u,11u,8u,65u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
    7u,12u,11u,14u,10u,8u,9u,13u,28u,27u,69u,70u,25u,65u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
    7u,10u,8u,9u,12u,11u,14u,13u,28u,27u,69u,70u,25u,65u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
    13u,10u,7u,9u,12u,11u,8u,14u,28u,27u,69u,70u,25u,65u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
    23u,17u,18u,21u,22u,20u,16u,19u,14u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
    23u,17u,18u,22u,21u,20u,16u,19u,14u,7u,8u,11u,12u,9u,10u,13u,28u,27u,69u,70u,25u,65u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
    9u,13u,10u,7u,12u,11u,8u,65u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
    8u,65u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
    8u,65u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
    0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
    0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
}};
}
const NativeUnitConversionDefinitions& native_unit_conversion_definitions() noexcept {return definitions;}
const std::array<Word,12*97>& native_gunnery_preference_words() noexcept {return preferences;}
void append_native_unit_conversions_008d9150(NativeGameTablesContext& c) noexcept {
    const Word upper=c.unit_frame_word_preimage&0xffffff00u;
    for(const auto& row:c.unit_definitions){
        const Word offset=c.unit_count_00f88bc0*16u;
        put(c.unit_rows_00f88a50,offset,address(row.source));
        put(c.unit_rows_00f88a50,offset+4,address(row.target));
        put(c.unit_rows_00f88a50,offset+8,row.factor_bits);
        put(c.unit_rows_00f88a50,offset+12,upper|(row.reciprocal&255u));
        c.unit_count_00f88bc0=c.unit_count_00f88bc0+1u;
    }
}
void build_native_gunnery_ranks_00727bd0(const void* input,void* output) noexcept {
    for(Word row=0;row<12;++row){
        const Word base=row*97u;
        for(Word slot=0;slot<97;++slot)put(output,(base+slot)*4u,0);
        Word rank=1;
        for(Word slot=0;slot<97;++slot){
            const Word id=read(input,(base+slot)*4u);
            if(id){put(output,(base+id)*4u,rank);++rank;}
        }
    }
}
} // namespace bsp
