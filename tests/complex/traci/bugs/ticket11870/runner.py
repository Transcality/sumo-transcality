#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
# Copyright (C) 2008-2025 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    runner.py
# @author  Mirko Barthauer
# @date    2022-10-26


from __future__ import print_function
from __future__ import absolute_import
import os
import sys

if "SUMO_HOME" in os.environ:
    sys.path.append(os.path.join(os.environ["SUMO_HOME"], "tools"))

import traci  # noqa
import sumolib  # noqa
import traci.constants as tc  # noqa

sumoBinary = sumolib.checkBinary('sumo')
traci.start([sumoBinary,
             "-n", "input_net2.net.xml",
             "-r", "input_routes.rou.xml",
             "--vehroute-output", "vehroutes.xml",
             "--stop-output", "stops.xml",
             "--no-step-log"
             ] + sys.argv[1:])

vehID = "ego"
traci.vehicle.setStop(vehID, "SC", 60, duration=7)

while traci.simulation.getMinExpectedNumber() > 0:
    if traci.vehicle.isStopped(vehID):
        traci.vehicle.setStop(vehID, "SC", 60, duration=0)
        traci.vehicle.moveToXY(vehID, "", -1, 150.47, 94.99, keepRoute=2)
    traci.simulationStep()
traci.close()
