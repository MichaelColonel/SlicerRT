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

// TestMe2 Logic includes
#include <vtkSlicerTestMe2Logic.h>

// TestMe2 includes
#include "qSlicerTestMe2Module.h"
#include "qSlicerTestMe2ModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerTestMe2ModulePrivate
{
public:
  qSlicerTestMe2ModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTestMe2ModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTestMe2ModulePrivate::qSlicerTestMe2ModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTestMe2Module methods

//-----------------------------------------------------------------------------
qSlicerTestMe2Module::qSlicerTestMe2Module(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTestMe2ModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTestMe2Module::~qSlicerTestMe2Module()
{
}

//-----------------------------------------------------------------------------
QString qSlicerTestMe2Module::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerTestMe2Module::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTestMe2Module::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerTestMe2Module::icon() const
{
  return QIcon(":/Icons/TestMe2.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTestMe2Module::categories() const
{
  return QStringList() << "Radiotherapy";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTestMe2Module::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerTestMe2Module::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerTestMe2Module
::createWidgetRepresentation()
{
  return new qSlicerTestMe2ModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTestMe2Module::createLogic()
{
  return vtkSlicerTestMe2Logic::New();
}
