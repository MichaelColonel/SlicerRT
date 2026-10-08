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

// U70RoomOpcUa Logic includes
#include <vtkSlicerU70RoomOpcUaLogic.h>

// U70RoomOpcUa includes
#include "qSlicerU70RoomOpcUaModule.h"
#include "qSlicerU70RoomOpcUaModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70RoomOpcUaModulePrivate
{
public:
  qSlicerU70RoomOpcUaModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerU70RoomOpcUaModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerU70RoomOpcUaModulePrivate::qSlicerU70RoomOpcUaModulePrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70RoomOpcUaModule methods

//-----------------------------------------------------------------------------
qSlicerU70RoomOpcUaModule::qSlicerU70RoomOpcUaModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70RoomOpcUaModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerU70RoomOpcUaModule::~qSlicerU70RoomOpcUaModule() {}

//-----------------------------------------------------------------------------
QString qSlicerU70RoomOpcUaModule::helpText() const
{
  return tr("Siemens PLC OPC UA server: Channel-26 (Room-1 and Room-2), Channel-26A (Room-3).");
}

//-----------------------------------------------------------------------------
QString qSlicerU70RoomOpcUaModule::acknowledgementText() const
{
  return tr("This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70RoomOpcUaModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << tr("Mikhail Polkovnikov");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerU70RoomOpcUaModule::icon() const
{
  return QIcon(":/Icons/U70RoomOpcUa.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70RoomOpcUaModule::categories() const
{
  return QStringList() << tr("Lutch U-70");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70RoomOpcUaModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
QString qSlicerU70RoomOpcUaModule::title() const
{
  return tr("OPC-UA Server for Robots");
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomOpcUaModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerU70RoomOpcUaModule::createWidgetRepresentation()
{
  return new qSlicerU70RoomOpcUaModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerU70RoomOpcUaModule::createLogic()
{
  return vtkSlicerU70RoomOpcUaLogic::New();
}
