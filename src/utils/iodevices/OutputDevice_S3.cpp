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
/// @file    OutputDevice_S3.cpp
/// @author  aminissn
/// @date    2026
///
// An output device that writes to an object in Amazon S3 (or a compatible store)
/****************************************************************************/
#include <config.h>

#include <cstdlib>
#include <mutex>
#include <streambuf>
#include <vector>
#include <utils/common/MsgHandler.h>
#include <utils/common/UtilExceptions.h>
#include "OutputDevice_S3.h"

#ifdef _MSC_VER
#pragma warning(push)
/* Disable warning about changed memory layout due to virtual base class */
#pragma warning(disable: 4435)
#endif
#include <arrow/filesystem/s3fs.h>
#include <arrow/io/interfaces.h>
#include <arrow/result.h>
#include <arrow/status.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


// ===========================================================================
// helper class definitions
// ===========================================================================
namespace {

/// @brief serializes the initialization of the S3 APIs
std::mutex gS3InitMutex;

/// @brief a stream buffer forwarding its content to an arrow output stream
class ArrowStreamBuf : public std::streambuf {
public:
    ArrowStreamBuf(std::shared_ptr<arrow::io::OutputStream> out) : myOut(out), myBuffer(1 << 16) {
        setp(myBuffer.data(), myBuffer.data() + myBuffer.size());
    }

    /// @brief writes the remaining data and completes the upload
    arrow::Status close() {
        if (flushBuffer() != 0) {
            return myStatus;
        }
        return myOut->Close();
    }

protected:
    int_type overflow(int_type ch) override {
        if (flushBuffer() != 0) {
            return traits_type::eof();
        }
        if (!traits_type::eq_int_type(ch, traits_type::eof())) {
            *pptr() = traits_type::to_char_type(ch);
            pbump(1);
        }
        return traits_type::not_eof(ch);
    }

    int sync() override {
        return flushBuffer();
    }

    /// @brief only reports the write position (tellp), which the parquet writer needs
    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override {
        if (off != 0 || dir != std::ios_base::cur || (which & std::ios_base::out) == 0) {
            return pos_type(off_type(-1));
        }
        return pos_type(myWritten + (pptr() - pbase()));
    }

private:
    int flushBuffer() {
        const std::ptrdiff_t n = pptr() - pbase();
        if (n > 0) {
            myStatus = myOut->Write(pbase(), n);
            if (!myStatus.ok()) {
                return -1;
            }
            myWritten += n;
            pbump((int) - n);
        }
        return 0;
    }

    std::shared_ptr<arrow::io::OutputStream> myOut;
    std::vector<char> myBuffer;
    arrow::Status myStatus;
    /// @brief the number of bytes passed to the arrow stream
    off_type myWritten = 0;
};


/// @brief the S3 options configured by the SUMO_S3_* environment variables
arrow::fs::S3Options
getS3Options() {
    arrow::fs::S3Options options = arrow::fs::S3Options::Defaults();
    const char* const region = std::getenv("SUMO_S3_REGION");
    if (region != nullptr) {
        options.region = region;
    }
    const char* const endpoint = std::getenv("SUMO_S3_ENDPOINT");
    if (endpoint != nullptr) {
        std::string endpointStr = endpoint;
        for (const std::string scheme : {"http", "https"}) {
            if (endpointStr.compare(0, scheme.size() + 3, scheme + "://") == 0) {
                options.scheme = scheme;
                endpointStr = endpointStr.substr(scheme.size() + 3);
            }
        }
        options.endpoint_override = endpointStr;
    }
    const char* const accessKey = std::getenv("SUMO_S3_ACCESS_KEY");
    const char* const secretKey = std::getenv("SUMO_S3_SECRET_KEY");
    if (accessKey != nullptr && secretKey != nullptr) {
        const char* const sessionToken = std::getenv("SUMO_S3_SESSION_TOKEN");
        options.ConfigureAccessKey(accessKey, secretKey, sessionToken != nullptr ? sessionToken : "");
    }
    return options;
}

}


// ===========================================================================
// OutputDevice_S3::Impl definition
// ===========================================================================
struct OutputDevice_S3::Impl {
    std::unique_ptr<ArrowStreamBuf> buffer;
    std::unique_ptr<std::ostream> stream;
};


// ===========================================================================
// method definitions
// ===========================================================================
OutputDevice_S3::OutputDevice_S3(const std::string& fullName)
    : OutputDevice(0, fullName), myImpl(new Impl()) {
    // the path is bucket/key
    const std::string path = fullName.substr(5);
    const std::string::size_type bucketEnd = path.find('/');
    if (bucketEnd == std::string::npos || bucketEnd == 0 || bucketEnd + 1 == path.size()) {
        throw IOError(TLF("Invalid S3 URL '%', expected s3://bucket/key.", fullName));
    }
    {
        std::lock_guard<std::mutex> lock(gS3InitMutex);
        const arrow::Status status = arrow::fs::EnsureS3Initialized();
        if (!status.ok()) {
            throw IOError(TLF("Could not initialize S3 (%).", status.ToString()));
        }
    }
    const auto fs = arrow::fs::S3FileSystem::Make(getS3Options());
    if (!fs.ok()) {
        throw IOError(TLF("Could not connect to S3 for '%' (%).", fullName, fs.status().ToString()));
    }
    const auto out = (*fs)->OpenOutputStream(path);
    if (!out.ok()) {
        throw IOError(TLF("Could not build output file '%' (%).", fullName, out.status().ToString()));
    }
    myImpl->buffer.reset(new ArrowStreamBuf(*out));
    myImpl->stream.reset(new std::ostream(myImpl->buffer.get()));
}


OutputDevice_S3::~OutputDevice_S3() {
    // we need to cleanup the formatter first, because it still might have cached data
    delete myFormatter;
    myFormatter = nullptr;
    const arrow::Status status = myImpl->buffer->close();
    if (!status.ok()) {
        WRITE_ERRORF(TL("Could not complete the upload of '%' (%)."), myFilename, status.ToString());
    }
}


void
OutputDevice_S3::finalize() {
    std::lock_guard<std::mutex> lock(gS3InitMutex);
    if (arrow::fs::IsS3Initialized()) {
        const arrow::Status status = arrow::fs::FinalizeS3();
        if (!status.ok()) {
            WRITE_WARNINGF(TL("Could not shut down S3 (%)."), status.ToString());
        }
    }
}


std::ostream&
OutputDevice_S3::getOStream() {
    return *myImpl->stream;
}


/****************************************************************************/
