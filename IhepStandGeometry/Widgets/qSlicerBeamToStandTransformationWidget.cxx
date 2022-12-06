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

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerAbstractCoreModule.h>
#include <qSlicerModuleManager.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLNode.h>

// SlicerRT IhepStandGeometry MRML includes
#include <vtkMRMLIhepStandGeometryNode.h>

// SlicerRT MRML IEC and IhepStandGeometry includes
#include <vtkSlicerIECTransformLogic.h>

#include <vtkMRMLRTBeamNode.h>
#include <vtkMRMLRTIonBeamNode.h>
#include <vtkMRMLRTFixedIonBeamNode.h>
#include <vtkMRMLRTFixedBeamNode.h>
#include <vtkMRMLRTPlanNode.h>

// Logic includes
#include "vtkSlicerIhepStandGeometryLogic.h"

// VTK includes
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

// Qt includes
#include <QDebug>

// Widgets includes
#include "qSlicerBeamToStandTransformationWidget.h"
#include "ui_qSlicerBeamToStandTransformationWidget.h"

class vtkSlicerIhepStandGeometryLogic;

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

  vtkSlicerIhepStandGeometryLogic* logic() const;
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
vtkSlicerIhepStandGeometryLogic* qSlicerBeamToStandTransformationWidgetPrivate::logic() const
{
  Q_Q(const qSlicerBeamToStandTransformationWidget);
  qSlicerAbstractCoreModule* aModule = qSlicerCoreApplication::application()->moduleManager()->module("IhepStandGeometry");
  if (aModule)
  {
    vtkSlicerIhepStandGeometryLogic* moduleLogic = vtkSlicerIhepStandGeometryLogic::SafeDownCast(aModule->logic());
    return moduleLogic;
  }
  return nullptr;
}

// --------------------------------------------------------------------------
void qSlicerBeamToStandTransformationWidgetPrivate::init()
{
  Q_Q(qSlicerBeamToStandTransformationWidget);

  // Buttons
  QObject::connect( this->PushButton_Translate, SIGNAL(clicked()), 
    q, SLOT(onTranslatePatientToFixedIsocenterClicked()));
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
  d->init();
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

  vtkSlicerIhepStandGeometryLogic* logic = d->logic();
  if (!logic)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid IhepStandGeometry logic";
    return;
  }

  // Calculate transform and translation
  double translate[3] = {};
  vtkNew< vtkTransform > beamToFixedBeamTransform;

  vtkMRMLRTBeamNode* beamNode = d->ParameterNode->GetPatientBeamNode();
  vtkMRMLRTBeamNode* fixedBeam = d->ParameterNode->GetFixedBeamNode();
//  vtkMRMLRTBeamNode* xrayBeam = d->ParameterNode->GetExternalXrayBeamNode();
  if (beamNode && fixedBeam)
  {
    vtkMRMLRTFixedIonBeamNode* fixedBeamNode = vtkMRMLRTFixedIonBeamNode::SafeDownCast(fixedBeam);

    logic->GetPatientIsocenterToFixedIsocenterTranslate(d->ParameterNode, translate);
    logic->GetPatientBeamToFixedBeamTransform( d->ParameterNode, beamNode, fixedBeamNode, beamToFixedBeamTransform);

    this->setIsocenterTranslation(translate);
    this->setTransformMatrix(beamToFixedBeamTransform);
  }
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

//-----------------------------------------------------------------------------
void qSlicerBeamToStandTransformationWidget::onTranslatePatientToFixedIsocenterClicked()
{
  Q_D(qSlicerBeamToStandTransformationWidget);

  if (!d->ParameterNode)
  {
    return;
  }

  vtkSlicerIhepStandGeometryLogic* logic = d->logic();
  if (!logic)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid IhepStandGeometry logic";
    return;
  }

  // Calculate translation
  double translate[3] = {};
  logic->GetPatientIsocenterToFixedIsocenterTranslate( d->ParameterNode, translate);

  double vertical = d->ParameterNode->GetTableTopVerticalPositionOrigin() + translate[2]; // Z_t
  double longitudinal = d->ParameterNode->GetTableTopLongitudinalPosition() + translate[1]; // Y_t
  double lateral = d->ParameterNode->GetTableTopLateralPosition() + translate[0]; // X_t

  d->ParameterNode->DisableModifiedEventOn();
  d->ParameterNode->SetTableTopVerticalPositionOrigin(vertical);
  d->ParameterNode->SetTableTopLongitudinalPosition(longitudinal);
  d->ParameterNode->SetTableTopLateralPosition(lateral);
  d->ParameterNode->DisableModifiedEventOff();
  d->ParameterNode->Modified();
}
