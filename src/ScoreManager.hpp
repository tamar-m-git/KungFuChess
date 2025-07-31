#pragma once
#include "EventSystem.hpp"
#include <string>

// Score manager for tracking captured pieces points
class ScoreManager : public ISubscriber {
public:
    ScoreManager();
    void onEvent(const GameEvent& event) override;
    
    // Get score text for display
    std::string getWhiteScoreText() const;
    std::string getBlackScoreText() const;
    
private:
    int white_score_ = 0;
    int black_score_ = 0;
    
    int getPieceValue(char piece_type) const;
    void updateScore(const std::string& captured_piece, const std::string& captor_piece);
};