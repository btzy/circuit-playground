#include <stdexcept>
#include <string>
#include <numeric>
#include <algorithm>

#include <SDL.h>
#include <SDL_ttf.h>

#include <iostream>

#include "mainwindow.hpp"


using namespace std::literals::string_literals; // gives the 's' suffix for strings

#ifdef _WIN32
// hack for issue with window resizing on Windows not giving live events
int resizeEventForwarder(void* main_window_void_ptr, SDL_Event* event) {
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_RESIZED) {
        SDL_Window* event_window = SDL_GetWindowFromID(event->window.windowID);
        MainWindow* main_window = static_cast<MainWindow*>(main_window_void_ptr);
        if (event_window == main_window->window) {
            main_window->layoutComponents();
            main_window->render();
        }
    }
    return 0;
}
#endif // _WIN32




MainWindow::MainWindow() : closing(false), toolbox(*this), playArea(*this) {

    // unset all the input handle selection state
    std::fill_n(selectedToolIndices, NUM_INPUT_HANDLES, EMPTY_INDEX);

    // update dpi once first, so we can use it to create the properly sized window
    updateDpiFields(false);

    // TODO: allow high DPI with SDL_WINDOW_ALLOW_HIGHDPI flag and test whether it changes anything:
    window = SDL_CreateWindow("Circuit Playground", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, logicalToPhysicalSize(640), logicalToPhysicalSize(480), SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == nullptr) {
        throw std::runtime_error("SDL_CreateWindow() failed:  "s + SDL_GetError());
    }

    #ifdef _WIN32
    // On Windows, when the user is resizing the window, we don't get any events until the resize is complete.
    // This tries to fix this
    SDL_AddEventWatch(resizeEventForwarder, static_cast<void*>(this));
    #endif // _WIN32

    // Create the renderer.  SDL_RENDERER_PRESENTVSYNC turns on the monitor refresh rate synchronization.
    // For some reason my SDL2 doesn't have any software renderer, so I can't do "SDL_RENDERER_SOFTWARE | SDL_RENDERER_ACCELERATED"
    // So we try creating hardware renderer, and if it doesn't work then we try creating software renderer.
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
    }
    if (renderer == nullptr) {
        throw std::runtime_error("SDL_CreateRenderer() failed:  "s + SDL_GetError());
    }

    // if not already the default, set the blend mode to none
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // update dpi again (in case the window was opened on something other than the default monitor)
    if (updateDpiFields()) {
        // resize the window, if the dpi changed
        SDL_SetWindowSize(window, logicalToPhysicalSize(640), logicalToPhysicalSize(480));
    }

    // do the layout
    layoutComponents();
}


MainWindow::~MainWindow() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


bool MainWindow::updateDpiFields(bool useWindow) {

    int display_index = 0;

    if (useWindow) {
        display_index = SDL_GetWindowDisplayIndex(window);
        if (display_index < 0) { // means that SDL_GetWindowDisplayIndex doesn't work, then we just use the default monitor
            display_index = 0;
        }
    }


    float dpi_float;
    SDL_GetDisplayDPI(display_index, nullptr, &dpi_float, nullptr); // well we expect horizontal and vertical dpis to be the same
    int dpi = static_cast<int>(dpi_float + 0.5f); // round to nearest int

    int default_dpi;
#if defined(__APPLE__)
    default_dpi = 72;
#elif defined(__linux__)
    default_dpi = 144;
#else
    default_dpi = 96; // Windows default is 96; I think the Linux default is also 96.
#endif

                      // use gcd, so the multipliers don't become too big
    int gcd = std::gcd(dpi, default_dpi);

    int tmp_physicalMultiplier = physicalMultiplier;
    int tmp_logicalMultiplier = logicalMultiplier;
    // update the fields
    physicalMultiplier = dpi / gcd;
    logicalMultiplier = default_dpi / gcd;

    // check if the fields changed
    bool fields_changed = tmp_physicalMultiplier != physicalMultiplier || tmp_logicalMultiplier != logicalMultiplier;

    if (fields_changed) {
        // tell the components to update their cached sizes
        playArea.updateDpi();
        toolbox.updateDpi();

        // remember to update my own pseudo-constants
        TOOLBOX_WIDTH = logicalToPhysicalSize(LOGICAL_TOOLBOX_WIDTH);
    }

    // return true if the fields changed
    return fields_changed;
}


void MainWindow::layoutComponents() {

    // update the two dpi member fields
    updateDpiFields();

    // get the size of the render target (this is a physical size)
    int pixelWidth, pixelHeight;
    SDL_GetRendererOutputSize(renderer, &pixelWidth, &pixelHeight);

    // position all the components:
    playArea.renderArea = SDL_Rect{0, 0, pixelWidth - TOOLBOX_WIDTH, pixelHeight};
    toolbox.renderArea = SDL_Rect{pixelWidth - TOOLBOX_WIDTH, 0, TOOLBOX_WIDTH, pixelHeight};

}


void MainWindow::start() {

    // Clear the window with a black background
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    // Show the window to the user
    SDL_ShowWindow(window);

    // event/drawing loop:
    while (true) {
        SDL_Event event;

        // get the next event to process, if any
        while(SDL_PollEvent(&event) != 0) {
            processEvent(event);
            if (closing) break;
        }

        // immediately break out of the loop if the user pressed the close button
        if (closing) break;

        // draw everything onto the screen
        render();
    }
}


void MainWindow::processEvent(const SDL_Event& event) {
    switch (event.type) {
    case SDL_WINDOWEVENT:
        processWindowEvent(event.window);
        break;
    case SDL_MOUSEMOTION:
        processMouseMotionEvent(event.motion);
        break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        processMouseButtonEvent(event.button);
        break;
    case SDL_MOUSEWHEEL:
        processMouseWheelEvent(event.wheel);
        break;
    case SDL_KEYDOWN:
        processKeyboardEvent(event.key);
    }
}


void MainWindow::processWindowEvent(const SDL_WindowEvent& event) {
    switch (event.event) {
    case SDL_WINDOWEVENT_CLOSE: // close button was pressed (or some other close command like Alt-F4)
        closing = true;
        break;
    case SDL_WINDOWEVENT_RESIZED: // window got resized by window manager or by user (will NOT be triggered by programmatic resize, e.g. SDL_SetWindowSize)
        layoutComponents();
        break;
    case SDL_WINDOWEVENT_LEAVE:
        playArea.processMouseLeave();
        toolbox.processMouseLeave();
        break;
    }
}


void MainWindow::processMouseMotionEvent(const SDL_MouseMotionEvent& event) {
    playArea.processMouseMotionEvent(event);
    toolbox.processMouseMotionEvent(event);
}


void MainWindow::processMouseButtonEvent(const SDL_MouseButtonEvent& event) {
    SDL_Point position{event.x, event.y};
    if (SDL_PointInRect(&position, &playArea.renderArea)) {
        playArea.processMouseButtonEvent(event);
    }
    else if (SDL_PointInRect(&position, &toolbox.renderArea)) {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            toolbox.processMouseButtonDownEvent(event);
        } else {
            // forward mouseup event to playArea
            playArea.processMouseButtonEvent(event);
        }
    }
}

void MainWindow::processMouseWheelEvent(const SDL_MouseWheelEvent& event) {
    SDL_Point position;
    // poll the mouse position since it's not reflected in the event
    SDL_GetMouseState(&position.x, &position.y);
    if (SDL_PointInRect(&position, &playArea.renderArea)) {
        playArea.processMouseWheelEvent(event);
    }
}


void MainWindow::processKeyboardEvent(const SDL_KeyboardEvent& event) {
    playArea.processKeyboardEvent(event);
}


// TODO: needs some way to use the old data when resizing, for consistency?
void MainWindow::render() {


    // Clear the window with a black background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, 255);
    SDL_RenderClear(renderer);

    // TODO: draw everything to the screen - buttons, status info, play area, etc.
    playArea.render(renderer);
    toolbox.render(renderer);

    // Then display to the user
    SDL_RenderPresent(renderer);
}
