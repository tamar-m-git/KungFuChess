#include "ScoreManager.hpp"
#include <iostream>

ScoreManager::ScoreManager() {
    std::cout << "🏆 ScoreManager initialized" << std::endl;
}

void ScoreManager::onEvent(const GameEvent& event) {
    if (event.type == "piece_captured") {
        auto captured_it = event.data.find("captured");
        auto captor_it = event.data.find("captor");
        
        if (captured_it != event.data.end() && captor_it != event.data.end()) {
            updateScore(captured_it->second, captor_it->second);
        }
    }
}

int ScoreManager::getPieceValue(char piece_type) const {
    switch (piece_type) {
        case 'P': return 1;  // Pawn
        case 'N': return 3;  // Knight
        case 'B': return 3;  // Bishop
        case 'R': return 5;  // Rook
        case 'Q': return 9;  // Queen
        case 'K': return 0;  // King (shouldn't be captured)
        default: return 0;
    }
}

void ScoreManager::updateScore(const std::string& captured_piece, const std::string& captor_piece) {
    if (captured_piece.length() >= 2 && captor_piece.length() >= 2) {
        char captured_type = captured_piece[0];
        char captor_color = captor_piece[1];
        
        int points = getPieceValue(captured_type);
        
        if (captor_color == 'W') {
            white_score_ += points;
            std::cout << "🏆 White scored " << points << " points! Total: " << white_score_ << std::endl;
        } else if (captor_color == 'B') {
            black_score_ += points;
            std::cout << "🏆 Black scored " << points << " points! Total: " << black_score_ << std::endl;
        }
    }
}

std::string ScoreManager::getWhiteScoreText() const {
    return "WHITE SCORE: " + std::to_string(white_score_);
}

std::string ScoreManager::getBlackScoreText() const {
    return "BLACK SCORE: " + std::to_string(black_score_);
}