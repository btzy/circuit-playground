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

#include <cstdint> // for int32_t and uint32_t
#include <string>

#include <boost/logic/tribool.hpp>

#include "declarations.hpp"
#include "canvasstate.hpp"
#include "simulator.hpp"
#include "historymanager.hpp"
#include "notificationdisplay.hpp"


/**
 * Manages the gamestate, rendering the gamestate, and invoking the simulator.
 */

class StateManager {
private:
    CanvasState defaultState; // stores a cache of the simulator state.  this is guaranteed to be updated if the simulator is not running.
    Simulator simulator; // stores the 'live' states and has methods to compile and run the simulation

    boost::tribool changed = false; // whether canvasstate changed since the last write to the undo stack
    ext::point deltaTrans{ 0, 0 }; // difference in viewport translation from previous gamestate (TODO: move this into a proper UndoDelta class)

    bool hasSelection = false; // whether selection/base contain meaningful data (neccessary to prevent overwriting defaultState)

    HistoryManager historyManager; // stores the undo/redo stack

    NotificationDisplay::UniqueNotification resetNotification;
    NotificationDisplay::UniqueNotification runningNotification;

    /**
     * Explicitly scans the current gamestate to determine if it changed. Updates 'changed'.
     * This should only be used if no faster alternative exists.
     */
    bool evaluateChangedState();

    /**
     * If a current simulator exists, stop and recompiles the simulator.
     * Note: this function is not used at all.
     */
    void reloadSimulator();

    friend class StatefulAction;
    friend class EditAction;
    friend class SaveableAction;
    friend class HistoryAction;
    friend class FileOpenAction;
    friend class FileSaveAction;
    friend class SelectionAction;
    friend class ChangeSimulationSpeedAction;
    friend class ClipboardAction;
    friend class UndoButton;
    friend class RedoButton;
    friend class ScreenInputAction;
    friend class FileCommunicatorSelectAction;

public:

    StateManager(Simulator::period_t);
    ~StateManager();

    /**
     * Draw a rectangle of elements onto a pixel buffer supplied by PlayArea.
     * Pixel format: pixel = R | (G << 8) | (B << 16)
     * useDefaultView: whether we want to render the default view (instead of live view)
     */
    void fillSurface(bool useDefaultView, uint32_t* pixelBuffer, uint32_t pixelFormat, const SDL_Rect& surfaceRect, int32_t pitch);

    /**
     * Take a snapshot of the gamestate and save it in the history.
     * Will check if the state is actually changed, before attempting to save.
     * Returns true if the state was really saved (i.e. something was actually modified)
     */
    bool saveToHistory();

    /**
     * Starts the simulator.
     * @pre simulator is stopped
     */
    void startSimulatorUnchecked();

    /**
     * Toggle between running and pausing the simulator.
     */
    void startOrStopSimulator(MainWindow& mainWindow);

    /**
     * Stops the simulator.
     * @pre simulator is running
     */
    void stopSimulatorUnchecked();

    /**
     * Steps the simulator.
     * @pre simulator is stopped
     */
    void stepSimulatorUnchecked();

    /**
     * Starts the simulator if it is currently stopped.
     */
    void startSimulator();

    /**
    * Stops the simulator if it is currently running.
    */
    void stopSimulator();

    /**
     * Steps the simulator if it is currently stopped.
     */
    void stepSimulator();

    /**
    * Whether the simulator is running.
    */
    bool simulatorRunning() const;

    /**
     * Reset all elements in the simulation to their default state.
     * This will work even if simulator is running.
     */
    void resetSimulator(MainWindow&);

    /**
     * Update defaultState with a new snapshot from the simulator.
     */
    void updateDefaultState();

    /**
     * Get the element at the given canvas point.
     * Returns std::monostate if the point is outside the canvas bounds.
     */
    CanvasState::element_variant_t getElementAtPoint(const ext::point& pt) const;
};
