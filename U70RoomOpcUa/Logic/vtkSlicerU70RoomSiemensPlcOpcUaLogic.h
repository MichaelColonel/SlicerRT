/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkSlicerU70RoomSiemensPlcOpcUaLogic - slicer logic class for Siemens PLC OPC UA nodes manipulation
// .SECTION Description

#ifndef __vtkSlicerU70RoomSiemensPlcOpcUaLogic_h
#define __vtkSlicerU70RoomSiemensPlcOpcUaLogic_h

// Slicer includes
#include "vtkMRMLAbstractLogic.h"

// U70Room OPC UA MRML includes
#include <vtkMRMLSiemensPlcOpcUaNode.h>

// STD includes
#include <cstdlib>

#include "vtkSlicerU70RoomOpcUaModuleLogicExport.h"

#include <QSharedPointer>
#include <QStringList>

class QOpcUaClient;
class QOpcUaNode;

class VTK_SLICER_U70ROOMOPCUA_MODULE_LOGIC_EXPORT vtkSlicerU70RoomSiemensPlcOpcUaLogic : public vtkMRMLAbstractLogic
{
public:
  static vtkSlicerU70RoomSiemensPlcOpcUaLogic* New();
  vtkTypeMacro(vtkSlicerU70RoomSiemensPlcOpcUaLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Called by the widget (C++ only; not wrapped for Python)
  std::list< QOpcUaNode* > ServerConnected(const QSharedPointer< QOpcUaClient >& sharedClient,
    const QHash< QString, QString >& hashNodesFullNamesIds);
  void UnregisterOpcUaNode(const std::string& nodeFullName);
  void UnregisterAllOpcUaNodes();
  void UpdateMRMLNodeFromOpcUaNodeValue(vtkMRMLSiemensPlcOpcUaNode*, const std::string& fullNodeName);

  // Public API for other modules (C++ and Python)
  std::vector< std::string > GetRegisteredNodeIds();
  bool ReadNodesValues(const std::list< std::string >& fullNamesList);

  bool WriteMode(vtkMRMLSiemensPlcOpcUaNode::ModeType mode);
  uint8_t GetMode(bool& res);
  bool ReadMode();

  uint16_t GetStatusRtk(bool& res);
  bool ReadStatusRtk();

  uint16_t GetStatusR1(bool& res);
  bool ReadStatusR1();
  uint16_t GetStatusR2(bool& res);
  bool ReadStatusR2();

  bool WriteTabletopOrientation(vtkMRMLSiemensPlcOpcUaNode::TabletopOrientation ttOrientation);
  uint8_t GetTabletopOrientation(bool& res);
  bool ReadTabletopOrientation();

  bool WriteAutoManualR1LoadToIsoPressed(bool);
  bool GetAutoManualR1LoadToIsoPressed(bool& res);
  bool ReadAutoManualR1LoadToIsoPressed();

  bool GetAutoManualR1LoadToIsoEnabled(bool& res);
  bool ReadAutoManualR1LoadToIsoEnabled();

  bool WriteAutoManualR1ToLoadPressed(bool);
  bool GetAutoManualR1ToLoadPressed(bool& res);
  bool ReadAutoManualR1ToLoadPressed();

  bool GetAutoManualR1ToLoadEnabled(bool& res);
  bool ReadAutoManualR1ToLoadEnabled();

  bool WriteAutoManualR1ToNewCoordsPressed(bool);
  bool GetAutoManualR1ToNewCoordsPressed(bool& res);
  bool ReadAutoManualR1ToNewCoordsPressed();

  bool GetAutoManualR1ToNewCoordsEnabled(bool& res);
  bool ReadAutoManualR1ToNewCoordsEnabled();

  bool WriteAutoManualR2ToHomePressed(bool);
  bool GetAutoManualR2ToHomePressed(bool& res);
  bool ReadAutoManualR2ToHomePressed();

  bool GetAutoManualR2ToHomeEnabled(bool& res);
  bool ReadAutoManualR2ToHomeEnabled();

  bool WriteAutoManualR2ToPlane1Pressed(bool);
  bool GetAutoManualR2ToPlane1Pressed(bool& res);
  bool ReadAutoManualR2ToPlane1Pressed();

  bool GetAutoManualR2ToPlane1Enabled(bool& res);
  bool ReadAutoManualR2ToPlane1Enabled();

  bool WriteAutoManualR2ToPlane2Pressed(bool);
  bool GetAutoManualR2ToPlane2Pressed(bool& res);
  bool ReadAutoManualR2ToPlane2Pressed();

  bool GetAutoManualR2ToPlane2Enabled(bool& res);
  bool ReadAutoManualR2ToPlane2Enabled();

  bool WriteAutoManualApplyTabletopOrientationPressed(bool);
  bool GetAutoManualApplyTabletopOrientationPressed(bool& res);
  bool ReadAutoManualApplyTabletopOrientationPressed();

  bool GetAutoManualApplyTabletopOrientationEnabled(bool& res);
  bool ReadAutoManualApplyTabletopOrientationEnabled();

  bool WriteAutoManualEmergencyEvacuationPressed(bool);
  bool GetAutoManualEmergencyEvacuationPressed(bool& res);
  bool ReadAutoManualEmergencyEvacuationPressed();

  bool GetAutoManualEmergencyEvacuationEnabled(bool& res);
  bool ReadAutoManualEmergencyEvacuationEnabled();

  bool WriteResetErrorsPressed(bool);
  bool GetResetErrorsPressed(bool& res);
  bool ReadResetErrorsPressed();

  bool GetResetErrorsEnabled(bool& res);
  bool ReadResetErrorsEnabled();

  bool WriteNewCoordX(int32_t);
  int32_t GetNewCoordX(bool& res);
  bool ReadNewCoordX();

  bool WriteNewCoordY(int32_t);
  int32_t GetNewCoordY(bool& res);
  bool ReadNewCoordY();

  bool WriteNewCoordZ(int32_t);
  int32_t GetNewCoordZ(bool& res);
  bool ReadNewCoordZ();

  bool WriteNewCoordAngleA(int32_t);
  int32_t GetNewCoordAngleA(bool& res);
  bool ReadNewCoordAngleA();

  bool WriteNewCoordAngleB(int32_t);
  int32_t GetNewCoordAngleB(bool& res);
  bool ReadNewCoordAngleB();

  bool WriteNewCoordAngleC(int32_t);
  int32_t GetNewCoordAngleC(bool& res);
  bool ReadNewCoordAngleC();

  double GetCurrentCoordsR1A1(bool& res);
  bool ReadCurrentCoordsR1A1();
  double GetCurrentCoordsR1A2(bool& res);
  bool ReadCurrentCoordsR1A2();
  double GetCurrentCoordsR1A3(bool& res);
  bool ReadCurrentCoordsR1A3();
  double GetCurrentCoordsR1A4(bool& res);
  bool ReadCurrentCoordsR1A4();
  double GetCurrentCoordsR1A5(bool& res);
  bool ReadCurrentCoordsR1A5();
  double GetCurrentCoordsR1A6(bool& res);
  bool ReadCurrentCoordsR1A6();
  double GetCurrentCoordsR2A1(bool& res);
  bool ReadCurrentCoordsR2A1();
  double GetCurrentCoordsR2A2(bool& res);
  bool ReadCurrentCoordsR2A2();
  double GetCurrentCoordsR2A3(bool& res);
  bool ReadCurrentCoordsR2A3();
  double GetCurrentCoordsR2A4(bool& res);
  bool ReadCurrentCoordsR2A4();
  double GetCurrentCoordsR2A5(bool& res);
  bool ReadCurrentCoordsR2A5();
  double GetCurrentCoordsR2A6(bool& res);
  bool ReadCurrentCoordsR2A6();

protected:
  vtkSlicerU70RoomSiemensPlcOpcUaLogic();
  ~vtkSlicerU70RoomSiemensPlcOpcUaLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

private:
  vtkSlicerU70RoomSiemensPlcOpcUaLogic(const vtkSlicerU70RoomSiemensPlcOpcUaLogic&); // Not implemented
  void operator=(const vtkSlicerU70RoomSiemensPlcOpcUaLogic&);            // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal; // For access from the callback function
};

#endif
