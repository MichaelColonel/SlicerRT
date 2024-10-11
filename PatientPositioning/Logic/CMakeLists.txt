project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerSubjectHierarchyModuleLogic_INCLUDE_DIRS}
  ${vtkSlicerBeamsModuleMRML_INCLUDE_DIRS}
  ${vtkSlicerBeamsModuleLogic_INCLUDE_DIRS}
  ${RapidJSON_INCLUDE_DIR}
  )

set(${KIT}_SRCS
  vtkSlicer${MODULE_NAME}Logic.cxx
  vtkSlicer${MODULE_NAME}Logic.h
  vtkSlicerTableTopRobotTransformLogic.cxx
  vtkSlicerTableTopRobotTransformLogic.h
#  vtkSlicerFixedReferenceBeamsLogic.cxx
#  vtkSlicerFixedReferenceBeamsLogic.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicerBeamsModuleLogic
  vtkSlicerModelsModuleLogic
  vtkSlicerSubjectHierarchyModuleLogic
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
