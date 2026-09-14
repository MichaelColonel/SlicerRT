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

#ifndef __vtkMRMLSiemensPlcOpcUaNode_h
#define __vtkMRMLSiemensPlcOpcUaNode_h

// U70RoomOpcUa includes
#include "vtkSlicerU70RoomOpcUaModuleMRMLExport.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLModelNode.h>

// STD includes
#include <array>

class vtkMRMLRTBeamNode;
class vtkMRMLTableNode;

class VTK_SLICER_U70ROOMOPCUA_MODULE_MRML_EXPORT vtkMRMLSiemensPlcOpcUaNode : public vtkMRMLNode
{
public:
  static constexpr size_t MESSAGES_SIZE = 64;
  enum ModeType : int {
    UNKNOWN = 0,
    AUTOMATIC_MANUAL = 1,
    SERVICE = 3,
    KUKA_CONTROLLERS = 77,
    ModeType_Last
  };

  static vtkMRMLSiemensPlcOpcUaNode *New();
  vtkTypeMacro(vtkMRMLSiemensPlcOpcUaNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Create instance of a GAD node. 
  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes from name/value pairs 
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format. 
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object 
  void Copy(vtkMRMLNode *node) override;

  /// Copy node content (excludes basic data, such a name and node reference)
  vtkMRMLCopyContentMacro(vtkMRMLScadaOpcUaNode);

  /// Get unique node XML tag name
  const char* GetNodeTagName() override { return "SiemensPlcOpcUa"; };

  /// Handles events registered in the observer manager
  void ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData) override;


  vtkGetMacro(ErrorMessages, uint64_t);
  vtkSetMacro(ErrorMessages, uint64_t);
  vtkGetMacro(ServiceMessages, uint64_t);
  vtkSetMacro(ServiceMessages, uint64_t);
  vtkGetMacro(MiscMessages, uint64_t);
  vtkSetMacro(MiscMessages, uint64_t);

  vtkGetMacro(Mode, ModeType);
  vtkSetMacro(Mode, ModeType);

  vtkGetMacro(StateRtk, uint16_t);
  vtkSetMacro(StateRtk, uint16_t);

  vtkGetMacro(StateR1, uint16_t);
  vtkSetMacro(StateR1, uint16_t);

  vtkGetMacro(StateR2, uint16_t);
  vtkSetMacro(StateR2, uint16_t);

  vtkGetMacro(CoordsX, int32_t);
  vtkSetMacro(CoordsX, int32_t);

  vtkGetMacro(CoordsY, int32_t);
  vtkSetMacro(CoordsY, int32_t);

  vtkGetMacro(CoordsZ, int32_t);
  vtkSetMacro(CoordsZ, int32_t);

  vtkGetMacro(CoordsXrayCorrectionZ, int32_t);
  vtkSetMacro(CoordsXrayCorrectionZ, int32_t);

  vtkGetMacro(CoordsA, int32_t);
  vtkSetMacro(CoordsA, int32_t);

  vtkGetMacro(CoordsB, int32_t);
  vtkSetMacro(CoordsB, int32_t);

  vtkGetMacro(CoordsC, int32_t);
  vtkSetMacro(CoordsC, int32_t);

  vtkGetVector2Macro(AutoManualR1LoadToIso, bool);
  vtkSetVector2Macro(AutoManualR1LoadToIso, bool);

  vtkGetVector2Macro(AutoManualR1ToNewCoords, bool);
  vtkSetVector2Macro(AutoManualR1ToNewCoords, bool);

  vtkGetVector2Macro(AutoManualR1ToLoad, bool);
  vtkSetVector2Macro(AutoManualR1ToLoad, bool);

  vtkGetVector2Macro(AutoManualR2ToPlane1, bool);
  vtkSetVector2Macro(AutoManualR2ToPlane1, bool);

  vtkGetVector2Macro(AutoManualR2ToPlane2, bool);
  vtkSetVector2Macro(AutoManualR2ToPlane2, bool);

  vtkGetVector2Macro(AutoManualR2ToHome, bool);
  vtkSetVector2Macro(AutoManualR2ToHome, bool);

  vtkGetVector2Macro(AutoManualEmergencyEvac, bool);
  vtkSetVector2Macro(AutoManualEmergencyEvac, bool);

  vtkGetVector2Macro(AutoManualApplyTableTopPosition, bool);
  vtkSetVector2Macro(AutoManualApplyTableTopPosition, bool);

  vtkGetVector2Macro(ServiceR1BreakTest, bool);
  vtkSetVector2Macro(ServiceR1BreakTest, bool);

  vtkGetVector2Macro(ServiceR1MasterReferenceTest, bool);
  vtkSetVector2Macro(ServiceR1MasterReferenceTest, bool);

  vtkGetVector2Macro(ServiceR1Load, bool);
  vtkSetVector2Macro(ServiceR1Load, bool);

  vtkGetVector2Macro(ServiceR1Position1, bool);
  vtkSetVector2Macro(ServiceR1Position1, bool);

  vtkGetVector2Macro(ServiceR1Position2, bool);
  vtkSetVector2Macro(ServiceR1Position2, bool);

  vtkGetVector2Macro(ServiceR1Position3, bool);
  vtkSetVector2Macro(ServiceR1Position3, bool);

  vtkGetVector2Macro(ServiceR2BreakTest, bool);
  vtkSetVector2Macro(ServiceR2BreakTest, bool);

  vtkGetVector2Macro(ServiceR2MasterReferenceTest, bool);
  vtkSetVector2Macro(ServiceR2MasterReferenceTest, bool);

  vtkGetVector2Macro(ServiceR2Home, bool);
  vtkSetVector2Macro(ServiceR2Home, bool);

  vtkGetVector2Macro(ServiceR2Position1, bool);
  vtkSetVector2Macro(ServiceR2Position1, bool);

  vtkGetVector2Macro(ServiceR2Position2, bool);
  vtkSetVector2Macro(ServiceR2Position2, bool);

  vtkGetVector2Macro(ServiceRestart, bool);
  vtkSetVector2Macro(ServiceRestart, bool);

  vtkGetVector2Macro(KukaAllowT1, bool);
  vtkSetVector2Macro(KukaAllowT1, bool);

  vtkGetVector2Macro(KukaAllowXray, bool);
  vtkSetVector2Macro(KukaAllowXray, bool);

  vtkGetVector2Macro(KukaAllowBeam, bool);
  vtkSetVector2Macro(KukaAllowBeam, bool);

  vtkGetVector2Macro(ResetErrors, bool);
  vtkSetVector2Macro(ResetErrors, bool);

  vtkGetVector2Macro(MakeXray, bool);
  vtkSetVector2Macro(MakeXray, bool);

  vtkGetMacro(RobotsReadyForXray1, bool);
  vtkSetMacro(RobotsReadyForXray1, bool);

  vtkGetMacro(RobotsReadyForXray2, bool);
  vtkSetMacro(RobotsReadyForXray2, bool);

  vtkGetMacro(RobotsReadyForBeam, bool);
  vtkSetMacro(RobotsReadyForBeam, bool);

  vtkGetMacro(PatientOnTableTop, bool);
  vtkSetMacro(PatientOnTableTop, bool);

  vtkGetMacro(MlcState, uint8_t);
  vtkSetMacro(MlcState, uint8_t);

  vtkGetMacro(MlcSensorValue, int32_t);
  vtkSetMacro(MlcSensorValue, int32_t);

  vtkGetMacro(MlcIsSafe, bool);
  vtkSetMacro(MlcIsSafe, bool);

  vtkGetMacro(MlcIsOperational, bool);
  vtkSetMacro(MlcIsOperational, bool);

  vtkGetMacro(DoorLockSensor, bool);
  vtkSetMacro(DoorLockSensor, bool);

  vtkGetMacro(DoorLockSensor2, bool);
  vtkSetMacro(DoorLockSensor2, bool);

  vtkGetMacro(TableTopPosition, uint8_t);
  vtkSetMacro(TableTopPosition, uint8_t);

  vtkGetVector3Macro(KukaCoordsToTcsR1, int32_t);
  vtkSetVector3Macro(KukaCoordsToTcsR1, int32_t);

  vtkGetVector3Macro(KukaAngleToTcsR1, int32_t);
  vtkSetVector3Macro(KukaAngleToTcsR1, int32_t);

  vtkGetVector6Macro(AxisCoordsR1, int32_t);
  vtkSetVector6Macro(AxisCoordsR1, int32_t);

  vtkGetVector3Macro(KukaCoordsToTcsR2, int32_t);
  vtkSetVector3Macro(KukaCoordsToTcsR2, int32_t);

  vtkGetVector3Macro(KukaAngleToTcsR2, int32_t);
  vtkSetVector3Macro(KukaAngleToTcsR2, int32_t);

  vtkGetVector6Macro(AxisCoordsR2, int32_t);
  vtkSetVector6Macro(AxisCoordsR2, int32_t);

protected:
  vtkMRMLSiemensPlcOpcUaNode();
  ~vtkMRMLSiemensPlcOpcUaNode() override;
  vtkMRMLSiemensPlcOpcUaNode(const vtkMRMLSiemensPlcOpcUaNode&);
  void operator=(const vtkMRMLSiemensPlcOpcUaNode&);

  static const char* GetModeAsString(int id);
  static int GetModeFromString(const char* name);
  void SetMode(int id);

private:

  uint64_t ErrorMessages; // Error messages 0...63
  uint64_t ServiceMessages; // Service messages 0...63
  uint64_t MiscMessages; // Miscellaneous messages 0...63

  ModeType Mode{ vtkMRMLSiemensPlcOpcUaNode::UNKNOWN };

  uint16_t StateRtk;
  uint16_t StateR1;
  uint16_t StateR2;
  int32_t CoordsX;
  int32_t CoordsY;
  int32_t CoordsZ;
  int32_t CoordsXrayCorrectionZ;
  int32_t CoordsA;
  int32_t CoordsB;
  int32_t CoordsC;

  bool AutoManualR1LoadToIso[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualR1ToNewCoords[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualR1ToLoad[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualR2ToPlane1[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualR2ToPlane2[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualR2ToHome[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualEmergencyEvac[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualApplyTableTopPosition[2]{ false, false }; // [0] == isEnabled, [1] == isPressed

  bool ServiceR1BreakTest[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool ServiceR1MasterReferenceTest[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool ServiceR1Load[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool ServiceR1Position1[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool ServiceR1Position2[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool ServiceR1Position3[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool ServiceR2BreakTest[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool ServiceR2MasterReferenceTest[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool ServiceR2Home[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool ServiceR2Position1[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool ServiceR2Position2[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool ServiceRestart[2]{ false, false }; // [0] == isEnabled, [1] == isPressed

  bool KukaAllowT1[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool KukaAllowXray[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool KukaAllowBeam[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  
  bool ResetErrors[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool MakeXray[2]{ false, false }; // [0] == isEnabled, [1] == isPressed

  bool RobotsReadyForXray1;
  bool RobotsReadyForXray2;
  bool RobotsReadyForBeam;
  bool PatientOnTableTop;
  uint8_t MlcState;
  int32_t MlcSensorValue;
  bool MlcIsSafe;
  bool MlcIsOperational;
  bool DoorLockSensor;
  bool DoorLockSensor2;
  uint8_t TableTopPosition;

  // Table top robot
  int32_t KukaCoordsToTcsR1[3]; // X,Y,Z
  int32_t KukaAngleToTcsR1[3]; // A,B,C
  int32_t AxisCoordsR1[6]; // A1, A2, A3, A4, A5, A6
  // C-arm robot
  int32_t KukaCoordsToTcsR2[3]; // X,Y,Z
  int32_t KukaAngleToTcsR2[3]; // A,B,C
  int32_t AxisCoordsR2[6]; // A1, A2, A3, A4, A5, A6
  
};

#endif
