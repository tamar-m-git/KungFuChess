#include "AudioManager.hpp"

AudioManager::AudioManager() {
    std::cout << "🎵 AudioManager initializing with SFML..." << std::endl;
    
    // Load sound files
    if (!moveBuffer_.loadFromFile("sounds/move.mp3")) {
        std::cout << "⚠️ Could not load sounds/move.mp3" << std::endl;
    } else {
        std::cout << "✅ Loaded move sound" << std::endl;
    }
    
    if (!captureBuffer_.loadFromFile("sounds/capture.mp3")) {
        std::cout << "⚠️ Could not load sounds/capture.mp3" << std::endl;
    } else {
        std::cout << "✅ Loaded capture sound" << std::endl;
    }
    
    if (!gameStartBuffer_.loadFromFile("sounds/start.mp3")) {
        std::cout << "⚠️ Could not load sounds/start.mp3" << std::endl;
    } else {
        std::cout << "✅ Loaded game start sound" << std::endl;
    }
    
    if (!gameOverBuffer_.loadFromFile("sounds/gameOver.mp3")) {
        std::cout << "⚠️ Could not load sounds/gameOver.mp3" << std::endl;
    } else {
        std::cout << "✅ Loaded game over sound" << std::endl;
    }
    
    std::cout << "🎵 AudioManager ready!" << std::endl;
}

void AudioManager::onEvent(const GameEvent& event) {
    if (event.type == "piece_moved") {
        playMoveSound();
    } else if (event.type == "piece_captured") {
        playCaptureSound();
    } else if (event.type == "game_started") {
        playGameStartSound();
    } else if (event.type == "game_ended") {
        playGameEndSound();
    }
}

void AudioManager::playMoveSound() {
    playSound(moveBuffer_, "Piece moved");
}

void AudioManager::playCaptureSound() {
    playSound(captureBuffer_, "Piece captured");
}

void AudioManager::playGameStartSound() {
    playSound(gameStartBuffer_, "Game started");
}

void AudioManager::playGameEndSound() {
    playSound(gameOverBuffer_, "Game ended");
}

void AudioManager::playSound(const sf::SoundBuffer& buffer, const std::string& description) {
    std::cout << "🎵 Playing: " << description << std::endl;
    sound_.setBuffer(buffer);
    sound_.play();
}