#include "EventSystem.hpp"

void EventPublisher::subscribe(const std::string& eventType, std::shared_ptr<ISubscriber> subscriber) {
    subscribers_[eventType].push_back(subscriber);
}

void EventPublisher::publish(const GameEvent& event) {
    auto it = subscribers_.find(event.type);
    if (it != subscribers_.end()) {
        for (auto& subscriber : it->second) {
            subscriber->onEvent(event);
        }
    }
}