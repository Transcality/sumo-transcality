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
/// @file    TraCIServerAPI_TrafficLight.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    07.05.2009
///
// APIs for getting/setting traffic light values via TraCI
/****************************************************************************/
#pragma once
#include <config.h>

#include "TraCIServer.h"
#include <foreign/tcpip/storage.h>

class TraCISignalConstraint;

// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class TraCIServerAPI_TrafficLight
 * @brief APIs for getting/setting traffic light values via TraCI
 */
class TraCIServerAPI_TrafficLight {
public:
    /** @brief Processes a get value command (Command 0xa2: Get Traffic Lights Variable)
     *
     * @param[in] server The TraCI-server-instance which schedules this request
     * @param[in] inputStorage The storage to read the command from
     * @param[out] outputStorage The storage to write the result to
     */
    static bool processGet(TraCIServer& server, tcpip::Storage& inputStorage,
                           tcpip::Storage& outputStorage);


    /** @brief Processes a set value command (Command 0xc2: Change Traffic Lights State)
     *
     * @param[in] server The TraCI-server-instance which schedules this request
     * @param[in] inputStorage The storage to read the command from
     * @param[out] outputStorage The storage to write the result to
     */
    static bool processSet(TraCIServer& server, tcpip::Storage& inputStorage,
                           tcpip::Storage& outputStorage);
private:
    static void writeConstraint(TraCIServer& server, const libsumo::TraCISignalConstraint& c);

private:
    /// @brief invalidated copy constructor
    TraCIServerAPI_TrafficLight(const TraCIServerAPI_TrafficLight& s);

    /// @brief invalidated assignment operator
    TraCIServerAPI_TrafficLight& operator=(const TraCIServerAPI_TrafficLight& s);


};
