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

#ifndef __vtkSlicerU70RoomScadaOpcUaNodesNames_h
#define __vtkSlicerU70RoomScadaOpcUaNodesNames_h

#include "vtkSlicerU70RoomOpcUaModuleLogicExport.h"

#include <string>

/// \ingroup SlicerRt_U70RoomOpcUa
/// Common constants for SCADA OPC UA nodes names
class VTK_SLICER_U70ROOMOPCUA_MODULE_LOGIC_EXPORT vtkSlicerU70RoomScadaOpcUaNodesNames
{

public:
  //----------------------------------------------------------------------------
  // Constants
  //----------------------------------------------------------------------------
  static std::string GetScadaSysTime();
  static std::string GetModPosSysTime();
  static std::string GetModPosPlanStr();
  static std::string GetModPosIdp();
  static std::string GetModPosEcp();
  static std::string GetModPosImmob();
  static std::string GetModPlanOpen();
  static std::string GetModPlanNpos();
  static std::string GetScadaRemoteCtrl();
  static std::string GetScadaRemoteStateMachine();
  static std::string GetScadaRemoteCmd();
  static std::string GetScadaRemotePause();
  static std::string GetScadaRemoteAvr();
  static std::string GetScadaDoorBlock();
  static std::string GetScadaReadyEq();
  static std::string GetScadaReadyPlan();
  static std::string GetScadaReadyProc();
  static std::string GetModPosSt();
  static std::string GetScadaInQueue();
  static std::string GetModPosReadySt();
  static std::string GetScadaBlock();
  static std::string GetScadaCurrentLayer();
  static std::string GetScadaMaxLayer();
  static std::string GetScadaProcLayer();
  static std::string GetScadaProcEnd();
  static std::string GetModPosProcNote();
  static std::string GetModPosRecClose();
  static std::string GetModPosErrStr();
  static std::string GetModPosEventStr();
};

#endif
