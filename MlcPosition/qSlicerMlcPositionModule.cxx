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

// MlcPosition Logic includes
#include <vtkSlicerMlcPositionLogic.h>

// LoadableModuleTemplate includes
#include "qSlicerMlcPositionModule.h"
#include "qSlicerMlcPositionModuleWidget.h"

// Beams Module includes
#include <vtkSlicerBeamsModuleLogic.h>

// Segmentations Module includes
#include <qSlicerSegmentationsModule.h>
#include <vtkSlicerSegmentationsModuleLogic.h>

// Slicer Models includes
#include <vtkSlicerModelsLogic.h>

// Slicer includes
#include <qSlicerIOManager.h>
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <vtkSlicerConfigure.h> // For Slicer_USE_PYTHONQT

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerMlcPositionModulePrivate {
public:
  qSlicerMlcPositionModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerMlcPositionModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerMlcPositionModulePrivate::qSlicerMlcPositionModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerMlcPositionModule methods

//-----------------------------------------------------------------------------
qSlicerMlcPositionModule::qSlicerMlcPositionModule(QObject* parent)
  :
  Superclass(parent),
  d_ptr(new qSlicerMlcPositionModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerMlcPositionModule::~qSlicerMlcPositionModule()
{
}

//-----------------------------------------------------------------------------
QString
qSlicerMlcPositionModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString
qSlicerMlcPositionModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList
qSlicerMlcPositionModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Mikhail Polkovnikov (NRC \"Kurchatov Institute\" - IHEP)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon
qSlicerMlcPositionModule::icon() const
{
  return QIcon(":/Icons/MlcPosition.png");
}

//-----------------------------------------------------------------------------
QStringList
qSlicerMlcPositionModule::categories() const
{
  return QStringList() << "Radiotherapy";
}

//-----------------------------------------------------------------------------
QStringList
qSlicerMlcPositionModule::dependencies() const
{
  return QStringList() << "Segmentations" << "SubjectHierarchy" << "Beams";
}

//-----------------------------------------------------------------------------
void
qSlicerMlcPositionModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation*
qSlicerMlcPositionModule::createWidgetRepresentation()
{
  qSlicerMlcPositionModuleWidget* moduleWidget = new qSlicerMlcPositionModuleWidget();

  qSlicerAbstractCoreModule* segmentationsModule = qSlicerCoreApplication::application()->moduleManager()->module("Segmentations");
  if (segmentationsModule)
  {
    vtkSlicerSegmentationsModuleLogic* segmentationsLogic = vtkSlicerSegmentationsModuleLogic::SafeDownCast(segmentationsModule->logic());
    moduleWidget->setSegmentationsLogic(segmentationsLogic);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Segmentations module is not found";
  }

  qSlicerAbstractCoreModule* beamsModule = qSlicerCoreApplication::application()->moduleManager()->module("Beams");
  if (beamsModule)
  {
    vtkSlicerBeamsModuleLogic* beamsLogic = vtkSlicerBeamsModuleLogic::SafeDownCast(beamsModule->logic());
    moduleWidget->setBeamsLogic(beamsLogic);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Beams module is not found";
  }

  qSlicerAbstractCoreModule* modelsModule = qSlicerCoreApplication::application()->moduleManager()->module("Models");
  if (modelsModule)
  {
    vtkSlicerModelsLogic* modelsLogic = vtkSlicerModelsLogic::SafeDownCast(modelsModule->logic());
    moduleWidget->setModelsLogic(modelsLogic);
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Models module is not found";
  }

  return moduleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic*
qSlicerMlcPositionModule::createLogic()
{
  return vtkSlicerMlcPositionLogic::New();
}
