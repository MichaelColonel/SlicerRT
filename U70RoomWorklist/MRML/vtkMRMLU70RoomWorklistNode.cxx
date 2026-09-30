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

#include "vtkMRMLU70RoomWorklistNode.h"

// STD include
#include <cstring>
#include <climits>
#include <bitset>
#include <cmath>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLU70RoomWorklistNode);

//----------------------------------------------------------------------------
vtkMRMLU70RoomWorklistNode::vtkMRMLU70RoomWorklistNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLU70RoomWorklistNode::~vtkMRMLU70RoomWorklistNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomWorklistNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);
  // add new parameters here
  vtkMRMLWriteXMLStdStringMacro(patientID, PatientID);
  vtkMRMLWriteXMLStdStringMacro(patientName, PatientName);
  vtkMRMLWriteXMLStdStringMacro(scheduledProcedureStepStationAeTitle, ScheduledProcedureStepStationAeTitle);
  vtkMRMLWriteXMLEndMacro(); 
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomWorklistNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  // add new parameters here
  vtkMRMLReadXMLStdStringMacro(patientID, PatientID);
  vtkMRMLReadXMLStdStringMacro(patientName, PatientName);
  vtkMRMLReadXMLStdStringMacro(scheduledProcedureStepStationAeTitle, ScheduledProcedureStepStationAeTitle);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLU70RoomWorklistNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLU70RoomWorklistNode* node = vtkMRMLU70RoomWorklistNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();
  vtkMRMLCopyBeginMacro(node);
  // add new parameters here
  vtkMRMLCopyStdStringMacro(PatientID);
  vtkMRMLCopyStdStringMacro(PatientName);
  vtkMRMLCopyStdStringMacro(ScheduledProcedureStepStationAeTitle);
  vtkMRMLCopyEndMacro(); 

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomWorklistNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLU70RoomWorklistNode* node = vtkMRMLU70RoomWorklistNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);
  // add new parameters here
  vtkMRMLCopyStdStringMacro(PatientID);
  vtkMRMLCopyStdStringMacro(PatientName);
  vtkMRMLCopyStdStringMacro(ScheduledProcedureStepStationAeTitle);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLU70RoomWorklistNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  // add new parameters here
  vtkMRMLPrintStdStringMacro(PatientID);
  vtkMRMLPrintStdStringMacro(PatientName);
  vtkMRMLPrintStdStringMacro(ScheduledProcedureStepStationAeTitle);
  vtkMRMLPrintEndMacro(); 
}

//----------------------------------------------------------------------------
/*
void vtkMRMLU70RoomWorklistNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
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
*/
