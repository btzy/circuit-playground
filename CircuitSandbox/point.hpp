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
/**
 * A very simple point class
 */

#include <cstddef>
#include <algorithm>

#include "integral_division.hpp"

namespace ext {
    struct point {
        int32_t x, y;

        point() = default;
        point(const point&) = default;
        point(point&&) = default;
        point& operator=(const point&) = default;
        point& operator=(point&&) = default;
        constexpr point(int32_t x, int32_t y) :x(x), y(y) {}

        // + and - compound assignment operators
        constexpr point& operator+=(const point& other) noexcept {
            x += other.x;
            y += other.y;
            return *this;
        }
        constexpr point& operator-=(const point& other) noexcept {
            x -= other.x;
            y -= other.y;
            return *this;
        }
        constexpr point& operator*=(const int32_t& scale) noexcept {
            x *= scale;
            y *= scale;
            return *this;
        }
        constexpr point& operator/=(const int32_t& scale) noexcept {
            x /= scale;
            y /= scale;
            return *this;
        }
        constexpr point& operator%=(const int32_t& scale) noexcept {
            x %= scale;
            y %= scale;
            return *this;
        }

        // arithmetic operators
        constexpr point operator+(const point& other) const noexcept {
            point ret = *this;
            ret += other;
            return ret;
        }
        constexpr point operator-(const point& other) const noexcept {
            point ret = *this;
            ret -= other;
            return ret;
        }
        constexpr point operator*(const int32_t& scale) const noexcept {
            point ret = *this;
            ret *= scale;
            return ret;
        }
        constexpr point operator/(const int32_t& scale) const noexcept {
            point ret = *this;
            ret /= scale;
            return ret;
        }
        constexpr point operator%(const int32_t& scale) const noexcept {
            point ret = *this;
            ret %= scale;
            return ret;
        }

        // unary operators
        constexpr point operator+() const noexcept {
            return *this;
        }
        constexpr point operator-() const noexcept {
            return { -x, -y };
        }

        // equality operators
        constexpr bool operator==(const point& other) const noexcept {
            return x == other.x && y == other.y;
        }
        constexpr bool operator!=(const point& other) const noexcept {
            return !(*this == other);
        }

        // min/max points
        constexpr static point min() noexcept {
            return { std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::min() };
        }
        constexpr static point zero() noexcept {
            return { 0, 0 };
        }
        constexpr static point max() noexcept {
            return { std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max() };
        }

#ifdef SDL_h_ // additional convenience conversion functions if we are using SDL
        point(const SDL_Point& pt) noexcept : x(pt.x), y(pt.y) {}
        point(const SDL_MouseButtonEvent& ev) noexcept : x(ev.x), y(ev.y) {}
        point(const SDL_MouseMotionEvent& ev) noexcept : x(ev.x), y(ev.y) {}
#endif // SDL_h_

    };

    // integral division functions
    inline point div_floor(point pt, int32_t scale) noexcept {
        return { ext::div_floor(pt.x, scale), ext::div_floor(pt.y, scale) };
    }
    inline point div_ceil(point pt, int32_t scale) noexcept {
        return { ext::div_ceil(pt.x, scale), ext::div_ceil(pt.y, scale) };
    }
    inline point div_round(point pt, int32_t scale) noexcept {
        return { ext::div_round(pt.x, scale), ext::div_round(pt.y, scale) };
    }

    // top-left bound of two points
    constexpr inline point min(const point& pt1, const point& pt2) noexcept {
        return { std::min(pt1.x, pt2.x), std::min(pt1.y, pt2.y) };
    }
    // bottom-right bound of two points
    constexpr inline point max(const point& pt1, const point& pt2) noexcept {
        return { std::max(pt1.x, pt2.x), std::max(pt1.y, pt2.y) };
    }

#ifdef SDL_h_ // additional convenience conversion functions if we are using SDL
    constexpr inline bool point_in_rect(const point& pt, const SDL_Rect& rect) noexcept {
        return pt.x >= rect.x && pt.x < rect.x + rect.w && pt.y >= rect.y && pt.y < rect.y + rect.h;
    }
    constexpr inline point restrict_to_rect(const point& pt, const SDL_Rect& rect) noexcept {
        return { std::clamp(pt.x, rect.x, rect.x + rect.w - 1), std::clamp(pt.y, rect.y, rect.y + rect.h - 1) };
    }
#endif // SDL_h_
}
