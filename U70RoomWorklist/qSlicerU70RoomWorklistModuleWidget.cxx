/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QMessageBox>

// Slicer includes
#include "qSlicerU70RoomWorklistModuleWidget.h"
#include "ui_qSlicerU70RoomWorklistModuleWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLLayoutLogic.h>

// Slicer includes
#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>

// DCMTK includes
#include <dcmtk/config/osconfig.h>    /* make sure OS specific configuration is included first */
#include <dcmtk/dcmnet/assoc.h>
#include <dcmtk/dcmnet/scu.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcdatset.h>

// CTK includes
#include <ctkDICOMEcho.h>
#include <ctkDICOMQuery.h>
#include <ctkDICOMDatabase.h>

#include "qSlicerPatientsQueueWidget.h"
#include "qSlicerPatientsQueueTableModel.h"

//-----------------------------------------------------------------------------
class qSlicerU70RoomWorklistModuleWidgetPrivate : public Ui_qSlicerU70RoomWorklistModuleWidget
{
public:
  qSlicerU70RoomWorklistModuleWidgetPrivate();

  const char* PWL_LAYOUT_DESCRIPTION = \
    "<layout type=\"vertical\">" \
    " <item>" \
    "  <PatientsQueueWorklist></PatientsQueueWorklist>" \
    " </item>" \
    "</layout>";
  const int PWL_LAYOUT_ID = 1021;

  QScopedPointer< qSlicerPatientsQueueWidget > PatientsQueueWidget;
  std::unique_ptr< DcmDataset > WorklistQueryDataset;

  int PreviousLayoutId{ -1 };
  bool ModuleWindowInitialized{ false };
};

//-----------------------------------------------------------------------------
// qSlicerU70RoomWorklistModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerU70RoomWorklistModuleWidgetPrivate::qSlicerU70RoomWorklistModuleWidgetPrivate()
{
  this->WorklistQueryDataset.reset(new DcmDataset);
/*
(0008,0005) CS [ISO_IR 192]                             #  10, 1 SpecificCharacterSet
(0008,0050) SH (no value available)                     #   0, 0 AccessionNumber
(0010,0010) PN (no value available)                     #   0, 0 PatientName
(0010,0020) LO (no value available)                     #   0, 0 PatientID
(0010,0021) LO (no value available)                     #   0, 0 IssuerOfPatientID
(0010,0030) DA (no value available)                     #   0, 0 PatientBirthDate
(0010,0040) CS (no value available)                     #   0, 0 PatientSex
(0020,000d) UI (no value available)                     #   0, 0 StudyInstanceUID
(0032,1060) LO (no value available)                     #   0, 0 RequestedProcedureDescription
(0040,0100) SQ (Sequence with explicit length #=1)      #  80, 1 ScheduledProcedureStepSequence
  (fffe,e000) na (Item with explicit length #=9)          #  72, 1 Item
    (0008,0060) CS (no value available)                     #   0, 0 Modality
    (0040,0001) AE (no value available)                     #   0, 0 ScheduledStationAETitle
    (0040,0002) DA (no value available)                     #   0, 0 ScheduledProcedureStepStartDate
    (0040,0003) TM (no value available)                     #   0, 0 ScheduledProcedureStepStartTime
    (0040,0007) LO (no value available)                     #   0, 0 ScheduledProcedureStepDescription
    (0040,0009) SH (no value available)                     #   0, 0 ScheduledProcedureStepID
    (0040,0010) SH (no value available)                     #   0, 0 ScheduledStationName
    (0040,0011) SH (no value available)                     #   0, 0 ScheduledProcedureStepLocation
    (0040,0020) CS (no value available)                     #   0, 0 ScheduledProcedureStepStatus
  (fffe,e00d) na (ItemDelimitationItem for re-encoding)   #   0, 0 ItemDelimitationItem
(fffe,e0dd) na (SequenceDelimitationItem for re-encod.) #   0, 0 SequenceDelimitationItem
(0040,1001) SH (no value available)                     #   0, 0 RequestedProcedureID
*/
  // Clear the query
  this->WorklistQueryDataset->clear();

  // Insert all keys that we like to receive values for
  
  // Make clear we define our search values in UTF-8
//  this->WorklistQueryDataset->putAndInsertOFStringArray(DCM_SpecificCharacterSet, "ISO_IR 192");
  // Fill empty tags
  this->WorklistQueryDataset->insertEmptyElement(DCM_AccessionNumber);
  this->WorklistQueryDataset->insertEmptyElement(DCM_PatientID);
  this->WorklistQueryDataset->insertEmptyElement(DCM_PatientName);
  this->WorklistQueryDataset->insertEmptyElement(DCM_IssuerOfPatientID);
  this->WorklistQueryDataset->insertEmptyElement(DCM_PatientBirthDate);
  this->WorklistQueryDataset->insertEmptyElement(DCM_PatientSex);
  this->WorklistQueryDataset->insertEmptyElement(DCM_StudyInstanceUID);
  this->WorklistQueryDataset->insertEmptyElement(DCM_RequestedProcedureDescription);

  DcmItem* spsSequenceItem = nullptr;
  if (this->WorklistQueryDataset->findOrCreateSequenceItem(DCM_ScheduledProcedureStepSequence, spsSequenceItem).good())
  {
    // SPS Sequence dataset
    spsSequenceItem->insertEmptyElement(DCM_Modality);
    spsSequenceItem->insertEmptyElement(DCM_ScheduledStationAETitle);
    spsSequenceItem->insertEmptyElement(DCM_ScheduledProcedureStepStartDate);
    spsSequenceItem->insertEmptyElement(DCM_ScheduledProcedureStepStartTime);
    spsSequenceItem->insertEmptyElement(DCM_ScheduledProcedureStepDescription);
    spsSequenceItem->insertEmptyElement(DCM_ScheduledProcedureStepID);
    spsSequenceItem->insertEmptyElement(DCM_ScheduledStationName);
    spsSequenceItem->insertEmptyElement(DCM_ScheduledProcedureStepLocation);
    spsSequenceItem->insertEmptyElement(DCM_ScheduledProcedureStepStatus);
  }
  this->WorklistQueryDataset->insertEmptyElement(DCM_RequestedProcedureID);
//  this->WorklistQueryDataset->putAndInsertString(DCM_QueryRetrieveLevel, "STUDY");

  DcmFileFormat fileformat(this->WorklistQueryDataset.get(), OFTrue);
  if (fileformat.saveFile("/tmp/mwl_query.dcm", EXS_LittleEndianExplicit).good())
  {
    qDebug() << Q_FUNC_INFO << ": Save is good";
  }
}

//-----------------------------------------------------------------------------
// qSlicerU70RoomWorklistModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerU70RoomWorklistModuleWidget::qSlicerU70RoomWorklistModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerU70RoomWorklistModuleWidgetPrivate)
{
  Q_D(qSlicerU70RoomWorklistModuleWidget);

  d->PatientsQueueWidget.reset(new qSlicerPatientsQueueWidget(this));
}

//-----------------------------------------------------------------------------
qSlicerU70RoomWorklistModuleWidget::~qSlicerU70RoomWorklistModuleWidget() {}

//-----------------------------------------------------------------------------
void qSlicerU70RoomWorklistModuleWidget::setup()
{
  Q_D(qSlicerU70RoomWorklistModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  qSlicerSingletonViewFactory* viewFactory = new qSlicerSingletonViewFactory();
  viewFactory->setWidget(d->PatientsQueueWidget.get());
  viewFactory->setTagName("PatientsQueueWorklist");

  layoutManager->registerViewFactory(viewFactory);
  // Save previous layout
  d->PreviousLayoutId = layoutManager->layout();

  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  if (layoutNode)
  {
    if (!layoutNode->SetLayoutDescription(d->PWL_LAYOUT_ID, d->PWL_LAYOUT_DESCRIPTION))
    {
      layoutNode->AddLayoutDescription(d->PWL_LAYOUT_ID, d->PWL_LAYOUT_DESCRIPTION);
    }
  }

  // Buttons
  QObject::connect(d->PushButton_Worklist1, SIGNAL(clicked()),
    this, SLOT(onSetCustomLayoutClicked()));
  QObject::connect(d->PushButton_CheckConnection, SIGNAL(clicked()),
    this, SLOT(onCheckConnectionClicked()));
  QObject::connect(d->PushButton_WorklistQuery, SIGNAL(clicked()),
    this, SLOT(onWorklistQueryClicked()));
}


//-----------------------------------------------------------------------------
void qSlicerU70RoomWorklistModuleWidget::enter()
{
  Q_D(qSlicerU70RoomWorklistModuleWidget);
  this->Superclass::enter();

  this->onEnter();
}

//-----------------------------------------------------------------------------
void qSlicerU70RoomWorklistModuleWidget::exit()
{
  Q_D(qSlicerU70RoomWorklistModuleWidget);
  this->Superclass::exit();

  {
    QSignalBlocker block(d->PushButton_Worklist1);
    d->PushButton_Worklist1->setChecked(false);
  }
  QTimer::singleShot(50, this, SLOT(onSetCustomLayoutClicked()));  
}

void qSlicerU70RoomWorklistModuleWidget::onSetCustomLayoutClicked()
{
  Q_D(qSlicerU70RoomWorklistModuleWidget);
  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  if (d->PushButton_Worklist1->isChecked())
  {
    if (layoutManager)
    {
      layoutManager->setLayout(d->PWL_LAYOUT_ID);
    }
  }
  else
  {
    if (layoutManager)
    {
      layoutManager->setLayout(d->PreviousLayoutId);
    }
  }
  slicerApplication->processEvents();
}


void qSlicerU70RoomWorklistModuleWidget::onWorklistQueryClicked()
{
  Q_D(qSlicerU70RoomWorklistModuleWidget);

  QString hostStr = d->LineEdit_WorklistServerHost->text();
  int hostPort = d->SpinBox_WorklistServerPort->value();
  QString aeTitleStr = d->LineEdit_WorklistServerAETitle->text();

  if (hostStr.isEmpty())
  {
    return;
  }
  
  OFString host(hostStr.toStdString().c_str());
  OFString peerAET(aeTitleStr.toStdString().c_str());

  // Setup SCU
  DcmSCU scu;
  scu.setPeerHostName(host);
  scu.setPeerPort(hostPort);
  
  OFString verificationSOP = UID_FINDModalityWorklistInformationModel;
  OFList<OFString> ts;
  ts.push_back(UID_LittleEndianExplicitTransferSyntax);
  ts.push_back(UID_BigEndianExplicitTransferSyntax);
  ts.push_back(UID_LittleEndianImplicitTransferSyntax);
  OFCondition result1 = scu.addPresentationContext(verificationSOP, ts);
  if (result1.bad())
  {
    qWarning() << Q_FUNC_INFO << "Error while setting up a presentation context: " << result1.text() << "\n";
    return;
  }

  if (peerAET != "")
  {
    scu.setPeerAETitle(peerAET);
  }

  OFCondition result = scu.initNetwork();
  if (result.bad())
  {
    qWarning() << Q_FUNC_INFO << "Error setting up SCU: " << result.text() << "\n";
    return;
  }

  // Negotiate association
  result = scu.negotiateAssociation();
  if (result.bad())
  {
    qWarning() << Q_FUNC_INFO << "Error negotiating association: " << result.text() << "\n";
    return;
  }

  QList< ModalityWork > mwl; // modality worklist

  // Issue FIND request and let scu find presentation context itself (1)
  OFList< QRResponse* > findResponses;
  result = scu.sendFINDRequest(1, d->WorklistQueryDataset.get(), &findResponses);
  if (result.bad())
  {
    qWarning() << Q_FUNC_INFO << "Error issuing C-FIND request or received rejecting response: " << result.text() << "\n";
    return;
  }
  else
  {
//    qDebug() << Q_FUNC_INFO  << "Successfully sentC-FIND request to host " << hostStr << " on port " << hostPort \
      << " found responses " << findResponses.size() << '\n';
    for (QRResponse* retresp : findResponses)
    {
      if (retresp->m_dataset)
      {
        ModalityWork mw;
        if (mw.addWorkFromDicomResponse(retresp))
        {
          mwl.push_back(mw);
        }
      }
    }
  }

  for (const ModalityWork& mw : mwl)
  {
    qDebug() << Q_FUNC_INFO << "Patient's name:" << mw.PatientName << ", birthdate:" << mw.PatientBirthDate.toString("dd.MM.yyyy");
    for (const ModalityWork::ScheduledProcedureStep& sps : mw.spsSequence)
    {
      qDebug() << Q_FUNC_INFO << "Modality:" << sps.Modality << ", date & time:" << sps.StartDateTime.toString("dd.MM.yyyy HH:mm:ss");
    }
  }
  result = scu.releaseAssociation();
  if (result.bad())
  {
    qWarning() << Q_FUNC_INFO << "Error releasing association with peer: " << result.text() << "\n";
    return;
  }
}

void qSlicerU70RoomWorklistModuleWidget::onCheckConnectionClicked()
{
  Q_D(qSlicerU70RoomWorklistModuleWidget);

  QString hostStr = d->LineEdit_WorklistServerHost->text();
  int hostPort = d->SpinBox_WorklistServerPort->value();
  QString aeTitleStr = d->LineEdit_WorklistServerAETitle->text();

  QScopedPointer< ctkDICOMEcho > mwlServerEcho(new ctkDICOMEcho(this));
  mwlServerEcho->setCallingAETitle("ECHOSCU");
  mwlServerEcho->setCalledAETitle(aeTitleStr);
  mwlServerEcho->setHost(hostStr);
  mwlServerEcho->setPort(hostPort);

  if (mwlServerEcho->echo())
  {
    int res = QMessageBox::information(this, tr("Check connection (ECHO)"), tr("Connection to peer was successful!"));
    Q_UNUSED(res);
  }
  else
  {
    int res = QMessageBox::warning(this, tr("Check connection (ECHO)"), tr("Unable to execute C-ECHO!"));
    Q_UNUSED(res);
  }
}

void qSlicerU70RoomWorklistModuleWidget::onEnter()
{
  Q_D(qSlicerU70RoomWorklistModuleWidget);
  {
    QSignalBlocker block(d->PushButton_Worklist1);
    d->PushButton_Worklist1->setChecked(true);
  }
  QTimer::singleShot(50, this, SLOT(onSetCustomLayoutClicked()));
}
