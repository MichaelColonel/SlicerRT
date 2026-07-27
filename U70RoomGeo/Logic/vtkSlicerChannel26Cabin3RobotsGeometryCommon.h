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

#include "vtkSlicerU70RoomGeoModuleLogicExport.h"

#include <array>

/// \ingroup SlicerRt_SlicerRtCommon
/// Common constants for Channel-26 Cabin-3 robots geometry (in mm).
class VTK_SLICER_U70ROOMGEO_MODULE_LOGIC_EXPORT vtkSlicerChannel26Cabin3RobotsGeometryCommon
{

public:
  //----------------------------------------------------------------------------
  // Constants
  //----------------------------------------------------------------------------

  // Distance from the north (collimator) wall to the south (back) wall
  static constexpr double D_NORTH_SOUTH_WALL = 6745.;
  // Distance from the north (collimator) wall to C-arm x-ray robot base
  static constexpr double D_NORTH_WALL_CARM_XRAY_ROBOT_BASE = 4400.;
  // Distance from the east (right) wall to C-arm x-ray robot base
  static constexpr double D_EAST_WALL_CARM_XRAY_ROBOT_BASE = 930.;
  // Distance from the north (collimator) wall to table top robot base
  static constexpr double D_NORTH_WALL_TABLE_TOP_ROBOT_BASE = 4350.;
  // Distance from the east (right) wall to table top robot base
  static constexpr double D_EAST_WALL_TABLE_TOP_ROBOT_BASE = 2750.;

  // Distance from the north (collimator) wall to isocenter
  static constexpr double D_NORTH_WALL_ISOCENTER = 2450.;
  // Distance from the east (right) wall to isocenter
  static constexpr double D_EAST_WALL_ISOCENTER = 2270.;
  // Distance from basement (physical floor) to isocenter
  static constexpr double D_BASEMENT_ISOCENTER = 2150.;
  // Distance from basement (physical floor) to fake floor
  static constexpr double D_BASEMENT_FAKE_FLOOR = 1000.;
  // Distance from basement (physical floor) to C-Arm robot base
  static constexpr double D_BASEMENT_CARM_ROBOT_BASE = 1500.;

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
  static constexpr double CARM_ROBOT_BASE_FIXED_SIZE = 140.; // no in documentation
  // C-arm x-ray robot base rotation shoulder disk center offset X
  static constexpr double CARM_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_X = 330.;
  // C-arm x-ray robot base rotation shoulder disk center offset Y
  static constexpr double CARM_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y = 645;
  // C-arm x-ray robot base rotation size
  static constexpr double CARM_ROBOT_BASE_ROTATION_SIZE = CARM_ROBOT_BASE_ROTATION_SHOULDER_OFFSET_Y - CARM_ROBOT_BASE_FIXED_SIZE;
  // C-arm x-ray robot shoulder size
  static constexpr double CARM_ROBOT_SHOULDER_SIZE = 1350.;
  // C-arm x-ray robot shoulder top center rotation
  static constexpr double CARM_ROBOT_SHOLDER_TOP_CENTER_ROTATION = CARM_ROBOT_BASE_ROTATION_SIZE + CARM_ROBOT_SHOULDER_SIZE;
  // C-arm x-ray robot shoulder center
  static constexpr double CARM_ROBOT_SHOLDER_CENTER = CARM_ROBOT_BASE_ROTATION_SIZE + (CARM_ROBOT_SHOULDER_SIZE / 2.);
  // C-arm x-ray robot elbow height-elbow offset Y
  static constexpr double CARM_ROBOT_SHOULDER_ELBOW_OFFSET_Y = 115.;
  // C-arm x-ray robot elbow size
  static constexpr double CARM_ROBOT_ELBOW_SIZE = 1420.;
  // C-arm x-ray robot wrist size
  static constexpr double CARM_ROBOT_WRIST_SIZE = 240.;

  // C-arm x-ray offset (from C-arm mounting point to X-ray source mounting point)
  static constexpr double CARM_XRAY_ORIGIN_OFFSET_X = 739.5;
  static constexpr double CARM_XRAY_ORIGIN_OFFSET_Y = 0.0;
  static constexpr double CARM_XRAY_ORIGIN_OFFSET_Z = 676.0;
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
  static constexpr double CARM_XRAY_DETECTOR_WIDTH = 470.;
  // C-arm x-ray distance between mounting point and focal spot along Z-axis
  static constexpr double CARM_XRAY_MOUNTING_POINT_SOURCE_OFFSET_Z = 167.;

  // C-arm x-ray detector offset (from C-arm mounting point to X-ray detector bottom center)
  static constexpr double CARM_DETECTOR_ORIGIN_OFFSET_X = 720.01;
  static constexpr double CARM_DETECTOR_ORIGIN_OFFSET_Y = 0.2;
  static constexpr double CARM_DETECTOR_ORIGIN_OFFSET_Z = 751.;

  // Table top robot base fake floor hole diameter
  static constexpr double TABLE_TOP_ROBOT_FLOOR_HOLE_DIAMETER = 2224.;
  // Table top robot base rotation disk from basement height
  static constexpr double TABLE_TOP_ROBOT_DISK_HEIGHT = 30.;
  // Table top robot base rotation disk from basement height
  static constexpr double TABLE_TOP_ROBOT_DISK_BASEMENT_HEIGHT = 1000.;
  // Table top width
  static constexpr double TABLE_TOP_WIDTH = 530.;
  // Table top length
  static constexpr double TABLE_TOP_LENGTH = 2165.;
  // Table top height
  static constexpr double TABLE_TOP_HEIGHT = 58.;
  // Table top immobilization system markers step
  static constexpr double TABLE_TOP_MARKERS_STEP = 140.;
  // Table top immobilization system number of markers
  static constexpr int TABLE_TOP_NUMBER_OF_MARKERS = 15;

  // Table top flange height
  static constexpr double TABLE_TOP_FLANGE_HEIGHT = 200.;
  // Table flange height = 258 mm
  static constexpr double TABLE_FLANGE_HEIGHT = TABLE_TOP_FLANGE_HEIGHT + TABLE_TOP_HEIGHT;
  // Table top shorter edge to flange center lenght offset
  static constexpr double TABLE_TOP_FLANGE_LENGTH_OFFSET = 640.;

  // Table top model top surface initial center (origin) offset
  static constexpr std::array< double, 3 > INIT_TABLE_TOP_ORIGIN_OFFSET_RAS{
   0.0,
   TABLE_TOP_FLANGE_LENGTH_OFFSET - (TABLE_TOP_LENGTH / 2),
   TABLE_TOP_HEIGHT };
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
