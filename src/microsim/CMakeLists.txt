add_subdirectory(actions)
add_subdirectory(cfmodels)
add_subdirectory(devices)
add_subdirectory(engine)
add_subdirectory(lcmodels)
add_subdirectory(logging)
add_subdirectory(output)
add_subdirectory(traffic_lights)
add_subdirectory(transportables)
add_subdirectory(trigger)

set(microsim_STAT_SRCS
   MSBitSetLogic.h
   MSBaseVehicle.cpp
   MSBaseVehicle.h
   MSEdge.cpp
   MSEdge.h
   MSEdgeControl.cpp
   MSEdgeControl.h
   MSEdgeWeightsStorage.cpp
   MSEdgeWeightsStorage.h
   MSEventControl.cpp
   MSEventControl.h
   MSDynamicShapeUpdater.cpp
   MSDynamicShapeUpdater.h
   MSFrame.cpp
   MSFrame.h
   MSGlobals.cpp
   MSGlobals.h
   MSInsertionControl.cpp
   MSInsertionControl.h
   MSInternalJunction.cpp
   MSInternalJunction.h
   MSJunction.cpp
   MSJunction.h
   MSJunctionControl.cpp
   MSJunctionControl.h
   MSJunctionLogic.cpp
   MSJunctionLogic.h
   MSLane.cpp
   MSLane.h
   MSLaneChanger.cpp
   MSLaneChanger.h
   MSLaneChangerSublane.cpp
   MSLaneChangerSublane.h
   MSLink.cpp
   MSLink.h
   MSLogicJunction.cpp
   MSLogicJunction.h
   MSMoveReminder.cpp
   MSMoveReminder.h
   MSNet.cpp
   MSNet.h
   MSNoLogicJunction.cpp
   MSNoLogicJunction.h
   MSRightOfWayJunction.cpp
   MSRightOfWayJunction.h
   MSRoute.cpp
   MSRoute.h
   MSRouteHandler.cpp
   MSRouteHandler.h
   MSRouterDefs.h
   MSStop.cpp
   MSStop.h
   MSStoppingPlace.cpp
   MSStoppingPlace.h
   MSParkingArea.cpp
   MSParkingArea.h
   MSVehicle.cpp
   MSVehicle.h
   MSLeaderInfo.cpp
   MSLeaderInfo.h
   MSVehicleContainer.cpp
   MSVehicleContainer.h
   MSVehicleControl.cpp
   MSVehicleControl.h
   MSVehicleTransfer.cpp
   MSVehicleTransfer.h
   MSVehicleType.cpp
   MSVehicleType.h
   MSStateHandler.h
   MSStateHandler.cpp
   MSDriverState.h
   MSDriverState.cpp
   Command_RouteReplacement.h
   Command_RouteReplacement.cpp
   MSTrainHelper.cpp
   MSTrainHelper.h
)

add_library(microsim STATIC ${microsim_STAT_SRCS})
add_dependencies(microsim generate-version-h)
set_property(TARGET microsim PROPERTY PROJECT_LABEL "z_microsim")
