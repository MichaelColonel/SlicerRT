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

// LoadableModuleTemplate Logic includes
#include <vtkSlicerLoadableModuleTemplateLogic.h>

// LoadableModuleTemplate includes
#include "qSlicerLoadableModuleTemplateModule.h"
#include "qSlicerLoadableModuleTemplateModuleWidget.h"

// Beams Module includes
#include <vtkSlicerBeamsModuleLogic.h>

// Segmentations Module includes
#include <qSlicerSegmentationsModule.h>
#include <vtkSlicerSegmentationsModuleLogic.h>

// Slicer includes
#include <qSlicerIOManager.h>
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <vtkSlicerConfigure.h> // For Slicer_USE_PYTHONQT

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerLoadableModuleTemplateModulePrivate {
public:
  qSlicerLoadableModuleTemplateModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerLoadableModuleTemplateModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerLoadableModuleTemplateModulePrivate::qSlicerLoadableModuleTemplateModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerLoadableModuleTemplateModule methods

//-----------------------------------------------------------------------------
qSlicerLoadableModuleTemplateModule::qSlicerLoadableModuleTemplateModule(QObject* parent)
  :
  Superclass(parent),
  d_ptr(new qSlicerLoadableModuleTemplateModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerLoadableModuleTemplateModule::~qSlicerLoadableModuleTemplateModule()
{
}

//-----------------------------------------------------------------------------
QString
qSlicerLoadableModuleTemplateModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString
qSlicerLoadableModuleTemplateModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList
qSlicerLoadableModuleTemplateModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Mikhail Polkovnikov (NRC \"Kurchatov Institute\" - IHEP)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon
qSlicerLoadableModuleTemplateModule::icon() const
{
  return QIcon(":/Icons/LoadableModuleTemplate.png");
}

//-----------------------------------------------------------------------------
QStringList
qSlicerLoadableModuleTemplateModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList
qSlicerLoadableModuleTemplateModule::dependencies() const
{
  return QStringList() << "Segmentations" << "SubjectHierarchy" << "Beams";
//  return QStringList() << "Segmentations" << "SubjectHierarchy" << "SlicerRT";
}

//-----------------------------------------------------------------------------
void
qSlicerLoadableModuleTemplateModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation*
qSlicerLoadableModuleTemplateModule::createWidgetRepresentation()
{
  qSlicerLoadableModuleTemplateModuleWidget* moduleWidget = new qSlicerLoadableModuleTemplateModuleWidget();
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

  return moduleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic*
qSlicerLoadableModuleTemplateModule::createLogic()
{
  return vtkSlicerLoadableModuleTemplateLogic::New();
}
