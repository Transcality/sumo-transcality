#!/usr/bin/env python
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
# Copyright (C) 2009-2025 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    test.py
# @author  Pablo Alvarez Lopez
# @date    2016-11-25

# import common functions for netedit tests
import os
import sys

testRoot = os.path.join(os.environ.get('SUMO_HOME', '.'), 'tests')
neteditTestRoot = os.path.join(
    os.environ.get('TEXTTEST_HOME', testRoot), 'netedit')
sys.path.append(neteditTestRoot)
import neteditTestFunctions as netedit  # noqa

# Open netedit
neteditProcess, referencePosition = netedit.setupAndStart(neteditTestRoot)

# rebuild network
netedit.rebuildNetwork()

# Change to create mode
netedit.createEdgeMode()

# Create two nodes
netedit.leftClick(referencePosition, netedit.positions.network.junction.positionA, 0, -30)
netedit.leftClick(referencePosition, netedit.positions.network.junction.positionB, 0, -30)

# select two-way mode
netedit.changeEditMode(netedit.attrs.modes.network.twoWayMode)

netedit.leftClick(referencePosition, netedit.positions.network.junction.positionC, 0, 30)
netedit.leftClick(referencePosition, netedit.positions.network.junction.positionD, 0, 30)

# move to down (to avoid invalid selection)
netedit.moveMouse(referencePosition, netedit.positions.downLeft)

# save network
netedit.saveNetwork(False, referencePosition)

# save Netedit config
netedit.saveNeteditConfig(referencePosition)

# quit netedit
netedit.quit(neteditProcess)