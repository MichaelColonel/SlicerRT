project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${SlicerRtCommon_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${MRMLCore_INCLUDE_DIRS}
  ${vtkSlicerSegmentationsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerBeamsModuleMRML_INCLUDE_DIRS}
  )

# --------------------------------------------------------------------------
# Sources

set(${KIT}_SRCS
  vtkMRML${MODULE_NAME}Node.cxx
  vtkMRML${MODULE_NAME}Node.h
  vtkMRMLCabin26AGeometryNode.cxx
  vtkMRMLCabin26AGeometryNode.h
  vtkMRMLRTFixedBeamNode.h
  vtkMRMLRTFixedBeamNode.cxx
  vtkMRMLRTFixedIonBeamNode.h
  vtkMRMLRTFixedIonBeamNode.cxx
  vtkMRMLRTCabin26AIonBeamNode.h
  vtkMRMLRTCabin26AIonBeamNode.cxx
  )

SET (${KIT}_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${MRMLCore_INCLUDE_DIRS}
  CACHE INTERNAL "" FORCE)

# --------------------------------------------------------------------------
# Build the library

set(${KIT}_TARGET_LIBRARIES
  vtkSlicerRtCommon
  MRMLCore
  vtkSlicerSegmentationsModuleMRML
  vtkSlicerBeamsModuleMRML
  )

SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )

# Set linker flags, needed for OpenMP
if (NOT ${PLASTIMATCH_LDFLAGS} STREQUAL "")
  set_target_properties (${KIT}
    PROPERTIES LINK_FLAGS ${PLASTIMATCH_LDFLAGS})
endif ()
