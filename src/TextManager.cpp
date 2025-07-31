#include "TextManager.hpp"
#include <iostream>

TextManager::TextManager() {
    current_text_ = "GAME START";
    std::cout << "📝 TextManager initialized with: " << current_text_ << std::endl;
}

void TextManager::onEvent(const GameEvent& event) {
    if (event.type == "game_started") {
        setGameStartText();
    } else if (event.type == "game_playing") {
        setGamePlayingText();
    } else if (event.type == "game_ended") {
        if (event.data.find("winner") != event.data.end()) {
            setWinnerText(event.data.at("winner"));
        } else {
            setGameEndedText();
        }
    }
}

void TextManager::setGameStartText() {
    current_text_ = "GAME START";
    std::cout << "📝 Text updated to: " << current_text_ << std::endl;
}

void TextManager::setGamePlayingText() {
    current_text_ = "";
    std::cout << "📝 Text cleared for playing state" << std::endl;
}

void TextManager::setGameEndedText() {
    current_text_ = "GAME ENDED";
    std::cout << "📝 Text updated to: " << current_text_ << std::endl;
}

void TextManager::setWinnerText(const std::string& winner) {
    current_text_ = winner + " WINS!";
    std::cout << "📝 Text updated to: " << current_text_ << std::endl;
}