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

#include <variant>
#include <SDL.h>
#include "point.hpp"
#include "playarea.hpp"
#include "mainwindow.hpp"
#include "playareaaction.hpp"
#include "elements.hpp"
#include "screencommunicator.hpp"

class ScreenInputAction final : public PlayAreaAction {
private:
    ScreenCommunicator* communicator = nullptr;
    /*
     * Change the communicator currently being held down, and send that communicator the 'on' signal.
     * Will only change the receiver state if the current communicator is not the same as the new one.
     */
    void changeTarget(ScreenCommunicator* target) {
        if (communicator != target) {
            if (communicator != nullptr) {
                mainWindow.stateManager.simulator.sendCommunicatorEvent(communicator->communicatorIndex, false);
            }
            if (target != nullptr) {
                mainWindow.stateManager.simulator.sendCommunicatorEvent(target->communicatorIndex, true);
            }
            communicator = target;
        }
    }
    /*
     * Called when the mouse moves, to change the communicator depending on the type of element under the mouse, so that the new communicator can get the 'on' signal.
     */
    void changeMousePosition(const ext::point& canvasOffset) {
        if (canvas().contains(canvasOffset)) {
            // mouse is inside the play area
            std::visit([&](auto& element) {
                using ElementType = std::decay_t<decltype(element)>;
                if constexpr(std::is_same_v<ScreenCommunicatorElement, ElementType>) {
                    // there is a communicator under the mouse
                    changeTarget(element.communicator.get());
                }
                else {
                    // the thing under the mouse isn't a communicator
                    changeTarget(nullptr);
                }
            }, canvas()[canvasOffset]);
        }
        else {
            // mouse is out of the playarea, so we say that there is no communicator under the mouse
            changeTarget(nullptr);
        }
    }
    
public:

    ScreenInputAction(MainWindow& mainWindow, const ext::point& canvasOffset) :PlayAreaAction(mainWindow) {
        changeMousePosition(canvasOffset);
    }
    ~ScreenInputAction() override {
        changeTarget(nullptr);
    }

    // check if we need to start interaction by clicking the playarea
    static inline ActionEventResult startWithPlayAreaMouseButtonDown(const SDL_MouseButtonEvent& event, MainWindow& mainWindow, PlayArea& playArea, const ActionStarter& starter) {
        size_t inputHandleIndex = resolveInputHandleIndex(event);
        size_t currentToolIndex = mainWindow.selectedToolIndices[inputHandleIndex];

        return tool_tags_t::get(currentToolIndex, [&](const auto tool_tag) {
            // 'Tool' is the type of tool (e.g. Selector)
            using Tool = typename decltype(tool_tag)::type;

            if constexpr (std::is_base_of_v<Interactor, Tool>) {
                ext::point canvasOffset = playArea.canvasFromWindowOffset(event);
                if (mainWindow.stateManager.defaultState.contains(canvasOffset) &&
                    std::holds_alternative<ScreenCommunicatorElement>(mainWindow.stateManager.defaultState[canvasOffset])) {
                    // Only start the screen input action if the mouse was pressed down over a Screen Communicator.
                    starter.start<ScreenInputAction>(mainWindow, canvasOffset);
                    return ActionEventResult::PROCESSED;
                }
                else {
                    return ActionEventResult::UNPROCESSED;
                }
            }
            else {
                return ActionEventResult::UNPROCESSED;
            }
        }, ActionEventResult::UNPROCESSED);
    }

    ActionEventResult processPlayAreaMouseDrag(const SDL_MouseMotionEvent& event) override {
        // if mouse is outside, unset the communicator
        if (!ext::point_in_rect(event, playArea().renderArea)) {
            changeTarget(nullptr);
            return ActionEventResult::PROCESSED;
        }

        // compute the canvasOffset here
        ext::point canvasOffset = playArea().canvasFromWindowOffset(event);

        // report the change in mouse position, so that the underlying communicator can receive the 'on' signal
        changeMousePosition(canvasOffset);

        return ActionEventResult::PROCESSED;
    }

    ActionEventResult processPlayAreaMouseButtonUp() override {
        // we are done
        return ActionEventResult::COMPLETED;
    }
};
