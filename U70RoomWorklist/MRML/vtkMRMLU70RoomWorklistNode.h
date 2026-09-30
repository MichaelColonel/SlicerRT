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

#ifndef __vtkMRMLU70RoomWorklistNode_h
#define __vtkMRMLU70RoomWorklistNode_h

// U70RoomWorklist includes
#include "vtkSlicerU70RoomWorklistModuleMRMLExport.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLModelNode.h>

class vtkMRMLRTBeamNode;
class vtkMRMLTableNode;

class VTK_SLICER_U70ROOMWORKLIST_MODULE_MRML_EXPORT vtkMRMLU70RoomWorklistNode : public vtkMRMLNode
{
public:

  static vtkMRMLU70RoomWorklistNode *New();
  vtkTypeMacro(vtkMRMLU70RoomWorklistNode,vtkMRMLNode);
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
  vtkMRMLCopyContentMacro(vtkMRMLU70RoomWorklistNode);

  /// Get unique node XML tag name
  const char* GetNodeTagName() override { return "U70RoomWorklist"; };

  /// Handles events registered in the observer manager
//  void ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData) override;

  vtkGetMacro(PatientID, std::string);
  vtkSetMacro(PatientID, std::string);

  vtkGetMacro(PatientName, std::string);
  vtkSetMacro(PatientName, std::string);

  vtkGetMacro(ScheduledProcedureStepStationAeTitle, std::string);
  vtkSetMacro(ScheduledProcedureStepStationAeTitle, std::string);

protected:
  vtkMRMLU70RoomWorklistNode();
  ~vtkMRMLU70RoomWorklistNode() override;
  vtkMRMLU70RoomWorklistNode(const vtkMRMLU70RoomWorklistNode&);
  void operator=(const vtkMRMLU70RoomWorklistNode&);

private:
  std::string AccessionNumber;
  std::string PatientName;
  std::string PatientID;
  std::string IssuerOfPatientID;
  std::string PatientBirthDate;
  std::string PatientSex;
  std::string StudyInstanceUID;
  std::string RequestedProcedureDescription;
  std::string RequestedProcedureID;
  std::string ScheduledProcedureStepModality;
  std::string ScheduledProcedureStepStationAeTitle;
  std::string ScheduledProcedureStepStartDateTime;
  std::string ScheduledProcedureStepDescription;
  std::string ScheduledProcedureStepID;
  std::string ScheduledProcedureStepStationName;
  std::string ScheduledProcedureStepLocation;
  std::string ScheduledProcedureStepStatus;
};

#endif
