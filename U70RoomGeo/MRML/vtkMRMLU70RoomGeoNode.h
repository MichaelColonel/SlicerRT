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

#ifndef __vtkMRMLU70RoomGeoNode_h
#define __vtkMRMLU70RoomGeoNode_h

#include "vtkSlicerU70RoomGeoModuleMRMLExport.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkWeakPointer.h>

class vtkMRMLScalarVolumeNode;
class vtkMRMLMarkupsLineNode;
class vtkMRMLMarkupsFiducialNode;
class vtkMRMLSegmentationNode;

class vtkMRMLRTCarmBeamNode;
class vtkMRMLRTChannel26Cabin3BeamNode;
class vtkMRMLRTBeamNode;
class vtkMRMLChannel26GeometryNode;
class vtkMRMLDrrImageComputationNode;

class vtkTransform;

class VTK_SLICER_U70ROOMGEO_MODULE_MRML_EXPORT vtkMRMLU70RoomGeoNode : public vtkMRMLNode
{
public:
  enum U70RoomGeoType : int {
    CABIN1,
    CABIN2,
    CABIN3,
    U70RoomGeoType_Last
  };

  static vtkMRMLU70RoomGeoNode *New();
  vtkTypeMacro(vtkMRMLU70RoomGeoNode,vtkMRMLNode);
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
  vtkMRMLCopyContentMacro(vtkMRMLU70RoomGeoNode);

  /// Get unique node XML tag name
  const char* GetNodeTagName() override { return "U70RoomGeo"; };

  /// Handles events registered in the observer manager
  void ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData) override;

public:
  /// Get observed Channel-26 geometry node
  vtkMRMLChannel26GeometryNode* GetChannel26GeometryNode();
  /// Set and observe Channel-26 geometry node
  void SetAndObserveChannel26GeometryNode(vtkMRMLChannel26GeometryNode* node);

  /// Get observed beam axis (line node)
  vtkMRMLMarkupsLineNode* GetBeamAxisLineNode();
  /// Set and observe beam axis (line node)
  void SetAndObserveBeamAxisLineNode(vtkMRMLMarkupsLineNode* node);

  /// Get observed isocenter point (fiducial node)
  vtkMRMLMarkupsFiducialNode* GetIsocenterFiducialNode();
  /// Set and observe isocenter point (fiducial node)
  void SetAndObserveIsocenterFiducialNode(vtkMRMLMarkupsFiducialNode* node);

  /// Get path to the treatment machine descriptor JSON file
  vtkGetStringMacro(TreatmentMachineDescriptorFilePath);
  /// Set path to the treatment machine descriptor JSON file
  vtkSetStringMacro(TreatmentMachineDescriptorFilePath);

  /// Get treatment machine name
  vtkGetStringMacro(TreatmentMachineType);
  /// Set treatment machine name
  vtkSetStringMacro(TreatmentMachineType);

  /// Get/Set type of treatment machine room geometry 
  /// Possible values:
  /// vtkMRMLU70RoomGeoNode::CABIN1, vtkMRMLU70RoomGeoNode::CABIN2, vtkMRMLU70RoomGeoNode::CABIN3
  vtkGetMacro(U70RoomGeo, U70RoomGeoType);
  vtkSetMacro(U70RoomGeo, U70RoomGeoType);

  /// Get patient body segment ID
  vtkGetStringMacro(PatientBodySegmentID);
  /// Set patient body segment ID
  vtkSetStringMacro(PatientBodySegmentID);

  /// Get patient body segmentation node
  vtkMRMLSegmentationNode* GetPatientBodySegmentationNode();
  /// Set and observe patient body segmentation node
  void SetAndObservePatientBodySegmentationNode(vtkMRMLSegmentationNode* node);

protected:
  vtkMRMLU70RoomGeoNode();
  virtual ~vtkMRMLU70RoomGeoNode();
  vtkMRMLU70RoomGeoNode(const vtkMRMLU70RoomGeoNode&);
  void operator=(const vtkMRMLU70RoomGeoNode&);

  static const char* GetU70RoomGeoAsString(int id);
  static int GetU70RoomGeoFromString(const char* name);
  void SetU70RoomGeo(int id);

  /// Patient body segment ID in selected segmentation node
  char* PatientBodySegmentID;
  /// Path to the treatment machine descriptor JSON file
  char* TreatmentMachineDescriptorFilePath;
  /// Name of treatment machine used (must match folder name where the models can be found)
  char* TreatmentMachineType;
  /// Type of treatment machine room geometry 
  U70RoomGeoType U70RoomGeo{ U70RoomGeoType_Last };
};

#endif
