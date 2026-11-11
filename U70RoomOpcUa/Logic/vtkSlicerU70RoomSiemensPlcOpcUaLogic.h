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

// .NAME vtkSlicerU70RoomSiemensPlcOpcUaLogic - slicer logic class for Siemens PLC OPC UA nodes manipulation
// .SECTION Description

#ifndef __vtkSlicerU70RoomSiemensPlcOpcUaLogic_h
#define __vtkSlicerU70RoomSiemensPlcOpcUaLogic_h

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

class vtkMRMLSiemensPlcOpcUaNode;

class VTK_SLICER_U70ROOMOPCUA_MODULE_LOGIC_EXPORT vtkSlicerU70RoomSiemensPlcOpcUaLogic : public vtkMRMLAbstractLogic
{
public:
  static vtkSlicerU70RoomSiemensPlcOpcUaLogic* New();
  vtkTypeMacro(vtkSlicerU70RoomSiemensPlcOpcUaLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  uint8_t ReadMode(bool& res);
  uint16_t ReadStatusRtk(bool& res);
  uint16_t ReadStatusR1(bool& res);
  uint16_t ReadStatusR2(bool& res);

  double ReadCurrentCoordsR1A1(bool& res);
  double ReadCurrentCoordsR1A2(bool& res);
  double ReadCurrentCoordsR1A3(bool& res);
  double ReadCurrentCoordsR1A4(bool& res);
  double ReadCurrentCoordsR1A5(bool& res);
  double ReadCurrentCoordsR1A6(bool& res);
  double ReadCurrentCoordsR2A1(bool& res);
  double ReadCurrentCoordsR2A2(bool& res);
  double ReadCurrentCoordsR2A3(bool& res);
  double ReadCurrentCoordsR2A4(bool& res);
  double ReadCurrentCoordsR2A5(bool& res);
  double ReadCurrentCoordsR2A6(bool& res);

  void UpdateMRMLNodeFromOpcUaNodeValue(vtkMRMLSiemensPlcOpcUaNode*, const std::string& fullNodeName);

  // Called by the widget (C++ only; not wrapped for Python)
  void ServerConnected(const QSharedPointer< QOpcUaClient >& sharedClient, const QHash< QString, QString >& hashNodesFullNamesIds);
//  void RegisterNode(const std::string& nodeFullName, QOpcUaNode* node);
//  void RegisterNode(const std::string& nodeFullName, QOpcUaNode* node);
  void UnregisterOpcUaNode(const std::string& nodeFullName);
  void UnregisterAllOpcUaNodes();

protected:
  vtkSlicerU70RoomSiemensPlcOpcUaLogic();
  ~vtkSlicerU70RoomSiemensPlcOpcUaLogic() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

private:
  vtkSlicerU70RoomSiemensPlcOpcUaLogic(const vtkSlicerU70RoomSiemensPlcOpcUaLogic&); // Not implemented
  void operator=(const vtkSlicerU70RoomSiemensPlcOpcUaLogic&);            // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal; // For access from the callback function
};

#endif
