#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

// Event data structure
struct GameEvent {
    std::string type;
    std::unordered_map<std::string, std::string> data;
    
    GameEvent(const std::string& t) : type(t) {}
    GameEvent(const std::string& t, const std::unordered_map<std::string, std::string>& d) 
        : type(t), data(d) {}
};

// Subscriber interface
class ISubscriber {
public:
    virtual ~ISubscriber() = default;
    virtual void onEvent(const GameEvent& event) = 0;
};

// Publisher class
class EventPublisher {
public:
    void subscribe(const std::string& eventType, std::shared_ptr<ISubscriber> subscriber);
    void publish(const GameEvent& event);
    
private:
    std::unordered_map<std::string, std::vector<std::shared_ptr<ISubscriber>>> subscribers_;
};