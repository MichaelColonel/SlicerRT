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

#ifndef __vtkMRMLChannel26GeometryNode_h
#define __vtkMRMLChannel26GeometryNode_h

#include "vtkSlicerU70RoomGeoModuleMRMLExport.h"

#include "vtkSlicerChannel26Cabin3RobotsGeometryCommon.h"

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkSmartPointer.h>
#include <vtkStdString.h>

class vtkMRMLScalarVolumeNode;
class vtkMRMLLinearTransformNode;

class VTK_SLICER_U70ROOMGEO_MODULE_MRML_EXPORT vtkMRMLChannel26GeometryNode : public vtkMRMLNode
{
public:
  static vtkMRMLChannel26GeometryNode *New();
  vtkTypeMacro(vtkMRMLChannel26GeometryNode, vtkMRMLNode);
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
  vtkMRMLCopyContentMacro(vtkMRMLChannel26GeometryNode);

  /// Get unique node XML tag name
  const char* GetNodeTagName() override { return "Channel26Geometry"; };

  /// Handles events registered in the observer manager
  void ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData) override;

public:

  vtkGetMacro(TableTopLongitudinalAngle, double);
  vtkSetMacro(TableTopLongitudinalAngle, double);

  vtkGetMacro(TableTopLateralAngle, double);
  vtkSetMacro(TableTopLateralAngle, double);

  vtkGetMacro(TableTopVerticalAngle, double);
  vtkSetMacro(TableTopVerticalAngle, double);

  vtkGetVector3Macro(PatientToTableTopTranslation, double);
  vtkSetVector3Macro(PatientToTableTopTranslation, double);

  vtkGetVector3Macro(TableBaseFixedToFixedReferenceTranslation, double);
  vtkSetVector3Macro(TableBaseFixedToFixedReferenceTranslation, double);

  vtkGetVector3Macro(CarmBaseFixedToTableBaseFixedOffset, double);
  vtkSetVector3Macro(CarmBaseFixedToTableBaseFixedOffset, double);

  vtkGetVector6Macro(TableRobotAngles, double);
  vtkSetVector6Macro(TableRobotAngles, double);

  vtkGetVector6Macro(CarmRobotAngles, double);
  vtkSetVector6Macro(CarmRobotAngles, double);

  vtkGetVector3Macro(AnglesABC, double);
  vtkSetVector3Macro(AnglesABC, double);

  vtkGetMacro(PatientHeadFeetRotation, bool);
  vtkSetMacro(PatientHeadFeetRotation, bool);

protected:
  vtkMRMLChannel26GeometryNode();
  virtual ~vtkMRMLChannel26GeometryNode();
  vtkMRMLChannel26GeometryNode(const vtkMRMLChannel26GeometryNode&);
  void operator=(const vtkMRMLChannel26GeometryNode&);

  using CoordPos = vtkSlicerChannel26Cabin3RobotsGeometryCommon;

  /// IEC Table top longitudinal angle
  double TableTopLongitudinalAngle{ 0. };
  /// IEC Table top lateral angle
  double TableTopLateralAngle{ 0. };
  /// IEC Table top vertical angle
  double TableTopVerticalAngle{ 0. };
  /// Translate Patient to TableTop
  double PatientToTableTopTranslation[3] = { 0., 0., 0. };
  /// Translate TableRobotBaseFixed begin from FixedReference origin
  double TableBaseFixedToFixedReferenceTranslation[3] = {
    CoordPos::D_NORTH_WALL_ISOCENTER - CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE,
    CoordPos::D_EAST_WALL_ISOCENTER - CoordPos::D_EAST_WALL_TABLE_TOP_ROBOT_BASE,
    -1. * CoordPos::D_BASEMENT_ISOCENTER };
  /// Translate C-Arm BaseFixed to Table BaseFixed offset (robots)
  /// X offset = CarmBaseFixed - TableBaseFixed (along X-axis) (minus beam axis)
  /// Y offset = CarmBaseFixed basement height (along Z-axis)
  /// Z offset = CarmBaseFixed + TableBaseFixed (along Y-axis)
  double CarmBaseFixedToTableBaseFixedOffset[3] = {
    CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE - CoordPos::D_NORTH_WALL_CARM_XRAY_ROBOT_BASE,
    CoordPos::D_BASEMENT_CARM_ROBOT_BASE,
    CoordPos::D_EAST_WALL_CARM_XRAY_ROBOT_BASE - CoordPos::D_EAST_WALL_TABLE_TOP_ROBOT_BASE };
  /// Setup table top robot angles
  double TableRobotAngles[6] = { 0., 0., 0., 0., 0., 90. }; // A1=0, A2=-90, A3=90, A4=0, A5=-90, A6=0
  /// Setup x-ray c-arm robot angles
  double CarmRobotAngles[6] = { 0., 0., 0., 0., 0., 0. };
  /// table top plane correction angles
  double AnglesABC[3] = { 0., 0., 0. };
  /// Head first or feet first rotation
  bool PatientHeadFeetRotation{ false };
};

#endif
