#pragma once

#include "EventSystem.hpp"
#include <unordered_map>
#include <string>

struct PlayerScore {
    int captured_pieces = 0;
    int total_value = 0;
};

class ScoreManager : public ISubscriber {
public:
    ScoreManager();
    
    // ISubscriber interface
    void onEvent(const GameEvent& event) override;
    
    // Getters
    const PlayerScore& getWhiteScore() const { return white_score_; }
    const PlayerScore& getBlackScore() const { return black_score_; }
    
private:
    PlayerScore white_score_;
    PlayerScore black_score_;
    
    int get_piece_value(char piece_type);
    void update_score(char captured_color, char piece_type);
};