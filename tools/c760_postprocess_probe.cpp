#include "bsp/game_scene_post_process.hpp"
#include "bsp/scene_file.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
int main(int argc, char** argv) {
    try {
        if(argc != 2) throw std::runtime_error("schema path required");
        std::ifstream input(argv[1], std::ios::binary);
        if(!input) throw std::runtime_error("schema could not be read");
        std::string text((std::istreambuf_iterator<char>(input)), {});
        bsp::SceneLexer lexer(text, ";{}=:()");
        if(lexer.next().text != "properties" || lexer.next().text != "SceneRootProps")
            throw std::runtime_error("schema declaration");
        std::vector<std::string> errors;
        auto schema = bsp::parse_scene_property_block_008f5a00(lexer, errors);
        if(!errors.empty()) throw std::runtime_error(errors.front());
        bsp::SceneLexer authored(R"({ "g_PostProcess" { "Colorize" {
            TextureName = S ""; } "HDR" { MiddleGray = F 2.0; } } })", ";{}=:()");
        auto header = bsp::parse_scene_property_block_008f5a00(authored, errors);
        if(!errors.empty()) throw std::runtime_error(errors.front());
        auto record = bsp::game::read_scene_post_process_record(schema, header);
        auto names = record.remap_names();
        if(!names[0].empty() || names[1] != "ColourRemap.tga" ||
            names[2] != "volume_technicolor_002.tga" || names[3] != "volume_remap_UW.tga")
            throw std::runtime_error("authored empty / schema fallback mismatch");
        std::cout << "PASS: installed SceneRootProps, partial HDR override, authored empty Colorize, inherited remaps\n";
        return 0;
    } catch(const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
