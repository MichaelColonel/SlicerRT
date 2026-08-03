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
#include <QScopedPointer>

// Slicer includes
#include "qSlicerPatientsQueueModuleWidget.h"
#include "ui_qSlicerPatientsQueueModuleWidget.h"

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
#include <dcmtk/dcmnet/scu.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcdatset.h>

// CTK includes
#include <ctkDICOMEcho.h>

#include "qSlicerPatientsQueueWorklistWidget.h"

//-----------------------------------------------------------------------------
class qSlicerPatientsQueueModuleWidgetPrivate: public Ui_qSlicerPatientsQueueModuleWidget
{
public:
  qSlicerPatientsQueueModuleWidgetPrivate();

  const char* GENERIC_LAYOUT_DESCRIPTION = \
    "<layout type=\"vertical\" split=\"true\" >"\
    "  <item splitSize=\"500\"> "\
    "    <layout type=\"horizontal\">"\
    "    <item>" \
    "     <view class=\"vtkMRMLViewNode\" singletontag=\"1\">" \
    "       <property name=\"viewlabel\" action=\"default\">1</property>" \
    "     </view>" \
    "    </item>" \
    "     <item>" \
    "       <view class=\"vtkMRMLSliceNode\" singletontag=\"XrayDetectorSlice\">" \
    "         <property name=\"orientation\" action=\"default\">Axial</property>" \
    "         <property name=\"viewlabel\" action=\"default\">C</property>" \
    "         <property name=\"viewcolor\" action=\"default\">#C3B1E1</property>" \
    "         <property name=\"viewgroup\" action=\"default\">101</property>" \
    "       </view>" \
    "     </item>" \
    "    </layout>" \
    "  </item>" \
    "  <item splitSize=\"350\">" \
    "    <layout type=\"horizontal\">" \
    "      <item>" \
    "         <view class=\"vtkMRMLSliceNode\" singletontag=\"Red\">" \
    "           <property name=\"orientation\" action=\"default\">Axial</property>" \
    "           <property name=\"viewlabel\" action=\"default\">R</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#F34A33</property>" \
    "        </view>" \
    "      </item>" \
    "      <item>" \
    "         <view class=\"vtkMRMLSliceNode\" singletontag=\"Green\">" \
    "           <property name=\"orientation\" action=\"default\">Coronal</property>" \
    "           <property name=\"viewlabel\" action=\"default\">G</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#6EB04B</property>" \
    "        </view>" \
    "      </item>" \
    "      <item>" \
    "         <view class=\"vtkMRMLSliceNode\" singletontag=\"Yellow\">" \
    "           <property name=\"orientation\" action=\"default\">Sagittal</property>" \
    "           <property name=\"viewlabel\" action=\"default\">Y</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#EDD54C</property>" \
    "        </view>" \
    "      </item>" \
    "    </layout>" \
    "  </item>" \
    "  <item splitSize=\"0\">" \
    "    <layout type=\"horizontal\">" \
    "      <item>" \
    "        <view class=\"vtkMRMLViewNode\" singletontag=\"XrayDetectorSlice\">" \
    "           <property name=\"viewlabel\" action=\"default\">F</property>" \
    "           <property name=\"viewcolor\" action=\"default\">#C3B1E1</property>" \
    "           <property name=\"viewgroup\" action=\"default\">100</property>" \
    "        </view>" \
    "       </item>" \
    "    </layout>" \
    "  </item>" \
    "</layout>";
  const int GENERIC_LAYOUT_ID = 1020;

  const char* PWL_LAYOUT_DESCRIPTION = \
    "<layout type=\"vertical\">" \
    " <item>" \
    "  <PatientsQueueWorklist></PatientsQueueWorklist>" \
    " </item>" \
    "</layout>";
  const int PWL_LAYOUT_ID = 1021;

  QScopedPointer< qSlicerPatientsQueueWorklistWidget > WorklistWidget;
  std::unique_ptr< DcmDataset > WorklistQueryDataset;

  int PreviousLayoutId{ -1 };
  bool ModuleWindowInitialized{ false };
};

//-----------------------------------------------------------------------------
// qSlicerPatientsQueueModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerPatientsQueueModuleWidgetPrivate::qSlicerPatientsQueueModuleWidgetPrivate()
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
  this->WorklistQueryDataset->putAndInsertOFStringArray(DCM_SpecificCharacterSet, "ISO_IR 192");
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
// qSlicerPatientsQueueModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerPatientsQueueModuleWidget::qSlicerPatientsQueueModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerPatientsQueueModuleWidgetPrivate )
{
  Q_D(qSlicerPatientsQueueModuleWidget);
  d->WorklistWidget.reset(new qSlicerPatientsQueueWorklistWidget(this));
}

//-----------------------------------------------------------------------------
qSlicerPatientsQueueModuleWidget::~qSlicerPatientsQueueModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPatientsQueueModuleWidget::setup()
{
  Q_D(qSlicerPatientsQueueModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Get layout manager
  qSlicerApplication* slicerApplication = qSlicerApplication::application();
  qSlicerLayoutManager* layoutManager = slicerApplication->layoutManager();

  qSlicerSingletonViewFactory* viewFactory = new qSlicerSingletonViewFactory();
  viewFactory->setWidget(d->WorklistWidget.get());
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

/*
  // Save previous layout
  d->PreviousLayoutId = layoutManager->layout();

  vtkMRMLLayoutNode* layoutNode = layoutManager->layoutLogic()->GetLayoutNode();
  if (layoutNode)
  {
    if (!layoutNode->SetLayoutDescription(d->GENERIC_LAYOUT_ID, d->GENERIC_LAYOUT_DESCRIPTION))
    {
      layoutNode->AddLayoutDescription(d->GENERIC_LAYOUT_ID, d->GENERIC_LAYOUT_DESCRIPTION);
    }
  }
*/
  // Nodes

  // Buttons
  QObject::connect(d->PushButton_Worklist1, SIGNAL(clicked()),
    this, SLOT(onSetCustomLayoutClicked()));
  QObject::connect(d->PushButton_CheckConnection, SIGNAL(clicked()),
    this, SLOT(onCheckConnectionClicked()));
}

void qSlicerPatientsQueueModuleWidget::onSetCustomLayoutClicked()
{
  Q_D(qSlicerPatientsQueueModuleWidget);
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

void qSlicerPatientsQueueModuleWidget::onCheckConnectionClicked()
{
  Q_D(qSlicerPatientsQueueModuleWidget);

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
  
/*
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
  OFString verificationSOP = UID_VerificationSOPClass;
  OFList<OFString> ts;
  ts.push_back(UID_LittleEndianExplicitTransferSyntax);
  ts.push_back(UID_BigEndianExplicitTransferSyntax);
  ts.push_back(UID_LittleEndianImplicitTransferSyntax);
  scu.addPresentationContext(verificationSOP, ts);
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

  // Issue ECHO request and let scu find presentation context itself (0)
  result = scu.sendECHORequest(0);
  if (result.bad())
  {
    qWarning() << Q_FUNC_INFO << "Error issuing ECHO request or received rejecting response: " << result.text() << "\n";
    return;
  }
  qWarning() << Q_FUNC_INFO  << "Successfully sent DICOM Echo to host " << hostStr << " on port " << hostPort << "\n";

  result = scu.releaseAssociation();
  if (result.bad())
  {
    qWarning() << Q_FUNC_INFO << "Error releasing association with peer: " << result.text() << "\n";
    return;
  }

  int res = QMessageBox::information(this, tr("Check connection (ECHO)"), tr("Connection to peer was successful!"));
  Q_UNUSED(res);
*/
}
