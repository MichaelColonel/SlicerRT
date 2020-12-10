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
#include "qSlicerDrrImageComputationPlastimatchParametersWidget.h"
#include "ui_qSlicerDrrImageComputationPlastimatchParametersWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_DrrImageComputation
class qSlicerDrrImageComputationPlastimatchParametersWidgetPrivate : public Ui_qSlicerDrrImageComputationPlastimatchParametersWidget
{
  Q_DECLARE_PUBLIC(qSlicerDrrImageComputationPlastimatchParametersWidget);
protected:
  qSlicerDrrImageComputationPlastimatchParametersWidget* const q_ptr;

public:
  qSlicerDrrImageComputationPlastimatchParametersWidgetPrivate(
    qSlicerDrrImageComputationPlastimatchParametersWidget& object);
  virtual void setupUi(qSlicerDrrImageComputationPlastimatchParametersWidget*);
  void init();
  
  /// RT Image MRML node containing shown parameters
  vtkWeakPointer<vtkMRMLDrrImageComputationNode> ParameterNode;
};

// --------------------------------------------------------------------------
qSlicerDrrImageComputationPlastimatchParametersWidgetPrivate::qSlicerDrrImageComputationPlastimatchParametersWidgetPrivate(
  qSlicerDrrImageComputationPlastimatchParametersWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidgetPrivate::setupUi(qSlicerDrrImageComputationPlastimatchParametersWidget* widget)
{
  this->Ui_qSlicerDrrImageComputationPlastimatchParametersWidget::setupUi(widget);
}

// --------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidgetPrivate::init()
{
  Q_Q(qSlicerDrrImageComputationPlastimatchParametersWidget);

  // Range widgets
  QObject::connect( this->RangeWidget_IntensityRange, SIGNAL(valuesChanged( double, double)), 
    q, SLOT(onAutoscaleIntensityRangeChanged( double, double)));

  // Buttons
  QObject::connect( this->CheckBox_UseExponentialMapping, SIGNAL(toggled(bool)), q, SLOT(onUseExponentialMappingToggled(bool)));
  QObject::connect( this->CheckBox_AutoscaleIntensity, SIGNAL(toggled(bool)), q, SLOT(onAutoscalePixelsRangeToggled(bool)));
  QObject::connect( this->CheckBox_InvertIntensity, SIGNAL(toggled(bool)), q, SLOT(onInvertIntensityToggled(bool)));
  QObject::connect( this->GroupBox_UseImageWindow, SIGNAL(toggled(bool)), q, SLOT(onUseImageWindowToggled(bool)));

  // Button groups
  QObject::connect( this->ButtonGroup_ReconstructAlgorithm, SIGNAL(buttonClicked(int)), q, SLOT(onReconstructionAlgorithmChanged(int)));
  QObject::connect( this->ButtonGroup_Threading, SIGNAL(buttonClicked(int)), q, SLOT(onThreadingChanged(int)));
  QObject::connect( this->ButtonGroup_HuConversion, SIGNAL(buttonClicked(int)), q, SLOT(onHUConversionChanged(int)));

  // Coordinates widgets
  QObject::connect( this->RangeWidget_ImageWindowColumns, SIGNAL(valuesChanged( double, double)), 
    q, SLOT(onImageWindowColumnsValuesChanged( double, double)));
  QObject::connect( this->RangeWidget_ImageWindowRows, SIGNAL(valuesChanged( double, double)), 
    q, SLOT(onImageWindowRowsValuesChanged( double, double)));
}

//-----------------------------------------------------------------------------
// qSlicerDrrImageComputationPlastimatchParametersWidget methods

//-----------------------------------------------------------------------------
qSlicerDrrImageComputationPlastimatchParametersWidget::qSlicerDrrImageComputationPlastimatchParametersWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerDrrImageComputationPlastimatchParametersWidgetPrivate(*this) )
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);
  d->setupUi(this);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerDrrImageComputationPlastimatchParametersWidget::~qSlicerDrrImageComputationPlastimatchParametersWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  vtkMRMLDrrImageComputationNode* parameterNode = vtkMRMLDrrImageComputationNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT( updateWidgetFromMRML() ) );

  d->ParameterNode = parameterNode;
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  d->plainTextEdit_PlastimatchDrrArguments->clear();

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  // Image sub-window
  int imagerResolution[2] = {};
  d->ParameterNode->GetImagerResolution(imagerResolution);

  d->RangeWidget_ImageWindowColumns->setMinimum(0.);
  d->RangeWidget_ImageWindowColumns->setMaximum(double(imagerResolution[0] - 1));
  d->RangeWidget_ImageWindowRows->setMinimum(0.);
  d->RangeWidget_ImageWindowRows->setMaximum(double(imagerResolution[1] - 1));

  bool useImageWindow = d->ParameterNode->GetImageWindowFlag();
  int imageWindow[4] = {};
  d->ParameterNode->GetImageWindow(imageWindow);

  d->GroupBox_UseImageWindow->setChecked(useImageWindow);
  if (!useImageWindow)
  {
    d->RangeWidget_ImageWindowColumns->setValues( 0., double(imagerResolution[0] - 1));
    d->RangeWidget_ImageWindowRows->setValues( 0., double(imagerResolution[1] - 1));
  }
  else
  {
    d->RangeWidget_ImageWindowColumns->setValues( 
      static_cast<double>(std::max<int>( 0, imageWindow[0])),
      static_cast<double>(std::min<int>( imagerResolution[0] - 1, imageWindow[2])));
    d->RangeWidget_ImageWindowRows->setValues( 
      static_cast<double>(std::max<int>( 0, imageWindow[1])), 
      static_cast<double>(std::min<int>( imagerResolution[1] - 1, imageWindow[3])));
  }

  // Update widgets info from parameter node and update plastimatch drr command
  d->CheckBox_UseExponentialMapping->setChecked(d->ParameterNode->GetExponentialMappingFlag());
  d->CheckBox_AutoscaleIntensity->setChecked(d->ParameterNode->GetAutoscaleFlag());
  d->CheckBox_InvertIntensity->setChecked(d->ParameterNode->GetInvertIntensityFlag());

  float autoscaleRange[2] = { 0.f, 255.f };
  d->ParameterNode->GetAutoscaleRange(autoscaleRange);
  d->RangeWidget_IntensityRange->setValues( autoscaleRange[0], autoscaleRange[1]);

  switch (d->ParameterNode->GetAlgorithmReconstuction())
  {
  case vtkMRMLDrrImageComputationNode::EXACT:
    d->RadioButton_Exact->setChecked(true);
    break;
  case vtkMRMLDrrImageComputationNode::UNIFORM:
    d->RadioButton_Uniform->setChecked(true);
    break;
  default:
    break;
  }

  switch (d->ParameterNode->GetHUConversion())
  {
  case vtkMRMLDrrImageComputationNode::PREPROCESS:
    d->RadioButton_Preprocess->setChecked(true);
    break;
  case vtkMRMLDrrImageComputationNode::INLINE:
    d->RadioButton_Inline->setChecked(true);
    break;
  case vtkMRMLDrrImageComputationNode::NONE:
    d->RadioButton_None->setChecked(true);
    break;
  default:
    break;
  }

  switch (d->ParameterNode->GetThreading())
  {
  case vtkMRMLDrrImageComputationNode::CPU:
    d->RadioButton_CPU->setChecked(true);
    break;
  case vtkMRMLDrrImageComputationNode::CUDA:
    d->RadioButton_CUDA->setChecked(true);
    break;
  case vtkMRMLDrrImageComputationNode::OPENCL:
    d->RadioButton_OpenCL->setChecked(true);
    break;
  default:
    break;
  }
  this->updatePlastimatchDrrArguments();
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::onReconstructionAlgorithmChanged(int button_id)
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  QAbstractButton* button = d->ButtonGroup_ReconstructAlgorithm->button(button_id);
  QRadioButton* rbutton = qobject_cast<QRadioButton*>(button);

  if (rbutton == d->RadioButton_Exact)
  {
    d->ParameterNode->SetAlgorithmReconstuction(vtkMRMLDrrImageComputationNode::EXACT);
  }
  else if (rbutton == d->RadioButton_Uniform)
  {
    d->ParameterNode->SetAlgorithmReconstuction(vtkMRMLDrrImageComputationNode::UNIFORM);
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Invalid reconstruct algorithm button id";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::onThreadingChanged(int button_id)
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  QAbstractButton* button = d->ButtonGroup_Threading->button(button_id);
  QRadioButton* rbutton = qobject_cast<QRadioButton*>(button);

  if (rbutton == d->RadioButton_CPU)
  {
    d->ParameterNode->SetThreading(vtkMRMLDrrImageComputationNode::CPU);
  }
  else if (rbutton == d->RadioButton_CUDA)
  {
    d->ParameterNode->SetThreading(vtkMRMLDrrImageComputationNode::CUDA);
  }
  else if (rbutton == d->RadioButton_OpenCL)
  {
    d->ParameterNode->SetThreading(vtkMRMLDrrImageComputationNode::OPENCL);
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Invalid threading button id";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::onHUConversionChanged(int button_id)
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  QAbstractButton* button = d->ButtonGroup_HuConversion->button(button_id);
  QRadioButton* rbutton = qobject_cast<QRadioButton*>(button);

  if (rbutton == d->RadioButton_None)
  {
    d->ParameterNode->SetHUConversion(vtkMRMLDrrImageComputationNode::NONE);
  }
  else if (rbutton == d->RadioButton_Inline)
  {
    d->ParameterNode->SetHUConversion(vtkMRMLDrrImageComputationNode::INLINE);
  }
  else if (rbutton == d->RadioButton_Preprocess)
  {
    d->ParameterNode->SetHUConversion(vtkMRMLDrrImageComputationNode::PREPROCESS);
  }
  else
  {
    qWarning() << Q_FUNC_INFO << ": Invalid Hounsfield units conversion button id";
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::onUseExponentialMappingToggled(bool value)
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  d->ParameterNode->SetExponentialMappingFlag(value);
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::onAutoscalePixelsRangeToggled(bool value)
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  d->ParameterNode->SetAutoscaleFlag(value);
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::onInvertIntensityToggled(bool value)
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  d->ParameterNode->SetInvertIntensityFlag(value);
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::onAutoscaleIntensityRangeChanged( double min, double max)
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->ParameterNode->SetAutoscaleRange( min, max);
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::onImageWindowColumnsValuesChanged(double start_column, double end_column)
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  int imageWindow[4];
  d->ParameterNode->GetImageWindow(imageWindow);
  
  imageWindow[0] = start_column;
  imageWindow[2] = end_column;

  d->ParameterNode->SetImageWindow(imageWindow);
  d->ParameterNode->Modified(); // Update imager and image markups, DRR arguments
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::onImageWindowRowsValuesChanged( double start_row, double end_row)
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  int imageWindow[4];
  d->ParameterNode->GetImageWindow(imageWindow);
  
  imageWindow[1] = start_row;
  imageWindow[3] = end_row;

  d->ParameterNode->SetImageWindow(imageWindow);
  d->ParameterNode->Modified(); // Update imager and image markups, DRR arguments
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::onUseImageWindowToggled(bool value)
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (value)
  {
    int imagerResolution[2] = {};
    int imageWindow[4] = {};
    d->ParameterNode->GetImagerResolution(imagerResolution);

    double columns[2], rows[2];
    d->RangeWidget_ImageWindowColumns->values( columns[0], columns[1]);
    d->RangeWidget_ImageWindowRows->values( rows[0], rows[1]);

    imageWindow[0] = static_cast<int>(columns[0]); // c1 = x1
    imageWindow[1] = static_cast<int>(rows[0]); // r1 = y1
    imageWindow[2] = static_cast<int>(columns[1]); // c2 = x2
    imageWindow[3] = static_cast<int>(rows[1]); // r2 = y2

    d->ParameterNode->SetImageWindow(imageWindow);
  }

  d->ParameterNode->SetImageWindowFlag(value);
  d->ParameterNode->Modified(); // Update imager and image markups, DRR arguments
}

//-----------------------------------------------------------------------------
void qSlicerDrrImageComputationPlastimatchParametersWidget::updatePlastimatchDrrArguments()
{
  Q_D(qSlicerDrrImageComputationPlastimatchParametersWidget);

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  vtkMRMLRTBeamNode* beamNode = d->ParameterNode->GetBeamNode();
  if (!beamNode)
  {
    qWarning() << Q_FUNC_INFO << ": Invalid RT Beam node";
    return;
  }

  std::ostringstream command;
  command << "plastimatch drr ";
  switch (d->ParameterNode->GetThreading())
  {
  case vtkMRMLDrrImageComputationNode::CPU:
    command << "-A cpu \\\n";
    break;
  case vtkMRMLDrrImageComputationNode::CUDA:
    command << "-A cuda \\\n";
    break;
  case vtkMRMLDrrImageComputationNode::OPENCL:
    command << "-A opencl \\\n";
    break;
  default:
    break;
  }

  double n[3], vup[3]; // normal and view up in LPS coordinate system
  d->ParameterNode->GetNormalVector(n);
  d->ParameterNode->GetViewUpVector(vup);
  command << "\t--nrm" << " \"" << n[0] << " " << n[1] << " " << n[2] << "\" \\" << "\n";
  command << "\t--vup" << " \"" << vup[0] << " " << vup[1] << " " << vup[2] << "\" \\" << "\n";

  int imagerResolution[2];
  double imagerSpacing[2];
  double imageCenter[2];
  double isocenterPosition[3];
  d->ParameterNode->GetImagerResolution(imagerResolution);
  d->ParameterNode->GetImagerSpacing(imagerSpacing);
  d->ParameterNode->GetImageCenter(imageCenter);
  d->ParameterNode->GetIsocenterPositionLPS(isocenterPosition);

  command << "\t--sad " << beamNode->GetSAD() << " --sid " \
    << beamNode->GetSAD() + d->ParameterNode->GetIsocenterImagerDistance() << " \\" << "\n";
  command << "\t-r" << " \"" << imagerResolution[1] << " " \
    << imagerResolution[0] << "\" \\" << "\n";
  command << "\t-z" << " \"" << imagerResolution[1] * imagerSpacing[1] << " " \
    << imagerResolution[0] * imagerSpacing[0] << "\" \\" << "\n";
  command << "\t-c" << " \"" << imageCenter[1] << " " << imageCenter[0] << "\" \\" << "\n";

  int imageWindow[4];
  d->ParameterNode->GetImageWindow(imageWindow);
  // Isocenter LPS position
  command << "\t-o" << " \"" << isocenterPosition[0] << " " \
    << isocenterPosition[1] << " " << isocenterPosition[2] << "\" \\" << "\n";
  if (d->ParameterNode->GetImageWindowFlag())
  {
    command << "\t-w" << " \"" << imageWindow[1] << " " << imageWindow[3] << " " \
      << imageWindow[0] << " " << imageWindow[2] << "\" \\" << "\n";
  }

  if (d->ParameterNode->GetExponentialMappingFlag())
  {
    command << "\t-e ";
  }
  else
  {
    command << "\t ";
  }

  if (d->ParameterNode->GetAutoscaleFlag())
  {
    command << "--autoscale ";
  }
  float autoscaleRange[2];
  d->ParameterNode->GetAutoscaleRange(autoscaleRange);
  command << "--autoscale-range \"" << autoscaleRange[0] << " " << autoscaleRange[1] << "\" \\\n\t ";

  switch (d->ParameterNode->GetAlgorithmReconstuction())
  {
  case vtkMRMLDrrImageComputationNode::EXACT:
    command << "-i exact ";
    break;
  case vtkMRMLDrrImageComputationNode::UNIFORM:
    command << "-i uniform ";
    break;
  default:
    break;
  }

  switch (d->ParameterNode->GetHUConversion())
  {
  case vtkMRMLDrrImageComputationNode::NONE:
    command << "-P none ";
    break;
  case vtkMRMLDrrImageComputationNode::PREPROCESS:
    command << "-P preprocess ";
    break;
  case vtkMRMLDrrImageComputationNode::INLINE:
    command << "-P inline ";
    break;
  default:
    break;
  }

  command << "-O Out";

  d->plainTextEdit_PlastimatchDrrArguments->setPlainText(QString::fromStdString(command.str()));
}
