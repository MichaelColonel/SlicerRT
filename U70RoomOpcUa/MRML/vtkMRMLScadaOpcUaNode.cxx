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
  vtkMRMLWriteXMLIntMacro(sysTime, SysTime);
  vtkMRMLWriteXMLIntMacro(localTime, LocalTime);
  vtkMRMLWriteXMLEnumMacro(mode, Mode);
  vtkMRMLWriteXMLEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLScadaOpcUaNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);

  // add new parameters here
  vtkMRMLReadXMLIntMacro(sysTime, SysTime);
  vtkMRMLReadXMLIntMacro(localTime, LocalTime);
  vtkMRMLReadXMLEnumMacro(mode, Mode);
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
  vtkMRMLCopyIntMacro(SysTime);
  vtkMRMLCopyIntMacro(LocalTime);
  vtkMRMLCopyEnumMacro(Mode);
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
  vtkMRMLCopyIntMacro(SysTime);
  vtkMRMLCopyIntMacro(LocalTime);
  vtkMRMLCopyEnumMacro(Mode);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLScadaOpcUaNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  // add new parameters here
  vtkMRMLPrintIntMacro(SysTime);
  vtkMRMLPrintIntMacro(LocalTime);
  vtkMRMLPrintEnumMacro(Mode);
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
void vtkMRMLScadaOpcUaNode::SetMode(int id)
{
  switch (id)
  {
    case 0: this->SetMode(vtkMRMLScadaOpcUaNode::UNKNOWN); break;
    case 1: this->SetMode(vtkMRMLScadaOpcUaNode::AUTOMATIC); break;
    case 2: this->SetMode(vtkMRMLScadaOpcUaNode::MANUAL); break;
    case 3: this->SetMode(vtkMRMLScadaOpcUaNode::SERVICE); break;
    case 77: this->SetMode(vtkMRMLScadaOpcUaNode::KUKA_CONTROLLERS); break;
    default: this->SetMode(vtkMRMLScadaOpcUaNode::UNKNOWN); break;
  }
}

//---------------------------------------------------------------------------
const char* vtkMRMLScadaOpcUaNode::GetModeAsString(int id)
{
  switch (id)
  {
    case vtkMRMLScadaOpcUaNode::UNKNOWN: return "UNKNOWN";
    case vtkMRMLScadaOpcUaNode::AUTOMATIC: return "AUTOMATIC";
    case vtkMRMLScadaOpcUaNode::MANUAL: return "MANUAL";
    case vtkMRMLScadaOpcUaNode::SERVICE: return "SERVICE";
    case vtkMRMLScadaOpcUaNode::KUKA_CONTROLLERS: return "KUKA_CONTROLLERS";
    default: return "UNKNOWN";
  }
}

//---------------------------------------------------------------------------
int vtkMRMLScadaOpcUaNode::GetModeFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i = 0; i < vtkMRMLScadaOpcUaNode::ModeType_Last; i++)
  {
    if (std::strcmp(name, vtkMRMLScadaOpcUaNode::GetModeAsString(i)) == 0)
    {
      // found a matching name
      return i;
    }
  }
  // unknown name
  return -1;
}
