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

#ifndef __vtkSlicerChannel26Room2RobotsGeometryCommon_h
#define __vtkSlicerChannel26Room2RobotsGeometryCommon_h

#include "vtkSlicerU70RoomGeoModuleLogicExport.h"

#include <array>

/// \ingroup SlicerRt_SlicerRtCommon
/// Common constants for Channel-26 Room-2 robots geometry (in mm).
class VTK_SLICER_U70ROOMGEO_MODULE_LOGIC_EXPORT vtkSlicerChannel26Room2RobotsGeometryCommon
{

public:
  //----------------------------------------------------------------------------
  // Constants
  //----------------------------------------------------------------------------

  // Distance from the north (collimator) wall to the south (back) wall
  static constexpr double D_NORTH_SOUTH_WALL = 6745.;
  // Distance from the north (collimator) wall to C-arm x-ray robot base
  static constexpr double D_NORTH_WALL_CARM_XRAY_ROBOT_BASE = 1500.;
  // Distance from the east (right) wall to C-arm x-ray robot base
  static constexpr double D_EAST_WALL_CARM_XRAY_ROBOT_BASE = 930.;
  // Distance from the north (collimator) wall to table top robot base
  static constexpr double D_NORTH_WALL_TABLE_TOP_ROBOT_BASE = 4300.;
  // Distance from the east (right) wall to table top robot base
  static constexpr double D_EAST_WALL_TABLE_TOP_ROBOT_BASE = 2750.;

  // Distance from the north  wall to isocenter
  static constexpr double D_NORTH_WALL_ISOCENTER = 2940.;
  // Distance from the east (right) wall to isocenter
  static constexpr double D_EAST_WALL_ISOCENTER = 2270.;
  // Distance from basement (physical floor) to isocenter
  static constexpr double D_BASEMENT_ISOCENTER = 2150.;
  // Distance from basement (physical floor) to fake floor
  static constexpr double D_BASEMENT_FAKE_FLOOR = 1000.;
  // Distance from basement (physical floor) to C-Arm robot base
  static constexpr double D_BASEMENT_CARM_ROBOT_BASE = 1000.;
};

#endif
