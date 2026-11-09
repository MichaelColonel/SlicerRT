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
  static std::string GetScadaSysTime();
  static std::string GetModPosState();
  static std::string GetModPosSysTime();
  static std::string GetModPosEventString();
  static std::string GetScadaPlanFilename();
  static std::string GetScadaPatientId();
  static std::string GetScadaPlanOpen();
  static std::string GetScadaPlanEcp();
};

#endif
