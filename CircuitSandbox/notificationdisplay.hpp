/*
 * Circuit Sandbox
 * Copyright 2018 National University of Singapore <enterprise@nus.edu.sg>
 *
 * This file is part of Circuit Sandbox.
 * Circuit Sandbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
 * Circuit Sandbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with Circuit Sandbox.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <vector>
#include <memory>
#include <cstdint>
#include <utility>

#include <SDL.h>

#include "declarations.hpp"
#include "drawable.hpp"
#include "point.hpp"
#include "sdl_automatic.hpp"

using namespace std::literals;

namespace NotificationFlags {
    using Type = std::uint8_t;

    enum : Type {
        DEFAULT = 0b0001,
        BEGINNER = 0b0010
    };
}

class NotificationDisplay final : public Drawable {
public:
    using Flags = NotificationFlags::Type;
    struct ColorText {
        std::string text;
        SDL_Color color{ 0xFF, 0xFF, 0xFF, 0xFF };
    };
    using Data = std::vector<ColorText>;
private:
    enum struct Stage : std::uint8_t {
        VISIBLE,
        GONE
    };
    class Notification {
    public:
        Data data; // necessary in case a re-layout is invoked
        UniqueTexture texture;
        ext::point textureSize;
        Flags flags;
        // when the notification should disappear
        Drawable::RenderClock::time_point expireTime;

        Notification(std::vector<ColorText>&& data, Flags flags, Drawable::RenderClock::time_point expireTime = Drawable::RenderClock::time_point::max()) noexcept :
            data(std::move(data)),
            flags(flags),
            expireTime(expireTime) {}

        // sets texture and textureSize from the data
        void layout(SDL_Renderer* renderer, NotificationDisplay& notificationDisplay);
    };

    static constexpr int32_t LOGICAL_SPACING = 4;
    static constexpr ext::point LOGICAL_OFFSET{ 12, 12 };
    static constexpr ext::point LOGICAL_TEXT_PADDING{ 4, 4 };
    static constexpr SDL_Color backgroundColor{ 0,0,0,0x99 };

    // owner window
    MainWindow& mainWindow;

    // notification storage
    std::vector<std::shared_ptr<Notification>> notifications;

    // flags that decide which type of notifications are visible.
    Flags visibleFlags;

public:
    using NotificationHandle = std::weak_ptr<Notification>;

    static constexpr SDL_Color TEXT_COLOR = WHITE;
    static constexpr SDL_Color TEXT_COLOR_KEY = CYAN;
    static constexpr SDL_Color TEXT_COLOR_ACTION = GREEN;
    static constexpr SDL_Color TEXT_COLOR_CANCEL = YELLOW;
    static constexpr SDL_Color TEXT_COLOR_ERROR = RED;
    static constexpr SDL_Color TEXT_COLOR_STATE = BLUE;
    static constexpr SDL_Color TEXT_COLOR_FILE = MAGENTA;

    /**
     * Class for RAII notification handles.
     */
    struct UniqueNotification {
    private:
        NotificationDisplay* display;
        NotificationHandle handle;
    public:
        UniqueNotification(NotificationDisplay& display, NotificationHandle handle) noexcept : display(&display), handle(std::move(handle)) {}
        UniqueNotification() noexcept : display(nullptr) {}
        UniqueNotification(const UniqueNotification&) = delete;
        UniqueNotification& operator=(const UniqueNotification&) = delete;
        UniqueNotification(UniqueNotification&& other) noexcept : display(std::exchange(other.display, nullptr)), handle(std::move(other.handle)) {}
        UniqueNotification& operator=(UniqueNotification&& other) noexcept {
            _pre_destruct();
            display = std::exchange(other.display, nullptr);
            handle = std::move(other.handle);
            return *this;
        }
        ~UniqueNotification() noexcept {
            _pre_destruct();
        }
        void reset() noexcept {
            _pre_destruct();
            display = nullptr;
        }
        explicit operator bool() const noexcept {
            return display != nullptr;
        }
        template <typename... Args>
        UniqueNotification orElse(Args&&... args) && {
            assert(display);
            if (handle.expired()) {
                // this notification is not shown due to having no visible flags
                return display->uniqueAdd(std::forward<Args>(args)...);
            }
            else {
                return std::move(*this);
            }
        }
        template <typename... Args>
        void modify(Args&&... args) {
            assert(display);
            handle = display->modifyOrAdd(handle, std::forward<Args>(args)...);
        }
    private:
        inline void _pre_destruct() noexcept {
            if (display) {
                display->remove(handle);
            }
        }
    };

    NotificationDisplay(MainWindow&, Flags visibleFlags = NotificationFlags::DEFAULT);

    void render(SDL_Renderer*) override;

    /**
     * Draw all the notifications.
     */
    void layoutComponents(SDL_Renderer* renderer) override {
        for (auto& notification_ptr : notifications) {
            notification_ptr->layout(renderer, *this);
        }
    }

    /**
     * Add a notification to the display.
     * When adding notification: leading spaces allowed, but trailing spaces prohibited in description.
     */
    NotificationHandle add(Flags flags, Drawable::RenderClock::time_point expire, Data description);

    NotificationHandle add(Flags flags, Data description) {
        return add(flags, Drawable::RenderClock::time_point::max(), std::move(description));
    }

    NotificationHandle add(Flags flags, std::string description) {
        return add(flags, std::vector<ColorText>{ { description, NotificationDisplay::TEXT_COLOR } });
    }

    template <typename... Args>
    NotificationHandle add(Flags flags, Drawable::RenderClock::duration duration, Data description) {
        return add(flags, Drawable::renderTime + duration, std::move(description));
    }

    NotificationHandle add(Flags flags, Drawable::RenderClock::duration duration, std::string description) {
        return add(flags, duration, std::vector<ColorText>{ { description, NotificationDisplay::TEXT_COLOR } });
    }

    /**
     * Add a notification to the display, returning an RAII notification handle.
     */
    template <typename... Args>
    UniqueNotification uniqueAdd(Args&&... args) {
        return UniqueNotification(*this, add(std::forward<Args>(args)...));
    }

    /**
     * Remove a notification from the display.
     */
    void remove(const NotificationHandle& data) noexcept;

    /**
     * Modifies an object if it isn't removed yet, or add a new object if it is already removed.
     */
    NotificationHandle modifyOrAdd(const NotificationHandle& data, Flags flags, Drawable::RenderClock::time_point expire, Data description);

    NotificationHandle modifyOrAdd(const NotificationHandle& data, Flags flags, Drawable::RenderClock::duration duration, Data description) {
        return modifyOrAdd(data, flags, Drawable::renderTime + duration, std::move(description));
    }

    NotificationHandle modifyOrAdd(const NotificationHandle& data, Flags flags, Data description) {
        return modifyOrAdd(data, flags, Drawable::RenderClock::time_point::max(), std::move(description));
    }

    /**
     * Add a notification to the display, returning an RAII notification handle.
     */
    template <typename... Args>
    UniqueNotification uniqueModify(UniqueNotification&& uniqueNotif, Args&&... args) {
        if (uniqueNotif) {
            uniqueNotif.modify(std::forward<Args>(args)...);
            return std::move(uniqueNotif);
        }
        else {
            return uniqueAdd(std::forward<Args>(args)...);
        }
    }

    Flags getVisibleFlags() const noexcept {
        return visibleFlags;
    }

    Flags setVisibleFlags(Flags flags) noexcept {
        Flags ret = std::exchange(visibleFlags, flags);
        // remove all notifications that are now invisible
        for (size_t i = 0; i != notifications.size(); ++i) {
            if (!(flags & notifications[i]->flags)) {
                notifications.erase(notifications.begin() + i--);
            }
        }
        // return the old flags
        return ret;
    }
};
