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
#include <QLabel>

// Slicer includes
#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>

#include "qSlicerDrrImageComparisonModuleWidget.h"
#include "ui_qSlicerDrrImageComparisonModuleWidget.h"

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLLayoutLogic.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerDrrImageComparisonModuleWidgetPrivate: public Ui_qSlicerDrrImageComparisonModuleWidget
{
public:
  qSlicerDrrImageComparisonModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerDrrImageComparisonModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerDrrImageComparisonModuleWidgetPrivate::qSlicerDrrImageComparisonModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerDrrImageComparisonModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerDrrImageComparisonModuleWidget::qSlicerDrrImageComparisonModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerDrrImageComparisonModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerDrrImageComparisonModuleWidget::~qSlicerDrrImageComparisonModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComparisonModuleWidget::setup()
{
  Q_D(qSlicerDrrImageComparisonModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
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

}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComparisonModuleWidget::exit()
{
  Q_D(qSlicerDrrImageComparisonModuleWidget);
  this->Superclass::exit();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(this->previousLayout);
}


//-----------------------------------------------------------------------------
void qSlicerDrrImageComparisonModuleWidget::enter()
{
  Q_D(qSlicerDrrImageComparisonModuleWidget);
  this->Superclass::enter();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  this->previousLayout = layoutManager->layout();
  layoutManager->setLayout(42);

/*
viewFactory = slicer.qSlicerSingletonViewFactory()
viewFactory.setWidget(mywidget)
viewFactory.setTagName("helloLayout")
layoutManager.registerViewFactory(viewFactory)

layout = (
    "<layout type=\"horizontal\">"
    " <item>"
    "  <helloLayout></helloLayout>"
    " </item>"
    "</layout>"
)

customLayoutId = 42

layoutNode = slicer.app.layoutManager().layoutLogic().GetLayoutNode()
layoutNode.AddLayoutDescription(customLayoutId, layout)

layoutManager.setLayout(customLayoutId)
*/

}
