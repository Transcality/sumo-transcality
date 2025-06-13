# Define parquet_libs locally for unit tests
set(parquet_libs "")
if(HAVE_PARQUET AND PARQUET_LIBRARY)
    # Add both Parquet and Arrow libraries as they're needed together
    set(parquet_libs ${PARQUET_LIBRARY})
    
    # Arrow is required for Parquet functionality
    if(TARGET Arrow::arrow_shared)
        list(APPEND parquet_libs Arrow::arrow_shared)
    elseif(TARGET Arrow::arrow_static)
        list(APPEND parquet_libs Arrow::arrow_static)
    elseif(TARGET arrow_shared)
        list(APPEND parquet_libs arrow_shared)
    else()
        message(WARNING "Arrow target not found - Parquet linking may fail")
    endif()
endif()

function(setTestProperties targetname)
    set_target_properties(${targetname} PROPERTIES OUTPUT_NAME ${targetname}${BINARY_SUFFIX})
    set_target_properties(${targetname} PROPERTIES OUTPUT_NAME_DEBUG ${targetname}${BINARY_SUFFIX}D)
    add_dependencies(${targetname} install_dll install_test_dll)
    set_property(TARGET ${targetname} PROPERTY FOLDER "test_exe")
    add_test(${targetname} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${targetname})
    target_link_libraries(${targetname} ${ARGN} ${commonlibs} ${parquet_libs} ${TCMALLOC_LIBRARY} ${GTEST_BOTH_LIBRARIES})
endfunction()

add_subdirectory(utils)
add_subdirectory(microsim)
add_subdirectory(netbuild)
