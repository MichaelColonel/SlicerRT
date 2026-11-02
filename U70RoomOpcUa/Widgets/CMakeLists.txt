project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicer${MODULE_NAME}ModuleMRML_INCLUDE_DIRS}
  ${vtkSlicer${MODULE_NAME}ModuleLogic_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
#  qSlicerScadaOpcUaLogic.cxx
#  qSlicerScadaOpcUaLogic.h
  qSlicerSiemensPlcOpcUaWidget.cxx
  qSlicerSiemensPlcOpcUaWidget.h
  qSlicerScadaOpcUaWidget.cxx
  qSlicerScadaOpcUaWidget.h
  qSlicerOpcUaRobotsMessagesListModel.cxx
  qSlicerOpcUaRobotsMessagesListModel.h
  OpcUaModel.cxx
  OpcUaModel.h
  OpcUaTreeItem.cxx
  OpcUaTreeItem.h
  ScadaOpcUaModel.cxx
  ScadaOpcUaModel.h
  ScadaOpcUaTreeItem.cxx
  ScadaOpcUaTreeItem.h
#  opcuamodel.cpp
#  opcuamodel.h
#  treeitem.cpp
#  treeitem.h
  )

set(${KIT}_MOC_SRCS
#  qSlicerScadaOpcUaLogic.h
#  qSlicerScadaOpcUaRobotsControlWidget.h
  qSlicerSiemensPlcOpcUaWidget.h
  qSlicerScadaOpcUaWidget.h
  qSlicerOpcUaRobotsMessagesListModel.h
  OpcUaTreeItem.h
  OpcUaModel.h
  ScadaOpcUaTreeItem.h
  ScadaOpcUaModel.h
#  opcuamodel.h
#  treeitem.h
  )

set(${KIT}_UI_SRCS
  ../Resources/UI/qSlicerScadaOpcUaRobotsControlWidget.ui
  ../Resources/UI/qSlicerSiemensPlcOpcUaWidget.ui
  ../Resources/UI/qSlicerScadaOpcUaWidget.ui
  )

set(${KIT}_RESOURCES
  ../Resources/qSlicer${MODULE_NAME}Module.qrc
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicer${MODULE_NAME}ModuleMRML
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleWidgets(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )
