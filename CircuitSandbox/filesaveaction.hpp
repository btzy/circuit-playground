#pragma once

#include <fstream>
#include <cstring>

#include <boost/endian/conversion.hpp>
#include <boost/process/spawn.hpp>
#include <SDL.h>
#include <nfd.h>

#include "visitor.hpp"
#include "action.hpp"
#include "playarea.hpp"
#include "canvasstate.hpp"
#include "fileutils.hpp"

class FileSaveAction final : public Action {
private:
    enum class WriteResult : char {
        OK,
        IO_ERROR // file cannot be opened or read
    };

    WriteResult writeSave(CanvasState& state, const char* filePath) {
        std::ofstream saveFile(filePath, std::ios::binary);
        if (!saveFile.is_open()) return WriteResult::IO_ERROR;

        // write the magic sequence
        saveFile.write(CCSB_FILE_MAGIC, 4);

        // write the version number
        int32_t version = 0;
        boost::endian::native_to_little_inplace(version);
        saveFile.write(reinterpret_cast<char*>(&version), sizeof version);

        // write the width and height
        int32_t matrixWidth = state.width();
        int32_t matrixHeight = state.height();
        boost::endian::native_to_little_inplace(matrixWidth);
        boost::endian::native_to_little_inplace(matrixHeight);
        saveFile.write(reinterpret_cast<char*>(&matrixWidth), sizeof matrixWidth);
        saveFile.write(reinterpret_cast<char*>(&matrixHeight), sizeof matrixHeight);

        // write the matrix
        for (int32_t y = 0; y != state.height(); ++y) {
            for (int32_t x = 0; x != state.width(); ++x) {
                CanvasState::element_variant_t element = state[{x, y}];
                size_t element_index = element.index();
                bool logicLevel = false;
                bool defaultLogicLevel = false;

                std::visit(visitor{
                    [](std::monostate) {},
                    [&](const auto& element) {
                        logicLevel = element.getLogicLevel();
                        defaultLogicLevel = element.getDefaultLogicLevel();
                    },
                }, element);

                uint8_t elementData = static_cast<uint8_t>((element_index << 2) + (logicLevel << 1) + defaultLogicLevel);
                saveFile.write(reinterpret_cast<char*>(&elementData), 1);
            }
        }

        return WriteResult::OK;
    }

public:
    FileSaveAction(MainWindow& mainWindow, const char* filePath = nullptr) {

        // stop the simulator if running
        bool simulatorRunning = mainWindow.stateManager.simulator.running();
        if (simulatorRunning) mainWindow.stateManager.simulator.stop();

        char* properPath = nullptr;
        char* outPath = nullptr;
        if (filePath == nullptr) {
            nfdresult_t result = NFD_SaveDialog(CCSB_FILE_EXTENSION, nullptr, &outPath);
            mainWindow.suppressMouseUntilNextDown();
            if (result == NFD_OKAY) {
                properPath = new char[std::strlen(outPath) + 6];
                filePath = addExtensionIfNecessary(outPath, properPath);
            }
        }

        if (filePath != nullptr) { // means that the user wants to save to filePath
            mainWindow.stateManager.updateDefaultState();
            WriteResult result = writeSave(mainWindow.stateManager.defaultState, filePath);
            switch (result) {
            case WriteResult::OK:
                mainWindow.setUnsaved(false);
                mainWindow.setFilePath(filePath);
                mainWindow.stateManager.historyManager.setSaved();
                break;
            case WriteResult::IO_ERROR:
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Cannot Save File", "This file cannot be written to.", mainWindow.window);
                break;
            }
        }

        // start the simulator if necessary
        if (simulatorRunning) mainWindow.stateManager.simulator.start();

        // free the memory
        if (outPath != nullptr) {
            free(outPath); // note: freeing memory that was acquired by NFD library
        }
        if (properPath != nullptr) {
            delete[] properPath;
        }
    };

    static inline void start(MainWindow& mainWindow, const SDL_Keymod& modifiers, const ActionStarter& starter, const char* filePath = nullptr) {
        if (modifiers & KMOD_SHIFT) {
            // force "Save As" dialog
            starter.start<FileSaveAction>(mainWindow, nullptr);
        }
        else if (mainWindow.stateManager.historyManager.changedSinceLastSave()) {
            starter.start<FileSaveAction>(mainWindow, mainWindow.getFilePath());
        }
    }
};