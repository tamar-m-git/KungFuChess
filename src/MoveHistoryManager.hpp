#pragma once

#include "EventSystem.hpp"
#include <vector>
#include <string>

struct MoveRecord {
    std::string piece_id;
    std::string from_pos;
    std::string to_pos;
    std::string captured_piece = "";
    int timestamp;
};

class MoveHistoryManager : public ISubscriber {
public:
    MoveHistoryManager();
    
    // ISubscriber interface
    void onEvent(const GameEvent& event) override;
    
    // Getters
    const std::vector<MoveRecord>& getWhiteMoves() const { return white_move_history_; }
    const std::vector<MoveRecord>& getBlackMoves() const { return black_move_history_; }
    
private:
    std::vector<MoveRecord> white_move_history_;
    std::vector<MoveRecord> black_move_history_;
    
    void add_move_to_history(const std::string& piece_id, const std::string& from_pos, const std::string& to_pos, int timestamp);
};