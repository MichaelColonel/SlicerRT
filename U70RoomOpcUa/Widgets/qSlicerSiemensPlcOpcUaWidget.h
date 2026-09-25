/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerSiemensPlcOpcUaWidget_h
#define __qSlicerSiemensPlcOpcUaWidget_h

// Qt includes
#include <QWidget>
#include <QSharedPointer>
#include <QOpcUaNode>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// U70RoomOpcUaModuke Widgets includes
#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

#include <vtkMRMLSiemensPlcOpcUaNode.h>

class qSlicerSiemensPlcOpcUaWidgetPrivate;
class QOpcUaClient;

class vtkMRMLNode;

class Q_SLICER_MODULE_U70ROOMOPCUA_WIDGETS_EXPORT qSlicerSiemensPlcOpcUaWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
    
public:
  typedef QWidget Superclass;
  qSlicerSiemensPlcOpcUaWidget(QWidget *parent=0);
  ~qSlicerSiemensPlcOpcUaWidget() override;

public slots:
  /// Set SiemensPlcOpcUa MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);
  /// Set SiemensPlcOpcUa client
  void setSiemensPlcOpcUaClient(const QSharedPointer< QOpcUaClient >& opcUaClient);
  /// Update widget GUI from RT Image parameters node
  void updateWidgetFromMRML();
  /// Display table top angles
  void onParseServerInterfacesClicked();
  void onOpcUaClientConnected();
  void onOpcUaClientDisconnected();

  // Siemens PLC OPC UA slots (values, buttons, etc)
  void onOpcUaModeChanged(vtkMRMLSiemensPlcOpcUaNode::ModeType mode);

  // AutomaticManual R1 LoadToIso button
  void onAutoManualR1LoadToIsoPressed();
  void onAutoManualR1LoadToIsoReleased();
  // AutomaticManual R1 ToLoad button
  void onAutoManualR1ToLoadPressed();
  void onAutoManualR1ToLoadReleased();
  // AutomaticManual R1 ToNewCoords button
  void onAutoManualR1ToNewCoordsPressed();
  void onAutoManualR1ToNewCoordsReleased();
  // AutomaticManual R2 ToHome button
  void onAutoManualR2ToHomePressed();
  void onAutoManualR2ToHomeReleased();
  // AutomaticManual R2 ToPlane1 button
  void onAutoManualR2ToPlane1Pressed();
  void onAutoManualR2ToPlane1Released();
  // AutomaticManual R2 ToPlane2 button
  void onAutoManualR2ToPlane2Pressed();
  void onAutoManualR2ToPlane2Released();
  // AutomaticManual R1R2 Emergency Evacuation button
  void onAutoManualR1R2EmerEvacPressed();
  void onAutoManualR1R2EmerEvacReleased();
  // AutomaticManual ApplyTablePosition button
  void onAutoManualApplyTablePositionPressed();
  void onAutoManualApplyTablePositionReleased();

  // Service R1 BreakTest button
  void onServiceR1BreakTestPressed();
  void onServiceR1BreakTestReleased();
  // Service R1 MasterReferenceTest button
  void onServiceR1MasterReferenceTestPressed();
  void onServiceR1MasterReferenceTestReleased();
  // Service R1 Load button
  void onServiceR1LoadPressed();
  void onServiceR1LoadReleased();
  // Service R1 ServicePosition1 button
  void onServiceR1ServicePos1Pressed();
  void onServiceR1ServicePos1Released();
  // Service R1 ServicePosition2 button
  void onServiceR1ServicePos2Pressed();
  void onServiceR1ServicePos2Released();
  // Service R1 ServicePosition3 button
  void onServiceR1ServicePos3Pressed();
  void onServiceR1ServicePos3Released();
  // Service R2 BreakTest button
  void onServiceR2BreakTestPressed();
  void onServiceR2BreakTestReleased();
  // Service R2 MasterReferenceTest button
  void onServiceR2MasterReferenceTestPressed();
  void onServiceR2MasterReferenceTestReleased();
  // Service R2 Home button
  void onServiceR2HomePressed();
  void onServiceR2HomeReleased();
  // Service R2 ServicePosition1 button
  void onServiceR2ServicePos1Pressed();
  void onServiceR2ServicePos1Released();
  // Service R2 ServicePosition2 button
  void onServiceR2ServicePos2Pressed();
  void onServiceR2ServicePos2Released();
  // Service R2 ServicePosition3 button
  void onServiceR2ServicePos3Pressed();
  void onServiceR2ServicePos3Released();
  // Service RestartSM button
  void onServiceRestartSmPressed();
  void onServiceRestartSmReleased();

  // KUKA Controllers Movement
  void onKukaAllowT1Pressed();
  void onKukaAllowT1Released();
  // KUKA Controllers Movement
  void onKukaAllowXrayPressed();
  void onKukaAllowXrayReleased();
  // KUKA Controllers Movement
  void onKukaAllowBeamPressed();
  void onKukaAllowBeamReleased();

  // Reset errors button
  void onResetErrorsPressed();
  void onResetErrorsReleased();
  // MakeXray button
  void onMakeXrayPressed();
  void onMakeXrayReleased();

  void onServerInterfacesRead(QOpcUa::NodeAttributes attr);

protected:
  QScopedPointer<qSlicerSiemensPlcOpcUaWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSiemensPlcOpcUaWidget);
  Q_DISABLE_COPY(qSlicerSiemensPlcOpcUaWidget);
};

#endif
