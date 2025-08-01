#include "ScoreManager.hpp"
#include <iostream>

ScoreManager::ScoreManager() {
    // Initialize scores to zero
}

void ScoreManager::onEvent(const GameEvent& event) {
    if (event.type == "piece_captured") {
        auto it_captured = event.data.find("captured");
        if (it_captured != event.data.end() && it_captured->second.length() >= 2) {
            char captured_color = it_captured->second[1]; // W or B
            char piece_type = it_captured->second[0];     // P, R, N, etc.
            update_score(captured_color, piece_type);
        }
    }
}

int ScoreManager::get_piece_value(char piece_type) {
    switch (piece_type) {
        case 'P': return 1;  // Pawn
        case 'N': return 3;  // Knight
        case 'B': return 3;  // Bishop
        case 'R': return 5;  // Rook
        case 'Q': return 9;  // Queen
        case 'K': return 0;  // King (game ends)
        default: return 0;
    }
}

void ScoreManager::update_score(char captured_color, char piece_type) {
    int value = get_piece_value(piece_type);
    if (captured_color == 'W') {
        black_score_.captured_pieces++;
        black_score_.total_value += value;
    } else {
        white_score_.captured_pieces++;
        white_score_.total_value += value;
    }
}