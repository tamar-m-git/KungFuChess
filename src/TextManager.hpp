#pragma once
#include "EventSystem.hpp"
#include <string>

// Text manager for dynamic display text based on events
class TextManager : public ISubscriber {
public:
    TextManager();
    void onEvent(const GameEvent& event) override;
    
    // Get current display text
    std::string getCurrentText() const { return current_text_; }
    
private:
    std::string current_text_;
    
    void setGameStartText();
    void setGamePlayingText();
    void setGameEndedText();
    void setWinnerText(const std::string& winner);
};