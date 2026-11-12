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
constexpr std::string_view SCADA_PLAN_STR = "PlanStr";
constexpr std::string_view SCADA_IDP = "IDP";
constexpr std::string_view SCADA_ECP = "ECP";
constexpr std::string_view SCADA_IMMOB = "Immob";
constexpr std::string_view SCADA_PLAN_OPEN = "PlanOpen";
constexpr std::string_view SCADA_NPOS = "NPos";
constexpr std::string_view SCADA_REMOTE_CTRL = "RemoteCtrl";
constexpr std::string_view SCADA_REMOTE_STATE_MACHINE = "RemoteStateMachine";
constexpr std::string_view SCADA_REMOTE_CMD = "RemoteCmd";
constexpr std::string_view SCADA_REMOTE_PAUSE = "RemotePause";
constexpr std::string_view SCADA_REMOTE_AVR = "RemoteAvr";
constexpr std::string_view SCADA_DOOR_BLOCK = "DoorBlock";
constexpr std::string_view SCADA_READY_EQ = "ReadyEq";
constexpr std::string_view SCADA_READY_PLAN = "ReadyPlan";
constexpr std::string_view SCADA_READY_PROC = "ReadyProc";
constexpr std::string_view SCADA_ST = "St";
constexpr std::string_view SCADA_IN_QUEUE = "InQueue";
constexpr std::string_view SCADA_READY_ST = "ReadySt";
constexpr std::string_view SCADA_BLOCK = "Block";
constexpr std::string_view SCADA_CURRENT_LAYER = "CurrentLayer";
constexpr std::string_view SCADA_MAX_LAYER = "MaxLayer";
constexpr std::string_view SCADA_PROC_LAYER = "ProcLayer";
constexpr std::string_view SCADA_PROC_END = "ProcEnd";
constexpr std::string_view SCADA_PROC_NOTE = "ProcNote";
constexpr std::string_view SCADA_REC_CLOSE = "RecClose";
constexpr std::string_view SCADA_ERR_STR = "ErrStr";
constexpr std::string_view SCADA_EVENT_STR = "EventStr";

}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaSysTime()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_SYS_TIME);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosSysTime()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_SYS_TIME_MOD_POS);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosPlanStr()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_PLAN_STR);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosIdp()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_IDP);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosEcp()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_ECP);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosImmob()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_IMMOB);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosPlanOpen()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_PLAN_OPEN);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosPlanNpos()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_NPOS);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaRemoteCtrl()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_REMOTE_CTRL);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaRemoteStateMachine()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_REMOTE_STATE_MACHINE);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaRemoteCmd()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_REMOTE_CMD);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaRemotePause()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_REMOTE_PAUSE);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaRemoteAvr()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_REMOTE_AVR);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaDoorBlock()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_DOOR_BLOCK);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaReadyEq()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_READY_EQ);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaReadyPlan()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_READY_PLAN);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaReadyProc()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_READY_PROC);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosSt()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_ST);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaInQueue()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_IN_QUEUE);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosReadySt()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_READY_ST);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaBlock()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_BLOCK);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaCurrentLayer()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_CURRENT_LAYER);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaMaxLayer()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_MAX_LAYER);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaProcLayer()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_PROC_LAYER);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetScadaProcEnd()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_PROC_END);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosProcNote()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_PROC_NOTE);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosRecClose()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_REC_CLOSE);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosErrStr()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_ERR_STR);
}

std::string vtkSlicerU70RoomScadaOpcUaNodesNames::GetModPosEventStr()
{
  return std::string(PARENT_NODE_DISPLAYED_NAME) + std::string(DOT) + std::string(SCADA_EVENT_STR);
}
