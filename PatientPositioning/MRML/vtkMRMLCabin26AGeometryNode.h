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

#ifndef __vtkMRMLCabin26AGeometryNode_h
#define __vtkMRMLCabin26AGeometryNode_h

#include "vtkSlicerPatientPositioningModuleMRMLExport.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkSmartPointer.h>
#include <vtkStdString.h>

class vtkMRMLScalarVolumeNode;
class vtkMRMLLinearTransformNode;

class VTK_SLICER_PATIENTPOSITIONING_MODULE_MRML_EXPORT vtkMRMLCabin26AGeometryNode : public vtkMRMLNode
{
public:
  static vtkMRMLCabin26AGeometryNode *New();
  vtkTypeMacro(vtkMRMLCabin26AGeometryNode, vtkMRMLNode);
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
  vtkMRMLCopyContentMacro(vtkMRMLCabin26AGeometryNode);

  /// Get unique node XML tag name
  const char* GetNodeTagName() override { return "Cabin26AGeometry"; };

  /// Handles events registered in the observer manager
  void ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData) override;

public:
  vtkMRMLLinearTransformNode* GetBaseFixedToFixedReferenceTransformNode();
  void SetAndObserveBaseFixedToFixedReferenceTransformNode(vtkMRMLLinearTransformNode* node);

  vtkMRMLLinearTransformNode* GetBaseRotationToBaseFixedTransformNode();
  void SetAndObserveBaseRotationToBaseFixedTransformNode(vtkMRMLLinearTransformNode* node);

  vtkMRMLLinearTransformNode* GetShoulderToBaseRotationTransformNode();
  void SetAndObserveShoulderToBaseRotationTransformNode(vtkMRMLLinearTransformNode* node);

  vtkMRMLLinearTransformNode* GetElbowToShoulderTransformNode();
  void SetAndObserveElbowToShoulderTransformNode(vtkMRMLLinearTransformNode* node);

  vtkMRMLLinearTransformNode* GetWristToElbowTransformNode();
  void SetAndObserveWristToElbowTransformNode(vtkMRMLLinearTransformNode* node);

  vtkMRMLLinearTransformNode* GetTableTopToWristTransformNode();
  void SetAndObserveTableTopToWristTransformNode(vtkMRMLLinearTransformNode* node);

  vtkMRMLLinearTransformNode* GetPatientToTableTopTransformNode(); 
  void SetAndObservePatientToTableTopTransformNode(vtkMRMLLinearTransformNode* node);

  /// Get patient body segment ID
  vtkGetStringMacro(PatientBodySegmentID);
  /// Set treatment machine name
  vtkSetStringMacro(PatientBodySegmentID);

  vtkGetMacro(CollisionDetectionEnabled, bool);
  vtkSetMacro(CollisionDetectionEnabled, bool);

  vtkGetMacro(TableTopLongitudinalAngle, double);
  vtkSetMacro(TableTopLongitudinalAngle, double);

  vtkGetMacro(TableTopLateralAngle, double);
  vtkSetMacro(TableTopLateralAngle, double);

  vtkGetMacro(TableTopVerticalAngle, double);
  vtkSetMacro(TableTopVerticalAngle, double);

  vtkGetVector3Macro(PatientToTableTopTranslation, double);
  vtkSetVector3Macro(PatientToTableTopTranslation, double);

  vtkGetVector3Macro(BaseFixedToFixedReferenceTranslation, double);
  vtkSetVector3Macro(BaseFixedToFixedReferenceTranslation, double);

  vtkGetVector6Macro(TableTopRobotAngles, double);
  vtkSetVector6Macro(TableTopRobotAngles, double);

  vtkGetVector6Macro(CArmRobotAngles, double);
  vtkSetVector6Macro(CArmRobotAngles, double);

  vtkGetMacro(PatientHeadFeetRotation, bool);
  vtkSetMacro(PatientHeadFeetRotation, bool);

protected:
  vtkMRMLCabin26AGeometryNode();
  virtual ~vtkMRMLCabin26AGeometryNode();
  vtkMRMLCabin26AGeometryNode(const vtkMRMLCabin26AGeometryNode&);
  void operator=(const vtkMRMLCabin26AGeometryNode&);

  /// Patient body segment ID in selected segmentation node
  char* PatientBodySegmentID{ nullptr };

  bool CollisionDetectionEnabled{ true };

  /// IEC Table top longitudinal angle
  double TableTopLongitudinalAngle{ 0. };
  /// IEC Table top lateral angle
  double TableTopLateralAngle{ 0. };
  /// IEC Table top vertical angle
  double TableTopVerticalAngle{ 0. };
  /// Translate Patient to TableTop
  double PatientToTableTopTranslation[3] = { 0., 0., 0. };
  /// Translate BaseFixed begin (origin) from RAS origin
  double BaseFixedToFixedReferenceTranslation[3] = { -1685., 600., 1800. };
  /// Setup table top robot angles
  double TableTopRobotAngles[6] = { 0., 0., 0., 0., 0., 0. }; // A1=0, A2=-90, A3=90, A4=0, A5=-90, A6=0
  /// Setup x-ray c-arm robot angles
  double CArmRobotAngles[6] = { 0., 0., 0., 0., 0., 0. };

  /// Head first or feet first rotation
  bool PatientHeadFeetRotation{ false };
};

#endif
