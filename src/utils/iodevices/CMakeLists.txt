set(utils_iodevices_STAT_SRCS
   CSVFormatter.cpp
   CSVFormatter.h
   OutputDevice.cpp
   OutputDevice.h
   OutputDevice_CERR.cpp
   OutputDevice_CERR.h
   OutputDevice_COUT.cpp
   OutputDevice_COUT.h
   OutputDevice_File.cpp
   OutputDevice_File.h
   OutputDevice_String.cpp
   OutputDevice_String.h
   # OutputDevice_Network.cpp
   # OutputDevice_Network.h
   OutputDevice_Parquet.cpp
   OutputDevice_Parquet.h
   OutputDevice_ParquetUnstructured.cpp
   OutputDevice_ParquetUnstructured.h
   OutputFormatter.h
   ParquetFormatter.h
   ParquetUnstructuredFormatter.h
   PlainXMLFormatter.cpp
   PlainXMLFormatter.h
   StreamDevices.h

)

add_library(utils_iodevices STATIC ${utils_iodevices_STAT_SRCS})
add_dependencies(utils_iodevices generate-version-h)
set_property(TARGET utils_iodevices PROPERTY PROJECT_LABEL "z_utils_iodevices")
