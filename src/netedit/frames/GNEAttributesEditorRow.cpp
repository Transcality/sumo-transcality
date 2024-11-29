/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
// Copyright (C) 2001-2024 German Aerospace Center (DLR) and others.
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
/// @file    GNEAttributesEditorRow.h
/// @author  Pablo Alvarez Lopez
/// @date    Nov 2024
///
// Row used for edit attributes in GNEAttributesEditor
/****************************************************************************/
#include <config.h>

#include <netedit/elements/GNEAttributeCarrier.h>
#include <netedit/dialogs/GNEMultipleParametersDialog.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/GNEApplicationWindow.h>
#include <netedit/dialogs/GNEAllowVClassesDialog.h>
#include <netedit/dialogs/GNESingleParametersDialog.h>
#include <netedit/frames/common/GNEInspectorFrame.h>
#include <netedit/frames/demand/GNETypeFrame.h>
#include <utils/common/StringTokenizer.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/images/VClassIcons.h>
#include <utils/gui/images/POIIcons.h>
#include <utils/gui/windows/GUIAppEnum.h>

#include "GNEAttributesEditorRow.h"

// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEAttributesEditorRow) GNEAttributeRowMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,                  GNEAttributesEditorRow::onCmdSetAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_BOOL,             GNEAttributesEditorRow::onCmdToogleEnableAttribute),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_COLOR,            GNEAttributesEditorRow::onCmdOpenColorDialog),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_ALLOW,            GNEAttributesEditorRow::onCmdOpenAllowDialog),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE_INSPECTPARENT,    GNEAttributesEditorRow::onCmdInspectParent),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_MOVELANE_UP,                    GNEAttributesEditorRow::onCmdMoveElementLaneUp),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_MOVELANE_DOWN,                  GNEAttributesEditorRow::onCmdMoveElementLaneDown)
};

// Object implementation
FXIMPLEMENT(GNEAttributesEditorRow,          FXHorizontalFrame,      GNEAttributeRowMap,         ARRAYNUMBER(GNEAttributeRowMap))

// ===========================================================================
// defines
// ===========================================================================

#define TEXTCOLOR_BLACK FXRGB(0, 0, 0)
#define TEXTCOLOR_BLUE FXRGB(0, 0, 255)
#define TEXTCOLOR_RED FXRGB(255, 0, 0)
#define TEXTCOLOR_BACKGROUND_RED FXRGBA(255, 213, 213, 255)
#define TEXTCOLOR_BACKGROUND_WHITE FXRGB(255, 255, 255)

// ===========================================================================
// method definitions
// ===========================================================================

GNEAttributesEditorRow::GNEAttributesEditorRow(GNEAttributesEditor* attributeTable) :
    FXHorizontalFrame(attributeTable->getCollapsableFrame(), GUIDesignAuxiliarHorizontalFrame),
    myAttributeTable(attributeTable) {
    // get static tooltip menu
    const auto tooltipMenu = attributeTable->getFrameParent()->getViewNet()->getViewParent()->getGNEAppWindows()->getStaticTooltipMenu();
    // Create left label
    myAttributeLabel = new MFXLabelTooltip(this, tooltipMenu, "Label", nullptr, GUIDesignLabelThickedFixed(100));
    myAttributeLabel->hide();
    // create lef boolean checkBox for enable/disable attributes
    myAttributeCheckButton = new FXCheckButton(this, "Enable/Disable attribute checkBox", this,
            MID_GNE_SET_ATTRIBUTE_BOOL, GUIDesignCheckButtonAttribute);
    myAttributeCheckButton->hide();
    // create left button for inspect parent
    myAttributeParentButton = new MFXButtonTooltip(this, tooltipMenu, "Inspect parent button", nullptr, this,
            MID_GNE_SET_ATTRIBUTE_INSPECTPARENT, GUIDesignButtonAttribute);
    myAttributeParentButton->hide();
    // create lef button for edit allow/disallow vClasses
    myAttributeVClassButton = new MFXButtonTooltip(this, tooltipMenu, "Edit vClass button", nullptr, this,
            MID_GNE_SET_ATTRIBUTE_ALLOW, GUIDesignButtonAttribute);
    myAttributeVClassButton->hide();
    // set tip text for edit vClasses button
    myAttributeVClassButton->setTipText(TL("Open dialog for editing vClasses"));
    myAttributeVClassButton->setHelpText(TL("Open dialog for editing vClasses"));
    // create lef attribute for edit color
    myAttributeColorButton = new MFXButtonTooltip(this, tooltipMenu, "color button", GUIIconSubSys::getIcon(GUIIcon::COLORWHEEL), this,
            MID_GNE_SET_ATTRIBUTE_COLOR, GUIDesignButtonAttribute);
    myAttributeColorButton->hide();
    // set tip text for color button
    myAttributeColorButton->setTipText(TL("Open dialog for editing color"));
    myAttributeColorButton->setHelpText(TL("Open dialog for editing color"));
    // create right text field for string attributes
    myValueTextField = new MFXTextFieldTooltip(this, tooltipMenu, GUIDesignTextFieldNCol, this,
            MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    myValueTextField->hide();
    // create right combo box for discrete attributes
    myValueComboBox = new MFXComboBoxIcon(this, GUIDesignComboBoxNCol, true, GUIDesignComboBoxVisibleItemsMedium, this,
                                          MID_GNE_SET_ATTRIBUTE, GUIDesignComboBoxAttribute);
    myValueComboBox->hide();
    // Create right check button
    myValueCheckButton = new FXCheckButton(this, "check button", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);
    myValueCheckButton->hide();
    // create right move lane up button
    myValueLaneUpButton = new MFXButtonTooltip(this, tooltipMenu, "", GUIIconSubSys::getIcon(GUIIcon::ARROW_UP), this,
            MID_GNE_MOVELANE_UP, GUIDesignButtonIcon);
    myValueLaneUpButton->hide();
    // set tip texts
    myValueLaneUpButton->setTipText(TL("Move element up one lane"));
    myValueLaneUpButton->setHelpText(TL("Move element up one lane"));
    // create right move lane down button
    myValueLaneDownButton = new MFXButtonTooltip(this, tooltipMenu, "", GUIIconSubSys::getIcon(GUIIcon::ARROW_DOWN), this,
            MID_GNE_MOVELANE_DOWN, GUIDesignButtonIcon);
    myValueLaneDownButton->hide();
    // set tip texts
    myValueLaneDownButton->setTipText(TL("Move element down one lane"));
    myValueLaneDownButton->setHelpText(TL("Move element down one lane"));
}




void
GNEAttributesEditorRow::showAttributeRow(const GNEAttributeProperties& attrProperty) {
    const auto multipleEditedACs = (myAttributeTable->myEditedACs.size() > 1);
    const auto& tagProperty = attrProperty.getTagPropertyParent();
    myAttribute = attrProperty.getAttr();
    if (multipleEditedACs && attrProperty.isUnique()) {
        // disable editing for unique attributes in case of multi-selection
        hideAttributeRow();
    } else {
        const auto firstEditedAC = myAttributeTable->myEditedACs.front();
        // declare a flag for enabled attributes
        bool attributeEnabled = firstEditedAC->isAttributeEnabled(myAttribute);
        // certain attributes are always enabled (usually vType attributes)
        if (attrProperty.isAlwaysEnabled()) {
            attributeEnabled = true;
        }
        // extra check for person and container expected, because depends of triggered
        if (tagProperty.isVehicleStop()) {
            if ((myAttribute == SUMO_ATTR_EXPECTED) && (firstEditedAC->isAttributeEnabled(SUMO_ATTR_TRIGGERED) == false)) {
                attributeEnabled = false;
            } else if ((myAttribute == SUMO_ATTR_EXPECTED_CONTAINERS) && (firstEditedAC->isAttributeEnabled(SUMO_ATTR_CONTAINER_TRIGGERED) == false)) {
                attributeEnabled = false;
            }
        }
        // check if this attribute is computed
        const bool computedAttribute = multipleEditedACs ? false : firstEditedAC->isAttributeComputed(myAttribute);
        // get string value depending if attribute is enabled
        const std::string value = attributeEnabled ? getAttributeValue(attrProperty) : firstEditedAC->getAlternativeValueForDisabledAttributes(myAttribute);
        // get parent if we're editing single vTypes
        GNEAttributeCarrier* ACParent = nullptr;
        if (!multipleEditedACs && attrProperty.isVType()) {
            const auto& ACs = myAttributeTable->myFrameParent->getViewNet()->getNet()->getAttributeCarriers();
            // parent can be either type or distribution
            if (myAttribute == SUMO_ATTR_TYPE) {
                ACParent = ACs->retrieveDemandElement(SUMO_TAG_VTYPE, firstEditedAC->getAttribute(SUMO_ATTR_TYPE), false);
            }
            if (ACParent == nullptr) {
                ACParent = ACs->retrieveDemandElement(SUMO_TAG_VTYPE_DISTRIBUTION, firstEditedAC->getAttribute(SUMO_ATTR_TYPE), false);
            }
        }
        // show elements depending of attribute properties
        if (attrProperty.isActivatable()) {
            showAttributeCheckButton(attrProperty, attributeEnabled);
        } else if ((myAttribute == SUMO_ATTR_TYPE) && (tagProperty.isVehicle() || tagProperty.isPerson() || tagProperty.isContainer())) {
            showAttributeParent(attrProperty, attributeEnabled);
        } else if (myAttribute == SUMO_ATTR_ALLOW) {
            showAttributeVClass(attrProperty, attributeEnabled);
        } else if (myAttribute == SUMO_ATTR_COLOR) {
            showAttributeColor(attrProperty, attributeEnabled);
        } else {
            showAttributeLabel(attrProperty);
        }
        // continue depending of type of attribute
        if (attrProperty.isBool()) {
            showValueCheckButton(attrProperty, value, attributeEnabled, computedAttribute);
        } else if (attrProperty.isDiscrete() || attrProperty.isVType()) {
            showValueComboBox(attrProperty, value, attributeEnabled, computedAttribute);
        } else {
            showValueString(attrProperty, value, attributeEnabled, computedAttribute);
        }
        // check if show move lane buttons
        if (!multipleEditedACs && !tagProperty.isNetworkElement() && (myAttribute == SUMO_ATTR_LANE)) {
            showMoveLaneButtons(value);
            myValueLaneUpButton->show();
            myValueLaneDownButton->show();
        } else {
            myValueLaneUpButton->hide();
            myValueLaneDownButton->hide();
        }
        // Show GNEAttributesEditorRow
        show();
    }
}


void
GNEAttributesEditorRow::hideAttributeRow() {
    hide();
}


bool
GNEAttributesEditorRow::isAttributeRowShown() const {
    return shown();
}


bool
GNEAttributesEditorRow::isCurrentValueValid() const {
    if (myValueCheckButton->shown()) {
        // check box have always a valid Value
        return true;
    } else if (myValueTextField->shown()) {
        return ((myValueTextField->getTextColor() == TEXTCOLOR_BLACK) || (myValueTextField->getTextColor() == TEXTCOLOR_BLUE));
    } else if (myValueComboBox->shown()) {
        return ((myValueComboBox->getTextColor() == TEXTCOLOR_BLACK) || (myValueComboBox->getTextColor() == TEXTCOLOR_BLUE));
    } else {
        return false;
    }
}


long
GNEAttributesEditorRow::onCmdOpenColorDialog(FXObject*, FXSelector, void*) {
    const auto& attrProperty = myAttributeTable->myEditedACs.front()->getTagProperty().getAttributeProperties(myAttribute);
    // create FXColorDialog
    FXColorDialog colordialog(this, TL("Color Dialog"));
    colordialog.setTarget(this);
    colordialog.setIcon(GUIIconSubSys::getIcon(GUIIcon::COLORWHEEL));
    // If previous attribute wasn't correct, set black as default color
    if (GNEAttributeCarrier::canParse<RGBColor>(myValueTextField->getText().text())) {
        colordialog.setRGBA(MFXUtils::getFXColor(GNEAttributeCarrier::parse<RGBColor>(myValueTextField->getText().text())));
    } else if (!attrProperty.getDefaultValue().empty()) {
        colordialog.setRGBA(MFXUtils::getFXColor(GNEAttributeCarrier::parse<RGBColor>(attrProperty.getDefaultValue())));
    } else {
        colordialog.setRGBA(MFXUtils::getFXColor(RGBColor::BLACK));
    }
    // execute dialog to get a new color in the text field
    if (colordialog.execute()) {
        myValueTextField->setText(toString(MFXUtils::getRGBColor(colordialog.getRGBA())).c_str(), TRUE);
    }
    return 1;
}


long
GNEAttributesEditorRow::onCmdOpenAllowDialog(FXObject*, FXSelector, void*) {
    // declare values to be modified
    std::string allowedVehicles = myValueTextField->getText().text();
    // declare accept changes
    bool acceptChanges = false;
    // open GNEAllowVClassesDialog (also used to modify SUMO_ATTR_CHANGE_LEFT etc)
    GNEAllowVClassesDialog(myAttributeTable->getFrameParent()->getViewNet(), myAttribute, &allowedVehicles, &acceptChanges).execute();
    // continue depending of acceptChanges
    if (acceptChanges) {
        myValueTextField->setText(allowedVehicles.c_str(), TRUE);
    }
    return 1;
}


long
GNEAttributesEditorRow::onCmdInspectParent(FXObject*, FXSelector, void*) {
    myAttributeTable->inspectParent();
    return 1;
}


long
GNEAttributesEditorRow::onCmdMoveElementLaneUp(FXObject*, FXSelector, void*) {
    myAttributeTable->moveLaneUp();
    return 1;
}


long
GNEAttributesEditorRow::onCmdMoveElementLaneDown(FXObject*, FXSelector, void*) {
    myAttributeTable->moveLaneDown();
    return 1;
}


long
GNEAttributesEditorRow::onCmdSetAttribute(FXObject* obj, FXSelector, void*) {
    const auto& editedAC = myAttributeTable->myEditedACs.front();
    const auto& attrProperties = editedAC->getTagProperty().getAttributeProperties(myAttribute);
    // continue depending of clicked object
    if (obj == myValueCheckButton) {
        // Set true o false depending of the checkBox
        if (myValueCheckButton->getCheck()) {
            myValueCheckButton->setText("true");
        } else {
            myValueCheckButton->setText("false");
        }
        myAttributeTable->setAttribute(myAttribute, myValueCheckButton->getText().text());
    } else if (obj == myValueComboBox) {
        const std::string newValue = myValueComboBox->getText().text();
        // check if the new comboBox value is valid
        if (editedAC->isValid(myAttribute, newValue)) {
            myAttributeTable->setAttribute(myAttribute, newValue);
            myValueComboBox->setTextColor(TEXTCOLOR_BLACK);
            myValueComboBox->setBackColor(TEXTCOLOR_BACKGROUND_WHITE);
        } else {
            // edit colors
            myValueComboBox->setTextColor(TEXTCOLOR_RED);
            if (newValue.empty()) {
                myValueComboBox->setBackColor(TEXTCOLOR_BACKGROUND_RED);
            }
            // Write Warning in console if we're in testing mode
            WRITE_DEBUG(TLF("ComboBox value '%' for attribute % of % isn't valid", newValue, attrProperties.getAttrStr(), attrProperties.getTagPropertyParent().getTagStr()));
        }
    } else if (obj == myValueTextField) {
        // check if we're merging junction
        //if (!mergeJunction(myACAttr.getAttr(), newVal)) {
        // if its valid for the first AC than its valid for all (of the same type)


        // first check if set default value
        if (myValueTextField->getText().empty() && attrProperties.hasDefaultValue()) {
            // update text field without notify
            myValueTextField->setText(attrProperties.getDefaultValue().c_str(), FALSE);
        }
        // if we're editing an angle, check if filter between [0,360]
        if ((myAttribute == SUMO_ATTR_ANGLE) && GNEAttributeCarrier::canParse<double>(myValueTextField->getText().text())) {
            // filter anglea and update text field without notify
            const double angle = fmod(GNEAttributeCarrier::parse<double>(myValueTextField->getText().text()), 360);
            myValueTextField->setText(toString(angle).c_str(), FALSE);
        }
        // if we're editing a position or a shape, strip whitespace after comma
        if ((myAttribute == SUMO_ATTR_POSITION) || (myAttribute == SUMO_ATTR_SHAPE)) {
            std::string shape(myValueTextField->getText().text());
            while (shape.find(", ") != std::string::npos) {
                shape = StringUtils::replace(shape, ", ", ",");
            }
            myValueTextField->setText(toString(shape).c_str(), FALSE);
        }
        // if we're editing a int, strip decimal value
        if (attrProperties.isInt() && GNEAttributeCarrier::canParse<double>(myValueTextField->getText().text())) {
            double doubleValue = GNEAttributeCarrier::parse<double>(myValueTextField->getText().text());
            if ((doubleValue - (int)doubleValue) == 0) {
                myValueTextField->setText(toString((int)doubleValue).c_str(), FALSE);
            }

        }
        // after apply all filters, obtain value
        const std::string newValue = myValueTextField->getText().text();
        // check if the new textField value is valid
        if (editedAC->isValid(myAttribute, newValue)) {
            myAttributeTable->setAttribute(myAttribute, newValue);
            myValueTextField->setTextColor(TEXTCOLOR_BLACK);
        } else {
            // edit colors
            myValueTextField->setTextColor(TEXTCOLOR_RED);
            if (newValue.empty()) {
                myValueTextField->setBackColor(TEXTCOLOR_BACKGROUND_RED);
            }
            // Write Warning in console if we're in testing mode
            WRITE_DEBUG(TLF("TextField value '%' for attribute % of % isn't valid", newValue, attrProperties.getAttrStr(), attrProperties.getTagPropertyParent().getTagStr()));
        }
    }
    return 1;
}


long
GNEAttributesEditorRow::onCmdToogleEnableAttribute(FXObject*, FXSelector, void*) {
    myAttributeTable->toggleEnableAttribute(myAttribute, myAttributeCheckButton->getCheck() == TRUE);
    return 0;
}


GNEAttributesEditorRow::GNEAttributesEditorRow() :
    myAttributeTable(nullptr) {
}


const std::string
GNEAttributesEditorRow::getAttributeValue(const GNEAttributeProperties& attrProperty) const {
    // Declare a set of occurring values and insert attribute's values of item (note: We use a set to avoid repeated values)
    std::set<std::string> values;
    // iterate over edited attributes and insert every value in set
    for (const auto& editedAC : myAttributeTable->myEditedACs) {
        if (editedAC->hasAttribute(attrProperty.getAttr())) {
            values.insert(editedAC->getAttribute(attrProperty.getAttr()));
        }
    }
    // merge all values in a single string
    std::ostringstream oss;
    for (auto it = values.begin(); it != values.end(); it++) {
        if (it != values.begin()) {
            oss << " ";
        }
        oss << *it;
    }
    // obtain value to be shown in row
    return oss.str();
}


void
GNEAttributesEditorRow::showAttributeCheckButton(const GNEAttributeProperties& attrProperty, const bool enabled) {
    myAttributeCheckButton->setText(attrProperty.getAttrStr().c_str());
    myAttributeCheckButton->setCheck(enabled);
    myAttributeCheckButton->show();
    // hide other elements
    myAttributeLabel->hide();
    myAttributeParentButton->hide();
    myAttributeVClassButton->hide();
    myAttributeColorButton->hide();
    // enable depending of supermode
    enableDependingOfSupermode(attrProperty);
}


void
GNEAttributesEditorRow::showAttributeParent(const GNEAttributeProperties& attrProperty, const bool enabled) {
    // set icon and text
    myAttributeParentButton->setIcon(GUIIconSubSys::getIcon(attrProperty.getTagPropertyParent().getGUIIcon()));
    myAttributeParentButton->setText(attrProperty.getAttrStr().c_str());
    if (enabled) {
        myAttributeParentButton->enable();
    } else {
        myAttributeParentButton->disable();
    }
    myAttributeParentButton->show();
    // hide other elements
    myAttributeLabel->hide();
    myAttributeCheckButton->hide();
    myAttributeVClassButton->hide();
    myAttributeColorButton->hide();
    // enable depending of supermode
    enableDependingOfSupermode(attrProperty);
}


void
GNEAttributesEditorRow::showAttributeVClass(const GNEAttributeProperties& attrProperty, const bool enabled) {
    // set icon and text
    myAttributeVClassButton->setText(attrProperty.getAttrStr().c_str());
    if (enabled) {
        myAttributeVClassButton->enable();
    } else {
        myAttributeVClassButton->disable();
    }
    myAttributeVClassButton->show();
    // hide other elements
    myAttributeLabel->hide();
    myAttributeCheckButton->hide();
    myAttributeParentButton->hide();
    myAttributeColorButton->hide();
    // enable depending of supermode
    enableDependingOfSupermode(attrProperty);
}


void
GNEAttributesEditorRow::showAttributeColor(const GNEAttributeProperties& attrProperty, const bool enabled) {
    myAttributeColorButton->setText(attrProperty.getAttrStr().c_str());
    myAttributeColorButton->show();
    if (enabled) {
        myAttributeColorButton->enable();
    } else {
        myAttributeColorButton->disable();
    }
    // hide other elements
    myAttributeLabel->hide();
    myAttributeCheckButton->hide();
    myAttributeParentButton->hide();
    myAttributeVClassButton->hide();
    // enable depending of supermode
    enableDependingOfSupermode(attrProperty);
}


void
GNEAttributesEditorRow::showAttributeLabel(const GNEAttributeProperties& attrProperty) {
    myAttributeLabel->setText(attrProperty.getAttrStr().c_str());
    myAttributeLabel->show();
    // hide other elements
    myAttributeCheckButton->hide();
    myAttributeParentButton->hide();
    myAttributeVClassButton->hide();
    myAttributeColorButton->hide();
    // enable depending of supermode
    enableDependingOfSupermode(attrProperty);
}


void
GNEAttributesEditorRow::showValueCheckButton(const GNEAttributeProperties& attrProperty, const std::string& value,
        const bool computed, const bool enabled) {
    // first we need to check if all boolean values are equal
    bool allValuesEqual = true;
    // declare  boolean vector
    std::vector<bool> booleanVector;
    // check if value can be parsed to a boolean vector
    if (GNEAttributeCarrier::canParse<std::vector<bool> >(value)) {
        booleanVector = GNEAttributeCarrier::parse<std::vector<bool> >(value);
    }
    // iterate over booleans comparing all element with the first
    for (const auto& booleanValue : booleanVector) {
        if (booleanValue != booleanVector.front()) {
            allValuesEqual = false;
        }
    }
    // use checkbox or textfield depending if all booleans are equal
    if (allValuesEqual) {
        if (enabled) {
            myValueCheckButton->enable();
        } else {
            myValueCheckButton->disable();
        }
        // set check button
        if ((booleanVector.size() > 0) && booleanVector.front()) {
            myValueCheckButton->setCheck(true);
            myValueCheckButton->setText("true");
        } else {
            myValueCheckButton->setCheck(false);
            myValueCheckButton->setText("false");
        }
        // show check button
        myValueCheckButton->show();
        // hide other value elements
        myValueTextField->hide();
        myValueComboBox->hide();
        myValueLaneUpButton->hide();
        myValueLaneDownButton->hide();
        // enable depending of supermode
        enableDependingOfSupermode(attrProperty);
    } else {
        // show value as string
        showValueString(attrProperty, value, enabled, computed);
    }
}


void
GNEAttributesEditorRow::showValueComboBox(const GNEAttributeProperties& attrProperty, const std::string& value,
        const bool computed, const bool enabled) {
    // first we need to check if all boolean values are equal
    bool allValuesEqual = true;
    // declare  boolean vector
    std::vector<std::string> stringVector;
    // check if value can be parsed to a boolean vector
    if (GNEAttributeCarrier::canParse<std::vector<std::string> >(value)) {
        stringVector = GNEAttributeCarrier::parse<std::vector<std::string> >(value);
    }
    // iterate over string comparing all element with the first
    for (const auto& stringValue : stringVector) {
        if (stringValue != stringVector.front()) {
            allValuesEqual = false;
        }
    }
    // use checkbox or textfield depending if all booleans are equal
    if (allValuesEqual) {
        // clear and enable comboBox
        myValueComboBox->clearItems();
        myValueComboBox->setTextColor(TEXTCOLOR_BLACK);
        myValueComboBox->setBackColor(TEXTCOLOR_BACKGROUND_WHITE);
        if (enabled) {
            myValueComboBox->enable();
        } else {
            myValueComboBox->disable();
        }
        // fill depeding of ACAttr
        if (attrProperty.getAttr() == SUMO_ATTR_VCLASS) {
            // add all vClasses with their icons
            for (const auto& vClassStr : SumoVehicleClassStrings.getStrings()) {
                myValueComboBox->appendIconItem(vClassStr.c_str(), VClassIcons::getVClassIcon(getVehicleClassID(vClassStr)));
            }
        } else if (attrProperty.isVType()) {
            // get ACs
            const auto& ACs = myAttributeTable->myFrameParent->getViewNet()->getNet()->getAttributeCarriers();
            // fill comboBox with all vTypes and vType distributions sorted by ID
            std::map<std::string, GNEDemandElement*> sortedTypes;
            for (const auto& type : ACs->getDemandElements().at(SUMO_TAG_VTYPE)) {
                sortedTypes[type.second->getID()] = type.second;
            }
            for (const auto& sortedType : sortedTypes) {
                myValueComboBox->appendIconItem(sortedType.first.c_str(), sortedType.second->getACIcon());
            }
            sortedTypes.clear();
            for (const auto& typeDistribution : ACs->getDemandElements().at(SUMO_TAG_VTYPE_DISTRIBUTION)) {
                sortedTypes[typeDistribution.second->getID()] = typeDistribution.second;
            }
            for (const auto& sortedType : sortedTypes) {
                myValueComboBox->appendIconItem(sortedType.first.c_str(), sortedType.second->getACIcon());
            }
        } else if (attrProperty.getAttr() == SUMO_ATTR_ICON) {
            // add all POIIcons with their icons
            for (const auto& POIIcon : SUMOXMLDefinitions::POIIcons.getValues()) {
                myValueComboBox->appendIconItem(SUMOXMLDefinitions::POIIcons.getString(POIIcon).c_str(), POIIcons::getPOIIcon(POIIcon));
            }
        } else if ((attrProperty.getAttr() == SUMO_ATTR_RIGHT_OF_WAY) && (myAttributeTable->myEditedACs.size() == 1) &&
                   (attrProperty.getTagPropertyParent().getTag() == SUMO_TAG_JUNCTION)) {
            // special case for junction types
            if (myAttributeTable->myEditedACs.front()->getAttribute(SUMO_ATTR_TYPE) == "priority") {
                myValueComboBox->appendIconItem(SUMOXMLDefinitions::RightOfWayValues.getString(RightOfWay::DEFAULT).c_str(), nullptr);
                myValueComboBox->appendIconItem(SUMOXMLDefinitions::RightOfWayValues.getString(RightOfWay::EDGEPRIORITY).c_str(), nullptr);
            } else if (myAttributeTable->myEditedACs.front()->getAttribute(SUMO_ATTR_TYPE) == "traffic_light") {
                myValueComboBox->appendIconItem(SUMOXMLDefinitions::RightOfWayValues.getString(RightOfWay::DEFAULT).c_str(), nullptr);
                myValueComboBox->appendIconItem(SUMOXMLDefinitions::RightOfWayValues.getString(RightOfWay::MIXEDPRIORITY).c_str(), nullptr);
                myValueComboBox->appendIconItem(SUMOXMLDefinitions::RightOfWayValues.getString(RightOfWay::ALLWAYSTOP).c_str(), nullptr);
            } else {
                myValueComboBox->disable();
            }
        } else {
            // fill comboBox with discrete values
            for (const auto& discreteValue : attrProperty.getDiscreteValues()) {
                myValueComboBox->appendIconItem(discreteValue.c_str(), nullptr);
            }
        }
        // set current value (or disable)
        const auto index = myValueComboBox->findItem(value.c_str());
        if (index < 0) {
            if (myValueComboBox->getNumItems() > 0) {
                myValueComboBox->setCurrentItem(0);
            } else {
                myValueComboBox->disable();
            }
        } else {
            myValueComboBox->setCurrentItem(index);
        }
        // show comboBox button
        myValueComboBox->show();
        // hide other value elements
        myValueTextField->hide();
        myValueCheckButton->hide();
        myValueLaneUpButton->hide();
        myValueLaneDownButton->hide();
        // enable depending of supermode
        enableDependingOfSupermode(attrProperty);
    } else {
        // show value as string
        showValueString(attrProperty, value, computed, enabled);
    }
}


void
GNEAttributesEditorRow::showValueString(const GNEAttributeProperties& attrProperty, const std::string& value,
                                        const bool enabled, const bool computed) {
    // clear and enable comboBox
    myValueTextField->setText(value.c_str());
    if (computed) {
        myValueTextField->setTextColor(TEXTCOLOR_BLUE);
    } else {
        myValueTextField->setTextColor(TEXTCOLOR_BLACK);
    }
    if (enabled) {
        myValueTextField->enable();
    } else {
        myValueTextField->disable();
    }
    // show list of values
    myValueTextField->show();
    // hide other value elements
    myValueCheckButton->hide();
    myValueComboBox->hide();
    // enable depending of supermode
    enableDependingOfSupermode(attrProperty);
}


void
GNEAttributesEditorRow::showMoveLaneButtons(const std::string& laneID) {
    // retrieve lane
    const auto lane = myAttributeTable->myFrameParent->getViewNet()->getNet()->getAttributeCarriers()->retrieveLane(laneID, false);
    // check lane
    if (lane) {
        // check if disable move up
        if ((lane->getIndex() + 1) >= (int)lane->getParentEdge()->getLanes().size()) {
            myValueLaneUpButton->disable();
        } else {
            myValueLaneUpButton->enable();
        }
        // check if disable move down
        if ((lane->getIndex() - 1) < 0) {
            myValueLaneDownButton->disable();
        } else {
            myValueLaneDownButton->enable();
        }
    } else {
        // if lane doesn't exist, disable both
        myValueLaneUpButton->disable();
        myValueLaneDownButton->disable();
    }
}


void
GNEAttributesEditorRow::enableDependingOfSupermode(const GNEAttributeProperties& attrProperty) {
    const auto& editModes = myAttributeTable->myFrameParent->getViewNet()->getEditModes();
    const auto& tagProperty = attrProperty.getTagPropertyParent();
    // by default we assume that elements are disabled
    bool enableElements = false;
    if (editModes.isCurrentSupermodeNetwork() && (tagProperty.isNetworkElement() || tagProperty.isAdditionalElement())) {
        enableElements = true;
    } else if (editModes.isCurrentSupermodeDemand() && tagProperty.isDemandElement()) {
        enableElements = true;
    } else if (editModes.isCurrentSupermodeData() && (tagProperty.isDataElement() || tagProperty.isMeanData())) {
        enableElements = true;
    }
    if (enableElements) {
        // only enable elements showns
        if (myAttributeCheckButton->shown()) {
            myAttributeCheckButton->enable();
        }
        if (myAttributeParentButton->shown()) {
            myAttributeParentButton->enable();
        }
        if (myAttributeVClassButton->shown()) {
            myAttributeVClassButton->enable();
        }
        if (myAttributeColorButton->shown()) {
            myAttributeColorButton->enable();
        }
        if (myValueTextField->shown()) {
            myValueTextField->enable();
        }
        if (myValueComboBox->shown()) {
            myValueComboBox->enable();
        }
        if (myValueCheckButton->shown()) {
            myValueCheckButton->enable();
        }
        if (myValueLaneUpButton->shown()) {
            myValueLaneUpButton->enable();
        }
        if (myValueLaneDownButton->shown()) {
            myValueLaneDownButton->enable();
        }
    } else {
        myAttributeCheckButton->disable();
        myAttributeParentButton->disable();
        myAttributeVClassButton->disable();
        myAttributeColorButton->disable();
        myValueTextField->disable();
        myValueComboBox->disable();
        myValueCheckButton->disable();
        myValueLaneUpButton->disable();
        myValueLaneDownButton->disable();
    }
}

/*
bool
GNEAttributesEditorRow::mergeJunction(SumoXMLAttr attr, const std::string& newVal) const {
    const auto viewNet = myAttributesEditorParent->getFrameParent()->getViewNet();
    const auto& inspectedElements = viewNet->getInspectedElements();
    // check if we're editing junction position
    if (inspectedElements.isInspectingSingleElement() && (inspectedElements.getFirstAC()->getTagProperty().getTag() == SUMO_TAG_JUNCTION) && (attr == SUMO_ATTR_POSITION)) {
        // retrieve original junction
        GNEJunction* movedJunction = viewNet->getNet()->getAttributeCarriers()->retrieveJunction(inspectedElements.getFirstAC()->getID());
        // parse position
        const Position newPosition = GNEAttributeCarrier::parse<Position>(newVal);
        // iterate over network junction
        for (const auto& targetjunction : viewNet->getNet()->getAttributeCarriers()->getJunctions()) {
            // check distance position
            if ((targetjunction.second->getPositionInView().distanceTo2D(newPosition) < POSITION_EPS) &&
                    viewNet->askMergeJunctions(movedJunction, targetjunction.second)) {
                viewNet->getNet()->mergeJunctions(movedJunction, targetjunction.second, viewNet->getUndoList());
                return true;
            }
        }
    }
    // nothing to merge
    return false;
}
*/


/****************************************************************************/