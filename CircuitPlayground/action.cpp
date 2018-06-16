#include "action.hpp"
#include "selectionaction.hpp"
#include "pencilaction.hpp"

bool Action::processMouseButtonDown(const SDL_MouseButtonEvent& event) {
    return forwardEvent([&, this]() {
        return data->processMouseButtonDown(event);
    }, [&, this]() {
        return action_tags_t::for_each([&, this](auto action_tag, auto) {
            using action_t = typename decltype(action_tag)::type;
            return action_t::startWithMouseButtonDown(event, playArea, data);
        });
    });
}

bool Action::processMouseDrag(const SDL_MouseMotionEvent& event) {
    return forwardEvent([&, this]() {
        return data->processMouseDrag(event);
    }, [&, this]() {
        return action_tags_t::for_each([&, this](auto action_tag, auto) {
            using action_t = typename decltype(action_tag)::type;
            return action_t::startWithMouseDrag(event, playArea, data);
        });
    });
}

bool Action::processMouseButtonUp(const SDL_MouseButtonEvent& event) {
    return forwardEvent([&, this]() {
        return data->processMouseButtonUp(event);
    }, [&, this]() {
        return action_tags_t::for_each([&, this](auto action_tag, auto) {
            using action_t = typename decltype(action_tag)::type;
            return action_t::startWithMouseButtonUp(event, playArea, data);
        });
    });
}

// should we expect the mouse to be in the playarea?
// returns true if the event was consumed, false otherwise
bool Action::processMouseWheel(const SDL_MouseWheelEvent& event) {
    return forwardEvent([&, this]() {
        return data->processMouseWheel(event);
    }, [&, this]() {
        return action_tags_t::for_each([&, this](auto action_tag, auto) {
            using action_t = typename decltype(action_tag)::type;
            return action_t::startWithMouseWheel(event, playArea, data);
        });
    });
}

// returns true if the event was consumed, false otherwise
bool Action::processKeyboard(const SDL_KeyboardEvent& event) {
    return forwardEvent([&, this]() {
        return data->processKeyboard(event);
    }, [&, this]() {
        return action_tags_t::for_each([&, this](auto action_tag, auto) {
            using action_t = typename decltype(action_tag)::type;
            return action_t::startWithKeyboard(event, playArea, data);
        });
    });
}
