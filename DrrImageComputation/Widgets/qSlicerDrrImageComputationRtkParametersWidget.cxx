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
  this->Ui_qSlicerDrrImageComputationRtkParametersWidget::setupUi(widget);
}

// --------------------------------------------------------------------------
void qSlicerDrrImageComputationRtkParametersWidgetPrivate::init()
{
  Q_Q(qSlicerDrrImageComputationRtkParametersWidget);

  // Button groups
  QObject::connect( this->ButtonGroup_ForwardProjectionFilter, SIGNAL(buttonClicked(int)), q, SLOT(onForwardProjectionFilterChanged(int)));
  // Slider
  QObject::connect( this->SliderWidget_ForwardProjectionFilterParameter, SIGNAL(valueChanged(int)), q, SLOT(onForwardProjectionFilterParameterChanged(double)));
  QObject::connect( this->SliderWidget_FillPadPixels, SIGNAL(valueChanged(int)), q, SLOT(onFillPadValueChanged(double)));
  
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
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationRtkParametersWidget::onForwardProjectionFilterChanged(int button_id)
{
  Q_D(qSlicerDrrImageComputationRtkParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  QAbstractButton* button = d->ButtonGroup_ForwardProjectionFilter->button(button_id);
  QRadioButton* rbutton = qobject_cast<QRadioButton*>(button);

  if (rbutton == d->RadioButton_Joseph)
  {
    d->ParameterNode->SetRtkForwardProjection(vtkMRMLDrrImageComputationNode::JOSEPH);
    d->SliderWidget_ForwardProjectionFilterParameter->setEnabled(false);
    d->Label_ForwardProjectionParameter->setText(QObject::tr("Parameter:"));
    d->ParameterNode->SetAttribute( "RTK_ForwardProjectionFilterParameter", "");
  }
  else if (rbutton == d->RadioButton_JosephAttenuated)
  {
    d->ParameterNode->SetRtkForwardProjection(vtkMRMLDrrImageComputationNode::JOSEPH_ATTENUATED);
    d->SliderWidget_ForwardProjectionFilterParameter->setEnabled(true);
    d->Label_ForwardProjectionParameter->setText(QObject::tr("Sigma zero:"));
  }
  else if (rbutton == d->RadioButton_Zeng)
  {
    d->ParameterNode->SetRtkForwardProjection(vtkMRMLDrrImageComputationNode::ZENG);
    d->SliderWidget_ForwardProjectionFilterParameter->setEnabled(true);
    d->Label_ForwardProjectionParameter->setText(QObject::tr("Alpha PSF:"));
  }
  else if (rbutton == d->RadioButton_CudaRayCast)
  {
    d->ParameterNode->SetRtkForwardProjection(vtkMRMLDrrImageComputationNode::CUDA_RAYCAST);
    d->SliderWidget_ForwardProjectionFilterParameter->setEnabled(false);
    d->Label_ForwardProjectionParameter->setText(QObject::tr("Parameter:"));
    d->ParameterNode->SetAttribute( "RTK_ForwardProjectionFilterParameter", "");
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Invalid Hounsfield units conversion button id";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationRtkParametersWidget::onForwardProjectionFilterParameterChanged(double value)
{
  Q_D(qSlicerDrrImageComputationRtkParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  std::string valueString = std::to_string(value);
  d->ParameterNode->SetAttribute( "RTK_ForwardProjectionFilterParameter", valueString.c_str());
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationRtkParametersWidget::onFillPadValueChanged(double value)
{
  Q_D(qSlicerDrrImageComputationRtkParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetFillPadSize(static_cast<int>(value));
}
