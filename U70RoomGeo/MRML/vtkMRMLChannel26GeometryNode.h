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

// MRML includes
#include <vtkMRML.h>
#include <vtkMRMLNode.h>
#include <vtkSmartPointer.h>
#include <vtkStdString.h>

#include "vtkSlicerChannel26Room3RobotsGeometryCommon.h"

class vtkMRMLScalarVolumeNode;
class vtkMRMLLinearTransformNode;

class VTK_SLICER_U70ROOMGEO_MODULE_MRML_EXPORT vtkMRMLChannel26GeometryNode : public vtkMRMLNode
{
public:
  enum U70RoomGeoType : int {
    R1 = 1, // ROOM1
    R2 = 2, // ROOM2
    R3 = 3, // ROOM3
    U70RoomGeoType_Last
  };
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

  vtkGetVector3Macro(PatientToTableTopTranslation, double);
  vtkSetVector3Macro(PatientToTableTopTranslation, double);

  void GetTableBaseFixedToFixedReferenceTranslation(double[3]);
  double* GetTableBaseFixedToFixedReferenceTranslation();
  vtkSetVector3Macro(TableBaseFixedToFixedReferenceTranslation, double);

  void GetCarmBaseFixedToTableBaseFixedOffset(double[3]);
  double* GetCarmBaseFixedToTableBaseFixedOffset();
  vtkSetVector3Macro(CarmBaseFixedToTableBaseFixedOffset, double);

  vtkGetVector6Macro(TableRobotAngles, double);
  vtkSetVector6Macro(TableRobotAngles, double);
  void GetTableRobotAnglesModel(double[6]);

  vtkGetVector6Macro(CarmRobotAngles, double);
  vtkSetVector6Macro(CarmRobotAngles, double);
  void GetCarmRobotAnglesModel(double[6]);

  vtkGetVector3Macro(AnglesABC, double);
  vtkSetVector3Macro(AnglesABC, double);

  vtkGetMacro(PatientHeadFeetRotation, bool);
  vtkSetMacro(PatientHeadFeetRotation, bool);

  /// Get/Set type of treatment machine room geometry 
  /// Possible values:
  /// vtkMRMLChannel26GeometryNode::R1, vtkMRMLChannel26GeometryNode::R2, vtkMRMLChannel26GeometryNode::R3
  vtkGetMacro(U70RoomGeo, U70RoomGeoType);
  vtkSetMacro(U70RoomGeo, U70RoomGeoType);
  void SetU70RoomGeo(int id);

protected:
  vtkMRMLChannel26GeometryNode();
  virtual ~vtkMRMLChannel26GeometryNode();
  vtkMRMLChannel26GeometryNode(const vtkMRMLChannel26GeometryNode&);
  void operator=(const vtkMRMLChannel26GeometryNode&);

//  using Room3CoordPos = vtkSlicerChannel26Room3RobotsGeometryCommon;

  static const char* GetU70RoomGeoAsString(int id);
  static int GetU70RoomGeoFromString(const char* name);

  /// Translate Patient to TableTop
  double PatientToTableTopTranslation[3] = { 0., 0., 0. };
  /// Translate TableRobotBaseFixed begin from FixedReference origin
  double TableBaseFixedToFixedReferenceTranslation[3] = {};
//    Room3CoordPos::D_NORTH_WALL_ISOCENTER - Room3CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE,
//    Room3CoordPos::D_EAST_WALL_ISOCENTER - Room3CoordPos::D_EAST_WALL_TABLE_TOP_ROBOT_BASE,
//    -1. * Room3CoordPos::D_BASEMENT_ISOCENTER };
  /// Translate C-Arm BaseFixed to Table BaseFixed offset (robots)
  /// X offset = CarmBaseFixed - TableBaseFixed (along X-axis) (minus beam axis)
  /// Y offset = CarmBaseFixed basement height (along Z-axis)
  /// Z offset = CarmBaseFixed + TableBaseFixed (along Y-axis)
  double CarmBaseFixedToTableBaseFixedOffset[3] = {};
//    Room3CoordPos::D_NORTH_WALL_TABLE_TOP_ROBOT_BASE - Room3CoordPos::D_NORTH_WALL_CARM_XRAY_ROBOT_BASE,
//    Room3CoordPos::D_BASEMENT_CARM_ROBOT_BASE,
//    Room3CoordPos::D_EAST_WALL_CARM_XRAY_ROBOT_BASE - Room3CoordPos::D_EAST_WALL_TABLE_TOP_ROBOT_BASE };
  /// Setup table top robot angles
  double TableRobotAngles[6] = { -150.58, -137.74, 87.92, -3.78, -92.17, -175.87 }; // A1=0, A2=-90, A3=90, A4=0, A5=-90, A6=0
//  double TableRobotAngles[6] = { 0., 0., 0., 0., 0., 90. }; // A1=0, A2=-90, A3=90, A4=0, A5=-90, A6=0
  /// Setup x-ray c-arm robot angles
  // double CarmRobotAngles[6] = { 178.77, -122.03, 142.06, 122.36, 33.38, -130.77 };
  double CarmRobotAngles[6] = { 0., -90., 81.075, 90., 90., -171. };
  /// table top plane correction angles (IEC Tabletop angles : vertical, lateral, longitudinal)
  double AnglesABC[3] = { 0., 0., 0. };
  /// Head first or feet first rotation
  bool PatientHeadFeetRotation{ false };
  /// Type of treatment machine room geometry 
  U70RoomGeoType U70RoomGeo{ vtkMRMLChannel26GeometryNode::U70RoomGeoType_Last };
};

#endif
