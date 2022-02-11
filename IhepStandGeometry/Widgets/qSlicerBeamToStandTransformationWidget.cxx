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

// SlicerRT IhepStandGeometry MRML includes
#include <vtkMRMLIhepStandGeometryNode.h>

// VTK includes
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

// Qt includes
#include <QDebug>

// FooBar Widgets includes
#include "qSlicerBeamToStandTransformationWidget.h"
#include "ui_qSlicerBeamToStandTransformationWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_IhepStandGeometry
class qSlicerBeamToStandTransformationWidgetPrivate
  : public Ui_qSlicerBeamToStandTransformationWidget
{
  Q_DECLARE_PUBLIC(qSlicerBeamToStandTransformationWidget);
protected:
  qSlicerBeamToStandTransformationWidget* const q_ptr;

public:
  qSlicerBeamToStandTransformationWidgetPrivate(
    qSlicerBeamToStandTransformationWidget& object);
  virtual void setupUi(qSlicerBeamToStandTransformationWidget*);

  void init();
  
  /// IhepStandGeometry MRML node containing shown parameters
  vtkWeakPointer<vtkMRMLIhepStandGeometryNode> ParameterNode;
};

// --------------------------------------------------------------------------
qSlicerBeamToStandTransformationWidgetPrivate::qSlicerBeamToStandTransformationWidgetPrivate(
  qSlicerBeamToStandTransformationWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerBeamToStandTransformationWidgetPrivate::setupUi(qSlicerBeamToStandTransformationWidget* widget)
{
  this->Ui_qSlicerBeamToStandTransformationWidget::setupUi(widget);
}


// --------------------------------------------------------------------------
void qSlicerBeamToStandTransformationWidgetPrivate::init()
{
  Q_Q(qSlicerBeamToStandTransformationWidget);

  // Buttons
  QObject::connect( this->PushButton_Translate, SIGNAL(clicked()), 
    q, SIGNAL(translatePatientToFixedIsocenter()));

/*
  // Range widgets
  QObject::connect( this->RangeWidget_IntensityRange, SIGNAL(valuesChanged( double, double)), 
    q, SLOT(onAutoscaleIntensityRangeChanged( double, double)));

  // Slicer widgets
  QObject::connect( this->SliderWidget_HounsfieldThreshold, SIGNAL(valueChanged(double)), 
    q, SLOT(onHUThresholdChanged(double)));

  // Buttons
  QObject::connect( this->CheckBox_UseExponentialMapping, SIGNAL(toggled(bool)), 
    q, SLOT(onUseExponentialMappingToggled(bool)));
  QObject::connect( this->CheckBox_AutoscaleIntensity, SIGNAL(toggled(bool)), 
    q, SLOT(onAutoscalePixelsRangeToggled(bool)));
  QObject::connect( this->CheckBox_InvertIntensity, SIGNAL(toggled(bool)), 
    q, SLOT(onInvertIntensityToggled(bool)));

  // Combo Boxes
  QObject::connect( this->ComboBox_ReconstructionAlgorithm, SIGNAL(currentIndexChanged(int)), 
    q, SLOT(onReconstructionAlgorithmChanged(int)));
  QObject::connect( this->ComboBox_Threading, SIGNAL(currentIndexChanged(int)), 
    q, SLOT(onThreadingChanged(int)));
  QObject::connect( this->ComboBox_HounsfieldConversion, SIGNAL(currentIndexChanged(int)), 
    q, SLOT(onHUConversionChanged(int)));
*/
}

//-----------------------------------------------------------------------------
// qSlicerBeamToStandTransformationWidget methods

//-----------------------------------------------------------------------------
qSlicerBeamToStandTransformationWidget
::qSlicerBeamToStandTransformationWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerBeamToStandTransformationWidgetPrivate(*this) )
{
  Q_D(qSlicerBeamToStandTransformationWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerBeamToStandTransformationWidget::~qSlicerBeamToStandTransformationWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerBeamToStandTransformationWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerBeamToStandTransformationWidget);

  vtkMRMLIhepStandGeometryNode* parameterNode = vtkMRMLIhepStandGeometryNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT( updateWidgetFromMRML() ) );

  d->ParameterNode = parameterNode;
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerBeamToStandTransformationWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerBeamToStandTransformationWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  qCritical() << Q_FUNC_INFO << ": NODE IS VALID";
}

//-----------------------------------------------------------------------------
void qSlicerBeamToStandTransformationWidget::setIsocenterTranslation(double translate[3])
{
  Q_D(qSlicerBeamToStandTransformationWidget);
  d->CoordinatesWidget_BeamToFixedIsocenterTranslate->setCoordinates(translate);
}

//-----------------------------------------------------------------------------
void qSlicerBeamToStandTransformationWidget::setTransformMatrix(const vtkMatrix4x4* transformMatrix)
{
  Q_D(qSlicerBeamToStandTransformationWidget);
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      double v = transformMatrix->GetElement( i, j );
      d->MatrixWidget_BeamToFixedBeamTransform->setValue( i, j, v );
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerBeamToStandTransformationWidget::setTransformMatrix(vtkTransform* transform)
{
  Q_D(qSlicerBeamToStandTransformationWidget);
  vtkMatrix4x4* matrix = transform->GetMatrix();
  this->setTransformMatrix(matrix);
}
