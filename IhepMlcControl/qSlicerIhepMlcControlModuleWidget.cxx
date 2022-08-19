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
#include <QTimer>

// Slicer includes
#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>

#include "qSlicerIhepMlcControlModuleWidget.h"
#include "ui_qSlicerIhepMlcControlModuleWidget.h"

#include "qSlicerIhepMlcControlLayoutWidget.h"

// MRML includes
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLLayoutLogic.h>

// Logic includes
#include "vtkSlicerIhepMlcControlLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerIhepMlcControlModuleWidgetPrivate: public Ui_qSlicerIhepMlcControlModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerIhepMlcControlModuleWidget);
protected:
  qSlicerIhepMlcControlModuleWidget* const q_ptr;
public:
  qSlicerIhepMlcControlModuleWidgetPrivate(qSlicerIhepMlcControlModuleWidget &object);
  virtual ~qSlicerIhepMlcControlModuleWidgetPrivate();
  vtkSlicerIhepMlcControlLogic* logic() const;

  void fillLeavesControlContainer(int numberOfPairs);

  qSlicerIhepMlcControlLayoutWidget* MlcControlWidget{ nullptr };
  int PreviousLayoutId{ 0 };
  int MlcCustomLayoutId{ 507 };
};

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidgetPrivate::qSlicerIhepMlcControlModuleWidgetPrivate(qSlicerIhepMlcControlModuleWidget &object)
  :
  q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidgetPrivate::~qSlicerIhepMlcControlModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerIhepMlcControlLogic* qSlicerIhepMlcControlModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerIhepMlcControlModuleWidget);
  return vtkSlicerIhepMlcControlLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidgetPrivate::fillLeavesControlContainer(int numberOfPairs)
{
  Q_Q(const qSlicerIhepMlcControlModuleWidget);
  if (!this->MlcControlWidget)
  {
    return;
  }

  for (int i = 0; i < numberOfPairs; ++i)
  {
    this->MlcControlWidget->fillLeavesControlContainer(i);
  }
}

//-----------------------------------------------------------------------------
// qSlicerIhepMlcControlModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerIhepMlcControlModuleWidget::qSlicerIhepMlcControlModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerIhepMlcControlModuleWidgetPrivate(*this) )
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

  d->MlcControlWidget = new qSlicerIhepMlcControlLayoutWidget;

  qSlicerSingletonViewFactory* viewFactory = new qSlicerSingletonViewFactory();
  viewFactory->setWidget(d->MlcControlWidget);
  viewFactory->setTagName("MlcControlLayout");

  const char* layoutString = \
    "<layout type=\"vertical\">" \
    " <item>" \
    "  <MlcControlLayout></MlcControlLayout>" \
    " </item>" \
    "</layout>";

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->registerViewFactory(viewFactory);
  // Save previous layout
  d->PreviousLayoutId = layoutManager->layout();

  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  if (layoutNode)
    {
    layoutNode->AddLayoutDescription( d->MlcCustomLayoutId, layoutString);
    }

  // Buttons
  QObject::connect( d->PushButton_SwitchLayout, SIGNAL(toggled(bool)),
    this, SLOT(onSwitchToMlcControlLayoutToggled(bool)));
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::exit()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->Superclass::exit();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(d->PreviousLayoutId);
}


//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::enter()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  this->Superclass::enter();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  d->PreviousLayoutId = layoutManager->layout();
  layoutManager->setLayout(d->MlcCustomLayoutId);
  QTimer::singleShot(100, this, SLOT(onSetMlcControlLayout()));
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSwitchToMlcControlLayoutToggled(bool toggled)
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(toggled ? d->MlcCustomLayoutId : d->PreviousLayoutId);
}

//-----------------------------------------------------------------------------
void qSlicerIhepMlcControlModuleWidget::onSetMlcControlLayout()
{
  Q_D(qSlicerIhepMlcControlModuleWidget);
  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();
  layoutManager->setLayout(d->MlcCustomLayoutId);
  QSignalBlocker blocker1(d->PushButton_SwitchLayout);
  d->PushButton_SwitchLayout->setChecked(true);
}
