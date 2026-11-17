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
#include "vtkSlicerU70RoomOpcUaModuleMRMLExport.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>

// STD includes
#include <array>

class vtkMRMLRTBeamNode;
class vtkMRMLTableNode;

class VTK_SLICER_U70ROOMOPCUA_MODULE_MRML_EXPORT vtkMRMLScadaOpcUaNode : public vtkMRMLNode
{
public:

  enum Events : int {
    ValueChangedEvent = 63332,
    ValueReadEvent
  };

  enum RemoteControlType : int32_t {
    RC_UNDEFINED = 1,
    RC_TEST = 2,
    RC_PATIENT_POSITIONING = 3,
    RC_XRAY_POSITIONING = 4,
    RC_THERAPY_PROCEDURE = 5,
    RemoteControlType_Last,
  };
  enum RemoteStateMachineType : int32_t {
    RSM_UNPOWERED = 1,
    RSM_PATIENT_POSITIONING = 2,
    RSM_XRAY_POSITIONING = 3,
    RSM_THERAPY_READY = 4,
    RSM_THERAPY_PROCEDURE = 5,
    RSM_PAUSE = 6,
    RSM_ACCIDENT_EMERGENCY = 7,
    RemoteStateMachineType_Last,
  };
  enum StateType : int32_t {
    ST_UNDEFINED = 0,
    ST_APPLICATION_IS_RUNNING = 1,
    ST_LOADING_THERAPY_PLAN = 2,
    ST_PATIENT_VERIFICATION = 3,
    ST_PATIENT_IMMOBILIZATION_VERIFICATION = 4,
    ST_PATIENT_POSITIONING = 5,
    ST_XRAY_EXPOSURE = 6,
    ST_XRAY_IMAGE_REGISTRATION = 7,
    ST_PATIENT_POSITION_CORRECTION = 8,
    ST_POSITIONING_FINISHED = 9,
    ST_WAIT_UNTIL_THERAPY_IS_FINISHED = 10,
    ST_PROCEDURE_COMMENTS_AND_RECORD_FINISH = 11,
    StateType_Last,
  };

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

  vtkGetMacro(TimeoutExpired, bool);
  vtkSetMacro(TimeoutExpired, bool);

  vtkGetMacro(SysTimeScada, int32_t);
  vtkSetMacro(SysTimeScada, int32_t);
  
  vtkGetMacro(SysTimeModPos, int32_t);
  vtkSetMacro(SysTimeModPos, int32_t);

  vtkGetMacro(PlanStr, std::string);
  vtkSetMacro(PlanStr, std::string);

  vtkGetMacro(IDP, std::string);
  vtkSetMacro(IDP, std::string);

  vtkGetMacro(ECP, bool);
  vtkSetMacro(ECP, bool);

  vtkGetMacro(Immob, std::string);
  vtkSetMacro(Immob, std::string);

  vtkGetMacro(PlanOpen, bool);
  vtkSetMacro(PlanOpen, bool);

  vtkGetMacro(NPos, int);
  vtkSetMacro(NPos, int);

  vtkGetMacro(RemoteCtrl, int);
  vtkSetMacro(RemoteCtrl, int);

  vtkGetMacro(RemoteStateMachine, int);
  vtkSetMacro(RemoteStateMachine, int);

  vtkGetMacro(RemoteCmd, bool);
  vtkSetMacro(RemoteCmd, bool);

  vtkGetMacro(RemotePause, bool);
  vtkSetMacro(RemotePause, bool);

  vtkGetMacro(RemoteAvr, bool);
  vtkSetMacro(RemoteAvr, bool);

  vtkGetMacro(DoorBlock, bool);
  vtkSetMacro(DoorBlock, bool);

  vtkGetMacro(ReadyEq, bool);
  vtkSetMacro(ReadyEq, bool);

  vtkGetMacro(ReadyPlan, bool);
  vtkSetMacro(ReadyPlan, bool);

  vtkGetMacro(ReadyProc, bool);
  vtkSetMacro(ReadyProc, bool);

  vtkGetMacro(St, StateType);
  vtkSetMacro(St, StateType);

  vtkGetMacro(InQueue, bool);
  vtkSetMacro(InQueue, bool);

  vtkGetMacro(ReadySt, bool);
  vtkSetMacro(ReadySt, bool);

  vtkGetMacro(Block, bool);
  vtkSetMacro(Block, bool);

  vtkGetMacro(CurrentLayer, int);
  vtkSetMacro(CurrentLayer, int);

  vtkGetMacro(MaxLayer, int);
  vtkSetMacro(MaxLayer, int);

  vtkGetMacro(ProcLayer, int);
  vtkSetMacro(ProcLayer, int);

  vtkGetMacro(ProcEnd, bool);
  vtkSetMacro(ProcEnd, bool);

  vtkGetMacro(ProcNote, std::string);
  vtkSetMacro(ProcNote, std::string);

  vtkGetMacro(RecClose, bool);
  vtkSetMacro(RecClose, bool);

  vtkGetMacro(ErrStr, std::string);
  vtkSetMacro(ErrStr, std::string);

  vtkGetMacro(EventStr, std::string);
  vtkSetMacro(EventStr, std::string);

protected:
  vtkMRMLScadaOpcUaNode();
  ~vtkMRMLScadaOpcUaNode() override;
  vtkMRMLScadaOpcUaNode(const vtkMRMLScadaOpcUaNode&);
  void operator=(const vtkMRMLScadaOpcUaNode&);

  static const char* GetStAsString(int32_t id);
  static int32_t GetStFromString(const char* name);
  void SetSt(int32_t id);

private:
  bool TimeoutExpired{ false };
  int32_t SysTimeScada{ 0 };
  int32_t SysTimeModPos{ 0 };
  std::string PlanStr;
  std::string IDP;
  bool ECP{ false };
  std::string Immob;
  bool PlanOpen{ false };
  int32_t NPos{ 0 };
  int32_t RemoteCtrl{ 0 };
  int32_t RemoteStateMachine{ 0 };
  bool RemoteCmd{ false };
  bool RemotePause{ false };
  bool RemoteAvr{ false };
  bool DoorBlock{ false };
  bool ReadyEq{ false };
  bool ReadyPlan{ false };
  bool ReadyProc{ false };
  StateType St{ StateType::ST_UNDEFINED };
  bool InQueue{ false };
  bool ReadySt{ false };
  bool Block{ false };
  int32_t CurrentLayer{ 0 };
  int32_t MaxLayer{ 0 };
  int32_t ProcLayer{ 0 };
  bool ProcEnd{ false };
  std::string ProcNote;
  bool RecClose{ false };
  std::string ErrStr;
  std::string EventStr;
};

#endif
