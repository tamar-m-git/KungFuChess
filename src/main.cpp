#include <iostream>
#include "Game.hpp"
#include "img/OpenCvImg.hpp"
#include <memory>

int main() {
    std::cout << "🎮 Starting KungFu Chess..." << std::endl;
    try {
        auto img_factory = std::make_shared<OpenCvImgFactory>();
        std::string pieces_root = "pieces/";
        std::cout << "📁 Loading game from: " << pieces_root << std::endl;
        auto game = create_game(pieces_root, img_factory);
        std::cout << "🚀 Starting game loop..." << std::endl;
        game.run(-1, true);
        std::cout << "✅ Game ended normally" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        std::cout << "Press Enter to continue..." << std::endl;
        std::cin.get();
        return 1;
    }
    return 0;
}