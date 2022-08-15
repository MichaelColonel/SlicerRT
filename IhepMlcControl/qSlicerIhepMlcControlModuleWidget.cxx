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

// Qt includes
#include <QDebug>

// Slicer includes
#include "qSlicerIhepMlcControlModuleWidget.h"
#include "ui_qSlicerIhepMlcControlModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerIhepMlcControlModuleWidgetPrivate: public Ui_qSlicerIhepMlcControlModuleWidget
{
public:
  qSlicerIhepMlcControlModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidgetPrivate::qSlicerIhepMlcControlModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidget::qSlicerIhepMlcControlModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerIhepMlcControlModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidget::~qSlicerIhepMlcControlModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::setup()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
/*
  this->testLabel = new QLabel(tr("Test me"));

  qSlicerSingletonViewFactory* viewFactory = new qSlicerSingletonViewFactory();
  viewFactory->setWidget(this->testLabel);
  viewFactory->setTagName("helloLayout");

  const char* layoutString = \
    "<layout type=\"horizontal\">" \
    " <item>" \
    "  <helloLayout></helloLayout>" \
    " </item>" \
    "</layout>";

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->registerViewFactory(viewFactory);

  int customLayoutId = 42;

  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  if (layoutNode)
    {
    layoutNode->AddLayoutDescription( customLayoutId, layoutString);
    }

//  layoutManager.setLayout(customLayoutId)
*/
}
