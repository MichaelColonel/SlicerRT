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

  static std::string GetServiceR1BreakTestButtonIsEnabled();
  static std::string GetServiceR1BreakTestButtonIsPressed();
  static std::string GetServiceR1MasterReferenceTestButtonIsEnabled();
  static std::string GetServiceR1MasterReferenceTestButtonIsPressed();
  static std::string GetServiceR1LoadButtonIsEnabled();
  static std::string GetServiceR1LoadButtonIsPressed();
  static std::string GetServiceR1ServicePosition1ButtonIsEnabled();
  static std::string GetServiceR1ServicePosition1ButtonIsPressed();
  static std::string GetServiceR1ServicePosition2ButtonIsEnabled();
  static std::string GetServiceR1ServicePosition2ButtonIsPressed();
  static std::string GetServiceR1ServicePosition3ButtonIsEnabled();
  static std::string GetServiceR1ServicePosition3ButtonIsPressed();
  static std::string GetServiceR2BreakTestButtonIsEnabled();
  static std::string GetServiceR2BreakTestButtonIsPressed();
  static std::string GetServiceR2MasterReferenceTestButtonIsEnabled();
  static std::string GetServiceR2MasterReferenceTestButtonIsPressed();
  static std::string GetServiceR2HomeButtonIsEnabled();
  static std::string GetServiceR2HomeButtonIsPressed();
  static std::string GetServiceR2ServicePosition1ButtonIsEnabled();
  static std::string GetServiceR2ServicePosition1ButtonIsPressed();
  static std::string GetServiceR2ServicePosition2ButtonIsEnabled();
  static std::string GetServiceR2ServicePosition2ButtonIsPressed();
  static std::string GetServiceR2ServicePosition3ButtonIsEnabled();
  static std::string GetServiceR2ServicePosition3ButtonIsPressed();
  static std::string GetServiceRestartButtonIsEnabled();
  static std::string GetServiceRestartButtonIsPressed();
/*
  static std::string GetKukaControllersAllowT1ButtonIsEnabled();
  static std::string GetKukaControllersAllowT1ButtonIsPressed();
  static std::string GetKukaControllersAllowXrayButtonIsEnabled();
  static std::string GetKukaControllersAllowXrayButtonIsPressed();
  static std::string GetKukaControllersAllowBeamButtonIsEnabled();
  static std::string GetKukaControllersAllowBeamButtonIsPressed();

  static std::string GetResetErrorsButtonIsEnabled();
  static std::string GetResetErrorsButtonIsPressed();
  static std::string GetMakeXrayButtonIsEnabled();
  static std::string GetMakeXrayButtonIsPressed();

  static std::string GetReflectionTargetCoordsR1X();
  static std::string GetReflectionTargetCoordsR1Y();
  static std::string GetReflectionTargetCoordsR1Z();
  static std::string GetReflectionTargetCoordsR1A();
  static std::string GetReflectionTargetCoordsR1B();
  static std::string GetReflectionTargetCoordsR1C();

  static std::string GetReflectionTargetCoordsR2X();
  static std::string GetReflectionTargetCoordsR2Y();
  static std::string GetReflectionTargetCoordsR2Z();
  static std::string GetReflectionTargetCoordsR2A();
  static std::string GetReflectionTargetCoordsR2B();
  static std::string GetReflectionTargetCoordsR2C();
*/
  static std::string GetCurrentCoordR1X();
  static std::string GetCurrentCoordR1Y();
  static std::string GetCurrentCoordR1Z();
  static std::string GetCurrentCoordR1A();
  static std::string GetCurrentCoordR1B();
  static std::string GetCurrentCoordR1C();
  static std::string GetCurrentAxisCoordR1A1();
  static std::string GetCurrentAxisCoordR1A2();
  static std::string GetCurrentAxisCoordR1A3();
  static std::string GetCurrentAxisCoordR1A4();
  static std::string GetCurrentAxisCoordR1A5();
  static std::string GetCurrentAxisCoordR1A6();

  static std::string GetCurrentCoordR2X();
  static std::string GetCurrentCoordR2Y();
  static std::string GetCurrentCoordR2Z();
  static std::string GetCurrentCoordR2A();
  static std::string GetCurrentCoordR2B();
  static std::string GetCurrentCoordR2C();
  static std::string GetCurrentAxisCoordR2A1();
  static std::string GetCurrentAxisCoordR2A2();
  static std::string GetCurrentAxisCoordR2A3();
  static std::string GetCurrentAxisCoordR2A4();
  static std::string GetCurrentAxisCoordR2A5();
  static std::string GetCurrentAxisCoordR2A6();
/*
  static std::string GetCoordinatesAcceptedR1Flag();
  static std::string GetCoordinatesNotAcceptedR1Flag();
  static std::string GetCoordinatesAcceptedR2Flag();
  static std::string GetCoordinatesNotAcceptedR2Flag();

  static std::string GetComingHomeHoldButtonPressedFlag();
  static std::string GetReadyForXray1Flag();
  static std::string GetReadyForXray2Flag();
  static std::string GetReadyForBeamFlag();
  static std::string GetEmergencyEvacuationFlag();
  static std::string GetPatientOnTheTableFlag();
  static std::string GetTabletopOrientation();
*/
};

#endif
