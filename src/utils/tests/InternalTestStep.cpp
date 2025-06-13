/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
// Copyright (C) 2001-2025 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    InternalTestStep.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Mar 2025
///
// Single operation used in InternalTests
/****************************************************************************/
#include <config.h>

#include <fxkeys.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>

#include "InternalTestStep.h"
#include "InternalTest.h"

// this offsets corresponds to the offset of the test magenta square
constexpr int MOUSE_OFFSET_X = 24;
constexpr int MOUSE_OFFSET_Y = 25;
constexpr int MOUSE_REFERENCE_X = 304;
constexpr int MOUSE_REFERENCE_Y = 168;

// ===========================================================================
// member method definitions
// ===========================================================================

InternalTestStep::InternalTestStep(InternalTest* testSystem, const std::string& step) :
    myTestSystem(testSystem) {
    // add this testStep to test system
    testSystem->myTestSteps.push_back(this);
    // first parse step
    const auto function = parseStep(step);
    // continue depending of function
    if (function == "setupAndStart") {
        processSetupAndStartFunction();
    } else if (function == "leftClick") {
        processLeftClickFunction();
    } else if (function == "modifyAttribute") {
        processModifyAttributeFunction();
    } else if (function == "modifyAttributeOverlapped") {
        processModifyAttributeOverlappedFunction();
    } else if (function == "modifyBoolAttribute") {
        processModifyBoolAttributeFunction();
    } else if (function == "modifyBoolAttributeOverlapped") {
        processModifyBoolAttributeOverlappedFunction();
    } else if (function == "modifyColorAttribute") {
        processModifyColorAttributeFunction();
    } else if (function == "modifyColorAttributeOverlapped") {
        processModifyColorAttributeOverlappedFunction();
    } else if (function == "supermode") {
        processSupermodeFunction();
    } else if (function == "changeMode") {
        processChangeModeFunction();
    } else if (function == "changeElement") {
        processChangeElementArgument();
    } else if (function == "compute") {
        processComputeFunction();
    } else if (function == "saveExistentShortcut") {
        processSaveExistentShortcutFunction();
    } else if (function == "checkUndoRedo") {
        processCheckUndoRedoFunction();
    } else if (function == "delete") {
        processDeleteFunction();
    } else if (function == "selection") {
        processSelectionFunction();
    } else if (function == "undo") {
        processUndoFunction();
    } else if (function == "redo") {
        processRedoFunction();
    } else if (function == "quit") {
        processQuitFunction();
    } else if (function.size() > 0) {
        std::cout << function << std::endl;
        throw ProcessError("Function " + function + " not implemented in InternalTestStep");
    }
}


InternalTestStep::InternalTestStep(InternalTest* testSystem, FXSelector messageType,
                                   FXSelector messageID, Category category) :
    myTestSystem(testSystem),
    myMessageType(messageType),
    myMessageID(messageID),
    myCategory(category) {
    // add this testStep to test system
    testSystem->myTestSteps.push_back(this);
}


InternalTestStep::InternalTestStep(InternalTest* testSystem, FXSelector messageType,
                                   Category category, FXEvent* event, const bool updateView) :
    myTestSystem(testSystem),
    myMessageType(messageType),
    myCategory(category),
    myUpdateView(updateView),
    myEvent(event) {
    // add this testStep to test system
    testSystem->myTestSteps.push_back(this);
}


InternalTestStep::InternalTestStep(InternalTestStep* parent, FXSelector messageType, FXEvent* event) :
    myTestSystem(parent->myTestSystem),
    myMessageType(messageType),
    myEvent(event) {
    // add this testStep to parent key steps
    parent->myKeySteps.push_back(this);
}


InternalTestStep::~InternalTestStep() {
    if (myEvent) {
        delete myEvent;
    }
    // remove all key steps
    for (auto keyStep : myKeySteps) {
        delete keyStep;
    }
    myKeySteps.clear();
}


FXSelector
InternalTestStep::getMessageType() const {
    return myMessageType;
}


FXSelector
InternalTestStep::getMessageID() const {
    return myMessageID;
}


FXSelector
InternalTestStep::getSelector() const {
    return FXSEL(myMessageType, myMessageID);
}


bool
InternalTestStep::updateView() const {
    return myUpdateView;
}


InternalTestStep::Category
InternalTestStep::getCategory() const {
    return myCategory;
}


void*
InternalTestStep::getEvent() const {
    return myEvent;
}


const std::vector<const InternalTestStep*>&
InternalTestStep::getKeySteps() const {
    return myKeySteps;
}


FXEvent*
InternalTestStep::buildMouseMoveEvent(const int posX, const int posY) const {
    FXEvent* moveEvent = new FXEvent();
    // common values
    moveEvent->synthetic = true;
    // set event values
    moveEvent->type = SEL_MOTION;
    moveEvent->win_x = posX + MOUSE_OFFSET_X;
    moveEvent->win_y = posY + MOUSE_OFFSET_Y;
    moveEvent->moved = true;
    moveEvent->rect = FXRectangle(0, 0, 0, 0);
    return moveEvent;
}


FXEvent*
InternalTestStep::buildMouseLeftClickPressEvent(const int posX, const int posY) const {
    FXEvent* leftClickPressEvent = new FXEvent();
    // common values
    leftClickPressEvent->synthetic = true;
    // set event values
    leftClickPressEvent->win_x = posX + MOUSE_OFFSET_X;
    leftClickPressEvent->win_y = posY + MOUSE_OFFSET_Y;
    leftClickPressEvent->click_x = posX + MOUSE_OFFSET_X;
    leftClickPressEvent->click_y = posY + MOUSE_OFFSET_Y;
    leftClickPressEvent->type = SEL_LEFTBUTTONPRESS;
    leftClickPressEvent->state = 256;
    leftClickPressEvent->code = 1;
    leftClickPressEvent->click_button = 1;
    leftClickPressEvent->click_count = 1;
    leftClickPressEvent->moved = false;
    return leftClickPressEvent;
}


FXEvent*
InternalTestStep::buildMouseLeftClickReleaseEvent(const int posX, const int posY) const {
    FXEvent* leftClickReleaseEvent = new FXEvent();
    // common values
    leftClickReleaseEvent->synthetic = true;
    // set event values
    leftClickReleaseEvent->win_x = posX + MOUSE_OFFSET_X;
    leftClickReleaseEvent->win_y = posY + MOUSE_OFFSET_Y;
    leftClickReleaseEvent->click_x = posX + MOUSE_OFFSET_X;
    leftClickReleaseEvent->click_y = posY + MOUSE_OFFSET_Y;
    leftClickReleaseEvent->type = SEL_LEFTBUTTONRELEASE;
    leftClickReleaseEvent->state = 256;
    leftClickReleaseEvent->code = 1;
    leftClickReleaseEvent->click_button = 1;
    leftClickReleaseEvent->click_count = 1;
    leftClickReleaseEvent->moved = false;
    return leftClickReleaseEvent;
}


FXEvent*
InternalTestStep::buildKeyPressEvent(const std::string& key) const {
    const auto keyValues = translateKey(key);
    FXEvent* keyPressEvent = new FXEvent();
    // set event values
    keyPressEvent->type = SEL_KEYPRESS;
    keyPressEvent->code = keyValues.first;
    keyPressEvent->text = keyValues.second;
    return keyPressEvent;
}


FXEvent*
InternalTestStep::buildKeyPressEvent(const char key) const {
    FXEvent* keyPressEvent = new FXEvent();
    // set event values
    keyPressEvent->type = SEL_KEYPRESS;
    keyPressEvent->code = (FXint)key;
    keyPressEvent->text.append(key);
    return keyPressEvent;
}


FXEvent*
InternalTestStep::buildKeyReleaseEvent(const std::string& key) const {
    const auto keyValues = translateKey(key);
    FXEvent* keyReleaseEvent = new FXEvent();
    // set event values
    keyReleaseEvent->type = SEL_KEYRELEASE;
    keyReleaseEvent->code = keyValues.first;
    keyReleaseEvent->text = keyValues.second;
    return keyReleaseEvent;
}


FXEvent*
InternalTestStep::buildKeyReleaseEvent(const char key) const {
    FXEvent* keyReleaseEvent = new FXEvent();
    // set event values
    keyReleaseEvent->type = SEL_KEYRELEASE;
    keyReleaseEvent->code = (FXint)key;
    keyReleaseEvent->text.append(key);
    return keyReleaseEvent;
}


std::string
InternalTestStep::parseStep(const std::string& rowText) {
    // first check if this is the netedit.setupAndStart function
    if (rowText.find("netedit.setupAndStart") != std::string::npos) {
        return "setupAndStart";
    } else if (rowText.compare(0, 8, "netedit.") != 0) {
        // proces only lines that start with "netedit."
        return "";
    } else {
        std::string functionName;
        // make a copy to help editing row
        std::string rowStr = rowText;
        // every function has the format <function>(<argument1>, <argument2>,....,)
        if (rowText.empty() || (rowText.front() == '(') || (rowText.back() != ')')) {
            writeError("parseStep", "function(arguments)");
            return "";
        }
        // first extract function
        while (rowStr.size() > 0) {
            if (rowStr.front() == '(') {
                break;
            } else {
                functionName.push_back(rowStr.front());
                rowStr.erase(rowStr.begin());
            }
        }
        // remove prefix "netedit." (size 8) from function
        functionName = functionName.substr(8);
        // check if there are at least two characters (to avoid cases like 'function)')
        if (rowStr.size() < 2) {
            writeError("parseStep", "function(arguments)");
            return functionName;
        }
        // remove both pharentesis
        rowStr.erase(rowStr.begin());
        rowStr.pop_back();
        // now parse arguments
        parseArguments(rowStr);
        // remove "netedit." from frunction
        return functionName;
    }
}


void
InternalTestStep::parseArguments(const std::string& arguments) {
    std::string current;
    bool inQuotes = false;
    for (size_t i = 0; i < arguments.length(); ++i) {
        char c = arguments[i];
        if (c == '\"' || c == '\'') {
            // Toggle quote state
            inQuotes = !inQuotes;
            current.push_back(c);
        } else if (c == ',' && !inQuotes) {
            // End of argument
            if (!current.empty()) {
                // Trim leading/trailing whitespace
                size_t start = current.find_first_not_of(" \t");
                size_t end = current.find_last_not_of(" \t");
                myArguments.push_back(current.substr(start, end - start + 1));
                current.clear();
            }
        } else {
            current += c;
        }
    }
    // Add the last argument
    if (!current.empty()) {
        size_t start = current.find_first_not_of(" \t");
        size_t end = current.find_last_not_of(" \t");
        myArguments.push_back(current.substr(start, end - start + 1));
    }
    // inQuotes MUST be false, in other case we have a case like < "argument1", argument2, "argument3 >
    if (inQuotes) {
        writeError("parseArguments", "<\"argument\", \"argument\">");
        myArguments.clear();
    }
}


std::pair<FXint, FXString>
InternalTestStep::translateKey(const std::string& key) const {
    std::pair<FXint, FXString> solution;
    // continue depending of key
    if (key == "backspace") {
        solution.first = KEY_BackSpace;
        solution.second = "\b";
    } else if (key == "space") {
        solution.first = KEY_space;
    } else if (key == "tab") {
        solution.first = KEY_Tab;
        solution.second = "\t";
    } else if (key == "clear") {
        solution.first = KEY_Clear;
    } else if (key == "enter" || key == "return") {
        solution.first = KEY_Return;
        solution.second = "\n";
    } else if (key == "pause") {
        solution.first = KEY_Pause;
    } else if (key == "sys_req") {
        solution.first = KEY_Sys_Req;
    } else if (key == "esc" || key == "escape") {
        solution.first = KEY_Escape;
        solution.second = "\x1B";
    } else if (key == "delete") {
        solution.first = KEY_Delete;
        solution.second = "\x7F";
    } else if (key == "multi_key") {
        solution.first = KEY_Multi_key;
        // function
    } else if (key == "shift") {
        solution.first = KEY_Shift_L;
    } else if (key == "control") {
        solution.first = KEY_Control_L;
        // Cursor
    } else if (key == "home") {
        solution.first = KEY_Home;
    } else if (key == "left") {
        solution.first = KEY_Left;
    } else if (key == "up") {
        solution.first = KEY_Up;
    } else if (key == "right") {
        solution.first = KEY_Right;
    } else if (key == "down") {
        solution.first = KEY_Down;
    } else if (key == "prior" || key == "page_up") {
        solution.first = KEY_Page_Up;
    } else if (key == "next" || key == "page_down") {
        solution.first = KEY_Page_Down;
    } else if (key == "end") {
        solution.first = KEY_End;
    } else if (key == "begin") {
        solution.first = KEY_Begin;
        // Function keys
    } else if (key == "f1") {
        solution.first = KEY_F1;
    } else if (key == "f2") {
        solution.first = KEY_F2;
    } else if (key == "f3") {
        solution.first = KEY_F3;
    } else if (key == "f4") {
        solution.first = KEY_F4;
    } else if (key == "f5") {
        solution.first = KEY_F5;
    } else if (key == "f6") {
        solution.first = KEY_F6;
    } else if (key == "f7") {
        solution.first = KEY_F7;
    } else if (key == "f8") {
        solution.first = KEY_F8;
    } else if (key == "f9") {
        solution.first = KEY_F9;
    } else if (key == "f10") {
        solution.first = KEY_F10;
    } else if (key == "f11" || key == "l1") {
        solution.first = KEY_F11;
    } else if (key == "f12" || key == "l2") {
        solution.first = KEY_F12;
    } else {
        writeError("translateKey", "<key>");
        solution.first = KEY_VoidSymbol;
    }
    return solution;
}


void
InternalTestStep::processSetupAndStartFunction() {
    myCategory = Category::INIT;
    // print in console the following lines
    std::cout << "TestFunctions: Netedit opened successfully" << std::endl;
    std::cout << "Finding reference" << std::endl;
    std::cout << "TestFunctions: 'reference.png' found. Position: " <<
              toString(MOUSE_REFERENCE_X) << " - " <<
              toString(MOUSE_REFERENCE_Y) << std::endl;
}


void
InternalTestStep::processLeftClickFunction() const {
    if ((myArguments.size() != 2) || (myTestSystem->myViewPositions.count(myArguments[1]) == 0)) {
        writeError("leftClick", "<reference, position>");
    } else {
        // parse arguments
        const int posX = myTestSystem->myViewPositions.at(myArguments[1]).first;
        const int posY = myTestSystem->myViewPositions.at(myArguments[1]).second;
        // print info
        std::cout << "TestFunctions: Clicked over position " <<
                  toString(posX + MOUSE_REFERENCE_X) << " - " <<
                  toString(posY + MOUSE_REFERENCE_Y) << std::endl;
        // add move, left button press and left button release
        new InternalTestStep(myTestSystem, SEL_MOTION, Category::VIEW, buildMouseMoveEvent(posX, posY), true);
        new InternalTestStep(myTestSystem, SEL_LEFTBUTTONPRESS, Category::VIEW, buildMouseLeftClickPressEvent(posX, posY), true);
        new InternalTestStep(myTestSystem, SEL_LEFTBUTTONRELEASE, Category::VIEW, buildMouseLeftClickReleaseEvent(posX, posY), true);
    }
}


void
InternalTestStep::processModifyAttributeFunction() const {
    if ((myArguments.size() != 2) ||
            !checkIntArgument(myArguments[0], myTestSystem->myAttributesEnum) ||
            !checkStringArgument(myArguments[1])) {
        writeError("modifyAttribute", "<int/attributeEnum, \"string\">");
    } else {
        const int numTabs = getIntArgument(myArguments[0], myTestSystem->myAttributesEnum);
        const std::string value = getStringArgument(myArguments[1]);
        // print info
        std::cout << value << std::endl;
        // focus frame
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, Category::APP);
        // jump to the element
        for (int i = 0; i < numTabs; i++) {
            buildPressKeyEvent("tab", false);
        }
        // write attribute character by character
        for (const char c : value) {
            buildPressKeyEvent(c, false);
        }
        // press enter to confirm changes (updating view)
        buildPressKeyEvent("enter", true);
    }
}


void
InternalTestStep::processModifyAttributeOverlappedFunction() const {
    if ((myArguments.size() != 2) ||
            !checkIntArgument(myArguments[0], myTestSystem->myAttributesEnum) ||
            !checkStringArgument(myArguments[1])) {
        writeError("modifyAttributeOverlapped", "<int/attributeEnum, \"string\">");
    } else {
        const int numTabs = getIntArgument(myArguments[0], myTestSystem->myAttributesEnum);
        const std::string value = getStringArgument(myArguments[1]);
        const int overlappedTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.editElements.overlapped");
        // print info
        std::cout << value << std::endl;
        // focus frame
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, Category::APP);
        // jump to the element
        for (int i = 0; i < (numTabs + overlappedTabs); i++) {
            buildPressKeyEvent("tab", false);
        }
        // write attribute character by character
        for (const char c : value) {
            buildPressKeyEvent(c, false);
        }
        // press enter to confirm changes (updating view)
        buildPressKeyEvent("enter", true);
    }
}


void
InternalTestStep::processModifyBoolAttributeFunction() const {
    if ((myArguments.size() != 1) ||
            !checkIntArgument(myArguments[0], myTestSystem->myAttributesEnum)) {
        writeError("modifyBoolAttribute", "<int/attributeEnum>");
    } else {
        const int numTabs = getIntArgument(myArguments[0], myTestSystem->myAttributesEnum);
        // focus frame
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, Category::APP);
        // jump to the element
        for (int i = 0; i < numTabs; i++) {
            buildPressKeyEvent("tab", false);
        }
        // toogle attribute
        buildPressKeyEvent("space", true);
    }
}


void
InternalTestStep::processModifyBoolAttributeOverlappedFunction() const {
    if ((myArguments.size() != 1) ||
            !checkIntArgument(myArguments[0], myTestSystem->myAttributesEnum)) {
        writeError("modifyBoolAttributeOverlapped", "<int/attributeEnum>");
    } else {
        const int numTabs = getIntArgument(myArguments[0], myTestSystem->myAttributesEnum);
        const int overlappedTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.editElements.overlapped");
        // focus frame
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, Category::APP);
        // jump to the element
        for (int i = 0; i < (numTabs + overlappedTabs); i++) {
            buildPressKeyEvent("tab", false);
        }
        // toogle attribute
        buildPressKeyEvent("space", true);
    }
}


void
InternalTestStep::processModifyColorAttributeFunction() const {
    if ((myArguments.size() != 2) ||
            !checkIntArgument(myArguments[0], myTestSystem->myAttributesEnum) ||
            !checkIntArgument(myArguments[1], myTestSystem->myAttributesEnum)) {
        writeError("processModifyColorAttributeFunction", "<int/attributeEnum, int>");
    } else {
        const int numTabs = getIntArgument(myArguments[0], myTestSystem->myAttributesEnum);
        const int colorIndex = getIntArgument(myArguments[1], myTestSystem->myAttributesEnum);
        // focus frame
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, Category::APP);
        // jump to the element
        for (int i = 0; i < numTabs; i++) {
            buildPressKeyEvent("tab", false);
        }
        // open dialog
        auto spaceEvent = buildPressKeyEvent("space", false);
        // go to the list of colors
        for (int i = 0; i < 2; i++) {
            buildTwoPressKeyEvent(spaceEvent, "shift", "tab");
        }
        // select color
        for (int i = 0; i < (1 + colorIndex); i++) {
            buildPressKeyEvent(spaceEvent, "down");
        }
        // go to button
        buildPressKeyEvent(spaceEvent, "tab");
        // press button
        buildPressKeyEvent(spaceEvent, "space");
    }
}


void
InternalTestStep::processModifyColorAttributeOverlappedFunction() const {
    if ((myArguments.size() != 2) ||
            !checkIntArgument(myArguments[0], myTestSystem->myAttributesEnum) ||
            !checkIntArgument(myArguments[1], myTestSystem->myAttributesEnum)) {
        writeError("processModifyColorAttributeOverlappedFunction", "<int/attributeEnum, int>");
    } else {
        const int numTabs = getIntArgument(myArguments[0], myTestSystem->myAttributesEnum);
        const int colorIndex = getIntArgument(myArguments[1], myTestSystem->myAttributesEnum);
        const int overlappedTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.editElements.overlapped");
        // focus frame
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, Category::APP);
        // jump to the element
        for (int i = 0; i < (numTabs + overlappedTabs); i++) {
            buildPressKeyEvent("tab", false);
        }
        // open dialog
        auto spaceEvent = buildPressKeyEvent("space", false);
        // go to the list of colors
        for (int i = 0; i < 2; i++) {
            buildTwoPressKeyEvent(spaceEvent, "shift", "tab");
        }
        // select color
        for (int i = 0; i < (1 + colorIndex); i++) {
            buildPressKeyEvent(spaceEvent, "down");
        }
        // go to button
        buildPressKeyEvent(spaceEvent, "tab");
        // press button
        //buildPressKeyEvent(spaceEvent, "space");
    }
}


void
InternalTestStep::processSaveExistentShortcutFunction() {
    if ((myArguments.size() != 1) ||
            !checkStringArgument(myArguments[0])) {
        writeError("save", "<\"string\">");
    } else {
        myCategory = Category::APP;
        const auto savingType = getStringArgument(myArguments[0]);
        if (savingType == "neteditConfig") {
            myMessageID = MID_HOTKEY_CTRL_SHIFT_E_SAVENETEDITCONFIG;
        } else {
            writeError("save", "<neteditConfig>");
        }
    }
}


void
InternalTestStep::processCheckUndoRedoFunction() const {
    if (myArguments.size() != 1) {
        writeError("checkUndoRedo", "<referencePosition>");
    } else {
        const int numUndoRedos = 9;
        // focus frame
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, Category::APP);
        // go to inspect mode
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_I_MODE_INSPECT, Category::APP);
        // click over reference
        std::cout << "TestFunctions: Clicked over position " <<
                  toString(MOUSE_REFERENCE_X) << " - " <<
                  toString(MOUSE_REFERENCE_Y) << std::endl;
        // add move, left button press and left button release
        new InternalTestStep(myTestSystem, SEL_MOTION, Category::VIEW, buildMouseMoveEvent(0, 0), true);
        new InternalTestStep(myTestSystem, SEL_LEFTBUTTONPRESS, Category::VIEW, buildMouseLeftClickPressEvent(0, 0), true);
        new InternalTestStep(myTestSystem, SEL_LEFTBUTTONRELEASE, Category::VIEW, buildMouseLeftClickReleaseEvent(0, 0), true);
        // undo
        for (int i = 0; i < numUndoRedos; i++) {
            new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_CTRL_Z_UNDO, Category::APP);
        }
        // focus frame
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, Category::APP);
        // go to inspect mode
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_I_MODE_INSPECT, Category::APP);
        // click over reference
        std::cout << "TestFunctions: Clicked over position " <<
                  toString(MOUSE_REFERENCE_X) << " - " <<
                  toString(MOUSE_REFERENCE_Y) << std::endl;
        // add move, left button press and left button release
        new InternalTestStep(myTestSystem, SEL_MOTION, Category::VIEW, buildMouseMoveEvent(0, 0), true);
        new InternalTestStep(myTestSystem, SEL_LEFTBUTTONPRESS, Category::VIEW, buildMouseLeftClickPressEvent(0, 0), true);
        new InternalTestStep(myTestSystem, SEL_LEFTBUTTONRELEASE, Category::VIEW, buildMouseLeftClickReleaseEvent(0, 0), true);
        // undo
        for (int i = 0; i < numUndoRedos; i++) {
            new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_CTRL_Y_REDO, Category::APP);
        }
    }
}


void
InternalTestStep::processDeleteFunction() const {
    if (myArguments.size() != 0) {
        writeError("delete", "<>");
    } else {
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_DEL, Category::APP);
    }
}


void
InternalTestStep::processSelectionFunction() const {
    if (myArguments.size() != 1 || !checkStringArgument(myArguments[0])) {
        writeError("selection", "<selection operation>");
    } else {
        const std::string selectionType = getStringArgument(myArguments[0]);
        // get number of tabls
        int numTabs = 0;
        if (selectionType == "default") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.selection.default");
        } else if (selectionType == "save") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.selection.save");
        } else if (selectionType == "load") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.selection.load");
        } else if (selectionType == "add") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.selection.add");
        } else if (selectionType == "remove") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.selection.remove");
        } else if (selectionType == "keep") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.selection.keep");
        } else if (selectionType == "replace") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.selection.replace");
        } else if (selectionType == "clear") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.selection.clear");
        } else if (selectionType == "invert") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.selection.invert");
        } else if (selectionType == "invertData") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.selection.invertData");
        } else if (selectionType == "delete") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.selection.delete");
        }
        // focus frame
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, Category::APP);
        // jump to the element
        for (int i = 0; i < numTabs; i++) {
            buildPressKeyEvent("tab", false);
        }
        if (selectionType == "save") {
            buildPressKeyEvent("enter", false);
            // complete
        } else if (selectionType == "load") {
            buildPressKeyEvent("enter", false);
            // complete
        } else {
            buildPressKeyEvent("space", true);
        }
    }
}


void
InternalTestStep::processUndoFunction() const {
    if ((myArguments.size() != 2) ||
            !checkStringArgument(myArguments[0])) {
        writeError("undo", "<referencePosition, int>");
    } else {
        const int numUndoRedos = getIntArgument(myArguments[1], myTestSystem->myAttributesEnum);
        // focus frame
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, Category::APP);
        // go to inspect mode
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_I_MODE_INSPECT, Category::APP);
        // click over reference
        std::cout << "TestFunctions: Clicked over position " <<
                  toString(MOUSE_REFERENCE_X) << " - " <<
                  toString(MOUSE_REFERENCE_Y) << std::endl;
        // add move, left button press and left button release
        new InternalTestStep(myTestSystem, SEL_MOTION, Category::VIEW, buildMouseMoveEvent(0, 0), true);
        new InternalTestStep(myTestSystem, SEL_LEFTBUTTONPRESS, Category::VIEW, buildMouseLeftClickPressEvent(0, 0), true);
        new InternalTestStep(myTestSystem, SEL_LEFTBUTTONRELEASE, Category::VIEW, buildMouseLeftClickReleaseEvent(0, 0), true);
        // undo
        for (int i = 0; i < numUndoRedos; i++) {
            new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_CTRL_Z_UNDO, Category::APP);
        }
    }
}


void
InternalTestStep::processRedoFunction() const {
    if ((myArguments.size() != 2) ||
            !checkStringArgument(myArguments[0])) {
        writeError("redo", "<referencePosition, int>");
    } else {
        const int numUndoRedos = getIntArgument(myArguments[1], myTestSystem->myAttributesEnum);
        // focus frame
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, Category::APP);
        // go to inspect mode
        new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_I_MODE_INSPECT, Category::APP);
        // click over reference
        std::cout << "TestFunctions: Clicked over position " <<
                  toString(MOUSE_REFERENCE_X) << " - " <<
                  toString(MOUSE_REFERENCE_Y) << std::endl;
        // add move, left button press and left button release
        new InternalTestStep(myTestSystem, SEL_MOTION, Category::VIEW, buildMouseMoveEvent(0, 0), true);
        new InternalTestStep(myTestSystem, SEL_LEFTBUTTONPRESS, Category::VIEW, buildMouseLeftClickPressEvent(0, 0), true);
        new InternalTestStep(myTestSystem, SEL_LEFTBUTTONRELEASE, Category::VIEW, buildMouseLeftClickReleaseEvent(0, 0), true);
        // undo
        for (int i = 0; i < numUndoRedos; i++) {
            new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_CTRL_Y_REDO, Category::APP);
        }
    }
}


void
InternalTestStep::processSupermodeFunction() {
    if ((myArguments.size() != 1) ||
            !checkStringArgument(myArguments[0])) {
        writeError("supermode", "<\"string\">");
    } else {
        myCategory = Category::APP;
        const std::string supermode = getStringArgument(myArguments[0]);
        if (supermode == "network") {
            myMessageID = MID_HOTKEY_F2_SUPERMODE_NETWORK;
        } else if (supermode == "demand") {
            myMessageID = MID_HOTKEY_F3_SUPERMODE_DEMAND;
        } else if (supermode == "data") {
            myMessageID = MID_HOTKEY_F4_SUPERMODE_DATA;
        } else {
            writeError("supermode", "<network/demand/data>");
        }
    }
}


void
InternalTestStep::processChangeModeFunction() {
    if ((myArguments.size() != 1) ||
            !checkStringArgument(myArguments[0])) {
        writeError("changeMode", "<\"string\">");
    } else {
        myCategory = Category::APP;
        const std::string networkMode = getStringArgument(myArguments[0]);
        if (networkMode == "inspect") {
            myMessageID = MID_HOTKEY_I_MODE_INSPECT;
        } else if (networkMode == "delete") {
            myMessageID = MID_HOTKEY_D_MODE_SINGLESIMULATIONSTEP_DELETE;
        } else if (networkMode == "select") {
            myMessageID = MID_HOTKEY_S_MODE_STOPSIMULATION_SELECT;
        } else if (networkMode == "move") {
            myMessageID = MID_HOTKEY_M_MODE_MOVE_MEANDATA;
        } else if (networkMode == "edge") {
            myMessageID = MID_HOTKEY_E_MODE_EDGE_EDGEDATA;
        } else if (networkMode == "trafficLight") {
            myMessageID = MID_HOTKEY_T_MODE_TLS_TYPE;
        } else if (networkMode == "connection") {
            myMessageID = MID_HOTKEY_C_MODE_CONNECT_CONTAINER;
        } else if (networkMode == "prohibition") {
            myMessageID = MID_HOTKEY_H_MODE_PROHIBITION_CONTAINERPLAN;
        } else if (networkMode == "crossing") {
            myMessageID = MID_HOTKEY_R_MODE_CROSSING_ROUTE_EDGERELDATA;
        } else if (networkMode == "additional") {
            myMessageID = MID_HOTKEY_A_MODE_STARTSIMULATION_ADDITIONALS_STOPS;
        } else if (networkMode == "wire") {
            myMessageID = MID_HOTKEY_W_MODE_WIRE_ROUTEDISTRIBUTION;
        } else if (networkMode == "taz") {
            myMessageID = MID_HOTKEY_Z_MODE_TAZ_TAZREL;
        } else if (networkMode == "shape") {
            myMessageID = MID_HOTKEY_P_MODE_POLYGON_PERSON;
        } else if (networkMode == "decal") {
            myMessageID = MID_HOTKEY_U_MODE_DECAL_TYPEDISTRIBUTION;
        } else {
            writeError("changeMode", "<inspect/delete/select/move...>");
        }
    }
}


void
InternalTestStep::processChangeElementArgument() const {
    if ((myArguments.size() != 2) ||
            !checkStringArgument(myArguments[0])) {
        writeError("selectAdditional", "<\"frame\", \"string\">");
    } else {
        const std::string frame = getStringArgument(myArguments[0]);
        const std::string element = getStringArgument(myArguments[1]);
        int numTabs = -1;
        // continue depending of frame
        if (frame == "additionalFrame") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.changeElement.additional");
        } else if (frame == "shapeFrame") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.changeElement.shape");
        } else if (frame == "vehicleFrame") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.changeElement.vehicle");
        } else if (frame == "routeFrame") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.changeElement.route");
        } else if (frame == "personFrame") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.changeElement.person");
        } else if (frame == "containerFrame") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.netedit.attrs.frames.changeElement.container");
        } else if (frame == "personPlanFrame") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.changeElement.personPlan");
        } else if (frame == "containerPlanFrame") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.netedit.attrs.frames.changeElement.containerPlan");
        } else if (frame == "stopFrameFrame") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.changeElement.stop");
        } else if (frame == "meanDataFrame") {
            numTabs = myTestSystem->myAttributesEnum.at("netedit.attrs.frames.changeElement.meanData");
        } else {
            WRITE_ERRORF("Invalid frame '%' used in function processChangeElementArgument", frame);
        }
        if (numTabs >= 0) {
            // show info
            std::cout << element << std::endl;
            // focus frame
            new InternalTestStep(myTestSystem, SEL_COMMAND, MID_HOTKEY_SHIFT_F12_FOCUSUPPERELEMENT, Category::APP);
            // jump to select additional argument
            for (int i = 0; i < numTabs; i++) {
                buildPressKeyEvent("tab", false);
            }
            // write additional character by character
            for (const char c : element) {
                buildPressKeyEvent(c, false);
            }
            // press enter to confirm changes (updating view)
            buildPressKeyEvent("enter", true);
        }
    }
}


void
InternalTestStep::processComputeFunction() {
    if (myArguments.size() > 0) {
        writeError("compute", "<>");
    } else {
        myCategory = Category::APP;
        myMessageID = MID_HOTKEY_F5_COMPUTE_NETWORK_DEMAND;
    }
}


void
InternalTestStep::processQuitFunction() {
    if (myArguments.size() == 0) {
        writeError("quit", "<neteditProcess>");
    } else {
        myCategory = Category::APP;
        myMessageID = MID_HOTKEY_CTRL_Q_CLOSE;
        //don't update view if we're closing to avoid problems with drawGL
        myUpdateView = false;
    }
}


bool
InternalTestStep::checkIntArgument(const std::string& argument, const std::map<std::string, int>& map) const {
    if (StringUtils::isInt(argument)) {
        return true;
    } else if (map.count(argument) > 0) {
        return true;
    } else {
        return false;
    }
}


int
InternalTestStep::getIntArgument(const std::string& argument, const std::map<std::string, int>& map) const {
    if (StringUtils::isInt(argument)) {
        return StringUtils::toInt(argument);
    } else {
        return map.at(argument);
    }
}


bool
InternalTestStep::checkBoolArgument(const std::string& argument) const {
    if (argument == "True") {
        return true;
    } else if (argument == "False") {
        return true;
    } else {
        return false;
    }
}


bool
InternalTestStep::getBoolArgument(const std::string& argument) const {
    if (argument == "True") {
        return true;
    } else {
        return false;
    }
}


bool
InternalTestStep::checkStringArgument(const std::string& argument) const {
    if (argument.size() < 2) {
        return false;
    } else if ((argument.front() != argument.back()) || ((argument.front() != '\'') && ((argument.front() != '\"')))) {
        return false;
    } else {
        return true;
    }
}


std::string
InternalTestStep::getStringArgument(const std::string& argument) const {
    std::string argumentParsed;
    for (int i = 1; i < ((int)argument.size() - 1); i++) {
        argumentParsed.push_back(argument[i]);
    }
    return argumentParsed;
}


std::string
InternalTestStep::stripSpaces(const std::string& str) const {
    auto start = std::find_if_not(str.begin(), str.end(), isspace);
    auto end = std::find_if_not(str.rbegin(), str.rend(), isspace).base();
    if (start < end) {
        return std::string(start, end);
    } else {
        return "";
    }
}


void
InternalTestStep::writeError(const std::string& function, const std::string& expected) const {
    WRITE_ERRORF("Invalid internal testStep function '%', requires '%' arguments ", function, expected);
}


InternalTestStep*
InternalTestStep::buildPressKeyEvent(const std::string& key, const bool updateView) const {
    new InternalTestStep(myTestSystem, SEL_KEYPRESS, Category::APP, buildKeyPressEvent(key), updateView);
    return new InternalTestStep(myTestSystem, SEL_KEYRELEASE, Category::APP, buildKeyReleaseEvent(key), updateView);
}


void
InternalTestStep::buildPressKeyEvent(InternalTestStep* parent, const std::string& key) const {
    new InternalTestStep(parent, SEL_KEYPRESS, buildKeyPressEvent(key));
    new InternalTestStep(parent, SEL_KEYRELEASE, buildKeyReleaseEvent(key));
}


InternalTestStep*
InternalTestStep::buildPressKeyEvent(const char key, const bool updateView) const {
    new InternalTestStep(myTestSystem, SEL_KEYPRESS, Category::APP, buildKeyPressEvent(key), updateView);
    return new InternalTestStep(myTestSystem, SEL_KEYRELEASE, Category::APP, buildKeyReleaseEvent(key), updateView);
}


InternalTestStep*
InternalTestStep::buildTwoPressKeyEvent(const std::string& keyA, const std::string& keyB, const bool updateView) const {
    new InternalTestStep(myTestSystem, SEL_KEYPRESS, Category::APP, buildKeyPressEvent(keyA), updateView);
    new InternalTestStep(myTestSystem, SEL_KEYPRESS, Category::APP, buildKeyPressEvent(keyB), updateView);
    new InternalTestStep(myTestSystem, SEL_KEYRELEASE, Category::APP, buildKeyReleaseEvent(keyB), updateView);
    return new InternalTestStep(myTestSystem, SEL_KEYRELEASE, Category::APP, buildKeyReleaseEvent(keyA), updateView);
}


void
InternalTestStep::buildTwoPressKeyEvent(InternalTestStep* parent, const std::string& keyA, const std::string& keyB) const {
    new InternalTestStep(parent, SEL_KEYPRESS, buildKeyPressEvent(keyA));
    new InternalTestStep(parent, SEL_KEYPRESS, buildKeyPressEvent(keyB));
    new InternalTestStep(parent, SEL_KEYRELEASE, buildKeyReleaseEvent(keyB));
    new InternalTestStep(parent, SEL_KEYRELEASE, buildKeyReleaseEvent(keyA));
}

/****************************************************************************/
