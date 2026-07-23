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

#ifndef __vtkMRMLScadaOpcUaNode_h
#define __vtkMRMLScadaOpcUaNode_h

// Beams includes
#include "vtkSlicerImagePositioningModuleMRMLExport.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLModelNode.h>

// STD includes
#include <array>

class vtkMRMLRTBeamNode;
class vtkMRMLTableNode;

class VTK_SLICER_IMAGEPOSITIONING_MODULE_MRML_EXPORT vtkMRMLScadaOpcUaNode : public vtkMRMLNode
{
public:
  static vtkMRMLScadaOpcUaNode *New();
  vtkTypeMacro(vtkMRMLScadaOpcUaNode,vtkMRMLNode);
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
  const char* GetNodeTagName() override { return "ScadaOpcUa"; };

  /// Handles events registered in the observer manager
  void ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData) override;

  vtkGetMacro(Mode, uint8_t);
  vtkSetMacro(Mode, uint8_t);

  vtkGetMacro(Status_RTK, uint16_t);
  vtkSetMacro(Status_RTK, uint16_t);

  vtkGetMacro(Status_R1_deka, uint16_t);
  vtkSetMacro(Status_R1_deka, uint16_t);

  vtkGetMacro(Status_R2_C_Duga, uint16_t);
  vtkSetMacro(Status_R2_C_Duga, uint16_t);

  vtkGetMacro(CoordFromASU_X, int32_t);
  vtkSetMacro(CoordFromASU_X, int32_t);

  vtkGetMacro(CoordFromASU_Y, int32_t);
  vtkSetMacro(CoordFromASU_Y, int32_t);

  vtkGetMacro(CoordFromASU_Z, int32_t);
  vtkSetMacro(CoordFromASU_Z, int32_t);

  vtkGetMacro(CoordFromASU_XRAY_Z_correction, int32_t);
  vtkSetMacro(CoordFromASU_XRAY_Z_correction, int32_t);

  vtkGetMacro(CoordFromASU_AngleA_R1, int32_t);
  vtkSetMacro(CoordFromASU_AngleA_R1, int32_t);

  vtkGetMacro(CoordFromASU_AngleB_R1, int32_t);
  vtkSetMacro(CoordFromASU_AngleB_R1, int32_t);

  vtkGetMacro(CoordFromASU_AngleC_R1, int32_t);
  vtkSetMacro(CoordFromASU_AngleC_R1, int32_t);

  vtkGetVector2Macro(AM_Buttons_R1_LoadToIso, bool);
  vtkSetVector2Macro(AM_Buttons_R1_LoadToIso, bool);

  vtkGetVector2Macro(AM_Buttons_R1_ToNewCoords, bool);
  vtkSetVector2Macro(AM_Buttons_R1_ToNewCoords, bool);

  vtkGetVector2Macro(AM_Buttons_R1_ToHome, bool);
  vtkSetVector2Macro(AM_Buttons_R1_ToHome, bool);

  vtkGetVector2Macro(AM_Buttons_R1_ToLoad, bool);
  vtkSetVector2Macro(AM_Buttons_R1_ToLoad, bool);

  vtkGetVector2Macro(AM_Buttons_R2_ToIso, bool);
  vtkSetVector2Macro(AM_Buttons_R2_ToIso, bool);

  vtkGetVector2Macro(AM_Buttons_R2_To2ndPl, bool);
  vtkSetVector2Macro(AM_Buttons_R2_To2ndPl, bool);

  vtkGetVector2Macro(AM_Buttons_MakeXRay, bool);
  vtkSetVector2Macro(AM_Buttons_MakeXRay, bool);

  vtkGetVector2Macro(AM_Buttons_R2_ToHome, bool);
  vtkSetVector2Macro(AM_Buttons_R2_ToHome, bool);

  vtkGetVector2Macro(AM_Buttons_R2_Set_New_Z, bool);
  vtkSetVector2Macro(AM_Buttons_R2_Set_New_Z, bool);

  vtkGetVector2Macro(MM_Buttons_R1_LoadToIso, bool);
  vtkSetVector2Macro(MM_Buttons_R1_LoadToIso, bool);

  vtkGetVector2Macro(MM_Buttons_R1_ToNewCoords, bool);
  vtkSetVector2Macro(MM_Buttons_R1_ToNewCoords, bool);

  vtkGetVector2Macro(MM_Buttons_R1_ToHome, bool);
  vtkSetVector2Macro(MM_Buttons_R1_ToHome, bool);

  vtkGetVector2Macro(MM_Buttons_R1_ToLoad, bool);
  vtkSetVector2Macro(MM_Buttons_R1_ToLoad, bool);

  vtkGetVector2Macro(MM_Buttons_R2_ToIso, bool);
  vtkSetVector2Macro(MM_Buttons_R2_ToIso, bool);

  vtkGetVector2Macro(MM_Buttons_R2_To2ndPl, bool);
  vtkSetVector2Macro(MM_Buttons_R2_To2ndPl, bool);

  vtkGetVector2Macro(MM_Buttons_MakeXRay, bool);
  vtkSetVector2Macro(MM_Buttons_MakeXRay, bool);

  vtkGetVector2Macro(MM_Buttons_R2_ToHome, bool);
  vtkSetVector2Macro(MM_Buttons_R2_ToHome, bool);

  vtkGetVector2Macro(MM_Buttons_R2_Set_New_Z, bool);
  vtkSetVector2Macro(MM_Buttons_R2_Set_New_Z, bool);

  vtkGetMacro(SysTime, uint64_t);
  vtkSetMacro(SysTime, uint64_t);

  vtkGetMacro(LocalTime, uint64_t);
  vtkSetMacro(LocalTime, uint64_t);

protected:
  vtkMRMLScadaOpcUaNode();
  ~vtkMRMLScadaOpcUaNode() override;
  vtkMRMLScadaOpcUaNode(const vtkMRMLScadaOpcUaNode&);
  void operator=(const vtkMRMLScadaOpcUaNode&);

private:
  uint64_t SysTime{ 0 };
  uint64_t LocalTime{ 0 };
  std::array< bool, 65 > ErrorMessages;
  std::array< bool, 65 > ServMessages;
  std::array< bool, 65 > Messages;
  uint8_t Mode;
  uint16_t Status_RTK;
  uint16_t Status_R1_deka;
  uint16_t Status_R2_C_Duga;
  int32_t CoordFromASU_X;
  int32_t CoordFromASU_Y;
  int32_t CoordFromASU_Z;
  int32_t CoordFromASU_XRAY_Z_correction;
  int32_t CoordFromASU_AngleA_R1;
  int32_t CoordFromASU_AngleB_R1;
  int32_t CoordFromASU_AngleC_R1;

  bool AM_Buttons_R1_LoadToIso[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AM_Buttons_R1_ToNewCoords[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AM_Buttons_R1_ToHome[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AM_Buttons_R1_ToLoad[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AM_Buttons_R2_ToIso[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AM_Buttons_R2_To2ndPl[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AM_Buttons_MakeXRay[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AM_Buttons_R2_ToHome[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool AM_Buttons_R2_Set_New_Z[2]{ false, false }; // [0] == isEnabled, [1] == isPressed

  bool MM_Buttons_R1_LoadToIso[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool MM_Buttons_R1_ToNewCoords[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool MM_Buttons_R1_ToHome[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool MM_Buttons_R1_ToLoad[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool MM_Buttons_R2_ToIso[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool MM_Buttons_R2_To2ndPl[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool MM_Buttons_MakeXRay[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool MM_Buttons_R2_ToHome[2]{ false, false }; // [0] == isEnabled, [1] == isPressed
  bool MM_Buttons_R2_Set_New_Z[2]{ false, false }; // [0] == isEnabled, [1] == isPressed

  struct Button {
    bool IsPressed{ false };
    bool IsEnable{ false };
  };
  struct AutomaticMovement {
    Button R1_LoadToIso;
    Button R1_ToNewCoords;
    Button R1_ToHome;
    Button R1_ToLoad;
    Button R2_ToIso;
    Button R2_To2ndPl;
    Button MakeXRay;
    Button R2_ToHome;
    Button R2_Set_New_Z;
  } AM; // Automatic movement
  struct ManualMovement {
    Button R1_LoadToIso;
    Button R1_ToNewCoords;
    Button R1_ToHome;
    Button R1_ToLoad;
    Button R2_ToIso;
    Button R2_To2ndPl;
    Button MakeXRay;
    Button R2_ToHome;
    Button R2_Set_New_Z;
  } MM; // Manual movement
  struct ServiceMovement {
    Button BreakTestR1;
    Button MasResTestR1;
    Button HomeR1;
    Button IsoR1;
    Button ServicePosR1;
    Button BreakTestR2;
    Button MasResTestR2;
    Button HomeR2;
    Button IsoR2;
    Button ServicePosR2;
    Button RestartSM;
  } SM;
  struct KukaControllersMode {
    Button AllowT1;
    Button AllowXRay;
    Button MakeXRay;
    Button AllowBeam;
  } KCM;
  Button ResetErrors;
  bool RobotsReadyForXray1;
  bool RobotsReadyForXray2;
  bool RobotsReadyForBeam;
  bool HUMAN_ON_DEKA;
  struct MlcPosition {
    uint8_t State;
    int32_t SensorValue;
    bool OperationPosition;
    bool SafePosition;
  } MLC_POSITION;
  int32_t KUKA_X_coordinate_to_TCS_R1;
  int32_t KUKA_Y_coordinate_to_TCS_R1;
  int32_t KUKA_Z_coordinate_to_TCS_R1;
  std::array< int32_t, 6 > AXIS_coord_R1;
  std::array< int32_t, 6 > AXIS_coord_R2;
};

#endif
