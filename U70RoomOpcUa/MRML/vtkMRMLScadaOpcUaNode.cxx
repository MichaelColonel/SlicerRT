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

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>

#include "vtkMRMLScadaOpcUaNode.h"

// STD include
#include <cstring>
#include <climits>
#include <bitset>
#include <cmath>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLScadaOpcUaNode);

//----------------------------------------------------------------------------
vtkMRMLScadaOpcUaNode::vtkMRMLScadaOpcUaNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLScadaOpcUaNode::~vtkMRMLScadaOpcUaNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLScadaOpcUaNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);
  // add new parameters here
  vtkMRMLWriteXMLIntMacro(sysTime, SysTimeScada);
  vtkMRMLWriteXMLIntMacro(localTime, SysTimeModPos);
  vtkMRMLWriteXMLStdStringMacro(planStr, PlanStr);
  vtkMRMLWriteXMLEnumMacro(st, St);
/*
  uint64_t SysTimeScada{ 0 };
  uint64_t SysTimeModPos{ 0 };
  std::string PlanStr;
  std::string IDP;
  bool ECP{ false };
  std::string Immob;
  bool PlanOpen{ false };
  int NPos{ 0 };
  int RemoteCtrl{ 0 };
  int RemoteStateMachine{ 0 };
  bool RemoteCmd{ false };
  bool RemotePause{ false };
  bool RemoteAvr{ false };
  bool DoorLock{ false };
  bool ReadyEq{ false };
  bool ReadyPlan{ false };
  bool ReadyProc{ false };
  int St{ 0 };
  bool inQueue{ false };
  bool ReadySt{ false };
  bool Block{ false };
  int CurrentLayer{ 0 };
  int MaxLayer{ 0 };
  int ProcLayer{ 0 };
  bool ProcEnd{ false };
  std:;string ProcNote;
  bool ProcClose{ false };
  std::string ErrStr;
  std::string EventStr;
*/
  vtkMRMLWriteXMLEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLScadaOpcUaNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);

  // add new parameters here
  vtkMRMLReadXMLIntMacro(sysTime, SysTimeScada);
  vtkMRMLReadXMLIntMacro(localTime, SysTimeModPos);
  vtkMRMLReadXMLStdStringMacro(planStr, PlanStr);
  vtkMRMLReadXMLEnumMacro(st, St);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLScadaOpcUaNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLScadaOpcUaNode* node = vtkMRMLScadaOpcUaNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();
  vtkMRMLCopyBeginMacro(node);
  // add new parameters here
  vtkMRMLCopyIntMacro(SysTimeScada);
  vtkMRMLCopyIntMacro(SysTimeModPos);
  vtkMRMLCopyStdStringMacro(PlanStr);
  vtkMRMLCopyEnumMacro(St);
  vtkMRMLCopyEndMacro(); 

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLScadaOpcUaNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLScadaOpcUaNode* node = vtkMRMLScadaOpcUaNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);
  // add new parameters here
  vtkMRMLCopyIntMacro(SysTimeScada);
  vtkMRMLCopyIntMacro(SysTimeModPos);
  vtkMRMLCopyStdStringMacro(PlanStr);
  vtkMRMLCopyEnumMacro(St);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLScadaOpcUaNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  // add new parameters here
  vtkMRMLPrintIntMacro(SysTimeScada);
  vtkMRMLPrintIntMacro(SysTimeModPos);
  vtkMRMLPrintStdStringMacro(PlanStr);
  vtkMRMLPrintEnumMacro(St);
  vtkMRMLPrintEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLScadaOpcUaNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, eventID, callData);

  if (!this->Scene)
  {
    vtkErrorMacro("ProcessMRMLEvents: Invalid MRML scene");
    return;
  }
  if (this->Scene->IsBatchProcessing())
  {
    return;
  }

  switch (eventID)
  {
  default:
    break;
  }
}


//---------------------------------------------------------------------------
void vtkMRMLScadaOpcUaNode::SetSt(int id)
{
  switch (id)
  {
    case 1: this->SetSt(vtkMRMLScadaOpcUaNode::ST_APPLICATION_IS_RUNNING); break;
    case 2: this->SetSt(vtkMRMLScadaOpcUaNode::ST_LOADING_THERAPY_PLAN); break;
    case 3: this->SetSt(vtkMRMLScadaOpcUaNode::ST_PATIENT_VERIFICATION); break;
    case 4: this->SetSt(vtkMRMLScadaOpcUaNode::ST_PATIENT_IMMOBILIZATION_VERIFICATION); break;
    case 5: this->SetSt(vtkMRMLScadaOpcUaNode::ST_PATIENT_POSITIONING); break;
    case 6: this->SetSt(vtkMRMLScadaOpcUaNode::ST_XRAY_EXPOSURE); break;
    case 7: this->SetSt(vtkMRMLScadaOpcUaNode::ST_XRAY_IMAGE_REGISTRATION); break;
    case 8: this->SetSt(vtkMRMLScadaOpcUaNode::ST_PATIENT_POSITION_CORRECTION); break;
    case 9: this->SetSt(vtkMRMLScadaOpcUaNode::ST_POSITIONING_FINISHED); break;
    case 10: this->SetSt(vtkMRMLScadaOpcUaNode::ST_WAIT_UNTIL_THERAPY_IS_FINISHED); break;
    case 11: this->SetSt(vtkMRMLScadaOpcUaNode::ST_PROCEDURE_COMMENTS_AND_RECORD_FINISH); break;
    default: this->SetSt(vtkMRMLScadaOpcUaNode::ST_UNDEFINED); break;
  }
}

//---------------------------------------------------------------------------
const char* vtkMRMLScadaOpcUaNode::GetStAsString(int id)
{
  switch (id)
  {
    case vtkMRMLScadaOpcUaNode::ST_UNDEFINED: return "ST_UNDEFINED";
    case vtkMRMLScadaOpcUaNode::ST_APPLICATION_IS_RUNNING: return "ST_APPLICATION_IS_RUNNING";
    case vtkMRMLScadaOpcUaNode::ST_LOADING_THERAPY_PLAN: return "ST_LOADING_THERAPY_PLAN";
    case vtkMRMLScadaOpcUaNode::ST_PATIENT_VERIFICATION: return "ST_PATIENT_VERIFICATION";
    case vtkMRMLScadaOpcUaNode::ST_PATIENT_IMMOBILIZATION_VERIFICATION: return "ST_PATIENT_IMMOBILIZATION_VERIFICATION";
    case vtkMRMLScadaOpcUaNode::ST_PATIENT_POSITIONING: return "ST_PATIENT_POSITIONING";
    case vtkMRMLScadaOpcUaNode::ST_XRAY_EXPOSURE: return "ST_XRAY_EXPOSURE";
    case vtkMRMLScadaOpcUaNode::ST_XRAY_IMAGE_REGISTRATION: return "ST_XRAY_IMAGE_REGISTRATION";
    case vtkMRMLScadaOpcUaNode::ST_PATIENT_POSITION_CORRECTION: return "ST_PATIENT_POSITION_CORRECTION";
    case vtkMRMLScadaOpcUaNode::ST_POSITIONING_FINISHED: return "ST_POSITIONING_FINISHED";
    case vtkMRMLScadaOpcUaNode::ST_WAIT_UNTIL_THERAPY_IS_FINISHED: return "ST_WAIT_UNTIL_THERAPY_IS_FINISHED";
    case vtkMRMLScadaOpcUaNode::ST_PROCEDURE_COMMENTS_AND_RECORD_FINISH: return "ST_PROCEDURE_COMMENTS_AND_RECORD_FINISH";
    default: return "ST_UNDEFINED";
  }
}

//---------------------------------------------------------------------------
int vtkMRMLScadaOpcUaNode::GetStFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i = 0; i < vtkMRMLScadaOpcUaNode::StateType_Last; i++)
  {
    if (std::strcmp(name, vtkMRMLScadaOpcUaNode::GetStAsString(i)) == 0)
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}
