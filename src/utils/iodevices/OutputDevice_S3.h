/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
// Copyright (C) 2004-2026 German Aerospace Center (DLR) and others.
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
/// @file    OutputDevice_S3.h
/// @author  aminissn
/// @date    2026
///
// An output device that writes to an object in Amazon S3 (or a compatible store)
/****************************************************************************/
#pragma once
#include <config.h>

#include <memory>
#include <ostream>
#include "OutputDevice.h"


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class OutputDevice_S3
 * @brief An output device that streams to an S3 object given as s3://bucket/key
 *
 * The data is uploaded with the S3 filesystem of Apache Arrow, so every output format
 *  (XML, CSV, Parquet) can be written to S3. The object is complete once the device is closed.
 *
 * The connection is configured by the following environment variables, otherwise the
 *  default AWS credential chain and region are used:
 * - SUMO_S3_ACCESS_KEY, SUMO_S3_SECRET_KEY: the credentials
 * - SUMO_S3_SESSION_TOKEN: the session token for temporary credentials (optional)
 * - SUMO_S3_REGION: the region of the bucket
 * - SUMO_S3_ENDPOINT: a custom endpoint for S3-compatible storage (optional)
 *
 * All arrow types are hidden in the Impl so that callers do not depend on arrow.
 */
class OutputDevice_S3 : public OutputDevice {
public:
    /** @brief Constructor
     * @param[in] fullName The S3 URL of the object to write (s3://bucket/key)
     * @exception IOError If S3 is not available or the object could not be opened
     */
    OutputDevice_S3(const std::string& fullName);

    /// @brief Destructor (completes the upload)
    ~OutputDevice_S3();

    /** @brief Shuts down the S3 APIs (needs to be called before the end of the application)
     *
     * S3 cannot be used afterwards, so this is only called when the application closes.
     */
    static void finalize();

protected:
    /// @name Methods that override/implement OutputDevice-methods
    /// @{

    /** @brief Returns the associated ostream
     * @return The used stream
     */
    std::ostream& getOStream() override;
    /// @}

private:
    struct Impl;
    std::unique_ptr<Impl> myImpl;

};
