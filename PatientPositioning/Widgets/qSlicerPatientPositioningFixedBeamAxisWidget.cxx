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
#include <vtkMRMLChannel25GeometryNode.h>

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
  vtkWeakPointer< vtkMRMLChannel25GeometryNode > Channel25Node;
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
  QObject::connect( this->PushButton_Up, SIGNAL(clicked()), q, SLOT(onButtonUpClicked()));
  QObject::connect( this->PushButton_Down, SIGNAL(clicked()), q, SLOT(onButtonDownClicked()));
  QObject::connect( this->PushButton_Left, SIGNAL(clicked()), q, SLOT(onButtonLeftClicked()));
  QObject::connect( this->PushButton_Right, SIGNAL(clicked()), q, SLOT(onButtonRightClicked()));

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
  qvtkReconnect( d->Channel25Node, parameterNode->GetChannel25GeometryNode(), vtkCommand::ModifiedEvent, 
    this, SLOT( updateWidgetFromMRML() ) );

  d->ParameterNode = parameterNode;
  if (parameterNode)
  {
    d->Channel25Node = parameterNode->GetChannel25GeometryNode();
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
void qSlicerPatientPositioningFixedBeamAxisWidget::onButtonUpClicked()
{
  qDebug() << Q_FUNC_INFO << "Up";
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidget::onButtonDownClicked()
{
  qDebug() << Q_FUNC_INFO << "Down";
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidget::onButtonLeftClicked()
{
  qDebug() << Q_FUNC_INFO << "Left";
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningFixedBeamAxisWidget::onButtonRightClicked()
{
  qDebug() << Q_FUNC_INFO << "Right";
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
    d->CoordinatesWidget_FromIsocenterTranslation->setCoordinates(translationInFixedReference.GetData());
  }
  else if (rbutton == d->RadioButton_TableTop)
  {
    vtkVector3d translationInTableTop = d->PatientPositioningLogic->GetIsocenterToFixedBeamAxisTranslation(
      d->ParameterNode,
      vtkSlicerTableTopRobotTransformLogic::TableTop);
    d->CoordinatesWidget_FromIsocenterTranslation->setCoordinates(translationInTableTop.GetData());
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
      d->CoordinatesWidget_FromIsocenterTranslation->setCoordinates(translationInFixedReference.GetData());
    }
    else if (d->RadioButton_TableTop->isChecked())
    {
      vtkVector3d translationInTableTop = d->PatientPositioningLogic->GetIsocenterToFixedBeamAxisTranslation(
        d->ParameterNode,
        vtkSlicerTableTopRobotTransformLogic::TableTop);
      d->CoordinatesWidget_FromIsocenterTranslation->setCoordinates(translationInTableTop.GetData());
    }
  }
}
