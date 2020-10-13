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

#include <vtkMRMLRTImageNode.h>

// VTK includes
#include <vtkWeakPointer.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLNode.h>

// Qt includes
#include <QDebug>

// PlastimatchParameters Widgets includes
#include "qSlicerRtImagePlastimatchParametersWidget.h"
#include "ui_qSlicerRtImagePlastimatchParametersWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_RtImage
class qSlicerRtImagePlastimatchParametersWidgetPrivate : public Ui_qSlicerRtImagePlastimatchParametersWidget
{
  Q_DECLARE_PUBLIC(qSlicerRtImagePlastimatchParametersWidget);
protected:
  qSlicerRtImagePlastimatchParametersWidget* const q_ptr;

public:
  qSlicerRtImagePlastimatchParametersWidgetPrivate(
    qSlicerRtImagePlastimatchParametersWidget& object);
  virtual void setupUi(qSlicerRtImagePlastimatchParametersWidget*);
  void init();
  
  /// RT Image MRML node containing shown parameters
  vtkWeakPointer<vtkMRMLRTImageNode> ParameterNode;
};

// --------------------------------------------------------------------------
qSlicerRtImagePlastimatchParametersWidgetPrivate::qSlicerRtImagePlastimatchParametersWidgetPrivate(
  qSlicerRtImagePlastimatchParametersWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerRtImagePlastimatchParametersWidgetPrivate::setupUi(qSlicerRtImagePlastimatchParametersWidget* widget)
{
  this->Ui_qSlicerRtImagePlastimatchParametersWidget::setupUi(widget);
}

// --------------------------------------------------------------------------
void qSlicerRtImagePlastimatchParametersWidgetPrivate::init()
{
  Q_Q(qSlicerRtImagePlastimatchParametersWidget);

  // Range widgets
  QObject::connect( this->RangeWidget_IntensityRange, SIGNAL(valuesChanged( double, double)), 
    q, SLOT(onAutoscaleIntensityRangeChanged( double, double)));

  // Buttons
  QObject::connect( this->CheckBox_UseExponentialMapping, SIGNAL(toggled(bool)), q, SLOT(onUseExponentialMappingToggled(bool)));
  QObject::connect( this->CheckBox_AutoscaleIntensity, SIGNAL(toggled(bool)), q, SLOT(onAutoscalePixelsRangeToggled(bool)));

  // Button groups
  QObject::connect( this->ButtonGroup_ReconstructAlgorithm, SIGNAL(buttonClicked(int)), q, SLOT(onReconstructionAlgorithmChanged(int)));
  QObject::connect( this->ButtonGroup_Threading, SIGNAL(buttonClicked(int)), q, SLOT(onThreadingChanged(int)));
  QObject::connect( this->ButtonGroup_HuConversion, SIGNAL(buttonClicked(int)), q, SLOT(onHUConversionChanged(int)));
}

//-----------------------------------------------------------------------------
// qSlicerRtImagePlastimatchParametersWidget methods

//-----------------------------------------------------------------------------
qSlicerRtImagePlastimatchParametersWidget::qSlicerRtImagePlastimatchParametersWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerRtImagePlastimatchParametersWidgetPrivate(*this) )
{
  Q_D(qSlicerRtImagePlastimatchParametersWidget);
  d->setupUi(this);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerRtImagePlastimatchParametersWidget::~qSlicerRtImagePlastimatchParametersWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerRtImagePlastimatchParametersWidget::setRtImageNode(vtkMRMLNode* node)
{
  Q_D(qSlicerRtImagePlastimatchParametersWidget);

  vtkMRMLRTImageNode* parameterNode = vtkMRMLRTImageNode::SafeDownCast(node);
  
    // Connect display modified event to population of the table
  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent,
                 this, SLOT( updateWidgetFromMRML() ) );

  d->ParameterNode = parameterNode;
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerRtImagePlastimatchParametersWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerRtImagePlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  d->CheckBox_UseExponentialMapping->setChecked(d->ParameterNode->GetExponentialMappingFlag());
  d->CheckBox_AutoscaleIntensity->setChecked(d->ParameterNode->GetAutoscaleFlag());

  switch (d->ParameterNode->GetAlgorithmReconstuction())
  {
    case vtkMRMLRTImageNode::PlastimatchAlgorithmReconstuctionType::EXACT:
      d->RadioButton_Exact->setChecked(true);
      break;
    case vtkMRMLRTImageNode::PlastimatchAlgorithmReconstuctionType::UNIFORM:
      d->RadioButton_Uniform->setChecked(true);
      break;
    default:
      break;
  }

  switch (d->ParameterNode->GetHUConversion())
  {
    case vtkMRMLRTImageNode::PlastimatchHounsfieldUnitsConversionType::PREPROCESS:
      d->RadioButton_Preprocess->setChecked(true);
      break;
    case vtkMRMLRTImageNode::PlastimatchHounsfieldUnitsConversionType::INLINE:
      d->RadioButton_Inline->setChecked(true);
      break;
    case vtkMRMLRTImageNode::PlastimatchHounsfieldUnitsConversionType::NONE:
      d->RadioButton_None->setChecked(true);
      break;
    default:
      break;
  }

  switch (d->ParameterNode->GetThreading())
  {
    case vtkMRMLRTImageNode::PlastimatchThreadingType::CPU:
      d->RadioButton_CPU->setChecked(true);
      break;
    case vtkMRMLRTImageNode::PlastimatchThreadingType::CUDA:
      d->RadioButton_CUDA->setChecked(true);
      break;
    case vtkMRMLRTImageNode::PlastimatchThreadingType::OPENCL:
      d->RadioButton_OpenCL->setChecked(true);
      break;
    default:
      break;
  }
}

//-----------------------------------------------------------------------------
void qSlicerRtImagePlastimatchParametersWidget::onReconstructionAlgorithmChanged(int button_id)
{
  Q_D(qSlicerRtImagePlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  QAbstractButton* button = d->ButtonGroup_ReconstructAlgorithm->button(button_id);
  QRadioButton* rbutton = qobject_cast<QRadioButton*>(button);

  if (rbutton == d->RadioButton_Exact)
  {
    d->ParameterNode->SetAlgorithmReconstuction(vtkMRMLRTImageNode::PlastimatchAlgorithmReconstuctionType::EXACT);
  }
  else if (rbutton == d->RadioButton_Uniform)
  {
    d->ParameterNode->SetAlgorithmReconstuction(vtkMRMLRTImageNode::PlastimatchAlgorithmReconstuctionType::UNIFORM);
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Invalid reconstruct algorithm button id";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerRtImagePlastimatchParametersWidget::onThreadingChanged(int button_id)
{
  Q_D(qSlicerRtImagePlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  QAbstractButton* button = d->ButtonGroup_Threading->button(button_id);
  QRadioButton* rbutton = qobject_cast<QRadioButton*>(button);

  if (rbutton == d->RadioButton_CPU)
  {
    d->ParameterNode->SetThreading(vtkMRMLRTImageNode::PlastimatchThreadingType::CPU);
  }
  else if (rbutton == d->RadioButton_CUDA)
  {
    d->ParameterNode->SetThreading(vtkMRMLRTImageNode::PlastimatchThreadingType::CUDA);
  }
  else if (rbutton == d->RadioButton_OpenCL)
  {
    d->ParameterNode->SetThreading(vtkMRMLRTImageNode::PlastimatchThreadingType::OPENCL);
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Invalid threading button id";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerRtImagePlastimatchParametersWidget::onHUConversionChanged(int button_id)
{
  Q_D(qSlicerRtImagePlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  QAbstractButton* button = d->ButtonGroup_HuConversion->button(button_id);
  QRadioButton* rbutton = qobject_cast<QRadioButton*>(button);

  if (rbutton == d->RadioButton_None)
  {
    d->ParameterNode->SetHUConversion(vtkMRMLRTImageNode::PlastimatchHounsfieldUnitsConversionType::NONE);
  }
  else if (rbutton == d->RadioButton_Inline)
  {
    d->ParameterNode->SetHUConversion(vtkMRMLRTImageNode::PlastimatchHounsfieldUnitsConversionType::INLINE);
  }
  else if (rbutton == d->RadioButton_Preprocess)
  {
    d->ParameterNode->SetHUConversion(vtkMRMLRTImageNode::PlastimatchHounsfieldUnitsConversionType::PREPROCESS);
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Invalid Hounsfield units conversion button id";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerRtImagePlastimatchParametersWidget::onUseExponentialMappingToggled(bool value)
{
  Q_D(qSlicerRtImagePlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  d->ParameterNode->SetExponentialMappingFlag(value);
}

//-----------------------------------------------------------------------------
void qSlicerRtImagePlastimatchParametersWidget::onAutoscalePixelsRangeToggled(bool value)
{
  Q_D(qSlicerRtImagePlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  d->ParameterNode->SetAutoscaleFlag(value);
}

//-----------------------------------------------------------------------------
void qSlicerRtImagePlastimatchParametersWidget::onAutoscaleIntensityRangeChanged( double min, double max)
{
  Q_D(qSlicerRtImagePlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetAutoscaleRange( min, max);
}
