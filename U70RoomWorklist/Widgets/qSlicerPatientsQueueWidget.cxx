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

// U70RoomWorklist Widgets includes
#include "qSlicerPatientsQueueWidget.h"
#include "ui_qSlicerPatientsQueueWidget.h"

#include "qSlicerPatientsQueueTableModel.h"
#include "qSlicerPatientSPSTableModel.h"

#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>
#include <qMRMLSliceWidget.h>
#include <qSlicerSubjectHierarchyFolderPlugin.h>
#include <qSlicerSubjectHierarchyPluginHandler.h>
#include <qMRMLThreeDWidget.h>
#include <qMRMLThreeDView.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLSliceNode.h>

// U70RoomWorklist MRML includes
#include "vtkMRMLU70RoomWorklistNode.h"

// VTK includes
#include <vtkVector.h>
#include <vtkTransform.h>

//-----------------------------------------------------------------------------
class qSlicerPatientsQueueWidgetPrivate : public Ui_qSlicerPatientsQueueWidget
{
  Q_DECLARE_PUBLIC(qSlicerPatientsQueueWidget)
protected:
  qSlicerPatientsQueueWidget* const q_ptr;

public:
  qSlicerPatientsQueueWidgetPrivate(qSlicerPatientsQueueWidget& object);
  virtual void setupUi(qSlicerPatientsQueueWidget*);

  QScopedPointer< qSlicerPatientsQueueTableModel > PatientsQueueTableModel;
  QScopedPointer< qSlicerPatientSPSTableModel > PatientSPSTableModel;

  vtkWeakPointer< vtkMRMLU70RoomWorklistNode > ParameterNode;

};

// --------------------------------------------------------------------------
qSlicerPatientsQueueWidgetPrivate::qSlicerPatientsQueueWidgetPrivate(
  qSlicerPatientsQueueWidget& object)
  : q_ptr(&object)
  , PatientsQueueTableModel(new qSlicerPatientsQueueTableModel(&object))
  , PatientSPSTableModel(new qSlicerPatientSPSTableModel(&object))
{
}

// --------------------------------------------------------------------------
void qSlicerPatientsQueueWidgetPrivate::setupUi(
  qSlicerPatientsQueueWidget* widget)
{
  this->Ui_qSlicerPatientsQueueWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPatientsQueueWorklistWidget methods

//-----------------------------------------------------------------------------
qSlicerPatientsQueueWidget::qSlicerPatientsQueueWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerPatientsQueueWidgetPrivate(*this) )
{
  Q_D(qSlicerPatientsQueueWidget);
  d->setupUi(this);
  d->TableView_PatientsQueue->setModel(d->PatientsQueueTableModel.data());
  d->TableView_PatientSPS->setModel(d->PatientSPSTableModel.data());

  QObject::connect(d->TableView_PatientsQueue, SIGNAL(clicked(QModelIndex)),
    this, SLOT(onPatientsQueueModelIndexClicked(QModelIndex)));
  QObject::connect(d->TableView_PatientSPS, SIGNAL(clicked(QModelIndex)),
    this, SLOT(onScheduledProcedureStepModelIndexClicked(QModelIndex)));
}

//-----------------------------------------------------------------------------
qSlicerPatientsQueueWidget::~qSlicerPatientsQueueWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientsQueueWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerPatientsQueueWidget);

  vtkMRMLU70RoomWorklistNode* parameterNode = vtkMRMLU70RoomWorklistNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect(d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT(updateWidgetFromMRML()));

  d->ParameterNode = parameterNode;

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPatientsQueueWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerPatientsQueueWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << tr(": Invalid patient WORKLIST node!");
    return;
  }
  qDebug() << Q_FUNC_INFO << "Update children widgets";
}

//-----------------------------------------------------------------------------
void qSlicerPatientsQueueWidget::reset()
{
  Q_D(qSlicerPatientsQueueWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << tr(": Invalid patient WORKLIST node!");
    return;
  }

  d->TableView_PatientsQueue->reset();
  d->TableView_PatientSPS->reset();

  QList< ModalityWork > emptyMWL;
  d->PatientsQueueTableModel->setModalityWorkList(emptyMWL);
  d->PatientSPSTableModel->setScheduledProcedureStepList(ModalityWork());
}

//-----------------------------------------------------------------------------
void qSlicerPatientsQueueWidget::setModalityWorkList(const QList< ModalityWork >& mwl)
{
  Q_D(qSlicerPatientsQueueWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << tr(": Invalid patient WORKLIST node!");
    return;
  }

  d->TableView_PatientsQueue->reset();
  d->TableView_PatientSPS->reset();
  
  d->PatientsQueueTableModel->setModalityWorkList(mwl);
  d->PatientSPSTableModel->setScheduledProcedureStepList(ModalityWork());
}

//-----------------------------------------------------------------------------
void qSlicerPatientsQueueWidget::onPatientsQueueModelIndexClicked(const QModelIndex& index)
{
  Q_D(qSlicerPatientsQueueWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << tr(": Invalid patient WORKLIST node!");
    return;
  }

  const QList< ModalityWork >& mwl = d->PatientsQueueTableModel->getModalityWorkList();
  if (index.row() < 0 || !mwl.size())
  {
    return;
  }
  if (index.row() < mwl.size())
  {
    qDebug() << Q_FUNC_INFO << "Patient queue index:" << index.row();
    const ModalityWork& mw = d->PatientsQueueTableModel->getModalityWorkList().at(index.row());
    d->PatientSPSTableModel->setScheduledProcedureStepList(mw);

    d->ParameterNode->SetAccessionNumber(mw.AccessionNumber.toStdString());
    d->ParameterNode->SetPatientName(mw.PatientName.toStdString());
    d->ParameterNode->SetPatientID(mw.PatientID.toStdString());
    d->ParameterNode->SetIssuerOfPatientID(mw.IssuerOfPatientID.toStdString());
    d->ParameterNode->SetPatientBirthDate(mw.PatientBirthDate.toString("yyyyMMdd HHmmss").toStdString());
    d->ParameterNode->SetPatientSex(mw.PatientSex.toStdString());
    d->ParameterNode->SetStudyInstanceUID(mw.StudyInstanceUID.toStdString());
    d->ParameterNode->SetRequestedProcedureDescription(mw.RequestedProcedureDescription.toStdString());
    d->ParameterNode->SetRequestedProcedureID(mw.RequestedProcedureID.toStdString());    
  }
}

//-----------------------------------------------------------------------------
void qSlicerPatientsQueueWidget::onScheduledProcedureStepModelIndexClicked(const QModelIndex& index)
{
  Q_D(qSlicerPatientsQueueWidget);
  if (!d->ParameterNode)
  {
    qCritical() << Q_FUNC_INFO << tr(": Invalid patient WORKLIST node!");
    return;
  }

  int patientsQueueIndexRow = d->TableView_PatientsQueue->currentIndex().row();
  qDebug() << Q_FUNC_INFO << "Patients queue row index:" << patientsQueueIndexRow;
  qDebug() << Q_FUNC_INFO << "Patient SPS row index:" << index.row();

  const ModalityWork& mw = d->PatientsQueueTableModel->getModalityWorkList().at(patientsQueueIndexRow);
  const ModalityWork::ScheduledProcedureStep& patientSPS = mw.spsSequence.at(index.row());

  d->ParameterNode->SetRequestedProcedureDescription(patientSPS.Description.toStdString());
  d->ParameterNode->SetScheduledProcedureStepModality(patientSPS.Modality.toStdString());
  d->ParameterNode->SetScheduledProcedureStepStationAeTitle(patientSPS.StationAeTitle.toStdString());
  d->ParameterNode->SetScheduledProcedureStepStartDateTime(patientSPS.StartDateTime.toString("yyyyMMdd HHmmss").toStdString());
  d->ParameterNode->SetScheduledProcedureStepID(patientSPS.ID.toStdString());
  d->ParameterNode->SetScheduledProcedureStepStationName(patientSPS.StationName.toStdString());
  d->ParameterNode->SetScheduledProcedureStepLocation(patientSPS.Location.toStdString());
  d->ParameterNode->SetScheduledProcedureStepStatus(patientSPS.Status.toStdString());
}
