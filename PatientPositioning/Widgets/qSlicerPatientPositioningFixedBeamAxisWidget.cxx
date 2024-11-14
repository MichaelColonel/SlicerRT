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

// Qt includes
#include <QDebug>

// FixedBeamAxis Widgets includes
#include "qSlicerPatientPositioningFixedBeamAxisWidget.h"
#include "ui_qSlicerPatientPositioningFixedBeamAxisWidget.h"

// MRML includes
#include <vtkMRMLPatientPositioningNode.h>
#include <vtkMRMLCabin26AGeometryNode.h>

// Logic includes
#include <vtkSlicerPatientPositioningLogic.h>

// VTK includes
#include <vtkVector.h>

//-----------------------------------------------------------------------------
class qSlicerPatientPositioningFixedBeamAxisWidgetPrivate
  : public Ui_qSlicerPatientPositioningFixedBeamAxisWidget
{
  Q_DECLARE_PUBLIC(qSlicerPatientPositioningFixedBeamAxisWidget);
protected:
  qSlicerPatientPositioningFixedBeamAxisWidget* const q_ptr;

public:
  qSlicerPatientPositioningFixedBeamAxisWidgetPrivate(
    qSlicerPatientPositioningFixedBeamAxisWidget& object);
  virtual void setupUi(qSlicerPatientPositioningFixedBeamAxisWidget*);
  void init();

  vtkWeakPointer< vtkMRMLPatientPositioningNode > ParameterNode;
  vtkWeakPointer< vtkMRMLCabin26AGeometryNode > Cabin26ANode;
  vtkWeakPointer< vtkSlicerPatientPositioningLogic > PatientPositioningLogic;
};

// --------------------------------------------------------------------------
qSlicerPatientPositioningFixedBeamAxisWidgetPrivate
::qSlicerPatientPositioningFixedBeamAxisWidgetPrivate(
  qSlicerPatientPositioningFixedBeamAxisWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidgetPrivate
::setupUi(qSlicerPatientPositioningFixedBeamAxisWidget* widget)
{
  this->Ui_qSlicerPatientPositioningFixedBeamAxisWidget::setupUi(widget);
}

// --------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidgetPrivate::init()
{
  Q_Q(qSlicerPatientPositioningFixedBeamAxisWidget);

  // Buttons
  QObject::connect( this->PushButton_BevXPlus, SIGNAL(clicked()), q, SLOT(onBeamsEyeViewPlusXButtonClicked()));
  QObject::connect( this->PushButton_BevXMinus, SIGNAL(clicked()), q, SLOT(onBeamsEyeViewMinusXButtonClicked()));
  QObject::connect( this->PushButton_BevYMinus, SIGNAL(clicked()), q, SLOT(onBeamsEyeViewMinusYButtonClicked()));
  QObject::connect( this->PushButton_BevYPlus, SIGNAL(clicked()), q, SLOT(onBeamsEyeViewPlusYButtonClicked()));

  // ButtonGroup
  QObject::connect( this->ButtonGroup_FrameBasisTranslation, SIGNAL(buttonClicked(QAbstractButton*)),
    q, SLOT(onFrameBasisTranslationRadioButtonClicked(QAbstractButton*)));
}

//-----------------------------------------------------------------------------
// qSlicerPatientPositioningFixedBeamAxisWidget methods

//-----------------------------------------------------------------------------
qSlicerPatientPositioningFixedBeamAxisWidget
::qSlicerPatientPositioningFixedBeamAxisWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerPatientPositioningFixedBeamAxisWidgetPrivate(*this) )
{
  Q_D(qSlicerPatientPositioningFixedBeamAxisWidget);
  d->setupUi(this);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerPatientPositioningFixedBeamAxisWidget
::~qSlicerPatientPositioningFixedBeamAxisWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerPatientPositioningFixedBeamAxisWidget);

  vtkMRMLPatientPositioningNode* parameterNode = vtkMRMLPatientPositioningNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect( d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT( updateWidgetFromMRML() ) );
  qvtkReconnect( d->Cabin26ANode, parameterNode->GetCabin26AGeometryNode(), vtkCommand::ModifiedEvent, 
    this, SLOT( updateWidgetFromMRML() ) );

  d->ParameterNode = parameterNode;
  if (parameterNode)
  {
    d->Cabin26ANode = parameterNode->GetCabin26AGeometryNode();
  }

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidget::setPatientPositioningLogic(vtkSlicerPatientPositioningLogic* logic)
{
  Q_D(qSlicerPatientPositioningFixedBeamAxisWidget);
  d->PatientPositioningLogic = logic;
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidget::onBeamsEyeViewPlusXButtonClicked()
{
  Q_D(qSlicerPatientPositioningFixedBeamAxisWidget);
//  double viewUpVector[4] = { 1., 0., 0., 0. };
//  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("+X"));
  emit bevOrientationChanged(std::array< double, 3 >{ 1., 0., 0.});
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidget::onBeamsEyeViewMinusXButtonClicked()
{
  Q_D(qSlicerPatientPositioningFixedBeamAxisWidget);
//  double viewUpVector[4] = { -1., 0., 0., 0. };
//  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("-X"));
  emit bevOrientationChanged(std::array< double, 3 >{ -1., 0., 0.});
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidget::onBeamsEyeViewPlusYButtonClicked()
{
  Q_D(qSlicerPatientPositioningFixedBeamAxisWidget);
//  double viewUpVector[4] = { 0., 1., 0., 0. };
//  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("+Y"));
  emit bevOrientationChanged(std::array< double, 3 >{ 0., 1., 0.});
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidget::onBeamsEyeViewMinusYButtonClicked()
{
  Q_D(qSlicerPatientPositioningFixedBeamAxisWidget);
//  double viewUpVector[4] = { 0., -1., 0., 0. };
//  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("-Y"));
  emit bevOrientationChanged(std::array< double, 3 >{ 0., -1., 0.});
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidget::onFrameBasisTranslationRadioButtonClicked(QAbstractButton* button)
{
  Q_D(qSlicerPatientPositioningFixedBeamAxisWidget);

  QRadioButton* rbutton = qobject_cast< QRadioButton* >(button);
  if (rbutton == d->RadioButton_FixedReference)
  {
    vtkVector3d translationInFixedReference = d->PatientPositioningLogic->GetIsocenterToFixedBeamAxisTranslation(
      d->ParameterNode,
      vtkSlicerTableTopRobotTransformLogic::FixedReference);
    // Correct translation along X_f, Y_f, Z_f axises
    double pos[3] = { translationInFixedReference[0], translationInFixedReference[2], -1. * translationInFixedReference[1] };
    d->CoordinatesWidget_FromIsocenterTranslation->setCoordinates(pos);
  }
  else if (rbutton == d->RadioButton_TableTop)
  {
    vtkVector3d translationInTableTop = d->PatientPositioningLogic->GetIsocenterToFixedBeamAxisTranslation(
      d->ParameterNode,
      vtkSlicerTableTopRobotTransformLogic::TableTop);
    // Correct translation along X_t, Y_t, Z_t axises
    double pos[3] = { translationInTableTop[0], translationInTableTop[2], -1. * translationInTableTop[1] };
    d->CoordinatesWidget_FromIsocenterTranslation->setCoordinates(pos);
  }
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerPatientPositioningFixedBeamAxisWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  qDebug() << Q_FUNC_INFO << ": Calculate new Isocenter translations!";

  if (d->PatientPositioningLogic)
  {
    if (d->RadioButton_FixedReference->isChecked())
    {
      vtkVector3d translationInFixedReference = d->PatientPositioningLogic->GetIsocenterToFixedBeamAxisTranslation(
        d->ParameterNode,
        vtkSlicerTableTopRobotTransformLogic::FixedReference);
      // Correct translation along X_f, Y_f, Z_f axises
      double pos[3] = { translationInFixedReference[0], translationInFixedReference[2], -1. * translationInFixedReference[1] };
      d->CoordinatesWidget_FromIsocenterTranslation->setCoordinates(pos);
    }
    else /* if (d->RadioButton_TableTop->isChecked()) */
    {
      vtkVector3d translationInTableTop = d->PatientPositioningLogic->GetIsocenterToFixedBeamAxisTranslation(
        d->ParameterNode,
        vtkSlicerTableTopRobotTransformLogic::TableTop);
      // Correct translation along X_t, Y_t, Z_t axises
      double pos[3] = { translationInTableTop[0], translationInTableTop[2], -1. * translationInTableTop[1] };
      d->CoordinatesWidget_FromIsocenterTranslation->setCoordinates(pos);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidget::setTableTopAngles(double lateral, double longitudinal, double vertical)
{
  Q_D(qSlicerPatientPositioningFixedBeamAxisWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }
  d->Label_LateralAngle->setText(tr("%1").arg(lateral));
  d->Label_LongitudinalAngle->setText(tr("%1").arg(longitudinal));
  d->Label_VerticalAngle->setText(tr("%1").arg(vertical));
}
