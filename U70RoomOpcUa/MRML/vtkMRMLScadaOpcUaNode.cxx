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
  vtkMRMLWriteXMLBooleanMacro(timeoutExpired, TimeoutExpired);
  vtkMRMLWriteXMLIntMacro(sysTimeScada, SysTimeScada);
  vtkMRMLWriteXMLIntMacro(sysTimeModPos, SysTimeModPos);
  vtkMRMLWriteXMLStdStringMacro(planStr, PlanStr);
  vtkMRMLWriteXMLStdStringMacro(iDP, IDP);
  vtkMRMLWriteXMLBooleanMacro(eCP, ECP);
  vtkMRMLWriteXMLStdStringMacro(immob, Immob);
  vtkMRMLWriteXMLBooleanMacro(planOpen, PlanOpen);
  vtkMRMLWriteXMLIntMacro(nPos, NPos);
  vtkMRMLWriteXMLIntMacro(remoteCtrl, RemoteCtrl);
  vtkMRMLWriteXMLIntMacro(remoteStateMachine, RemoteStateMachine);
  vtkMRMLWriteXMLBooleanMacro(remoteCmd, RemoteCmd);
  vtkMRMLWriteXMLBooleanMacro(remotePause, RemotePause);
  vtkMRMLWriteXMLBooleanMacro(remoteAvr, RemoteAvr);
  vtkMRMLWriteXMLBooleanMacro(doorBlock, DoorBlock);
  vtkMRMLWriteXMLBooleanMacro(readyEq, ReadyEq);
  vtkMRMLWriteXMLBooleanMacro(readyPlan, ReadyPlan);
  vtkMRMLWriteXMLBooleanMacro(readyProc, ReadyProc);
  vtkMRMLWriteXMLEnumMacro(st, St);
  vtkMRMLWriteXMLBooleanMacro(inQueue, InQueue);
  vtkMRMLWriteXMLBooleanMacro(readySt, ReadySt);
  vtkMRMLWriteXMLBooleanMacro(block, Block);
  vtkMRMLWriteXMLIntMacro(currentLayer, CurrentLayer);
  vtkMRMLWriteXMLIntMacro(maxLayer, MaxLayer);
  vtkMRMLWriteXMLIntMacro(procLayer, ProcLayer);
  vtkMRMLWriteXMLBooleanMacro(procEnd, ProcEnd);
  vtkMRMLWriteXMLStdStringMacro(procNote, ProcNote);
  vtkMRMLWriteXMLBooleanMacro(recClose, RecClose);
  vtkMRMLWriteXMLStdStringMacro(errStr, ErrStr);
  vtkMRMLWriteXMLStdStringMacro(eventStr, EventStr);

  vtkMRMLWriteXMLEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLScadaOpcUaNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);

  // add new parameters here
  vtkMRMLReadXMLBooleanMacro(timeoutExpired, TimeoutExpired);
  vtkMRMLReadXMLIntMacro(sysTimeScada, SysTimeScada);
  vtkMRMLReadXMLIntMacro(sysTimeModPos, SysTimeModPos);
  vtkMRMLReadXMLStdStringMacro(planStr, PlanStr);
  vtkMRMLReadXMLStdStringMacro(iDP, IDP);
  vtkMRMLReadXMLBooleanMacro(eCP, ECP);
  vtkMRMLReadXMLStdStringMacro(immob, Immob);
  vtkMRMLReadXMLBooleanMacro(planOpen, PlanOpen);
  vtkMRMLReadXMLIntMacro(nPos, NPos);
  vtkMRMLReadXMLIntMacro(remoteCtrl, RemoteCtrl);
  vtkMRMLReadXMLIntMacro(remoteStateMachine, RemoteStateMachine);
  vtkMRMLReadXMLBooleanMacro(remoteCmd, RemoteCmd);
  vtkMRMLReadXMLBooleanMacro(remotePause, RemotePause);
  vtkMRMLReadXMLBooleanMacro(remoteAvr, RemoteAvr);
  vtkMRMLReadXMLBooleanMacro(doorBlock, DoorBlock);
  vtkMRMLReadXMLBooleanMacro(readyEq, ReadyEq);
  vtkMRMLReadXMLBooleanMacro(readyPlan, ReadyPlan);
  vtkMRMLReadXMLBooleanMacro(readyProc, ReadyProc);
  vtkMRMLReadXMLEnumMacro(st, St);
  vtkMRMLReadXMLBooleanMacro(inQueue, InQueue);
  vtkMRMLReadXMLBooleanMacro(readySt, ReadySt);
  vtkMRMLReadXMLBooleanMacro(block, Block);
  vtkMRMLReadXMLIntMacro(currentLayer, CurrentLayer);
  vtkMRMLReadXMLIntMacro(maxLayer, MaxLayer);
  vtkMRMLReadXMLIntMacro(procLayer, ProcLayer);
  vtkMRMLReadXMLBooleanMacro(procEnd, ProcEnd);
  vtkMRMLReadXMLStdStringMacro(procNote, ProcNote);
  vtkMRMLReadXMLBooleanMacro(recClose, RecClose);
  vtkMRMLReadXMLStdStringMacro(errStr, ErrStr);
  vtkMRMLReadXMLStdStringMacro(eventStr, EventStr);
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
  vtkMRMLCopyBooleanMacro(TimeoutExpired);
  vtkMRMLCopyIntMacro(SysTimeScada);
  vtkMRMLCopyIntMacro(SysTimeModPos);
  vtkMRMLCopyStdStringMacro(PlanStr);
  vtkMRMLCopyStdStringMacro(IDP);
  vtkMRMLCopyBooleanMacro(ECP);
  vtkMRMLCopyStdStringMacro(Immob);
  vtkMRMLCopyBooleanMacro(PlanOpen);
  vtkMRMLCopyIntMacro(NPos);
  vtkMRMLCopyIntMacro(RemoteCtrl);
  vtkMRMLCopyIntMacro(RemoteStateMachine);
  vtkMRMLCopyBooleanMacro(RemoteCmd);
  vtkMRMLCopyBooleanMacro(RemotePause);
  vtkMRMLCopyBooleanMacro(RemoteAvr);
  vtkMRMLCopyBooleanMacro(DoorBlock);
  vtkMRMLCopyBooleanMacro(ReadyEq);
  vtkMRMLCopyBooleanMacro(ReadyPlan);
  vtkMRMLCopyBooleanMacro(ReadyProc);
  vtkMRMLCopyEnumMacro(St);
  vtkMRMLCopyBooleanMacro(InQueue);
  vtkMRMLCopyBooleanMacro(ReadySt);
  vtkMRMLCopyBooleanMacro(Block);
  vtkMRMLCopyIntMacro(CurrentLayer);
  vtkMRMLCopyIntMacro(MaxLayer);
  vtkMRMLCopyIntMacro(ProcLayer);
  vtkMRMLCopyBooleanMacro(ProcEnd);
  vtkMRMLCopyStdStringMacro(ProcNote);
  vtkMRMLCopyBooleanMacro(RecClose);
  vtkMRMLCopyStdStringMacro(ErrStr);
  vtkMRMLCopyStdStringMacro(EventStr);

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
  vtkMRMLCopyBooleanMacro(TimeoutExpired);
  vtkMRMLCopyIntMacro(SysTimeScada);
  vtkMRMLCopyIntMacro(SysTimeModPos);
  vtkMRMLCopyStdStringMacro(PlanStr);
  vtkMRMLCopyStdStringMacro(IDP);
  vtkMRMLCopyBooleanMacro(ECP);
  vtkMRMLCopyStdStringMacro(Immob);
  vtkMRMLCopyBooleanMacro(PlanOpen);
  vtkMRMLCopyIntMacro(NPos);
  vtkMRMLCopyIntMacro(RemoteCtrl);
  vtkMRMLCopyIntMacro(RemoteStateMachine);
  vtkMRMLCopyBooleanMacro(RemoteCmd);
  vtkMRMLCopyBooleanMacro(RemotePause);
  vtkMRMLCopyBooleanMacro(RemoteAvr);
  vtkMRMLCopyBooleanMacro(DoorBlock);
  vtkMRMLCopyBooleanMacro(ReadyEq);
  vtkMRMLCopyBooleanMacro(ReadyPlan);
  vtkMRMLCopyBooleanMacro(ReadyProc);
  vtkMRMLCopyEnumMacro(St);
  vtkMRMLCopyBooleanMacro(InQueue);
  vtkMRMLCopyBooleanMacro(ReadySt);
  vtkMRMLCopyBooleanMacro(Block);
  vtkMRMLCopyIntMacro(CurrentLayer);
  vtkMRMLCopyIntMacro(MaxLayer);
  vtkMRMLCopyIntMacro(ProcLayer);
  vtkMRMLCopyBooleanMacro(ProcEnd);
  vtkMRMLCopyStdStringMacro(ProcNote);
  vtkMRMLCopyBooleanMacro(RecClose);
  vtkMRMLCopyStdStringMacro(ErrStr);
  vtkMRMLCopyStdStringMacro(EventStr);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLScadaOpcUaNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  // add new parameters here
  vtkMRMLPrintBooleanMacro(TimeoutExpired);
  vtkMRMLPrintIntMacro(SysTimeScada);
  vtkMRMLPrintIntMacro(SysTimeModPos);
  vtkMRMLPrintStdStringMacro(PlanStr);
  vtkMRMLPrintStdStringMacro(IDP);
  vtkMRMLPrintBooleanMacro(ECP);
  vtkMRMLPrintStdStringMacro(Immob);
  vtkMRMLPrintBooleanMacro(PlanOpen);
  vtkMRMLPrintIntMacro(NPos);
  vtkMRMLPrintIntMacro(RemoteCtrl);
  vtkMRMLPrintIntMacro(RemoteStateMachine);
  vtkMRMLPrintBooleanMacro(RemoteCmd);
  vtkMRMLPrintBooleanMacro(RemotePause);
  vtkMRMLPrintBooleanMacro(RemoteAvr);
  vtkMRMLPrintBooleanMacro(DoorBlock);
  vtkMRMLPrintBooleanMacro(ReadyEq);
  vtkMRMLPrintBooleanMacro(ReadyPlan);
  vtkMRMLPrintBooleanMacro(ReadyProc);
  vtkMRMLPrintEnumMacro(St);
  vtkMRMLPrintBooleanMacro(InQueue);
  vtkMRMLPrintBooleanMacro(ReadySt);
  vtkMRMLPrintBooleanMacro(Block);
  vtkMRMLPrintIntMacro(CurrentLayer);
  vtkMRMLPrintIntMacro(MaxLayer);
  vtkMRMLPrintIntMacro(ProcLayer);
  vtkMRMLPrintBooleanMacro(ProcEnd);
  vtkMRMLPrintStdStringMacro(ProcNote);
  vtkMRMLPrintBooleanMacro(RecClose);
  vtkMRMLPrintStdStringMacro(ErrStr);
  vtkMRMLPrintStdStringMacro(EventStr);
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
void vtkMRMLScadaOpcUaNode::SetSt(int32_t id)
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
const char* vtkMRMLScadaOpcUaNode::GetStAsString(int32_t id)
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
int32_t vtkMRMLScadaOpcUaNode::GetStFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int32_t i = 0; i < vtkMRMLScadaOpcUaNode::StateType_Last; i++)
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
