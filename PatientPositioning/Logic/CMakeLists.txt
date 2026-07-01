project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${SlicerRtCommon_INCLUDE_DIRS}
  ${vtkSlicer${MODULE_NAME}ModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerSubjectHierarchyModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerBeamsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerBeamsModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerDrrImageComputationModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerDrrImageComputationModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerImagePositioningModuleMRML_INCLUDE_DIRS}
  ${RapidJSON_INCLUDE_DIR}
  )

set(${KIT}_SRCS
  vtkSlicer${MODULE_NAME}Logic.cxx
  vtkSlicer${MODULE_NAME}Logic.h
  vtkSlicerChannel26Cabin3RobotsTransformLogic.cxx
  vtkSlicerChannel26Cabin3RobotsTransformLogic.h
  vtkSlicerChannel26Cabin3RobotsGeometryCommon.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicerBeamsModuleLogic
  vtkSlicerModelsModuleLogic
  vtkSlicerSubjectHierarchyModuleLogic
  vtkSlicerDrrImageComputationModuleMRML
  vtkSlicerImagePositioningModuleMRML
  vtkSlicerRtCommon
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
