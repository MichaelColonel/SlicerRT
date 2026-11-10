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

constexpr std::string_view PARENT_NODE_DISPLAYED_NAME = "ServerInterfaces";
constexpr std::string_view SCADA_SYS_TIME = "SysTimeScada";
constexpr std::string_view SCADA_SYS_TIME_MOD_POS = "SysTimeModPos";
constexpr std::string_view SCADA_PLAN_STRING = "PlanStr";
constexpr std::string_view SCADA_IDP = "IDP";
constexpr std::string_view SCADA_ECP = "ECP";
constexpr std::string_view SCADA_PLAN_OPEN = "PlanOpen";
constexpr std::string_view SCADA_EVENS_STR_MOD_POS = "EventStr";
constexpr std::string_view SCADA_STATE_MOD_POS = "St";

}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetScadaSysTime()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_SYS_TIME);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetModPosSysTime()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_SYS_TIME_MOD_POS);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetScadaPlanFilename()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_PLAN_STRING);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetScadaPatientId()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_IDP);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetScadaPlanOpen()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_PLAN_OPEN);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetScadaPlanEcp()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_ECP);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetModPosEventString()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_EVENS_STR_MOD_POS);
}

std::string vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetModPosState()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_STATE_MOD_POS);
}
