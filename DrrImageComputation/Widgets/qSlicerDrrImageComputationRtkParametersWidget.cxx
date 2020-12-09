/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// VTK includes
#include <vtkWeakPointer.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLNode.h>

// SlicerRT Beams MRML includes
#include <vtkMRMLRTBeamNode.h>

// SlicerRT DrrImageComputation MRML includes
#include <vtkMRMLDrrImageComputationNode.h>

// Qt includes
#include <QDebug>

// PlastimatchParameters Widgets includes
#include "qSlicerDrrImageComputationRtkParametersWidget.h"
#include "ui_qSlicerDrrImageComputationRtkParametersWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_DrrImageComputation
class qSlicerDrrImageComputationRtkParametersWidgetPrivate : public Ui_qSlicerDrrImageComputationRtkParametersWidget
{
  Q_DECLARE_PUBLIC(qSlicerDrrImageComputationRtkParametersWidget);
protected:
  qSlicerDrrImageComputationRtkParametersWidget* const q_ptr;

public:
  qSlicerDrrImageComputationRtkParametersWidgetPrivate(
    qSlicerDrrImageComputationRtkParametersWidget& object);
  virtual void setupUi(qSlicerDrrImageComputationRtkParametersWidget*);
  void init();
  
  /// RT Image MRML node containing shown parameters
  vtkWeakPointer<vtkMRMLDrrImageComputationNode> ParameterNode;
};

// --------------------------------------------------------------------------
qSlicerDrrImageComputationRtkParametersWidgetPrivate::qSlicerDrrImageComputationRtkParametersWidgetPrivate(
  qSlicerDrrImageComputationRtkParametersWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerDrrImageComputationRtkParametersWidgetPrivate::setupUi(qSlicerDrrImageComputationRtkParametersWidget* widget)
{
  this->Ui_qSlicerDrrImageComputationPlastimatchParametersWidget::setupUi(widget);
}

// --------------------------------------------------------------------------
void qSlicerDrrImageComputationRtkParametersWidgetPrivate::init()
{
  Q_Q(qSlicerDrrImageComputationRtkParametersWidget);
}

//-----------------------------------------------------------------------------
// qSlicerDrrImageComputationRtkParametersWidget methods

//-----------------------------------------------------------------------------
qSlicerDrrImageComputationRtkParametersWidget::qSlicerDrrImageComputationRtkParametersWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerDrrImageComputationRtkParametersWidgetPrivate(*this) )
{
  Q_D(qSlicerDrrImageComputationRtkParametersWidget);
  d->setupUi(this);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerDrrImageComputationRtkParametersWidget::~qSlicerDrrImageComputationRtkParametersWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationRtkParametersWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerDrrImageComputationRtkParametersWidget);

  vtkMRMLDrrImageComputationNode* parameterNode = vtkMRMLDrrImageComputationNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT( updateWidgetFromMRML() ) );

  d->ParameterNode = parameterNode;
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationRtkParametersWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerDrrImageComputationRtkParametersWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
}
