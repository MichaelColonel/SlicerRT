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

class VTK_SLICER_PATIENTPOSITIONING_MODULE_MRML_EXPORT vtkMRMLPatientPositioningNode : public vtkMRMLNode
{
public:
  enum CarmProjectionOrientation : int {
    ORIENTATION_HORIZONTAL,
    ORIENTATION_VERTICAL,
    ORIENTATION_ANGLE,
    CarmProjectionOrientation_Last
  };
  typedef std::pair< vtkWeakPointer<vtkMRMLScalarVolumeNode>, vtkWeakPointer<vtkMRMLScalarVolumeNode> > RtImagePair;
  
  typedef std::map< CarmProjectionOrientation, RtImagePair > OrientationRtImagePairMap;
  typedef std::map< CarmProjectionOrientation, vtkSmartPointer< vtkTransform > > OrientationTransformMap;

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
  /// Get patient ion beam node
  vtkMRMLRTBeamNode* GetBeamNode();
  /// Set and observe patient ion beam node
  void SetAndObserveBeamNode(vtkMRMLRTBeamNode* node);

  /// Get fixed reference ion beam node
  vtkMRMLRTChannel26Cabin3BeamNode* GetFixedReferenceBeamNode();
  /// Set and observe fixed reference ion beam node
  void SetAndObserveFixedReferenceBeamNode(vtkMRMLRTChannel26Cabin3BeamNode* node);

  /// Get C-arm x-ray beam node
  vtkMRMLRTBeamNode* GetCarmXrayBeamNode();
  /// Set and observe C-arm x-ray beam node
  void SetAndObserveCarmXrayBeamNode(vtkMRMLRTBeamNode* node);

  /// Get patient body segmentation node
  vtkMRMLSegmentationNode* GetPatientBodySegmentationNode();
  /// Set and observe patient body segmentation node
  void SetAndObservePatientBodySegmentationNode(vtkMRMLSegmentationNode* node);

  /// Get patient body segment ID
  vtkGetStringMacro(PatientBodySegmentID);
  /// Set patient body segment ID
  vtkSetStringMacro(PatientBodySegmentID);

  /// Get observed Channel-26 geometry node
  vtkMRMLChannel26GeometryNode* GetChannel26GeometryNode();
  /// Set and observe Channel-26 geometry node
  void SetAndObserveChannel26GeometryNode(vtkMRMLChannel26GeometryNode* node);

  /// Get observed Cabin-3 beam axis (line node)
  vtkMRMLMarkupsLineNode* GetCabin3BeamAxisLineNode();
  /// Set and observe Cabin-3 beam axis (line node)
  void SetAndObserveCabin3BeamAxisLineNode(vtkMRMLMarkupsLineNode* node);

  /// Get observed Cabin-3 isocenter point (fiducial node)
  vtkMRMLMarkupsFiducialNode* GetCabin3IsocenterFiducialNode();
  /// Set and observe Cabin-3 isocenter point (fiducial node)
  void SetAndObserveCabin3IsocenterFiducialNode(vtkMRMLMarkupsFiducialNode* node);

  /// Get observed DRR computation node
  vtkMRMLDrrImageComputationNode* GetDrrComputationNode();
  /// Set and observe DRR computation node
  void SetAndObserveDrrComputationNode(vtkMRMLDrrImageComputationNode* node);

  /// Get path to the treatment machine descriptor JSON file
  vtkGetStringMacro(TreatmentMachineDescriptorFilePath);
  /// Set path to the treatment machine descriptor JSON file
  vtkSetStringMacro(TreatmentMachineDescriptorFilePath);

  /// Get treatment machine name
  vtkGetStringMacro(TreatmentMachineType);
  /// Set treatment machine name
  vtkSetStringMacro(TreatmentMachineType);

  vtkTransform* GetRegistrationTransform(CarmProjectionOrientation proj);
  void SetRegistrationTransform(CarmProjectionOrientation proj,
    vtkTransform*);
  void GetRegistrationImages(CarmProjectionOrientation proj,
    vtkMRMLScalarVolumeNode* staticImage, vtkMRMLScalarVolumeNode* movedImage);
  vtkMRMLScalarVolumeNode* GetRegistrationStaticImage(CarmProjectionOrientation proj);
  vtkMRMLScalarVolumeNode* GetRegistrationMovedImage(CarmProjectionOrientation proj);
  void SetRegistrationImages(CarmProjectionOrientation proj,
    vtkMRMLScalarVolumeNode* staticImage, vtkMRMLScalarVolumeNode* movedImage);

protected:
  vtkMRMLPatientPositioningNode();
  virtual ~vtkMRMLPatientPositioningNode();
  vtkMRMLPatientPositioningNode(const vtkMRMLPatientPositioningNode&);
  void operator=(const vtkMRMLPatientPositioningNode&);

  /// Patient body segment ID in selected segmentation node
  char* PatientBodySegmentID;

  /// Path to the treatment machine descriptor JSON file
  char* TreatmentMachineDescriptorFilePath;
  /// Name of treatment machine used (must match folder name where the models can be found)
  char* TreatmentMachineType;
  OrientationRtImagePairMap OrientationImagesMap;
  OrientationTransformMap OrientationTransformMatrixMap;
};

#endif
