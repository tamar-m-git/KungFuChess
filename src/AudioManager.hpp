#pragma once
#include "EventSystem.hpp"
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>

// Audio manager with SFML support
class AudioManager : public ISubscriber {
public:
    AudioManager();
    void onEvent(const GameEvent& event) override;
    
private:
    void playMoveSound();
    void playCaptureSound();
    void playGameStartSound();
    void playGameEndSound();
    void playChangeSound();
    
    // SFML sound objects
    sf::SoundBuffer moveBuffer, captureBuffer, startBuffer, gameOverBuffer, changeBuffer;
    sf::Sound moveSound, captureSound, startSound, gameOverSound, changeSound;

};