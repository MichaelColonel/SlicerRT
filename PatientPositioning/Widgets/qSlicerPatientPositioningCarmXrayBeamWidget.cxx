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
#include "qSlicerPatientPositioningCarmXrayBeamWidget.h"
#include "ui_qSlicerPatientPositioningCarmXrayBeamWidget.h"

// MRML includes
#include <vtkMRMLPatientPositioningNode.h>
#include <vtkMRMLCabin26AGeometryNode.h>
#include <vtkMRMLDrrImageComputationNode.h>

#include <vtkMRMLRTPlanNode.h>
#include <vtkMRMLRTFixedBeamNode.h>
#include <vtkMRMLRTCabin26AIonBeamNode.h>

#include <vtkMRMLScalarVolumeNode.h>


#include <vtkMRMLScene.h>

// Beams inlcudes
#include <vtkMRMLRTBeamNode.h>

// Logic includes
#include <vtkSlicerPatientPositioningLogic.h>
#include <vtkSlicerDrrImageComputationLogic.h>

// VTK includes
#include <vtkVector.h>

//-----------------------------------------------------------------------------
class qSlicerPatientPositioningCarmXrayBeamWidgetPrivate
  : public Ui_qSlicerPatientPositioningCarmXrayBeamWidget
{
  Q_DECLARE_PUBLIC(qSlicerPatientPositioningCarmXrayBeamWidget);
protected:
  qSlicerPatientPositioningCarmXrayBeamWidget* const q_ptr;

public:
  qSlicerPatientPositioningCarmXrayBeamWidgetPrivate(
    qSlicerPatientPositioningCarmXrayBeamWidget& object);
  virtual void setupUi(qSlicerPatientPositioningCarmXrayBeamWidget*);
  void init();

  vtkWeakPointer< vtkMRMLPatientPositioningNode > ParameterNode;
  vtkWeakPointer< vtkMRMLCabin26AGeometryNode > Cabin26ANode;
  vtkWeakPointer< vtkSlicerPatientPositioningLogic > PatientPositioningLogic;
};

// --------------------------------------------------------------------------
qSlicerPatientPositioningCarmXrayBeamWidgetPrivate
::qSlicerPatientPositioningCarmXrayBeamWidgetPrivate(
  qSlicerPatientPositioningCarmXrayBeamWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerPatientPositioningCarmXrayBeamWidgetPrivate
::setupUi(qSlicerPatientPositioningCarmXrayBeamWidget* widget)
{
  this->Ui_qSlicerPatientPositioningCarmXrayBeamWidget::setupUi(widget);
}

// --------------------------------------------------------------------------
void qSlicerPatientPositioningCarmXrayBeamWidgetPrivate::init()
{
  Q_Q(qSlicerPatientPositioningCarmXrayBeamWidget);

  // Buttons
  QObject::connect( this->PushButton_BevXPlus, SIGNAL(clicked()), q, SLOT(onBeamsEyeViewPlusXButtonClicked()));
  QObject::connect( this->PushButton_BevXMinus, SIGNAL(clicked()), q, SLOT(onBeamsEyeViewMinusXButtonClicked()));
  QObject::connect( this->PushButton_BevYMinus, SIGNAL(clicked()), q, SLOT(onBeamsEyeViewMinusYButtonClicked()));
  QObject::connect( this->PushButton_BevYPlus, SIGNAL(clicked()), q, SLOT(onBeamsEyeViewPlusYButtonClicked()));
  QObject::connect( this->PushButton_ComputeCarmXrayDrr, SIGNAL(clicked()), q, SLOT(onComputeDrrClicked()));
}

//-----------------------------------------------------------------------------
// qSlicerPatientPositioningCarmXrayBeamWidget methods

//-----------------------------------------------------------------------------
qSlicerPatientPositioningCarmXrayBeamWidget
::qSlicerPatientPositioningCarmXrayBeamWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerPatientPositioningCarmXrayBeamWidgetPrivate(*this) )
{
  Q_D(qSlicerPatientPositioningCarmXrayBeamWidget);
  d->setupUi(this);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerPatientPositioningCarmXrayBeamWidget
::~qSlicerPatientPositioningCarmXrayBeamWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningCarmXrayBeamWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerPatientPositioningCarmXrayBeamWidget);

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
void qSlicerPatientPositioningCarmXrayBeamWidget::setPatientPositioningLogic(vtkSlicerPatientPositioningLogic* logic)
{
  Q_D(qSlicerPatientPositioningCarmXrayBeamWidget);
  d->PatientPositioningLogic = logic;
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningCarmXrayBeamWidget::onBeamsEyeViewPlusXButtonClicked()
{
  Q_D(qSlicerPatientPositioningCarmXrayBeamWidget);
//  double viewUpVector[4] = { 1., 0., 0., 0. };
//  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("+X"));
  emit bevOrientationChanged(std::array< double, 3 >{ 1., 0., 0.});
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningCarmXrayBeamWidget::onBeamsEyeViewMinusXButtonClicked()
{
  Q_D(qSlicerPatientPositioningCarmXrayBeamWidget);
//  double viewUpVector[4] = { -1., 0., 0., 0. };
//  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("-X"));
  emit bevOrientationChanged(std::array< double, 3 >{ -1., 0., 0.});
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningCarmXrayBeamWidget::onBeamsEyeViewPlusYButtonClicked()
{
  Q_D(qSlicerPatientPositioningCarmXrayBeamWidget);
//  double viewUpVector[4] = { 0., 1., 0., 0. };
//  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("+Y"));
  emit bevOrientationChanged(std::array< double, 3 >{ 0., 1., 0.});
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningCarmXrayBeamWidget::onBeamsEyeViewMinusYButtonClicked()
{
  Q_D(qSlicerPatientPositioningCarmXrayBeamWidget);
//  double viewUpVector[4] = { 0., -1., 0., 0. };
//  this->onBeamsEyeViewButtonClicked(viewUpVector);
  d->Label_BevOrientation->setText(tr("-Y"));
  emit bevOrientationChanged(std::array< double, 3 >{ 0., -1., 0.});
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningCarmXrayBeamWidget::onComputeDrrClicked()
{
  Q_D(qSlicerPatientPositioningCarmXrayBeamWidget);
  if (!d->PatientPositioningLogic)
  {
    qWarning() << Q_FUNC_INFO << "PatientPositioning logic is invalid";
    return;
  }

  if (!d->ParameterNode)
  {
    qWarning() << Q_FUNC_INFO << "Parameter node is invalid";
    return;
  }

  vtkMRMLScalarVolumeNode* ctInputVolumeNode = nullptr;
  vtkMRMLRTBeamNode* beamNode = d->ParameterNode->GetBeamNode(); // ion beam node
  if (beamNode)
  {
    qWarning() << Q_FUNC_INFO << "1";
    vtkMRMLRTPlanNode* planNode = beamNode->GetParentPlanNode();
    if (planNode)
    {
      qWarning() << Q_FUNC_INFO << "2";
      ctInputVolumeNode = planNode->GetReferenceVolumeNode();
    }
  }
  vtkMRMLScene* scene = d->PatientPositioningLogic->GetMRMLScene();
  vtkMRMLNode* node = scene->GetFirstNodeByClass("vtkMRMLDrrImageComputationNode");
  vtkMRMLDrrImageComputationNode* drrNode = nullptr;
  if (!node)
  {
    qWarning() << Q_FUNC_INFO << "3";
    node = scene->AddNewNodeByClass("vtkMRMLDrrImageComputationNode", "CarmXray_Cabin26a");
  }
  if (node)
  {
    drrNode = vtkMRMLDrrImageComputationNode::SafeDownCast(node);
    if (drrNode)
    {
      qWarning() << Q_FUNC_INFO << "3.1";
    }
    if (ctInputVolumeNode)
    {
      qWarning() << Q_FUNC_INFO << "3.2";
    }
    if (drrNode && ctInputVolumeNode)
    {
      qWarning() << Q_FUNC_INFO << "4";
      vtkMRMLRTBeamNode* carmXrayBeamNode = d->ParameterNode->GetExternalXrayBeamNode();
      drrNode->SetAndObserveBeamNode(carmXrayBeamNode);
      // setup C-arm X-ray detector parameters
      drrNode->SetThreading(vtkMRMLDrrImageComputationNode::CUDA);
      drrNode->SetInvertIntensityFlag(false);
      drrNode->SetHUThresholdBelow(80);
      drrNode->SetIndependentBeamFlag(true);
    }
  }
  else
  {
    qWarning() << Q_FUNC_INFO << "DRR Image Computation node is invalid";
    return;
  }
  if (drrNode && ctInputVolumeNode)
  {
    qWarning() << Q_FUNC_INFO << "5";
    // compute DRR image here
    vtkSlicerDrrImageComputationLogic* drrLogic = d->PatientPositioningLogic->GetDrrImageComputationLogic();
    drrLogic->UpdateMarkupsNodes(drrNode);
    drrLogic->UpdateNormalAndVupVectors(drrNode);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    vtkMRMLScalarVolumeNode* drrImageNode = drrLogic->ComputePlastimatchDRR( drrNode, ctInputVolumeNode, true);
    if (drrImageNode)
    {
      // node is OK
      QApplication::restoreOverrideCursor();
      return;
    }
    QApplication::restoreOverrideCursor();
  }
}

//-----------------------------------------------------------------------------
void qSlicerPatientPositioningCarmXrayBeamWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerPatientPositioningCarmXrayBeamWidget);

  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid parameter node";
    return;
  }

  if (!d->PatientPositioningLogic)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid PatientPositioning logic";
    return;
  }
}
