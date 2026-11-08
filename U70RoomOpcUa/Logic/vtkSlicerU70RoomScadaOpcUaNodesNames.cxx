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

#include "vtkSlicerU70RoomScadaOpcUaNodesNames.h"

#include <string_view>

namespace {

constexpr std::string_view DOT = ".";
constexpr std::string_view PARENT_NODE_DISPLAYED_NAME = "Модуль позиционирования";
constexpr std::string_view SCADA_SYS_TIME = "SysTimeScada";
constexpr std::string_view SCADA_SYS_TIME_MOD_POS = "SysTimeModPos";
constexpr std::string_view SCADA_PLAN_STRING = "PlanStr";
constexpr std::string_view SCADA_IDP = "IDP";
constexpr std::string_view SCADA_ECP = "ECP";
constexpr std::string_view SCADA_PLAN_OPEN = "PlanOpen";
constexpr std::string_view SCADA_EVENS_STR_MOD_POS = "EventStr";
constexpr std::string_view SCADA_STATE_MOD_POS = "St";

}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaSysTime()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_SYS_TIME);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosSysTime()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_SYS_TIME_MOD_POS);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaPlanFilename()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_PLAN_STRING);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaPatientId()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_IDP);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaPlanOpen()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_PLAN_OPEN);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaPlanEcp()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_ECP);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosEventString()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_EVENS_STR_MOD_POS);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosState()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_STATE_MOD_POS);
}
