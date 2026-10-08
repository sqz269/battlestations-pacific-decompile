#include "bsp/game_native_game_tables.hpp"
#include <stdexcept>

namespace bsp::game {
GameNativeGameTables::GameNativeGameTables(std::uint32_t unit_frame_word_preimage) noexcept
    :context_{unit_rows_.data(),unit_count_,unit_frame_word_preimage,
        native_unit_conversion_definitions(),native_gunnery_preference_words().data(),
        rank_rows_.data()} {
    static_assert(sizeof(std::uint32_t)==4);
}

NativeGameTablesContext& GameNativeGameTables::borrow_construction_context(){
    if(context_borrowed_||unit_count_!=0)
        throw std::logic_error("Native game tables require a fresh, unborrowed append domain");
    context_borrowed_=true;
    return context_;
}
} // namespace bsp::game
