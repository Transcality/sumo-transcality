/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
// Copyright (C) 2014-2025 German Aerospace Center (DLR) and others.
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
/// @file    VTypesHandler.cpp
/// @author  Jakob Erdmann
/// @date    12.01.2022
///
// An XML-Handler for reading vTypes
/****************************************************************************/
#include <config.h>

#include <string>
#include <utility>
#include <iostream>
#include "VTypesHandler.h"


// ===========================================================================
// method definitions
// ===========================================================================
VTypesHandler::VTypesHandler(const std::string& file, std::map<std::string, SUMOVTypeParameter*>& vTypes) :
    SUMORouteHandler(file, "", true),
    myVTypes(vTypes)
{}

void
VTypesHandler::closeVType() {
    myVTypes[myCurrentVType->id] = myCurrentVType;
    myCurrentVType = nullptr;
}

VTypesHandler::~VTypesHandler() {}



/****************************************************************************/
