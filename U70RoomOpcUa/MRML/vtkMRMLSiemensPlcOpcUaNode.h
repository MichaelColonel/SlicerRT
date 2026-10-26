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

  enum CustomSignals : int {
    /// Fired if table robot axis coordinates are modified
    TableRobotAxisCoordsModified = 63322,
    /// Fired if C-Arm robot axis coordinates are modified
    CarmRobotAxisCoordsModified,
  };

  enum ModeType : int {
    UNKNOWN = 0,
    AUTOMATIC_MANUAL = 1,
    SERVICE = 3,
    KUKA_CONTROLLERS = 77,
    ModeType_Last
  };

  enum StatusRtkType : int {
    UNKNOWN_MODE = 1,
    XRAY_EXPOSURE = 2,
    RTK_ERROR = 3,
    STARTUP_MOVING_FROM_LOAD_TO_ISO = 51,
    BOTH_ROBOTS_IN_START_POSITION = 52,
    TABLETOP_MOVING_FROM_LOAD_TO_ISO = 53,
    CHOOSE_ACTION_TABLETOP_ISO_CARM_HOME = 54,
    TABLETOP_CORRECTION_ISO = 55,
    TABLETOP_RETURN_LOAD = 56,
    CARM_MOVING_TO_PLANE1 = 57,
    CHOOSE_ACTION_CARM_IN_PLANE1 = 58,
    CARM_MOVING_FROM_PLANE1_TO_PLANE2 = 59,
    CHOOSE_ACTION_CARM_IN_PLANE2 = 60,
    CARM_MOVING_TO_HOME_FROM_PLANE1_POSITION = 61,
    CARM_MOVING_FROM_PLANE2_TO_PLANE1 = 62,
    EMERGENCY_EVACUATION = 63,
    TABLETOP_IN_EMERGENCY_EVACUATION_POSITION = 64,
    WAITING_FOR_OPERATOR_ACTION = 100,
    ALLOW_T1 = 101,
    ALLOW_XRAY = 102,
    ALLOW_BEAM = 103,
    ROBOTS_READYNESS_CHECK_ACTIVE_PROGRAMM_RESET = 151,
    CARM_MOVING_TO_HOME = 152,
    TABLETOP_MOVING_TO_LOAD = 153,
    OPERATOR_ACTION_CHOOSE = 154,
    CARM_MOVING_TO_LOAD = 155,
    TABLETOP_MOVING_TO_COORDINATES = 156,
    CARM_MOVING_TO_COORDINATES = 157,
    TABLETOP_MASTER_REFERENCE_TEST = 158,
    CARM_MASTER_REFERENCE_TEST = 159,
    StatusRtkType_Last
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

  vtkGetVector2Macro(AutoManualApplyTabletopPosition, bool);
  vtkSetVector2Macro(AutoManualApplyTabletopPosition, bool);

  vtkGetVector2Macro(AutoManualApplyNewCoords, bool);
  vtkSetVector2Macro(AutoManualApplyNewCoords, bool);
  
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

  vtkGetVector2Macro(ServiceR2Position3, bool);
  vtkSetVector2Macro(ServiceR2Position3, bool);

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

  vtkGetMacro(DoorLockSensor1, bool);
  vtkSetMacro(DoorLockSensor1, bool);

  vtkGetMacro(DoorLockSensor2, bool);
  vtkSetMacro(DoorLockSensor2, bool);

  vtkGetMacro(TableTopPosition, uint8_t);
  vtkSetMacro(TableTopPosition, uint8_t);

  vtkGetMacro(EmergencyEvacSignal, bool);
  vtkSetMacro(EmergencyEvacSignal, bool);

  vtkGetMacro(HoldButtonPressedForMovingFlag, bool);
  vtkSetMacro(HoldButtonPressedForMovingFlag, bool);

  vtkGetVector6Macro(ConfirmedCoordsR1, int32_t);
  vtkSetVector6Macro(ConfirmedCoordsR1, int32_t);

  vtkGetVector6Macro(ConfirmedCoordsR2, int32_t);
  vtkSetVector6Macro(ConfirmedCoordsR2, int32_t);

  vtkGetVector6Macro(CurrentCoordsR1, int32_t);
  vtkSetVector6Macro(CurrentCoordsR1, int32_t);

  vtkGetVector6Macro(CurrentCoordsR2, int32_t);
  vtkSetVector6Macro(CurrentCoordsR2, int32_t);

  vtkGetVector6Macro(CurrentAxisCoordsR1, int32_t);
  vtkSetVector6Macro(CurrentAxisCoordsR1, int32_t);

  vtkGetVector6Macro(CurrentAxisCoordsR2, int32_t);
  vtkSetVector6Macro(CurrentAxisCoordsR2, int32_t);

  void GetCoordsAndAngles(double coordsAndAngles[6]);

  void GetCurrentAxisCoordsR1(double angles[6]);
  void GetCurrentAxisCoordsR2(double angles[6]);
  void SetCurrentAxisCoordsR1(double angles[6]);
  void SetCurrentAxisCoordsR2(double angles[6]);

  void GetCurrentCoordsR1(double kukaCoords[6]);
  void GetCurrentCoordsR2(double kukaCoords[6]);
  void GetConfirmedTargetCoordsR1(double kukaCoords[6]);
  void GetConfirmedTargetCoordsR2(double kukaCoords[6]);

protected:
  vtkMRMLSiemensPlcOpcUaNode();
  ~vtkMRMLSiemensPlcOpcUaNode() override;
  vtkMRMLSiemensPlcOpcUaNode(const vtkMRMLSiemensPlcOpcUaNode&);
  void operator=(const vtkMRMLSiemensPlcOpcUaNode&);

  static const char* GetModeAsString(int id);
  static int GetModeFromString(const char* name);
  void SetMode(int id);

private:

  uint64_t ErrorMessages{ 0 }; // Error messages 0...63
  uint64_t ServiceMessages{ 0 }; // Service messages 0...63
  uint64_t MiscMessages{ 0 }; // Miscellaneous messages 0...63

  ModeType Mode{ vtkMRMLSiemensPlcOpcUaNode::ModeType_Last };

  uint16_t StateRtk{ 0 };
  uint16_t StateR1{ 0 };
  uint16_t StateR2{ 0 };
  int32_t CoordsX{ 0 };
  int32_t CoordsY{ 0 };
  int32_t CoordsZ{ 0 };
  int32_t CoordsA{ 0 };
  int32_t CoordsB{ 0 };
  int32_t CoordsC{ 0 };

  bool AutoManualR1LoadToIso[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualR1ToNewCoords[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualR1ToLoad[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualR2ToPlane1[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualR2ToPlane2[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualR2ToHome[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualEmergencyEvac[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualApplyTabletopPosition[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AutoManualApplyNewCoords[2]{ false, false }; // [0] == isEnabled, [1] == isPressed

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
  bool ServiceR2Position3[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool ServiceRestart[2]{ false, false }; // [0] == isEnabled, [1] == isPressed

  bool KukaAllowT1[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool KukaAllowXray[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool KukaAllowBeam[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  
  bool ResetErrors[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool MakeXray[2]{ false, false }; // [0] == isEnabled, [1] == isPressed

  bool RobotsReadyForXray1{ false };
  bool RobotsReadyForXray2{ false };
  bool RobotsReadyForBeam{ false };
  bool PatientOnTableTop{ false };
  uint8_t MlcState{ 0 };
  int32_t MlcSensorValue{ 0 };
  bool MlcIsSafe{ false };
  bool MlcIsOperational{ false };
  bool DoorLockSensor1{ false };
  bool DoorLockSensor2{ false };
  uint8_t TableTopPosition{ 0 };
  bool EmergencyEvacSignal{ false };
  bool HoldButtonPressedForMovingFlag{ false };

  // Tabletop confirmed coords (X,Y,Z,A,B,C)
  int32_t ConfirmedCoordsR1[6] = { 0,0,0,0,0,0 }; // X,Y,Z,A,B,C
  // C-arm confirmed coords (X,Y,Z,A,B,C)
  int32_t ConfirmedCoordsR2[6] = { 0,0,0,0,0,0 }; // X,Y,Z,A,B,C
  // Tabletop current coords (X,Y,Z,A,B,C)
  int32_t CurrentCoordsR1[6] = { 0,0,0,0,0,0 }; // X,Y,Z,A,B,C
  // C-arm current coords (X,Y,Z,A,B,C)
  int32_t CurrentCoordsR2[6] = { 0,0,0,0,0,0 }; // X,Y,Z,A,B,C

  // Tabletop current axis coords (A1,A2,A3,A4,A5,A6)
  // { 150580, -137740, 87920, -3780, -92170, -175870 };
  int32_t CurrentAxisCoordsR1[6] = { -150580, -137740, 87920, -3780, -92170, -175870 }; // A1,A2,A3,A4,A5,A6
  // C-arm current axis coords (A1,A2,A3,A4,A5,A6)
  // { -770, -122030, 142060, 122360, 33380, -130770 };
  int32_t CurrentAxisCoordsR2[6] = { 178770, -122030, 142060, 122360, 33380, -130770 }; // A1,A2,A3,A4,A5,A6
};

#endif
