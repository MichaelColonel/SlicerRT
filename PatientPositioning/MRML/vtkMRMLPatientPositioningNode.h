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

class VTK_SLICER_PATIENTPOSITIONING_MODULE_MRML_EXPORT vtkMRMLPatientPositioningNode : public vtkMRMLNode
{
public:
  /// First DRR image, second X-ray system image
  typedef std::pair< vtkWeakPointer<vtkMRMLScalarVolumeNode>, vtkWeakPointer<vtkMRMLScalarVolumeNode> > DrrXrayImagePair;
  enum XrayProjectionType : int {
    Horizontal = 0,
    Vertical = 1,
    Angle = 2,
    XrayProjectionType_Last
  };
  typedef std::map< XrayProjectionType, DrrXrayImagePair > XrayProjectionImagesMap;

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
  void SetXrayImageNode(vtkMRMLScalarVolumeNode* node, XrayProjectionType projectionType = XrayProjectionType::Vertical);
  void SetDrrNode(vtkMRMLScalarVolumeNode* node, XrayProjectionType projectionType = XrayProjectionType::Vertical);
  /// Get DRR node
  vtkMRMLScalarVolumeNode* GetDrrNode(XrayProjectionType projectionType = XrayProjectionType::Vertical);
  /// Get X-ray image node
  vtkMRMLScalarVolumeNode* GetXrayImageNode(XrayProjectionType projectionType = XrayProjectionType::Vertical);
  void TranslateXrayImage(XrayProjectionType projectionType = XrayProjectionType::Vertical, double x = 0., double y = 0., double z = 0.);

  /// Get observed DRR node
  vtkMRMLScalarVolumeNode* GetObservedDrrNode();
  /// Set and observe DRR node.
  void SetAndObserveDrrNode(vtkMRMLScalarVolumeNode* node);
  /// Get observed x-ray image node
  vtkMRMLScalarVolumeNode* GetObservedXrayImageNode();
  /// Set and observe X-ray image node.
  void SetAndObserveXrayImageNode(vtkMRMLScalarVolumeNode* node);

  /// Get path to the treatment machine descriptor JSON file
  vtkGetStringMacro(TreatmentMachineDescriptorFilePath);
  /// Set path to the treatment machine descriptor JSON file
  vtkSetStringMacro(TreatmentMachineDescriptorFilePath);

protected:
  vtkMRMLPatientPositioningNode();
  ~vtkMRMLPatientPositioningNode();
  vtkMRMLPatientPositioningNode(const vtkMRMLPatientPositioningNode&);
  void operator=(const vtkMRMLPatientPositioningNode&);

  XrayProjectionImagesMap ImagesMap;

  /// Path to the treatment machine descriptor JSON file
  char* TreatmentMachineDescriptorFilePath;
};

#endif
