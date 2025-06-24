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

#ifndef __vtkSlicerCabin26ARobotsGeometryCommon_h
#define __vtkSlicerCabin26ARobotsGeometryCommon_h

#include "vtkSlicerPatientPositioningModuleLogicExport.h"

/// \ingroup SlicerRt_SlicerRtCommon
/// Common constants for Cabin26A robots geometry.
class VTK_SLICER_PATIENTPOSITIONING_MODULE_LOGIC_EXPORT vtkSlicerCabin26ARobotsGeometryCommon
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
  // Table top width
  static constexpr double TABLE_TOP_WIDTH = 530.;
  // Table top length
  static constexpr double TABLE_TOP_LENGTH = 2165.;
  // Distance from isocenter to physical floor
  static constexpr double D_ISOCENTER_PHYSCIAL_FLOOR = 2150.;
  // Distance from physical floor to fake floor
  static constexpr double D_PHYSCIAL_FLOOR_FAKE_FLOOR = 550.;
  // Table top robot base fake height
  static constexpr double TABLE_TOP_ROBOT_BASE_HEIGHT = 900.;
  //----------------------------------------------------------------------------
  // Utility functions
  //----------------------------------------------------------------------------
public:
};

#endif
