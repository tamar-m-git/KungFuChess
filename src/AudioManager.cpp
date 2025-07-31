#include "AudioManager.hpp"
#include <SFML/Audio.hpp>
#include <iostream>

AudioManager::AudioManager() {
    std::cout << "🎵 AudioManager initializing with SFML sound support..." << std::endl;
    
    // Load sound buffers
    if (!moveBuffer.loadFromFile("sounds/move.mp3")) {
        std::cout << "❌ Failed to load move.mp3" << std::endl;
    }
    if (!captureBuffer.loadFromFile("sounds/capture.mp3")) {
        std::cout << "❌ Failed to load capture.mp3" << std::endl;
    }
    if (!startBuffer.loadFromFile("sounds/start.mp3")) {
        std::cout << "❌ Failed to load start.mp3" << std::endl;
    }
    if (!gameOverBuffer.loadFromFile("sounds/gameOver.mp3")) {
        std::cout << "❌ Failed to load gameOver.mp3" << std::endl;
    }
    if (!changeBuffer.loadFromFile("sounds/change.mp3")) {
        std::cout << "❌ Failed to load change.mp3" << std::endl;
    }
    
    // Set buffers to sounds
    moveSound.setBuffer(moveBuffer);
    captureSound.setBuffer(captureBuffer);
    startSound.setBuffer(startBuffer);
    gameOverSound.setBuffer(gameOverBuffer);
    changeSound.setBuffer(changeBuffer);
    
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
    } else if (event.type == "pawn_promotion") {
        playChangeSound();
    }
}

void AudioManager::playMoveSound() {
    std::cout << "🎵 Playing: Piece moved" << std::endl;
    moveSound.play();
}

void AudioManager::playCaptureSound() {
    std::cout << "🎵 Playing: Piece captured" << std::endl;
    captureSound.play();
}

void AudioManager::playGameStartSound() {
    std::cout << "🎵 Playing: Game started" << std::endl;
    startSound.play();
}

void AudioManager::playGameEndSound() {
    std::cout << "🎵 Playing: Game ended" << std::endl;
    gameOverSound.play();
}

void AudioManager::playChangeSound() {
    std::cout << "🎵 Playing: Pawn promotion" << std::endl;
    changeSound.play();
}