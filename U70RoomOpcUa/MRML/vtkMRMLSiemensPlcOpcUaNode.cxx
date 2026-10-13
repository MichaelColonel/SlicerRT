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
  vtkMRMLWriteXMLIntMacro(coordsX, CoordsX);
  vtkMRMLWriteXMLIntMacro(coordsY, CoordsY);
  vtkMRMLWriteXMLIntMacro(coordsZ, CoordsZ);
  vtkMRMLWriteXMLIntMacro(coordsXrayCorrectionZ, CoordsXrayCorrectionZ);
  vtkMRMLWriteXMLIntMacro(coordsA, CoordsA);
  vtkMRMLWriteXMLIntMacro(coordsB, CoordsB);
  vtkMRMLWriteXMLIntMacro(coordsC, CoordsC);
  vtkMRMLWriteXMLVectorMacro(autoManualR1LoadToIso, AutoManualR1LoadToIso, bool, 2);
  vtkMRMLWriteXMLVectorMacro(autoManualR1ToNewCoords, AutoManualR1ToNewCoords, bool, 2);
  vtkMRMLWriteXMLVectorMacro(autoManualR1ToLoad, AutoManualR1ToLoad, bool, 2);
  vtkMRMLWriteXMLVectorMacro(autoManualR2ToPlane1, AutoManualR2ToPlane1, bool, 2);
  vtkMRMLWriteXMLVectorMacro(autoManualR2ToPlane2, AutoManualR2ToPlane2, bool, 2);
  vtkMRMLWriteXMLVectorMacro(autoManualR2ToHome, AutoManualR2ToHome, bool, 2);
  vtkMRMLWriteXMLVectorMacro(autoManualEmergencyEvac, AutoManualEmergencyEvac, bool, 2);
  vtkMRMLWriteXMLVectorMacro(autoManualApplyTableTopPosition, AutoManualApplyTableTopPosition, bool, 2);
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
  vtkMRMLReadXMLIntMacro(coordsX, CoordsX);
  vtkMRMLReadXMLIntMacro(coordsY, CoordsY);
  vtkMRMLReadXMLIntMacro(coordsZ, CoordsZ);
  vtkMRMLReadXMLIntMacro(coordsXrayCorrectionZ, CoordsXrayCorrectionZ);
  vtkMRMLReadXMLIntMacro(coordsA, CoordsA);
  vtkMRMLReadXMLIntMacro(coordsB, CoordsB);
  vtkMRMLReadXMLIntMacro(coordsC, CoordsC);
  vtkMRMLReadXMLVectorMacro(autoManualR1LoadToIso, AutoManualR1LoadToIso, bool, 2);
  vtkMRMLReadXMLVectorMacro(autoManualR1ToNewCoords, AutoManualR1ToNewCoords, bool, 2);
  vtkMRMLReadXMLVectorMacro(autoManualR1ToLoad, AutoManualR1ToLoad, bool, 2);
  vtkMRMLReadXMLVectorMacro(autoManualR2ToPlane1, AutoManualR2ToPlane1, bool, 2);
  vtkMRMLReadXMLVectorMacro(autoManualR2ToPlane2, AutoManualR2ToPlane2, bool, 2);
  vtkMRMLReadXMLVectorMacro(autoManualR2ToHome, AutoManualR2ToHome, bool, 2);
  vtkMRMLReadXMLVectorMacro(autoManualEmergencyEvac, AutoManualEmergencyEvac, bool, 2);
  vtkMRMLReadXMLVectorMacro(autoManualApplyTableTopPosition, AutoManualApplyTableTopPosition, bool, 2);
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
  vtkMRMLCopyIntMacro(CoordsX);
  vtkMRMLCopyIntMacro(CoordsY);
  vtkMRMLCopyIntMacro(CoordsZ);
  vtkMRMLCopyIntMacro(CoordsXrayCorrectionZ);
  vtkMRMLCopyIntMacro(CoordsA);
  vtkMRMLCopyIntMacro(CoordsB);
  vtkMRMLCopyIntMacro(CoordsC);
  vtkMRMLCopyVectorMacro(AutoManualR1LoadToIso, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualR1ToNewCoords, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualR1ToLoad, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualR2ToPlane1, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualR2ToPlane2, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualR2ToHome, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualEmergencyEvac, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualApplyTableTopPosition, bool, 2);
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
  vtkMRMLCopyIntMacro(CoordsX);
  vtkMRMLCopyIntMacro(CoordsY);
  vtkMRMLCopyIntMacro(CoordsZ);
  vtkMRMLCopyIntMacro(CoordsXrayCorrectionZ);
  vtkMRMLCopyIntMacro(CoordsA);
  vtkMRMLCopyIntMacro(CoordsB);
  vtkMRMLCopyIntMacro(CoordsC);
  vtkMRMLCopyVectorMacro(AutoManualR1LoadToIso, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualR1ToNewCoords, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualR1ToLoad, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualR2ToPlane1, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualR2ToPlane2, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualR2ToHome, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualEmergencyEvac, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualApplyTableTopPosition, bool, 2);
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
  vtkMRMLPrintIntMacro(CoordsX);
  vtkMRMLPrintIntMacro(CoordsY);
  vtkMRMLPrintIntMacro(CoordsZ);
  vtkMRMLPrintIntMacro(CoordsXrayCorrectionZ);
  vtkMRMLPrintIntMacro(CoordsA);
  vtkMRMLPrintIntMacro(CoordsB);
  vtkMRMLPrintIntMacro(CoordsC);
  vtkMRMLPrintVectorMacro(AutoManualR1LoadToIso, bool, 2);
  vtkMRMLPrintVectorMacro(AutoManualR1ToNewCoords, bool, 2);
  vtkMRMLPrintVectorMacro(AutoManualR1ToLoad, bool, 2);
  vtkMRMLPrintVectorMacro(AutoManualR2ToPlane1, bool, 2);
  vtkMRMLPrintVectorMacro(AutoManualR2ToPlane2, bool, 2);
  vtkMRMLPrintVectorMacro(AutoManualR2ToHome, bool, 2);
  vtkMRMLPrintVectorMacro(AutoManualEmergencyEvac, bool, 2);
  vtkMRMLPrintVectorMacro(AutoManualApplyTableTopPosition, bool, 2);
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

//---------------------------------------------------------------------------
void vtkMRMLSiemensPlcOpcUaNode::GetAxisCoordsR1(double angles[6])
{
  angles[0] = double(this->AxisCoordsR1[0]) / 1000.;
  angles[1] = double(this->AxisCoordsR1[1]) / 1000.;
  angles[2] = double(this->AxisCoordsR1[2]) / 1000.;
  angles[3] = double(this->AxisCoordsR1[3]) / 1000.;
  angles[4] = double(this->AxisCoordsR1[4]) / 1000.;
  angles[5] = double(this->AxisCoordsR1[5]) / 1000.;
}

//---------------------------------------------------------------------------
void vtkMRMLSiemensPlcOpcUaNode::GetAxisCoordsR2(double angles[6])
{
  angles[0] = double(this->AxisCoordsR2[0]) / 1000.;
  angles[1] = double(this->AxisCoordsR2[1]) / 1000.;
  angles[2] = double(this->AxisCoordsR2[2]) / 1000.;
  angles[3] = double(this->AxisCoordsR2[3]) / 1000.;
  angles[4] = double(this->AxisCoordsR2[4]) / 1000.;
  angles[5] = double(this->AxisCoordsR2[5]) / 1000.;
}

//---------------------------------------------------------------------------
void vtkMRMLSiemensPlcOpcUaNode::SetAxisCoordsR1(double angles[6])
{
  this->AxisCoordsR1[0] = static_cast< int32_t >(angles[0] * 1000.);
  this->AxisCoordsR1[1] = static_cast< int32_t >(angles[1] * 1000.);
  this->AxisCoordsR1[2] = static_cast< int32_t >(angles[2] * 1000.);
  this->AxisCoordsR1[3] = static_cast< int32_t >(angles[3] * 1000.);
  this->AxisCoordsR1[4] = static_cast< int32_t >(angles[4] * 1000.);
  this->AxisCoordsR1[5] = static_cast< int32_t >(angles[5] * 1000.);
}

//---------------------------------------------------------------------------
void vtkMRMLSiemensPlcOpcUaNode::SetAxisCoordsR2(double angles[6])
{
  this->AxisCoordsR2[0] = static_cast< int32_t >(angles[0] * 1000.);
  this->AxisCoordsR2[1] = static_cast< int32_t >(angles[1] * 1000.);
  this->AxisCoordsR2[2] = static_cast< int32_t >(angles[2] * 1000.);
  this->AxisCoordsR2[3] = static_cast< int32_t >(angles[3] * 1000.);
  this->AxisCoordsR2[4] = static_cast< int32_t >(angles[4] * 1000.);
  this->AxisCoordsR2[5] = static_cast< int32_t >(angles[5] * 1000.);
}
