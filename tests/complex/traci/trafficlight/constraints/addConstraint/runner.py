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
# @author  Michael Behrisch
# @author  Daniel Krajzewicz
# @date    2011-03-04


from __future__ import print_function
from __future__ import absolute_import
import os
import sys

if "SUMO_HOME" in os.environ:
    sys.path.append(os.path.join(os.environ["SUMO_HOME"], "tools"))
import traci  # noqa
import sumolib  # noqa

traci.start([sumolib.checkBinary('sumo'),
             '-n', 'input_net.net.xml',
             '-a', 'input_additional.add.xml',
             '-r', 'input_routes.rou.xml',
             '--no-step-log',
             ] + sys.argv[1:])

print("constraints for 'A'")
for c in traci.trafficlight.getConstraints("A"):
    print("   %s" % c)
print("constraints for 'A' and trip 't0'")
for c in traci.trafficlight.getConstraints("A", "t0"):
    print("   %s" % c)
print("constraints for 'D'")
for c in traci.trafficlight.getConstraints("D"):
    print("   %s" % c)
print("constraints where 'D' is foeSignal")
for c in traci.trafficlight.getConstraintsByFoe("D"):
    print("   %s" % c)

traci.trafficlight.addConstraint("A", "t5", "D", "t4", limit=1)

for i in range(100):
    traci.simulationStep()
print("constraints for 'A' (later)")
for c in traci.trafficlight.getConstraints("A"):
    print("   %s" % c)

traci.close()
