#include "MoveHistoryManager.hpp"
#include <iostream>

MoveHistoryManager::MoveHistoryManager() {
    // Initialize empty histories
}

void MoveHistoryManager::onEvent(const GameEvent& event) {
    if (event.type == "piece_moved") {
        auto it_piece = event.data.find("piece_id");
        auto it_from = event.data.find("from");
        auto it_to = event.data.find("to");
        auto it_timestamp = event.data.find("timestamp");
        
        if (it_piece != event.data.end() && it_from != event.data.end() && 
            it_to != event.data.end() && it_timestamp != event.data.end()) {
            
            int timestamp = std::stoi(it_timestamp->second);
            add_move_to_history(it_piece->second, it_from->second, it_to->second, timestamp);
        }
    }
}

void MoveHistoryManager::add_move_to_history(const std::string& piece_id, const std::string& from_pos, const std::string& to_pos, int timestamp) {
    MoveRecord move;
    move.piece_id = piece_id;
    move.from_pos = from_pos;
    move.to_pos = to_pos;
    move.timestamp = timestamp;
    
    // Add to appropriate player's history based on piece color
    if (piece_id.length() >= 2) {
        if (piece_id[1] == 'W') {
            white_move_history_.push_back(move);
            // Keep only last 15 moves for white player
            if (white_move_history_.size() > 15) {
                white_move_history_.erase(white_move_history_.begin());
            }
        } else if (piece_id[1] == 'B') {
            black_move_history_.push_back(move);
            // Keep only last 15 moves for black player
            if (black_move_history_.size() > 15) {
                black_move_history_.erase(black_move_history_.begin());
            }
        }
    }
}