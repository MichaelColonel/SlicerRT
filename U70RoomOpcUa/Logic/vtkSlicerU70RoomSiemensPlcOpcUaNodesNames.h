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

#ifndef __vtkSlicerU70RoomSiemensPlcOpcUaNodesNames_h
#define __vtkSlicerU70RoomSiemensPlcOpcUaNodesNames_h

#include "vtkSlicerU70RoomOpcUaModuleLogicExport.h"

#include <string>

/// \ingroup SlicerRt_U70RoomOpcUa
/// Common constants for Siemens PLC OPC UA nodes names
class VTK_SLICER_U70ROOMOPCUA_MODULE_LOGIC_EXPORT vtkSlicerU70RoomSiemensPlcOpcUaNodesNames
{

public:
  //----------------------------------------------------------------------------
  // Constants
  //----------------------------------------------------------------------------
  static std::string GetErrorsFlags();
  static std::string GetServiceFlags();
  static std::string GetMessagesFlags();

  static std::string GetMode();
  static std::string GetStatusRtk();
  static std::string GetStatusR1();
  static std::string GetStatusR2();

  static std::string GetAutoManualR1LoadToIsoButtonIsEnabled();
  static std::string GetAutoManualR1LoadToIsoButtonIsPressed();
  static std::string GetAutoManualR1ToLoadButtonIsEnabled();
  static std::string GetAutoManualR1ToLoadButtonIsPressed();
  static std::string GetAutoManualR1ToNewCoordsButtonIsEnabled();
  static std::string GetAutoManualR1ToNewCoordsButtonIsPressed();
  
  static std::string GetAutoManualR2ToPlane1ButtonIsEnabled();
  static std::string GetAutoManualR2ToPlane1ButtonIsPressed();
  static std::string GetAutoManualR2ToPlane2ButtonIsEnabled();
  static std::string GetAutoManualR2ToPlane2ButtonIsPressed();
  static std::string GetAutoManualR2ToHomeButtonIsEnabled();
  static std::string GetAutoManualR2ToHomeButtonIsPressed();

  static std::string GetAutoManualSetTabletopOrientationButtonIsEnabled();
  static std::string GetAutoManualSetTabletopOrientationButtonIsPressed();

  static std::string GetAutoManualR1R2EmergencyEvacuationButtonIsEnabled();
  static std::string GetAutoManualR1R2EmergencyEvacuationButtonIsPressed();

  static std::string GetCurrentAxisCoordR1A1();
  static std::string GetCurrentAxisCoordR1A2();
  static std::string GetCurrentAxisCoordR1A3();
  static std::string GetCurrentAxisCoordR1A4();
  static std::string GetCurrentAxisCoordR1A5();
  static std::string GetCurrentAxisCoordR1A6();

  static std::string GetCurrentAxisCoordR2A1();
  static std::string GetCurrentAxisCoordR2A2();
  static std::string GetCurrentAxisCoordR2A3();
  static std::string GetCurrentAxisCoordR2A4();
  static std::string GetCurrentAxisCoordR2A5();
  static std::string GetCurrentAxisCoordR2A6();
};

#endif
