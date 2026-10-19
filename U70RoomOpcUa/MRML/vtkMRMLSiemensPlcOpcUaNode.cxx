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
  vtkMRMLWriteXMLVectorMacro(autoManualApplyTabletopPosition, AutoManualApplyTabletopPosition, bool, 2);
  vtkMRMLWriteXMLVectorMacro(autoManualApplyNewCoords, AutoManualApplyNewCoords, bool, 2);
  vtkMRMLWriteXMLVectorMacro(currentAxisCoordsR1, CurrentAxisCoordsR1, int32_t, 6);
  vtkMRMLWriteXMLVectorMacro(currentAxisCoordsR2, CurrentAxisCoordsR2, int32_t, 6);
  vtkMRMLWriteXMLVectorMacro(currentCoordsR1, CurrentCoordsR1, int32_t, 6);
  vtkMRMLWriteXMLVectorMacro(currentCoordsR2, CurrentCoordsR2, int32_t, 6);
  vtkMRMLWriteXMLVectorMacro(reflectionCoordsR1, ReflectionCoordsR1, int32_t, 6);
  vtkMRMLWriteXMLVectorMacro(reflectionCoordsR2, ReflectionCoordsR2, int32_t, 6);
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
  vtkMRMLReadXMLVectorMacro(autoManualApplyTabletopPosition, AutoManualApplyTabletopPosition, bool, 2);
  vtkMRMLReadXMLVectorMacro(autoManualApplyNewCoords, AutoManualApplyNewCoords, bool, 2);
  vtkMRMLReadXMLVectorMacro(currentAxisCoordsR1, CurrentAxisCoordsR1, int32_t, 6);
  vtkMRMLReadXMLVectorMacro(currentAxisCoordsR2, CurrentAxisCoordsR2, int32_t, 6);
  vtkMRMLReadXMLVectorMacro(currentCoordsR1, CurrentCoordsR1, int32_t, 6);
  vtkMRMLReadXMLVectorMacro(currentCoordsR2, CurrentCoordsR2, int32_t, 6);
  vtkMRMLReadXMLVectorMacro(reflectionCoordsR1, ReflectionCoordsR1, int32_t, 6);
  vtkMRMLReadXMLVectorMacro(reflectionCoordsR2, ReflectionCoordsR2, int32_t, 6);
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
  vtkMRMLCopyVectorMacro(AutoManualApplyTabletopPosition, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualApplyNewCoords, bool, 2);
  vtkMRMLCopyVectorMacro(CurrentAxisCoordsR1, int32_t, 6);
  vtkMRMLCopyVectorMacro(CurrentAxisCoordsR2, int32_t, 6);
  vtkMRMLCopyVectorMacro(CurrentCoordsR1, int32_t, 6);
  vtkMRMLCopyVectorMacro(CurrentCoordsR2, int32_t, 6);
  vtkMRMLCopyVectorMacro(ReflectionCoordsR1, int32_t, 6);
  vtkMRMLCopyVectorMacro(ReflectionCoordsR2, int32_t, 6);
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
  vtkMRMLCopyVectorMacro(AutoManualApplyTabletopPosition, bool, 2);
  vtkMRMLCopyVectorMacro(AutoManualApplyNewCoords, bool, 2);
  vtkMRMLCopyVectorMacro(CurrentAxisCoordsR1, int32_t, 6);
  vtkMRMLCopyVectorMacro(CurrentAxisCoordsR2, int32_t, 6);
  vtkMRMLCopyVectorMacro(CurrentCoordsR1, int32_t, 6);
  vtkMRMLCopyVectorMacro(CurrentCoordsR2, int32_t, 6);
  vtkMRMLCopyVectorMacro(ReflectionCoordsR1, int32_t, 6);
  vtkMRMLCopyVectorMacro(ReflectionCoordsR2, int32_t, 6);
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
  vtkMRMLPrintVectorMacro(AutoManualApplyTabletopPosition, bool, 2);
  vtkMRMLPrintVectorMacro(AutoManualApplyNewCoords, bool, 2);
  vtkMRMLPrintVectorMacro(CurrentAxisCoordsR1, int32_t, 6);
  vtkMRMLPrintVectorMacro(CurrentAxisCoordsR2, int32_t, 6);
  vtkMRMLPrintVectorMacro(CurrentCoordsR1, int32_t, 6);
  vtkMRMLPrintVectorMacro(CurrentCoordsR2, int32_t, 6);
  vtkMRMLPrintVectorMacro(ReflectionCoordsR1, int32_t, 6);
  vtkMRMLPrintVectorMacro(ReflectionCoordsR2, int32_t, 6);
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
void vtkMRMLSiemensPlcOpcUaNode::GetCurrentAxisCoordsR1(double angles[6])
{
  angles[0] = double(this->CurrentAxisCoordsR1[0]) / 1000.;
  angles[1] = double(this->CurrentAxisCoordsR1[1]) / 1000.;
  angles[2] = double(this->CurrentAxisCoordsR1[2]) / 1000.;
  angles[3] = double(this->CurrentAxisCoordsR1[3]) / 1000.;
  angles[4] = double(this->CurrentAxisCoordsR1[4]) / 1000.;
  angles[5] = double(this->CurrentAxisCoordsR1[5]) / 1000.;
}

//---------------------------------------------------------------------------
void vtkMRMLSiemensPlcOpcUaNode::GetCurrentAxisCoordsR2(double angles[6])
{
  angles[0] = double(this->CurrentAxisCoordsR2[0]) / 1000.;
  angles[1] = double(this->CurrentAxisCoordsR2[1]) / 1000.;
  angles[2] = double(this->CurrentAxisCoordsR2[2]) / 1000.;
  angles[3] = double(this->CurrentAxisCoordsR2[3]) / 1000.;
  angles[4] = double(this->CurrentAxisCoordsR2[4]) / 1000.;
  angles[5] = double(this->CurrentAxisCoordsR2[5]) / 1000.;
}

//---------------------------------------------------------------------------
void vtkMRMLSiemensPlcOpcUaNode::SetCurrentAxisCoordsR1(double angles[6])
{
  this->CurrentAxisCoordsR1[0] = static_cast< int32_t >(angles[0] * 1000.);
  this->CurrentAxisCoordsR1[1] = static_cast< int32_t >(angles[1] * 1000.);
  this->CurrentAxisCoordsR1[2] = static_cast< int32_t >(angles[2] * 1000.);
  this->CurrentAxisCoordsR1[3] = static_cast< int32_t >(angles[3] * 1000.);
  this->CurrentAxisCoordsR1[4] = static_cast< int32_t >(angles[4] * 1000.);
  this->CurrentAxisCoordsR1[5] = static_cast< int32_t >(angles[5] * 1000.);
}

//---------------------------------------------------------------------------
void vtkMRMLSiemensPlcOpcUaNode::SetCurrentAxisCoordsR2(double angles[6])
{
  this->CurrentAxisCoordsR2[0] = static_cast< int32_t >(angles[0] * 1000.);
  this->CurrentAxisCoordsR2[1] = static_cast< int32_t >(angles[1] * 1000.);
  this->CurrentAxisCoordsR2[2] = static_cast< int32_t >(angles[2] * 1000.);
  this->CurrentAxisCoordsR2[3] = static_cast< int32_t >(angles[3] * 1000.);
  this->CurrentAxisCoordsR2[4] = static_cast< int32_t >(angles[4] * 1000.);
  this->CurrentAxisCoordsR2[5] = static_cast< int32_t >(angles[5] * 1000.);
}
