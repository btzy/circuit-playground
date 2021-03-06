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

#include <memory>
#include <stdexcept>
#include <string>
#include <cstring>
#include <SDL.h>
#include <SDL_ttf.h>

struct FontDeleter {
    void operator()(TTF_Font* ptr) const noexcept {
        TTF_CloseFont(ptr);
    }
};

class Font {
private:
    char* fontPath;
    const int logicalSize;
    int physicalSize;
    std::unique_ptr<TTF_Font, FontDeleter> font;
public:
    Font(const char* fontName, int logicalSize) :logicalSize(logicalSize), physicalSize(-1) {
        char* cwd = SDL_GetBasePath();
        if (cwd == nullptr) {
            using namespace std::literals::string_literals;
            throw std::runtime_error("SDL_GetBasePath() failed:  "s + SDL_GetError());
        }
        fontPath = new char[std::strlen(cwd) + std::strlen(fontName) + 1];
        std::strcpy(fontPath, cwd);
        std::strcat(fontPath, fontName);
        SDL_free(cwd);
    }

    ~Font() {
        delete[] fontPath;
    }

    template <typename MainWindow>
    void updateDPI(const MainWindow& mainWindow) {
        int newPhysicalSize = mainWindow.logicalToPhysicalSize(logicalSize);
        if (newPhysicalSize != physicalSize) {
            font.reset(nullptr); // delete the old font first
            physicalSize = newPhysicalSize;
            font.reset(TTF_OpenFont(fontPath, physicalSize)); // make the new font
        }
    }

    operator TTF_Font*() const {
        return font.get();
    }
};
