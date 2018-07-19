#include "clipboardmanager.hpp"

void ClipboardManager::generateThumbnail(int32_t index, SDL_Renderer* renderer) {
    Clipboard& clipboard = clipboards[index];
    SDL_Surface* surface = SDL_CreateRGBSurface(0, clipboard.state.width(), clipboard.state.height(), 32, 0x000000FFu, 0x0000FF00u, 0x00FF0000u, 0);
    clipboard.state.fillSurface(reinterpret_cast<uint32_t*>(surface->pixels));
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    clipboard.thumbnail.reset(texture);
}

CanvasState ClipboardManager::read() const {
    return defaultClipboard;
}

CanvasState ClipboardManager::read(int32_t index) {
    // overwrite the default clipboard too, unless the selected clipboard is empty
    if (!clipboards[index].state.empty()) {
        defaultClipboard = clipboards[index].state;
    }
    return clipboards[index].state;
}

void ClipboardManager::write(const CanvasState& state) {
    defaultClipboard = state;
}

void ClipboardManager::write(const CanvasState& state, int32_t index, SDL_Renderer* renderer) {
    defaultClipboard = clipboards[index].state = state;
    generateThumbnail(index, renderer);
}

std::array<size_t, NUM_CLIPBOARDS> ClipboardManager::getOrder() const {
    std::array<size_t, NUM_CLIPBOARDS> order;
    for (int i = 0; i < NUM_CLIPBOARDS; ++i) order[i] = i;
    return order;
}

SDL_Texture* ClipboardManager::getThumbnail(int32_t index) const {
    return clipboards[index].thumbnail.get();
}
