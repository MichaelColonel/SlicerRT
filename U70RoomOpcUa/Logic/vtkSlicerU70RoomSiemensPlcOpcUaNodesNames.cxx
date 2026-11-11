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

  This file was originally developed by Vinith Suriyakumar and Csaba Pinter,
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "vtkSlicerU70RoomSiemensPlcOpcUaNodesNames.h"

#include <string_view>

namespace {

constexpr std::string_view DOT = ".";

constexpr std::string_view PARENT = "ServerInterfaces";

constexpr std::string_view ERROR_FLAGS = "ErrorMes";
constexpr std::string_view SERVICE_FLAGS = "ServMes";
constexpr std::string_view MESSAGES_FLAGS = "Messages";

constexpr std::string_view KUKAX = "KUKA_X";
constexpr std::string_view KUKAY = "KUKA_Y";
constexpr std::string_view KUKAZ = "KUKA_Z";
constexpr std::string_view KUKAA = "KUKA_A";
constexpr std::string_view KUKAB = "KUKA_B";
constexpr std::string_view KUKAC = "KUKA_C";

constexpr std::string_view AXISCOORDA1 = "AXIS_cord_A1";
constexpr std::string_view AXISCOORDA2 = "AXIS_cord_A2";
constexpr std::string_view AXISCOORDA3 = "AXIS_cord_A3";
constexpr std::string_view AXISCOORDA4 = "AXIS_cord_A4";
constexpr std::string_view AXISCOORDA5 = "AXIS_cord_A5";
constexpr std::string_view AXISCOORDA6 = "AXIS_cord_A6";

constexpr std::string_view ASU = "ASU";
constexpr std::string_view RTK_PLC = "RTK_PLC";

constexpr std::string_view MODE = "Mode";
constexpr std::string_view STATUS_RTK = "Status_RTK";
constexpr std::string_view STATUS_R1 = "Status_R1_deka";
constexpr std::string_view STATUS_R2 = "Status_R2_C_Duga";
constexpr std::string_view RTK_HOMERETURN_HOLDPRESSED = "RTK_ComingHome_PressBtn";
constexpr std::string_view SETCOORDS = "SetCoords";

constexpr std::string_view READYXRAY1 = "RobotsReadyForXray1";
constexpr std::string_view READYXRAY2 = "RobotsReadyForXray2";
constexpr std::string_view READYBEAM = "RobotsReadyForBeam";
constexpr std::string_view TABLEPATIENT = "HUMAN_ON_DEKA";
constexpr std::string_view TABLEPOSITION = "DekaPosition";
constexpr std::string_view EMEVACFLAG = "EmEvacuationSignal";
constexpr std::string_view COORDS_ACCEPTED_R1 = "CoordsAccepted_R1";
constexpr std::string_view COORDS_NOTACCEPTED_R1 = "CoordsNotAccepted_R1";

constexpr std::string_view COORDS_ACCEPTED_R2 = "CoordsAccepted_R2";
constexpr std::string_view COORDS_NOTACCEPTED_R2 = "CoordsNotAccepted_R2";

constexpr std::string_view ISPRESSED = "isPressed";
constexpr std::string_view ISENABLED = "isEnable";

constexpr std::string_view BUTTONS = "Buttons";
constexpr std::string_view AM_MM = "AM_MM";

constexpr std::string_view R1_LOADTOISO = "R1_LoadToIso";
constexpr std::string_view R1_TONEWCOORDS = "R1_ToNewCoords";
constexpr std::string_view R1_TOLOAD = "R1_ToLoad";

constexpr std::string_view R2_TOPLANE1 = "R2_ToPL1";
constexpr std::string_view R2_TOPLANE2 = "R2_ToPL2";
constexpr std::string_view R2_TOHOME = "R2_ToHome";

constexpr std::string_view R1R2_EMEVAL = "R1R2_EmEvac";

constexpr std::string_view APPLY_DEKA_POSITION = "ApplyDekaPosition";
constexpr std::string_view APPLY_NEW_COORDS = "ApplyNewCoords";

constexpr std::string_view SM = "SM";

constexpr std::string_view R1_BREAKTEST = "BrakeTestR1";
constexpr std::string_view R1_MASREFTEST = "MasRefTestR1";
constexpr std::string_view R1_LOAD = "LoadR1";
constexpr std::string_view R1_SERVICEPOS1 = "ServicePos1_R1";
constexpr std::string_view R1_SERVICEPOS2 = "ServicePos2_R1";
constexpr std::string_view R1_SERVICEPOS3 = "ServicePos3_R1";

constexpr std::string_view R2_BREAKTEST = "BrakeTestR2";
constexpr std::string_view R2_MASREFTEST = "MasRefTestR2";
constexpr std::string_view R2_HOME = "HomeR2";
constexpr std::string_view R2_SERVICEPOS1 = "ServicePos1_R2";
constexpr std::string_view R2_SERVICEPOS2 = "ServicePos2_R2";
constexpr std::string_view R2_SERVICEPOS3 = "ServicePos3_R2";
constexpr std::string_view RESTARTSM = "RestartSM";

constexpr std::string_view KCM = "KCM";
constexpr std::string_view ALLOW_T1 = "AllowT1";
constexpr std::string_view ALLOW_XRAY = "AllowXRay";
constexpr std::string_view ALLOW_BEAM = "AllowBeam";

constexpr std::string_view RESET_ERRORS = "ResetErrors";
constexpr std::string_view MAKE_XRAY = "MakeXRay";

constexpr std::string_view REFLTARGETCOORDS_R1 = "ReflTargetCoords_R1";
constexpr std::string_view REFLTARGETCOORDS_R2 = "ReflTargetCoords_R2";
constexpr std::string_view CURRENTCOORDS_R1 = "CurrentCoords_R1";
constexpr std::string_view CURRENTCOORDS_R2 = "CurrentCoords_R2";

std::string GetRtkPlc()
{
  return std::string(PARENT) + std::string(DOT) + std::string(RTK_PLC);
}

std::string GetRtkPlcCurrentCoordsR1()
{
  return GetRtkPlc() + std::string(CURRENTCOORDS_R1);
}

std::string GetRtkPlcCurrentCoordsR2()
{
  return GetRtkPlc() + std::string(CURRENTCOORDS_R2);
}

std::string GetRtkPlcAsu()
{
  return GetRtkPlc() + std::string(DOT) + std::string(ASU);
}

std::string GetRtkPlcAsuButtons()
{
  return GetRtkPlcAsu() + std::string(DOT) + std::string(BUTTONS);
}

} // namespace

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetErrorsFlags()
{
  return GetRtkPlcAsu() + std::string(DOT) + std::string(ERROR_FLAGS);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetServiceFlags()
{
  return GetRtkPlcAsu() + std::string(DOT) + std::string(SERVICE_FLAGS);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMessagesFlags()
{
  return GetRtkPlcAsu() + std::string(DOT) + std::string(MESSAGES_FLAGS);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetMode()
{
  return GetRtkPlcAsu() + std::string(DOT) + std::string(MODE);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetStatusRtk()
{
  return GetRtkPlcAsu() + std::string(DOT) + std::string(STATUS_RTK);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetStatusR1()
{
  return GetRtkPlcAsu() + std::string(DOT) + std::string(STATUS_R1);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetStatusR2()
{
  return GetRtkPlcAsu() + std::string(DOT) + std::string(STATUS_R2);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1LoadToIsoButtonIsEnabled()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R1_LOADTOISO) + std::string(DOT) + std::string(ISENABLED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1LoadToIsoButtonIsPressed()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R1_LOADTOISO) + std::string(DOT) + std::string(ISPRESSED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToLoadButtonIsEnabled()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R1_TOLOAD) + std::string(DOT) + std::string(ISENABLED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToLoadButtonIsPressed()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R1_TOLOAD) + std::string(DOT) + std::string(ISENABLED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToNewCoordsButtonIsEnabled()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R1_TONEWCOORDS) + std::string(DOT) + std::string(ISENABLED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1ToNewCoordsButtonIsPressed()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R1_TONEWCOORDS) + std::string(DOT) + std::string(ISPRESSED);
}
  
std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane1ButtonIsEnabled()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R2_TOPLANE1) + std::string(DOT) + std::string(ISENABLED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane1ButtonIsPressed()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R2_TOPLANE1) + std::string(DOT) + std::string(ISPRESSED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane2ButtonIsEnabled()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R2_TOPLANE2) + std::string(DOT) + std::string(ISENABLED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToPlane2ButtonIsPressed()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R2_TOPLANE2) + std::string(DOT) + std::string(ISPRESSED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToHomeButtonIsEnabled()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R2_TOHOME) + std::string(DOT) + std::string(ISENABLED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR2ToHomeButtonIsPressed()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R2_TOHOME) + std::string(DOT) + std::string(ISPRESSED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualSetTabletopOrientationButtonIsEnabled()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(APPLY_DEKA_POSITION) + std::string(DOT) + std::string(ISENABLED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualSetTabletopOrientationButtonIsPressed()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(APPLY_DEKA_POSITION) + std::string(DOT) + std::string(ISPRESSED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1R2EmergencyEvacuationButtonIsEnabled()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R1R2_EMEVAL) + std::string(DOT) + std::string(ISENABLED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetAutoManualR1R2EmergencyEvacuationButtonIsPressed()
{
  return GetRtkPlcAsuButtons() + std::string(DOT) + std::string(R1R2_EMEVAL) + std::string(DOT) + std::string(ISPRESSED);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A1()
{
  return GetRtkPlcCurrentCoordsR1() + std::string(DOT) + std::string(AXISCOORDA1);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A2()
{
  return GetRtkPlcCurrentCoordsR1() + std::string(DOT) + std::string(AXISCOORDA2);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A3()
{
  return GetRtkPlcCurrentCoordsR1() + std::string(DOT) + std::string(AXISCOORDA3);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A4()
{
  return GetRtkPlcCurrentCoordsR1() + std::string(DOT) + std::string(AXISCOORDA4);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A5()
{
  return GetRtkPlcCurrentCoordsR1() + std::string(DOT) + std::string(AXISCOORDA5);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR1A6()
{
  return GetRtkPlcCurrentCoordsR1() + std::string(DOT) + std::string(AXISCOORDA6);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A1()
{
  return GetRtkPlcCurrentCoordsR2() + std::string(DOT) + std::string(AXISCOORDA1);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A2()
{
  return GetRtkPlcCurrentCoordsR2() + std::string(DOT) + std::string(AXISCOORDA2);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A3()
{
  return GetRtkPlcCurrentCoordsR2() + std::string(DOT) + std::string(AXISCOORDA3);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A4()
{
  return GetRtkPlcCurrentCoordsR2() + std::string(DOT) + std::string(AXISCOORDA4);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A5()
{
  return GetRtkPlcCurrentCoordsR2() + std::string(DOT) + std::string(AXISCOORDA5);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetCurrentAxisCoordR2A6()
{
  return GetRtkPlcCurrentCoordsR2() + std::string(DOT) + std::string(AXISCOORDA6);
}
