# Remove the redundant FIND_LIBRARY call - the main CMakeLists.txt already handles this properly
# if(WITH_PARQUET)
#     FIND_LIBRARY(PARQUET_LIBRARY NAMES parquet)
# endif()

# Build library lists conditionally including Parquet libraries when available
set(parquet_libs "")
if(HAVE_PARQUET AND PARQUET_LIBRARY)
    # Add both Parquet and Arrow libraries as they're needed together
    set(parquet_libs ${PARQUET_LIBRARY})
    
    # Arrow is required for Parquet functionality - add Arrow::arrow_shared which should always be available
    # when Arrow is found via find_package(Arrow)
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

set(netconvertlibs
        netwrite netimport netbuild foreign_eulerspiral ${GDAL_LIBRARY} netimport_vissim netimport_vissim_typeloader netimport_vissim_tempstructs ${commonlibs} ${parquet_libs} ${TCMALLOC_LIBRARY})

set(sumolibs
        traciserver netload microsim_cfmodels microsim_engine microsim_lcmodels microsim_devices microsim_trigger microsim_output microsim_transportables microsim_actions
        microsim_traffic_lights microsim mesosim ${commonvehiclelibs} ${GEOS_LIBRARY} ${parquet_libs})

if (OPENSCENEGRAPH_FOUND)
    set(osgviewlib osgview)
endif ()
set(sumoguilibs
        gui guinetload guisim gui gui_dialogs utils_gui_windows utils_gui_globjects utils_gui_div utils_gui_settings
        utils_gui_images utils_gui_cursors utils_gui_shortcuts utils_tests utils_gui_tracker utils_foxtools utils_traction_wire ${osgviewlib} microsim traciserver libsumoguistatic ${sumolibs} mesogui
        ${GDAL_LIBRARY} ${FFMPEG_LIBRARIES} ${OPENSCENEGRAPH_LIBRARIES} ${GL2PS_LIBRARIES})
if (MSVC)
    set(sumoguilibs ${sumoguilibs} opengl32 glu32)
endif ()

add_subdirectory(activitygen)
add_subdirectory(dfrouter)
add_subdirectory(duarouter)
if (FMI)
    add_subdirectory(fmi)
endif ()
add_subdirectory(foreign)
add_subdirectory(jtrrouter)
add_subdirectory(libsumo)
add_subdirectory(libtraci)
add_subdirectory(marouter)
add_subdirectory(mesosim)
add_subdirectory(microsim)
add_subdirectory(netbuild)
add_subdirectory(netgen)
add_subdirectory(netimport)
add_subdirectory(netload)
add_subdirectory(netwrite)
add_subdirectory(od)
if (OPENSCENEGRAPH_FOUND)
    add_subdirectory(osgview)
endif ()
add_subdirectory(polyconvert)
add_subdirectory(router)
add_subdirectory(tools)
add_subdirectory(traci-server)
add_subdirectory(traci_testclient)
add_subdirectory(utils)
if (FOX_FOUND)
    add_subdirectory(gui)
    add_subdirectory(guinetload)
    add_subdirectory(guisim)
    add_subdirectory(mesogui)
    if (NETEDIT)
        add_subdirectory(netedit)
    endif ()
endif ()

add_executable(sumo sumo_main.cpp)
set_target_properties(sumo PROPERTIES OUTPUT_NAME sumo${BINARY_SUFFIX})
set_target_properties(sumo PROPERTIES OUTPUT_NAME_DEBUG sumo${BINARY_SUFFIX}D)
target_link_libraries(sumo microsim traciserver libsumostatic ${sumolibs} ${TCMALLOC_LIBRARY})

add_dependencies(sumo generate-version-h install_dll)

if (FOX_FOUND)
    add_executable(sumo-gui guisim_main.cpp sumo-gui.rc)
    set_target_properties(sumo-gui PROPERTIES OUTPUT_NAME sumo-gui${BINARY_SUFFIX})
    set_target_properties(sumo-gui PROPERTIES OUTPUT_NAME_DEBUG sumo-gui${BINARY_SUFFIX}D)
    target_link_libraries(sumo-gui ${sumoguilibs} ${TCMALLOC_LIBRARY})
    if (MSVC)
        if (CONSOLE_RELEASE)
            set_target_properties(sumo-gui PROPERTIES LINK_FLAGS_RELEASE "/ENTRY:mainCRTStartup")
        else ()
            set_target_properties(sumo-gui PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
        endif()
        if (FFMPEG_FOUND)
            set_target_properties(sumo-gui PROPERTIES LINK_FLAGS "/SAFESEH:NO")
        endif ()
        if (CMAKE_VERSION VERSION_LESS "3.16.0")
            message(STATUS "DPI awareness for sumo-gui will be disabled. Please consider using CMake version >= 3.16 to enable it")
        else ()
            set_target_properties(sumo-gui PROPERTIES VS_DPI_AWARE "ON")
        endif ()
    endif ()
    add_dependencies(sumo-gui generate-version-h install_dll install_mo)
    install(TARGETS sumo-gui RUNTIME DESTINATION bin)
endif ()

add_executable(netconvert netconvert_main.cpp)
set_target_properties(netconvert PROPERTIES OUTPUT_NAME netconvert${BINARY_SUFFIX})
set_target_properties(netconvert PROPERTIES OUTPUT_NAME_DEBUG netconvert${BINARY_SUFFIX}D)
target_link_libraries(netconvert ${netconvertlibs})
add_dependencies(netconvert generate-version-h install_dll)

add_executable(od2trips od2trips_main.cpp)
set_target_properties(od2trips PROPERTIES OUTPUT_NAME od2trips${BINARY_SUFFIX})
set_target_properties(od2trips PROPERTIES OUTPUT_NAME_DEBUG od2trips${BINARY_SUFFIX}D)
target_link_libraries(od2trips od router ${commonvehiclelibs} ${parquet_libs} ${TCMALLOC_LIBRARY})
add_dependencies(od2trips generate-version-h install_dll)

install(TARGETS sumo netconvert od2trips RUNTIME DESTINATION bin)
install(DIRECTORY libsumo DESTINATION include
        FILES_MATCHING PATTERN "*.h"
        PATTERN "*Helper.h" EXCLUDE
        PATTERN "Subscription.h" EXCLUDE)

configure_file(config.h.cmake config.h)

# obtain version from git folder
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../.git/index")
    set(VERSION_DEP "${CMAKE_CURRENT_SOURCE_DIR}/../.git/index")
endif()

# define version.h dependency
add_custom_command(OUTPUT version.h
                   COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../tools/build_config/version.py ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS ${VERSION_DEP}
                  )
add_custom_target(generate-version-h DEPENDS version.h)
set_property(TARGET generate-version-h PROPERTY FOLDER "CMake")
