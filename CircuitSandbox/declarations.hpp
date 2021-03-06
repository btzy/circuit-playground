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
 * Stores all stuff that is global to the whole program, and are not associated with anything else.
 */

#include <SDL.h>

#include "tag_tuple.hpp"

// drawables
class MainWindow;
class Toolbox;
class PlayArea;
class ButtonBar;
class NotificationDisplay;

// all the available actions
class Action;
class PlayAreaAction;
template <typename> class PencilAction;
class SelectionAction;
class HistoryAction;
class FileNewAction;
class FileOpenAction;
class FileSaveAction;
class ScreenInputAction;
class FileCommunicatorSelectAction;

// all the tools and elements
struct Selector;
struct Panner;
struct Interactor;
struct Eraser;
struct ConductiveWire;
struct InsulatedWire;
struct Signal;
struct Source;
struct PositiveRelay;
struct NegativeRelay;
struct AndGate;
struct OrGate;
struct NandGate;
struct NorGate;
struct ScreenCommunicatorElement;
struct FileInputCommunicatorElement;
struct FileOutputCommunicatorElement;

// compile-time type tag which stores the list of available elements
using tool_tags_t = ext::tag_tuple<Selector, Panner, Interactor, Eraser, ConductiveWire, InsulatedWire, Signal, Source, PositiveRelay, NegativeRelay, AndGate, OrGate, NandGate, NorGate, ScreenCommunicatorElement, FileInputCommunicatorElement, FileOutputCommunicatorElement>;

// list of actions that have a static startWithPlayAreaMouseButtonDown(const SDL_MouseButtonEvent&, MainWindow&, PlayArea& const ActionStarter&);
using playarea_action_tags_t = ext::tag_tuple<SelectionAction, ScreenInputAction, FileCommunicatorSelectAction, PencilAction<Eraser>, PencilAction<ConductiveWire>, PencilAction<InsulatedWire>, PencilAction<Signal>, PencilAction<Source>, PencilAction<PositiveRelay>, PencilAction<NegativeRelay>, PencilAction<AndGate>, PencilAction<OrGate>, PencilAction<NandGate>, PencilAction<NorGate>, PencilAction<ScreenCommunicatorElement>, PencilAction<FileInputCommunicatorElement>, PencilAction<FileOutputCommunicatorElement>>;

// communicators
class ScreenCommunicator;
class FileInputCommunicator;
class FileOutputCommunicator;

// colors
constexpr inline SDL_Color RED          { 0xe6, 0x32, 0x32, 0xff };
constexpr inline SDL_Color YELLOW       { 0xe6, 0xe6, 0x2e, 0xff };
constexpr inline SDL_Color CYAN         { 0x32, 0xe6, 0xc8, 0xff };
constexpr inline SDL_Color INDIGO       { 0x32, 0x50, 0xe6, 0xff };
constexpr inline SDL_Color MAGENTA      { 0xc8, 0x32, 0xe6, 0xff };
constexpr inline SDL_Color DARK_GREEN   { 0x0A, 0x66, 0x44, 0xff };
constexpr inline SDL_Color ORANGE       { 0xe6, 0x8c, 0x32, 0xff };
constexpr inline SDL_Color GREEN        { 0x6e, 0xe6, 0x32, 0xff };
constexpr inline SDL_Color MAROON       { 0xa5, 0x0c, 0x0c, 0xff };
constexpr inline SDL_Color PURPLE       { 0x6e, 0x32, 0xe6, 0xff };
constexpr inline SDL_Color BLUE         { 0x32, 0xaa, 0xe6, 0xff };
constexpr inline SDL_Color WHITE        { 0xff, 0xff, 0xff, 0xff };
constexpr inline SDL_Color LIGHT_GREY   { 0x99, 0x99, 0x99, 0xff };
constexpr inline SDL_Color DARK_GREY    { 0x18, 0x18, 0x18, 0xff };
constexpr inline SDL_Color BLACK        { 0x00, 0x00, 0x00, 0xff };

// strings
// we use compiler #defines so we can have adjacent string concatenation
#define CIRCUIT_SANDBOX_STRING "Circuit Sandbox"
#define CIRCUIT_SANDBOX_VERSION_STRING "v0.4" // change this when there is a new update
#define WINDOW_TITLE_STRING CIRCUIT_SANDBOX_STRING " " CIRCUIT_SANDBOX_VERSION_STRING

/**
 * The number of distinct "input handles": 5 (SDL2 supports this many mouse buttons) + 1 (for touch input)
 */
constexpr static size_t NUM_INPUT_HANDLES = 6;

// number of clipboards (excluding the default clipboard)
constexpr static size_t NUM_CLIPBOARDS = 10;

/**
 * Resolve the mouse button event to the input handle index (used with 'selectedToolIndices');
 * Uses fields 'which' and 'button'
 */
inline size_t resolveInputHandleIndex(const SDL_MouseButtonEvent& event) {
    if (event.which == SDL_TOUCH_MOUSEID) {
        return 0;
    }
    else {
        switch (event.button) {
        case SDL_BUTTON_LEFT:
            return 1;
        case SDL_BUTTON_MIDDLE:
            return 2;
        case SDL_BUTTON_RIGHT:
            return 3;
        case SDL_BUTTON_X1:
            return 4;
        case SDL_BUTTON_X2:
            return 5;
        default:
#if defined(__GNUC__)
            __builtin_unreachable();
#elif defined(_MSC_VER)
            __assume(0);
#endif
        }
    }
}
