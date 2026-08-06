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

#include "vtkMRMLChannel26GeometryNode.h"

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLChannel26GeometryNode);

//----------------------------------------------------------------------------
vtkMRMLChannel26GeometryNode::vtkMRMLChannel26GeometryNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLChannel26GeometryNode::~vtkMRMLChannel26GeometryNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkMRMLWriteXMLBeginMacro(of);

  vtkMRMLWriteXMLVectorMacro(carmRobotAngles, CarmRobotAngles, double, 6);
  vtkMRMLWriteXMLVectorMacro(tableRobotAngles, TableRobotAngles, double, 6);
  vtkMRMLWriteXMLVectorMacro(tableBaseFixedToFixedReferenceTranslation, TableBaseFixedToFixedReferenceTranslation, double, 3);
  vtkMRMLWriteXMLVectorMacro(carmBaseFixedToTableTopBaseFixedOffset, CarmBaseFixedToTableBaseFixedOffset, double, 3);
  vtkMRMLWriteXMLVectorMacro(patientToTableTopTranslation, PatientToTableTopTranslation, double, 3);
  vtkMRMLWriteXMLFloatMacro(tableTopLateralAngle, TableTopLateralAngle);
  vtkMRMLWriteXMLFloatMacro(tableTopLongitudinalAngle, TableTopLongitudinalAngle);
  vtkMRMLWriteXMLFloatMacro(tableTopVerticalAngle, TableTopVerticalAngle);
  vtkMRMLWriteXMLBooleanMacro(patientHeadFeetRotation, PatientHeadFeetRotation);

  // add new parameters here
  vtkMRMLWriteXMLEndMacro();

}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  vtkMRMLNode::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);

  vtkMRMLReadXMLVectorMacro(carmRobotAngles, CarmRobotAngles, double, 6);
  vtkMRMLReadXMLVectorMacro(tableRobotAngles, TableRobotAngles, double, 6);
  vtkMRMLReadXMLVectorMacro(tableBaseFixedToFixedReferenceTranslation, TableBaseFixedToFixedReferenceTranslation, double, 3);
  vtkMRMLReadXMLVectorMacro(carmBaseFixedToTableBaseFixedOffset, CarmBaseFixedToTableBaseFixedOffset, double, 3);
  vtkMRMLReadXMLVectorMacro(patientToTableTopTranslation, PatientToTableTopTranslation, double, 3);
  vtkMRMLReadXMLFloatMacro(tableTopLateralAngle, TableTopLateralAngle);
  vtkMRMLReadXMLFloatMacro(tableTopLongitudinalAngle, TableTopLongitudinalAngle);
  vtkMRMLReadXMLFloatMacro(tableTopVerticalAngle, TableTopVerticalAngle);
  vtkMRMLReadXMLBooleanMacro(patientHeadFeetRotation, PatientHeadFeetRotation);

  // add new parameters here
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);

  // Note: ReportString is not read from XML, it is a strictly temporary value
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLChannel26GeometryNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLChannel26GeometryNode* node = vtkMRMLChannel26GeometryNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  // Copy beam parameters
  this->DisableModifiedEventOn();

  vtkMRMLCopyBeginMacro(node);

  vtkMRMLCopyVectorMacro(CarmRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableBaseFixedToFixedReferenceTranslation, double, 6);
  vtkMRMLCopyVectorMacro(CarmBaseFixedToTableBaseFixedOffset, double, 6);
  vtkMRMLCopyVectorMacro(PatientToTableTopTranslation, double, 3);
  vtkMRMLCopyFloatMacro(TableTopLateralAngle);
  vtkMRMLCopyFloatMacro(TableTopLongitudinalAngle);
  vtkMRMLCopyFloatMacro(TableTopVerticalAngle);
  vtkMRMLCopyBooleanMacro(PatientHeadFeetRotation);

  // add new parameters here
  vtkMRMLCopyEndMacro(); 

  this->EndModify(disabledModify);

  this->InvokePendingModifiedEvent();
}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::CopyContent(vtkMRMLNode *anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLChannel26GeometryNode* node = vtkMRMLChannel26GeometryNode::SafeDownCast(anode);
  if (!node)
  {
    return;
  }

  vtkMRMLCopyBeginMacro(node);

  vtkMRMLCopyVectorMacro(CarmRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableRobotAngles, double, 6);
  vtkMRMLCopyVectorMacro(TableBaseFixedToFixedReferenceTranslation, double, 6);
  vtkMRMLCopyVectorMacro(CarmBaseFixedToTableBaseFixedOffset, double, 6);
  vtkMRMLCopyVectorMacro(PatientToTableTopTranslation, double, 3);
  vtkMRMLCopyFloatMacro(TableTopLateralAngle);
  vtkMRMLCopyFloatMacro(TableTopLongitudinalAngle);
  vtkMRMLCopyFloatMacro(TableTopVerticalAngle);
  vtkMRMLCopyBooleanMacro(PatientHeadFeetRotation);

  // add new parameters here
  vtkMRMLCopyEndMacro();

}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);

  vtkMRMLPrintVectorMacro(CarmRobotAngles, double, 6);
  vtkMRMLPrintVectorMacro(TableRobotAngles, double, 6);
  vtkMRMLPrintVectorMacro(TableBaseFixedToFixedReferenceTranslation, double, 6);
  vtkMRMLPrintVectorMacro(CarmBaseFixedToTableBaseFixedOffset, double, 6);
  vtkMRMLPrintVectorMacro(PatientToTableTopTranslation, double, 3);
  vtkMRMLPrintFloatMacro(TableTopLateralAngle);
  vtkMRMLPrintFloatMacro(TableTopLongitudinalAngle);
  vtkMRMLPrintFloatMacro(TableTopVerticalAngle);
  vtkMRMLPrintBooleanMacro(PatientHeadFeetRotation);

  // add new parameters here
  vtkMRMLPrintEndMacro(); 

}

//----------------------------------------------------------------------------
void vtkMRMLChannel26GeometryNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
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
}
