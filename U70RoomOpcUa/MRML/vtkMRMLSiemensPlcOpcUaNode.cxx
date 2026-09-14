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

#include "vtkMRMLSiemensPlcOpcUaNode.h"

// STD include
#include <cstring>
#include <climits>
#include <bitset>
#include <cmath>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSiemensPlcOpcUaNode);

//----------------------------------------------------------------------------
vtkMRMLSiemensPlcOpcUaNode::vtkMRMLSiemensPlcOpcUaNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLSiemensPlcOpcUaNode::~vtkMRMLSiemensPlcOpcUaNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLSiemensPlcOpcUaNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);
  // add new parameters here
  vtkMRMLWriteXMLIntMacro(errorMessages, ErrorMessages);
  vtkMRMLWriteXMLIntMacro(serviceMessages, ServiceMessages);
  vtkMRMLWriteXMLIntMacro(miscMessages, MiscMessages);
  vtkMRMLWriteXMLEnumMacro(mode, Mode);
  vtkMRMLWriteXMLIntMacro(stateRtk, StateRtk);
  vtkMRMLWriteXMLIntMacro(stateR1, StateR1);
  vtkMRMLWriteXMLIntMacro(stateR2, StateR2);
  vtkMRMLWriteXMLEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLSiemensPlcOpcUaNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);

  // add new parameters here
  vtkMRMLReadXMLIntMacro(errorMessages, ErrorMessages);
  vtkMRMLReadXMLIntMacro(serviceMessages, ServiceMessages);
  vtkMRMLReadXMLIntMacro(miscMessages, MiscMessages);
  vtkMRMLReadXMLEnumMacro(mode, Mode);
  vtkMRMLReadXMLIntMacro(stateRtk, StateRtk);
  vtkMRMLReadXMLIntMacro(stateR1, StateR1);
  vtkMRMLReadXMLIntMacro(stateR2, StateR2);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLSiemensPlcOpcUaNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLSiemensPlcOpcUaNode* node = vtkMRMLSiemensPlcOpcUaNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();
  vtkMRMLCopyBeginMacro(node);
  // add new parameters here
  vtkMRMLCopyIntMacro(ErrorMessages);
  vtkMRMLCopyIntMacro(ServiceMessages);
  vtkMRMLCopyIntMacro(MiscMessages);
  vtkMRMLCopyEnumMacro(Mode);
  vtkMRMLCopyIntMacro(StateRtk);
  vtkMRMLCopyIntMacro(StateR1);
  vtkMRMLCopyIntMacro(StateR2);
  vtkMRMLCopyEndMacro(); 

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLSiemensPlcOpcUaNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLSiemensPlcOpcUaNode* node = vtkMRMLSiemensPlcOpcUaNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);
  // add new parameters here
  vtkMRMLCopyIntMacro(ErrorMessages);
  vtkMRMLCopyIntMacro(ServiceMessages);
  vtkMRMLCopyIntMacro(MiscMessages);
  vtkMRMLCopyEnumMacro(Mode);
  vtkMRMLCopyIntMacro(StateRtk);
  vtkMRMLCopyIntMacro(StateR1);
  vtkMRMLCopyIntMacro(StateR2);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLSiemensPlcOpcUaNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  // add new parameters here
  vtkMRMLPrintIntMacro(ErrorMessages);
  vtkMRMLPrintIntMacro(ServiceMessages);
  vtkMRMLPrintIntMacro(MiscMessages);
  vtkMRMLPrintEnumMacro(Mode);
  vtkMRMLPrintIntMacro(StateRtk);
  vtkMRMLPrintIntMacro(StateR1);
  vtkMRMLPrintIntMacro(StateR2);
  vtkMRMLPrintEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLSiemensPlcOpcUaNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
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
void vtkMRMLSiemensPlcOpcUaNode::SetMode(int id)
{
  switch (id)
  {
    case 0: this->SetMode(vtkMRMLSiemensPlcOpcUaNode::UNKNOWN); break;
    case 1: this->SetMode(vtkMRMLSiemensPlcOpcUaNode::AUTOMATIC_MANUAL); break;
    case 3: this->SetMode(vtkMRMLSiemensPlcOpcUaNode::SERVICE); break;
    case 77: this->SetMode(vtkMRMLSiemensPlcOpcUaNode::KUKA_CONTROLLERS); break;
    default: this->SetMode(vtkMRMLSiemensPlcOpcUaNode::UNKNOWN); break;
  }
}

//---------------------------------------------------------------------------
const char* vtkMRMLSiemensPlcOpcUaNode::GetModeAsString(int id)
{
  switch (id)
  {
    case vtkMRMLSiemensPlcOpcUaNode::UNKNOWN: return "UNKNOWN";
    case vtkMRMLSiemensPlcOpcUaNode::AUTOMATIC_MANUAL: return "AUTOMATIC_MANUAL";
    case vtkMRMLSiemensPlcOpcUaNode::SERVICE: return "SERVICE";
    case vtkMRMLSiemensPlcOpcUaNode::KUKA_CONTROLLERS: return "KUKA_CONTROLLERS";
    default: return "UNKNOWN";
  }
}

//---------------------------------------------------------------------------
int vtkMRMLSiemensPlcOpcUaNode::GetModeFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i = 0; i < vtkMRMLSiemensPlcOpcUaNode::ModeType_Last; i++)
  {
    if (std::strcmp(name, vtkMRMLSiemensPlcOpcUaNode::GetModeAsString(i)) == 0)
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}
