/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkSlicerU70RoomScadaOpcUaLogic - slicer logic class for SCADA OPC UA nodes manipulation
// .SECTION Description

#ifndef __vtkSlicerU70RoomScadaOpcUaLogic_h
#define __vtkSlicerU70RoomScadaOpcUaLogic_h

// Slicer includes
#include "vtkMRMLAbstractLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerU70RoomOpcUaModuleLogicExport.h"

#include <QSharedPointer>
#include <QStringList>

class QOpcUaClient;
class QOpcUaNode;

class vtkMRMLScadaOpcUaNode;

class VTK_SLICER_U70ROOMOPCUA_MODULE_LOGIC_EXPORT vtkSlicerU70RoomScadaOpcUaLogic : public vtkMRMLAbstractLogic
{
public:
  static vtkSlicerU70RoomScadaOpcUaLogic* New();
  vtkTypeMacro(vtkSlicerU70RoomScadaOpcUaLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Called by the widget (C++ only; not wrapped for Python)
  void ServerConnected(const QSharedPointer< QOpcUaClient >& sharedClient,
    const QHash< QString, QString >& hashNodesFullNamesIds);
  void UnregisterOpcUaNode(const std::string& nodeFullName);
  void UnregisterAllOpcUaNodes();
  bool UpdateMRMLNodeFromOpcUaNodeValue(vtkMRMLScadaOpcUaNode*, const std::string& fullNodeName);

  // Public API for other modules (C++ and Python)
  std::vector< std::string > GetRegisteredNodeIds();

  bool WritePlanFileName(const std::string& textString);
  bool WritePatientID(const std::string& textString);
  bool WritePlanOpenFlag(bool);
  bool WriteSysTimeModPos(int32_t val);
  bool WriteStateModPos(int32_t state);
  bool WriteEventString(const std::string& text);
  bool ReadNodesValues(const std::list< std::string >& fullNamesList);
  void SetInitialReadValuesFlag(bool flag = false);

protected:
  vtkSlicerU70RoomScadaOpcUaLogic();
  ~vtkSlicerU70RoomScadaOpcUaLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

private:
  vtkSlicerU70RoomScadaOpcUaLogic(const vtkSlicerU70RoomScadaOpcUaLogic&); // Not implemented
  void operator=(const vtkSlicerU70RoomScadaOpcUaLogic&);            // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal; // For access from the callback function
};

#endif
