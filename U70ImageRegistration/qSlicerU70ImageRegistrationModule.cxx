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

// U70ImageRegistration Logic includes
#include <vtkSlicerU70ImageRegistrationLogic.h>

// U70ImageRegistration includes
#include "qSlicerU70ImageRegistrationModule.h"
#include "qSlicerU70ImageRegistrationModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerU70ImageRegistrationModulePrivate
{
public:
  qSlicerU70ImageRegistrationModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerU70ImageRegistrationModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationModulePrivate::qSlicerU70ImageRegistrationModulePrivate() {}

//-----------------------------------------------------------------------------
// qSlicerU70ImageRegistrationModule methods

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationModule::qSlicerU70ImageRegistrationModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70ImageRegistrationModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerU70ImageRegistrationModule::~qSlicerU70ImageRegistrationModule() {}

//-----------------------------------------------------------------------------
QString qSlicerU70ImageRegistrationModule::helpText() const
{
  return tr("X-ray image registration for patient positioning.");
}

//-----------------------------------------------------------------------------
QString qSlicerU70ImageRegistrationModule::acknowledgementText() const
{
  return tr("This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70ImageRegistrationModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << tr("Mikhail Polkovnikov");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerU70ImageRegistrationModule::icon() const
{
  return QIcon(":/Icons/U70ImageRegistration.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70ImageRegistrationModule::categories() const
{
  return QStringList() << tr("Lutch U-70");
}

//-----------------------------------------------------------------------------
QStringList qSlicerU70ImageRegistrationModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
QString qSlicerU70ImageRegistrationModule::title() const
{
  return tr("Registration of X-ray Images");
}

//-----------------------------------------------------------------------------
void qSlicerU70ImageRegistrationModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerU70ImageRegistrationModule::createWidgetRepresentation()
{
  return new qSlicerU70ImageRegistrationModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerU70ImageRegistrationModule::createLogic()
{
  return vtkSlicerU70ImageRegistrationLogic::New();
}
