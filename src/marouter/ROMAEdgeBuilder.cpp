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
/// @file    ROMAEdgeBuilder.cpp
/// @author  Daniel Krajzewicz
/// @author  Laura Bieker
/// @author  Michael Behrisch
/// @date    Tue, 20 Jan 2004
///
// Interface for building instances of duarouter-edges
/****************************************************************************/
#include <config.h>

#include "ROMAEdgeBuilder.h"
#include "ROMAEdge.h"


// ===========================================================================
// method definitions
// ===========================================================================
ROMAEdgeBuilder::ROMAEdgeBuilder() {
}


ROMAEdgeBuilder::~ROMAEdgeBuilder() {}


ROEdge*
ROMAEdgeBuilder::buildEdge(const std::string& name, RONode* from, RONode* to, const int priority, const std::string& type) {
    return new ROMAEdge(name, from, to, getNextIndex(), priority, type);
}


/****************************************************************************/
