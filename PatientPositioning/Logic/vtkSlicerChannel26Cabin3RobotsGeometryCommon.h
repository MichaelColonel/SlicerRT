/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkSlicerChannel26Cabin3RobotsGeometryCommon_h
#define __vtkSlicerChannel26Cabin3RobotsGeometryCommon_h

#include "vtkSlicerPatientPositioningModuleLogicExport.h"

#include <array>

/// \ingroup SlicerRt_SlicerRtCommon
/// Common constants for Channel-26 Cabin-3 robots geometry (in mm).
class VTK_SLICER_PATIENTPOSITIONING_MODULE_LOGIC_EXPORT vtkSlicerChannel26Cabin3RobotsGeometryCommon
{

public:
  //----------------------------------------------------------------------------
  // Constants
  //----------------------------------------------------------------------------
  // Distance from collimator wall to isocenter
  static constexpr double D_COLL_WALL_ISOCENTER = 2450.;
  // Distance from collimator wall to back wall
  static constexpr double D_COLL_WALL_BACK_WALL = 6745.;
  // Distance from isocenter to right wall
  static constexpr double D_ISOCENTER_RIGHT_WALL = 2270.;
  // Distance from collimator wall to C-arm x-ray robot base
  static constexpr double D_COLL_WALL_CARM_XRAY_ROBOT_BASE = 4400.;
  // Distance from right wall to C-arm x-ray robot base
  static constexpr double D_RIGHT_WALL_CARM_XRAY_ROBOT_BASE = 930.;
  // Distance from collimator wall to table top robot base
  static constexpr double D_COLL_WALL_TABLE_TOP_ROBOT_BASE = 4350.;
  // Distance from right wall to table top robot base
  static constexpr double D_RIGHT_WALL_TABLE_TOP_ROBOT_BASE = 2700.;
  // Distance from back right wall to left wall
  static constexpr double D_BACK_RIGHT_WALL_LEFT_WALL = 4644.;
  // Table top robot base fake floor hole diameter
  static constexpr double TABLE_TOP_ROBOT_FLOOR_HOLE_DIAMETER = 1800.;
  // Wall thickess (right, left, back)
  static constexpr double WALL_THICKNESS_RIGHT_LEFT_BACK = 1000.;
  static constexpr double WALL_THICKNESS_DEFAULT = 1000.;
  // Table top width
  static constexpr double TABLE_TOP_WIDTH = 530.;
  // Table top length
  static constexpr double TABLE_TOP_LENGTH = 2165.;
  // Distance from isocenter to physical floor
  static constexpr double D_ISOCENTER_PHYSCIAL_FLOOR = 2150.;
  // Distance from physical floor to fake floor
  static constexpr double D_PHYSCIAL_FLOOR_FAKE_FLOOR = 550.;
  // Table top robot base fake height
  static constexpr double TABLE_TOP_ROBOT_BASE_FAKE_FLOOR_HEIGHT = 900.;

  // Table top robot KUKA KR 300 R2700-2
  // Table top robot base fixed size (no in documentation)
  static constexpr double TABLE_ROBOT_BASE_MOUNTING_OFFSET_Y = -30.; // no in documentation
  static constexpr double TABLE_ROBOT_BASE_FIXED_SIZE = 225.6; // no in documentation
  // Table top robot base rotation shoulder disk center offset X
  static constexpr double TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X = 330.;
  // Table top robot base rotation shoulder disk center offset Y
  static constexpr double TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y = 645;
  // Table top robot base rotation size
  static constexpr double TABLE_ROBOT_BASE_ROTATION_SIZE = TABLE_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y - TABLE_ROBOT_BASE_FIXED_SIZE;
  // Table top robot shoulder size
  static constexpr double TABLE_ROBOT_SHOULDER_SIZE = 1150.;
  // Table top robot shoulder top center rotation
  static constexpr double TABLE_ROBOT_SHOLDER_TOP_CENTER_ROTATION = TABLE_ROBOT_BASE_ROTATION_SIZE + TABLE_ROBOT_SHOULDER_SIZE;
  // Table top robot shoulder center
  static constexpr double TABLE_ROBOT_SHOLDER_CENTER = TABLE_ROBOT_BASE_ROTATION_SIZE + (TABLE_ROBOT_SHOULDER_SIZE / 2.);
  // Table top robot elbow height-elbow offset Y
  static constexpr double TABLE_ROBOT_SHOULDER_ELBOW_OFFSET_Y = 115.;
  // Table top robot elbow size
  static constexpr double TABLE_ROBOT_ELBOW_SIZE = 1220.;
  // Table top robot wrist size
  static constexpr double TABLE_ROBOT_WRIST_SIZE = 240.;
  // Table top robot flange length (for model)
  static constexpr double TABLE_ROBOT_FLANGE_LENGTH = 39.90;
  // Table top robot wrist length (for model)
  static constexpr double TABLE_ROBOT_WRIST_LENGTH = TABLE_ROBOT_WRIST_SIZE - TABLE_ROBOT_FLANGE_LENGTH;
  // Table top robot elbow wrist length (for model)
  static constexpr double TABLE_ROBOT_ELBOW_WRIST_LENGTH = 351.47;
  // Table top robot elbow shoulder length (for model) 868.53 mm
  static constexpr double TABLE_ROBOT_ELBOW_SHOULDER_LENGTH = TABLE_ROBOT_ELBOW_SIZE - TABLE_ROBOT_ELBOW_WRIST_LENGTH;

  // C-arm x-ray robot KUKA KR 210 R3100-2
  // C-arm x-ray robot base fixed size (no in documentation)
  static constexpr double CARM_XRAY_ROBOT_BASE_FIXED_SIZE = 140.; // no in documentation
  // C-arm x-ray robot base rotation size
  static constexpr double CARM_XRAY_ROBOT_BASE_ROTATION_SIZE = 645. - TABLE_ROBOT_BASE_FIXED_SIZE;
  // C-arm x-ray robot base rotation shoulder disk center offset X
  static constexpr double CARM_XRAY_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X = 330.;
  // C-arm x-ray robot base rotation shoulder disk center offset Y
  static constexpr double CARM_XRAY_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y = 645;
  // C-arm x-ray robot shoulder size
  static constexpr double CARM_XRAY_SHOULDER_SIZE = 1350.;
  // C-arm x-ray robot shoulder top center rotation
  static constexpr double CARM_XRAY_SHOLDER_TOP_CENTER_ROTATION = TABLE_ROBOT_BASE_ROTATION_SIZE + TABLE_ROBOT_SHOULDER_SIZE;
  // C-arm x-ray robot shoulder center
  static constexpr double CARM_XRAY_SHOLDER_CENTER = TABLE_ROBOT_BASE_ROTATION_SIZE + (TABLE_ROBOT_SHOULDER_SIZE / 2.);
  // C-arm x-ray robot elbow height-elbow offset Y
  static constexpr double CARM_XRAY_SHOULDER_ELBOW_OFFSET_Y = 115.;
  // C-arm x-ray robot elbow size
  static constexpr double CARM_XRAY_ELBOW_SIZE = 1420.;
  // C-arm x-ray robot wrist size
  static constexpr double CARM_XRAY_WRIST_SIZE = 240.;

  // C-arm x-ray
  // C-arm x-ray outer size
  static constexpr double CARM_XRAY_OUTER_SIZE = 1518.;
  // C-arm x-ray inner size
  static constexpr double CARM_XRAY_INNER_SIZE = 1372.;
  // C-arm x-ray from focal spot to jaws distance
  static constexpr double CARM_XRAY_FOCAL_SPOT_JAWS_DISTANCE = 51.;
  // C-arm x-ray distance from x-ray window to detector surface
  static constexpr double CARM_XRAY_WINDOW_DETECTOR_DISTANCE = 1149.;
  // C-arm x-ray distance from jaws window to detector surface
  static constexpr double CARM_XRAY_JAWS_DETECTOR_DISTANCE = 970.;
  // C-arm x-ray from focal spot jaws to detector surface distance
  static constexpr double CARM_XRAY_FOCAL_SPOT_DETECTOR_DISTANCE = 1200.;
  // C-arm x-ray side size from outer c-arm border to far detector edge
  static constexpr double CARM_XRAY_SIDE_OUTER_DETECTOR_SIZE = 980.;
  // C-arm x-ray side size from inner c-arm border to far detector edge
  static constexpr double CARM_XRAY_SIDE_INNER_DETECTOR_SIZE = 855.;
  // C-arm x-ray source width
  static constexpr double CARM_XRAY_SOURCE_WIDTH = 485.;
  // C-arm x-ray detector width
  static constexpr double CARM_XRAY_DETECTOR_WIDTH = 537.;

  // Table top height
  static constexpr double TABLE_TOP_HEIGHT = 91.;
  // Table flange height
  static constexpr double TABLE_FLANGE_HEIGHT = 300.;

  // Table top model top surface initial center (origin) offset
  static constexpr std::array< double, 3 > INIT_TABLE_TOP_ORIGIN_OFFSET_RAS{ 0.5, 821.6, -210.};
  // Table flange (TableTop->TableFlange origin) model initial origin offset
  static constexpr std::array< double, 3 > INIT_TABLE_FLANGE_ORIGIN_OFFSET_RAS{
    INIT_TABLE_TOP_ORIGIN_OFFSET_RAS[0],
    INIT_TABLE_TOP_ORIGIN_OFFSET_RAS[1], 
    INIT_TABLE_TOP_ORIGIN_OFFSET_RAS[2] - TABLE_TOP_HEIGHT
  };
  // (A6 angle rotation)
  // Table robot flange (TableFlange->TableRobotFlange origin) model initial origin offset
  static constexpr std::array< double, 3 > INIT_TABLE_ROBOT_FLANGE_ORIGIN_OFFSET_RAS{
    INIT_TABLE_FLANGE_ORIGIN_OFFSET_RAS[0],
    INIT_TABLE_FLANGE_ORIGIN_OFFSET_RAS[1], 
    INIT_TABLE_FLANGE_ORIGIN_OFFSET_RAS[2] - TABLE_FLANGE_HEIGHT
  };
  // (A5 angle rotation)
  // Table robot wrist (TableRobotFlange->TableRobotWrist origin) model initial origin offset
  static constexpr std::array< double, 3 > INIT_TABLE_ROBOT_WRIST_ORIGIN_OFFSET_RAS{
    INIT_TABLE_ROBOT_FLANGE_ORIGIN_OFFSET_RAS[0],
    INIT_TABLE_ROBOT_FLANGE_ORIGIN_OFFSET_RAS[1], 
    INIT_TABLE_ROBOT_FLANGE_ORIGIN_OFFSET_RAS[2] - TABLE_ROBOT_WRIST_SIZE
  };

  //----------------------------------------------------------------------------
  // Utility functions
  //----------------------------------------------------------------------------
public:
};

#endif
