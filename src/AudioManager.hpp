#pragma once
#include "EventSystem.hpp"
#include <iostream>
#include <string>
#include <SFML/Audio.hpp>

// Audio manager using SFML
class AudioManager : public ISubscriber {
public:
    AudioManager();
    void onEvent(const GameEvent& event) override;
    
private:
    void playMoveSound();
    void playCaptureSound();
    void playGameStartSound();
    void playGameEndSound();
    
    // SFML sound objects
    sf::SoundBuffer moveBuffer_;
    sf::SoundBuffer captureBuffer_;
    sf::SoundBuffer gameStartBuffer_;
    sf::SoundBuffer gameOverBuffer_;
    sf::Sound sound_;
    
    // Helper to play sound
    void playSound(const sf::SoundBuffer& buffer, const std::string& description);
};