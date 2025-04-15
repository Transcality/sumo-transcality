/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
// Copyright (C) 2004-2024 German Aerospace Center (DLR) and others.
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
/// @file    OutputDevice_ParquetUnstructured.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @author  Jakob Erdmann
/// @author  Max Schrader
/// @author  Pranav Sateesh
/// @date    2025
///
// An output device that encapsulates an Parquet file with unstructured format
/****************************************************************************/
#include <config.h>

#ifdef HAVE_PARQUET

#include <iostream>
#include <cstring>
#include <cerrno>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <utils/common/StringUtils.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/MsgHandler.h>

#include "OutputDevice_ParquetUnstructured.h"

#include <arrow/io/file.h>
#include <arrow/util/config.h>

#include <parquet/api/reader.h>
#include <parquet/exception.h>
#include <parquet/stream_reader.h>
#include <parquet/stream_writer.h>

#ifdef HAVE_S3
#include <arrow/filesystem/s3fs.h>
#include "S3Utils.h"
#endif

// ===========================================================================
// method definitions
// ===========================================================================
OutputDevice_ParquetUnstructured::OutputDevice_ParquetUnstructured(const std::string& fullName)
    : OutputDevice(fullName, new ParquetUnstructuredFormatter()), myFullName(fullName) {
    // Default to ZSTD compression but allow for environment variable override
    const char* compressionEnv = std::getenv("SUMO_PARQUET_COMPRESSION");
    parquet::Compression::type compression = parquet::Compression::ZSTD;
    
    if (compressionEnv != nullptr) {
        std::string compressionStr = compressionEnv;
        if (compressionStr == "SNAPPY") {
            compression = parquet::Compression::SNAPPY; // Faster but less compression
            std::cout << "Using SNAPPY compression for Parquet files" << std::endl;
        } else if (compressionStr == "NONE" || compressionStr == "UNCOMPRESSED") {
            compression = parquet::Compression::UNCOMPRESSED; // Maximum speed
            std::cout << "Using NO compression for Parquet files (maximum speed)" << std::endl;
        } else if (compressionStr == "GZIP") {
            compression = parquet::Compression::GZIP; // Better compression but slower
            std::cout << "Using GZIP compression for Parquet files" << std::endl;
        } else if (compressionStr != "ZSTD") {
            std::cerr << "Unknown compression type: " << compressionStr << ", using ZSTD" << std::endl;
        }
    }
    
    // Set the row group size from environment
    const char* rowGroupEnv = std::getenv("SUMO_PARQUET_ROWGROUP_SIZE");
    if (rowGroupEnv != nullptr) {
        try {
            int size = std::stoi(rowGroupEnv);
            if (size > 0) {
                myRowGroupSize = size;
                std::cout << "Using custom row group size: " << myRowGroupSize << std::endl;
            }
        } catch (...) {
            // Ignore invalid values
        }
    }
    
    // Set the buffer size to 1000 for schema inference from first ~1000 rows
    myBufferSize = 1000;
    
    // Apply the compression setting
    builder.compression(compression);
    
    // Use these writer properties for better performance
    builder.version(parquet::ParquetVersion::PARQUET_2_6);
    builder.data_page_version(parquet::ParquetDataPageVersion::V2);
    builder.enable_dictionary(); // Enable dictionary encoding for better compression
    
    // Use large write batch size for better performance
    builder.write_batch_size(10000); // Default is 1000
    
    // Check if this is an S3 URL
    myIsS3 = fullName.substr(0, 5) == "s3://";
}

void OutputDevice_ParquetUnstructured::createNewFile() {
    if (myFile != nullptr) {
        try {
            myFile->Close();
        } catch (...) {
            // Ignore errors on close
        }
        myFile = nullptr;
    }

    auto formatter = dynamic_cast<ParquetUnstructuredFormatter*>(&this->getFormatter());
    if (formatter == nullptr) {
        throw IOError("Formatter is not a ParquetUnstructuredFormatter");
    }

    // Now that we're creating the file, finalize the schema based on buffered rows
    formatter->finalizeSchema();
    
    // If we have buffered rows but schema isn't finalized, force it
    if (!formatter->isSchemaFinalized() && formatter->getBufferedRowCount() > 0) {
        // This will build the schema from buffer
        const auto& nodeVector = formatter->getNodeVector();
        
        // If still not finalized, we can't proceed
        if (!formatter->isSchemaFinalized()) {
            std::cerr << "Warning: Cannot finalize schema for Parquet file: " << myFilename << std::endl;
            return;
        }
    }
    
    // Get the node vector for the schema
    const auto& nodeVector = formatter->getNodeVector();
    
    // Only create the file if we have a valid schema with fields
    if (!formatter->isSchemaFinalized() || formatter->getAllFields().empty() || nodeVector.empty()) {
        std::cerr << "Warning: Cannot create Parquet file with empty schema: " << myFilename << std::endl;
        return;  // Don't create file with an empty schema
    }

    try {
        // Create output stream (either S3 or file-based)
        myFile = createOutputStream();
        if (myFile == nullptr) {
            std::cerr << "Error creating output stream for " << myFilename << std::endl;
            return;
        }
        
        // Use ParquetUnstructuredStream which is optimized for unstructured data
        myStreamDevice = std::make_unique<ParquetUnstructuredStream>(
            parquet::ParquetFileWriter::Open(myFile, 
                std::static_pointer_cast<parquet::schema::GroupNode>(
                    parquet::schema::GroupNode::Make("schema", parquet::Repetition::REQUIRED, nodeVector)
                ), 
                builder.build()
            )
        );
        
        // Reset counters
        myRowsInCurrentGroup = 0;
    } catch (const std::exception& e) {
        std::cerr << "Error creating Parquet file: " << e.what() << std::endl;
        myFile = nullptr;
        myStreamDevice = nullptr;
    }
}

bool OutputDevice_ParquetUnstructured::closeTag(const std::string& comment) {
    UNUSED_PARAMETER(comment);
    
    auto formatter = dynamic_cast<ParquetUnstructuredFormatter*>(&this->getFormatter());
    if (formatter == nullptr) {
        return false;
    }

    // If we're at the root level, just clear the stack
    if (formatter->getDepth() < 2) {
        formatter->clearStack();
        return false;
    }

    // Let the formatter process the tag
    try {
        formatter->closeTag(getOStream());

        // Check various conditions for schema/file management
        const bool needsSchemaFinalization = !formatter->isSchemaFinalized() && 
                                            formatter->getBufferedRowCount() >= myBufferSize;
        const bool needsFileCreation = formatter->isSchemaFinalized() && myFile == nullptr;
        const bool canWriteDirectly = formatter->isSchemaFinalized() && 
                                    myFile != nullptr && 
                                    formatter->getBufferedRowCount() > 0;
        
        // Scenario 1: Schema not finalized yet but buffer threshold reached
        if (needsSchemaFinalization) {
            formatter->finalizeSchema();
            createNewFile();
            std::vector<unstructured_parquet::XMLElement> rows = formatter->consumeBufferedRows();
            writeBufferedRows(rows);
        }
        // Scenario 2: Schema finalized but file not created yet
        else if (needsFileCreation) {
            createNewFile();
            std::vector<unstructured_parquet::XMLElement> rows = formatter->consumeBufferedRows();
            writeBufferedRows(rows);
        }
        // Scenario 3: Schema finalized, file exists, and we have buffered rows
        else if (canWriteDirectly) {
            std::vector<unstructured_parquet::XMLElement> rows = formatter->consumeBufferedRows();
            writeBufferedRows(rows);
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error in OutputDevice_ParquetUnstructured::closeTag: " << e.what() << std::endl;
        return true; // Return true to prevent calling code from breaking
    }
}

OutputDevice_ParquetUnstructured::~OutputDevice_ParquetUnstructured() {
    try {
        // Get the formatter
        auto formatter = dynamic_cast<ParquetUnstructuredFormatter*>(&this->getFormatter());
        if (formatter == nullptr) {
            return;
        }
        
        // Process any remaining buffered rows
        if (formatter->getBufferedRowCount() > 0) {
            // Try to build the schema from buffered rows if needed
            if (!formatter->isSchemaFinalized()) {
                formatter->finalizeSchema();
                
                // If schema is still empty, try to build from node vector
                if (formatter->getAllFields().empty()) {
                    const auto& nodeVector = formatter->getNodeVector();
                    if (formatter->getAllFields().empty() || nodeVector.empty()) {
                        return; // Can't proceed with empty schema
                    }
                }
            }
            
            // Create file if needed
            if (myFile == nullptr) {
                try {
                    createNewFile();
                    if (myFile == nullptr) {
                        return; // File creation failed
                    }
                } catch (const std::exception&) {
                    return; // Error in file creation
                }
            }
            
            // Write buffered rows
            std::vector<unstructured_parquet::XMLElement> rows = formatter->consumeBufferedRows();
            writeBufferedRows(rows);
            
            // End the final row group if needed
            auto parquetStream = dynamic_cast<ParquetUnstructuredStream*>(myStreamDevice.get());
            if (parquetStream != nullptr && myRowsInCurrentGroup > 0) {
                try {
                    parquetStream->endRowGroup();
                } catch (const std::exception&) {
                    // Ignore errors on ending row group
                }
            }
        }

        // Clean up resources
        myStreamDevice.reset();
        
        if (myFile != nullptr) {
            try {
                auto status = myFile->Close();
                (void)status; // Explicitly acknowledge we're ignoring the return value
            } catch (const std::exception&) {
                // Ignore errors on close
            }
            myFile.reset();
        }
        
#ifdef HAVE_S3
        // Release S3 filesystem resources
        if (myIsS3 && myS3FileSystem != nullptr) {
            myS3FileSystem.reset();
        }
#endif

        // Simple stats output
        if (myTotalRowsWritten > 0) {
            std::cout << "Wrote " << myTotalRowsWritten << " rows to " << myFilename << std::endl;
        }
        
    } catch (const std::exception&) {
        // Ignore all exceptions in destructor
    }
}

// Create a dummy stream that will discard all output for safety
static OStreamDevice s_dummyStream(std::make_unique<std::ostringstream>().release());

StreamDevice& OutputDevice_ParquetUnstructured::getOStream() {
    // If the stream doesn't exist yet, return a dummy stream that discards output
    if (myStreamDevice == nullptr) {
        return s_dummyStream;
    }
    return *myStreamDevice;
}

// New helper method to efficiently write buffered rows (add before writeRow method)
void OutputDevice_ParquetUnstructured::writeBufferedRows(const std::vector<unstructured_parquet::XMLElement>& rows) {
    // Skip if there are no rows to write
    if (rows.empty()) return;
    
    // Get the stream device - fast-fail if not available
    auto parquetStream = dynamic_cast<ParquetUnstructuredStream*>(myStreamDevice.get());
    if (!parquetStream) return;

    // Process only non-container elements
    static const std::unordered_set<std::string> skipElements = {
        "timestep", "interval", "meandata", "data", "summary", "step", "fcd-export"
    };
    
    // Write non-container elements directly
    for (auto& row : const_cast<std::vector<unstructured_parquet::XMLElement>&>(rows)) {
        // Skip container elements
        if (skipElements.count(row.getName()) > 0) {
            continue;
        }
        writeRow(row);
    }
}

// Optimized writeRow method with better performance
void OutputDevice_ParquetUnstructured::writeRow(unstructured_parquet::XMLElement& row) {
    // Fast fail conditions
    auto formatter = dynamic_cast<ParquetUnstructuredFormatter*>(&this->getFormatter());
    auto parquetStream = dynamic_cast<ParquetUnstructuredStream*>(myStreamDevice.get());
    if (!formatter || !formatter->isSchemaFinalized() || !parquetStream) {
        return;
    }

    // Skip known container elements
    static const std::unordered_set<std::string> skipElements = {
        "timestep", "interval", "meandata", "data", "summary", "step", "fcd-export"
    };
    if (skipElements.count(row.getName()) > 0) {
        return;
    }

    try {
        // Get schema information
        const std::set<std::string>& knownFields = formatter->getAllFields();
        int columnCount = parquetStream->getColumnCount();
        
        // Pre-allocate attributes array with nulls and build attribute map for O(1) lookups
        std::vector<const unstructured_parquet::AttributeBase*> orderedAttributes(columnCount, nullptr);
        std::unordered_map<std::string, const unstructured_parquet::AttributeBase*> attrMap;
        
        // Build attribute map
        for (const auto& attr : row.getAttributes()) {
            if (knownFields.find(attr->getName()) != knownFields.end()) {
                attrMap[attr->getName()] = attr.get();
            }
        }
        
        // Get column names once and organize attributes
        const auto& columnNames = parquetStream->getColumnNames();
        for (int i = 0; i < columnCount && i < static_cast<int>(columnNames.size()); i++) {
            auto it = attrMap.find(columnNames[i]);
            if (it != attrMap.end()) {
                orderedAttributes[i] = it->second;
            }
        }
        
        // Write attributes efficiently
        for (int i = 0; i < columnCount; i++) {
            // Set column position explicitly
            parquetStream->setColumnIndex(i);
            
            // Write attribute or null
            if (orderedAttributes[i]) {
                orderedAttributes[i]->print(*myStreamDevice);
            } else {
                parquetStream->writeNullOrDefault(i);
            }
        }
        
        // Finish the row and update counters
        myStreamDevice->endLine();
        myRowsInCurrentGroup++;
        myTotalRowsWritten++;
        
        // Create a new row group if needed
        if (myRowsInCurrentGroup >= myRowGroupSize) {
            parquetStream->endRowGroup();
            myRowsInCurrentGroup = 0;
        }
    } catch (const std::exception&) {
        // Just continue without error logging for speed
    }
}

std::shared_ptr<arrow::io::OutputStream> OutputDevice_ParquetUnstructured::createOutputStream() {
    // If it's an S3 URL, create S3 output stream
    if (myIsS3) {
#ifdef HAVE_S3
        // Parse S3 URL
        std::string bucket, key;
        if (!parseS3Url(this->myFilename, bucket, key)) {
            throw IOError("Invalid S3 URL format: " + this->myFilename);
        }
        
        // Initialize S3 filesystem if not already done
        if (myS3FileSystem == nullptr) {
            try {
                // First ensure S3 is initialized
                auto s3InitStatus = arrow::fs::EnsureS3Initialized();
                if (!s3InitStatus.ok()) {
                    throw IOError("Failed to initialize S3: " + s3InitStatus.ToString());
                }
                
                // Mark that S3 was initialized globally
                std::lock_guard<std::mutex> lock(sumo::s3::s3FinalizationMutex);
                sumo::s3::s3WasInitialized = true;
                
                // Get S3 options from environment variables
                arrow::fs::S3Options options = arrow::fs::S3Options::Defaults();
                
                // Configure S3 with minimal logging
                const char* region = std::getenv("SUMO_S3_REGION");
                if (region != nullptr) {
                    options.region = region;
                }
                
                const char* endpoint = std::getenv("SUMO_S3_ENDPOINT");
                if (endpoint != nullptr) {
                    options.endpoint_override = endpoint;
                } else if (!options.region.empty()) {
                    // If no endpoint specified but region is set, construct a regional endpoint
                    options.endpoint_override = "s3." + options.region + ".amazonaws.com";
                }
                
                // Enable automatic address resolving (helps with 301 redirects)
                options.request_timeout = std::chrono::duration<double>(std::chrono::seconds(60)).count();
                options.connect_timeout = std::chrono::duration<double>(std::chrono::seconds(30)).count();
                
                // Check for access key and secret key in environment
                const char* accessKey = std::getenv("SUMO_S3_ACCESS_KEY");
                const char* secretKey = std::getenv("SUMO_S3_SECRET_KEY");
                const char* sessionToken = std::getenv("SUMO_S3_SESSION_TOKEN");
                
                if (accessKey != nullptr && secretKey != nullptr) {
                    // Use explicit credentials
                    options.ConfigureAccessKey(
                        accessKey, 
                        secretKey, 
                        sessionToken != nullptr ? sessionToken : ""
                    );
                }
                
                // Create S3 filesystem
                auto result = arrow::fs::S3FileSystem::Make(options);
                if (!result.ok()) {
                    throw IOError("Failed to create S3 filesystem: " + result.status().ToString());
                }
                
                myS3FileSystem = result.ValueOrDie();
            } catch (const std::exception& e) {
                throw IOError("Error creating S3 filesystem: " + std::string(e.what()));
            }
        }
        
        // Create output stream - construct the full S3 path with bucket name
        std::string s3Path = bucket + "/" + key;
        auto result = myS3FileSystem->OpenOutputStream(s3Path);
        if (!result.ok()) {
            throw IOError("Could not open S3 output stream: " + result.status().ToString());
        }
        
        return result.ValueOrDie();
#else
        throw IOError("S3 support not enabled in this build");
#endif
    } else {
        // Regular file output
        auto result = arrow::io::FileOutputStream::Open(this->myFilename);
        if (!result.ok()) {
            throw IOError("Could not build output file '" + this->myFullName + "' (" + 
                          std::strerror(errno) + ").");
        }
        return result.ValueOrDie();
    }
}

bool OutputDevice_ParquetUnstructured::parseS3Url(const std::string& url, std::string& bucket, std::string& key) {
    // Simple S3 URL parser (s3://bucket/key)
    if (url.substr(0, 5) != "s3://") {
        return false;
    }
    
    size_t bucketStart = 5;  // After "s3://"
    size_t bucketEnd = url.find('/', bucketStart);
    
    if (bucketEnd == std::string::npos) {
        // URL format is s3://bucket with no key
        bucket = url.substr(bucketStart);
        key = "";
    } else {
        // URL format is s3://bucket/key
        bucket = url.substr(bucketStart, bucketEnd - bucketStart);
        key = url.substr(bucketEnd + 1);
    }
    
    // Error checking - bucket should not be empty for writing
    return !bucket.empty();
}

#endif
/****************************************************************************/