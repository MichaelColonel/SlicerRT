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

#ifndef __vtkMRMLPatientPositioningNode_h
#define __vtkMRMLPatientPositioningNode_h

#include "vtkSlicerPatientPositioningModuleMRMLExport.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkSmartPointer.h>

class vtkMRMLScalarVolumeNode;
class vtkMRMLChannel25GeometryNode;
class vtkMRMLMarkupsLineNode;

class VTK_SLICER_PATIENTPOSITIONING_MODULE_MRML_EXPORT vtkMRMLPatientPositioningNode : public vtkMRMLNode
{
public:
  static vtkMRMLPatientPositioningNode *New();
  vtkTypeMacro(vtkMRMLPatientPositioningNode,vtkMRMLNode);
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
  vtkMRMLCopyContentMacro(vtkMRMLPatientPositioningNode);

  /// Get unique node XML tag name
  const char* GetNodeTagName() override { return "PatientPositioning"; };

  /// Handles events registered in the observer manager
  void ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData) override;

public:
  /// Get observed Channel-25 geometry node
  vtkMRMLChannel25GeometryNode* GetChannel25GeometryNode();
  /// Set and observe Channel-25 geometry node
  void SetAndObserveChannel25GeometryNode(vtkMRMLChannel25GeometryNode* node);

  /// Get observed Channel-25 geometry node
  vtkMRMLMarkupsLineNode* GetFixedBeamAxis();
  /// Set and observe Channel-25 geometry node
  void SetAndObserveFixedBeamAxis(vtkMRMLMarkupsLineNode* node);

  /// Get path to the treatment machine descriptor JSON file
  vtkGetStringMacro(TreatmentMachineDescriptorFilePath);
  /// Set path to the treatment machine descriptor JSON file
  vtkSetStringMacro(TreatmentMachineDescriptorFilePath);

  /// Get treatment machine name
  vtkGetStringMacro(TreatmentMachineType);
  /// Set treatment machine name
  vtkSetStringMacro(TreatmentMachineType);

protected:
  vtkMRMLPatientPositioningNode();
  virtual ~vtkMRMLPatientPositioningNode();
  vtkMRMLPatientPositioningNode(const vtkMRMLPatientPositioningNode&);
  void operator=(const vtkMRMLPatientPositioningNode&);

  /// Path to the treatment machine descriptor JSON file
  char* TreatmentMachineDescriptorFilePath;
  /// Name of treatment machine used (must match folder name where the models can be found)
  char* TreatmentMachineType;
};

#endif
