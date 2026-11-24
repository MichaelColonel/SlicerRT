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
#include <QOpcUaClient>
#include <QWeakPointer>
#include <QSharedPointer>
#include <QScopedPointer>
#include <QOpcUaNode>
#include <QProgressDialog>
#include <QMessageBox>
#include <QTimer>

// Slicer includes
#include <qSlicerSingletonViewFactory.h>
#include <qSlicerLayoutManager.h>
#include <qSlicerApplication.h>

// SiemensPlcOpcUa Widgets includes
#include "qSlicerSiemensPlcOpcUaWidget.h"
#include "ui_qSlicerSiemensPlcOpcUaWidget.h"

#include "qSlicerOpcUaRobotsMessagesListModel.h"
#include "vtkSlicerU70RoomSiemensPlcOpcUaNodesNames.h"

// U70RoomOpcUa Logic includes
#include "vtkSlicerU70RoomOpcUaLogic.h"
#include "vtkSlicerU70RoomSiemensPlcOpcUaLogic.h"
#include "vtkSlicerU70RoomSiemensPlcOpcUaNodesNames.h"

#include "OpcUaModel.h"
#include "OpcUaTreeItem.h"
#include "qSlicerOpcUaInitialValueNodeReader.h"
#include "qSlicerOpcUaInitialValueNodeLoader.h"

#include <vtkMRMLSiemensPlcOpcUaNode.h>
#include <bitset>

namespace {

const QOpcUaMonitoringParameters OpcUaMonitorParameters(100);

}

//-----------------------------------------------------------------------------
class NodeData {
public:
  NodeData() = default;
  NodeData(QOpcUaNode* node, const QString& id, const QString& name, const QString& desc)
    : nodePtr(node)
    , nodeId(id)
    , nodeName(name)
    , nodeDescription(desc)
  {
  }
  QSharedPointer< QOpcUaNode > getNodePtr() const { return nodePtr; }
  void setNode(QOpcUaNode* node) { this->nodePtr.reset(node); }
  QOpcUaNode* getNode() { return nodePtr.data(); }
  QOpcUaNode* getNode() const { return nodePtr.data(); }
  QString getNodeId() const { return nodeId; }
  QString getNodeName() const { return nodeName; }
  QString getNodeDescription() const { return nodeDescription; }
protected:
  QSharedPointer< QOpcUaNode > nodePtr;
  QString nodeId;
  QString nodeName;
  QString nodeDescription;
};

//-----------------------------------------------------------------------------
class qSlicerSiemensPlcOpcUaWidgetPrivate : public Ui_qSlicerSiemensPlcOpcUaWidget
{
  Q_DECLARE_PUBLIC(qSlicerSiemensPlcOpcUaWidget)
protected:
  qSlicerSiemensPlcOpcUaWidget* const q_ptr;

public:
  qSlicerSiemensPlcOpcUaWidgetPrivate(qSlicerSiemensPlcOpcUaWidget& object);
  virtual void setupUi(qSlicerSiemensPlcOpcUaWidget*);

  bool ParseServerParentNode(const QString& nodeDisplayName);
  QOpcUaNode* FindNodeFromFullDisplayName(const QString& nodeDisplayName);
  void ExpandAllAsync(QTreeView *view, QAbstractItemModel* model,
    const QModelIndex &parentIndex = QModelIndex());

  QScopedPointer< OpcUaModel > SiemensPlcOpcUaModel;

  QScopedPointer< qSlicerOpcUaRobotsMessagesListModel > SiemensPlcErrorListModel;
  QScopedPointer< qSlicerOpcUaRobotsMessagesListModel > SiemensPlcServiceListModel;
  QScopedPointer< qSlicerOpcUaRobotsMessagesListModel > SiemensPlcMessagesListModel;

  QWeakPointer< QOpcUaClient > OpcUaClient; // client
  vtkWeakPointer< vtkMRMLSiemensPlcOpcUaNode > ParameterNode; // MRML OPC UA parameters node

  // Parse parent node
  QScopedPointer< QOpcUaNode > ServerInterfacesNode;
  // Monitored node
  QScopedPointer< QOpcUaNode > CurrentTimeNode; // Siemens PLC monitored node to prevent session timeout ending
  QMap< QString, NodeData > NodeNameDataMap; // key - node unique full name, value - node data
  vtkSlicerU70RoomOpcUaLogic* opcUaLogic{ nullptr };

  QScopedPointer< QProgressDialog > ProgressDialog;
  QScopedPointer< QTimer > ProgressTimer;
  QScopedPointer< qSlicerOpcUaInitialValueNodeReader > InitialValuesNodeReader;
  QScopedPointer< qSlicerOpcUaInitialValueNodeLoader > InitialValuesNodeLoader;

  bool AutoConnectServerFlag{ false };
};

// --------------------------------------------------------------------------
qSlicerSiemensPlcOpcUaWidgetPrivate::qSlicerSiemensPlcOpcUaWidgetPrivate(qSlicerSiemensPlcOpcUaWidget& object)
  : q_ptr(&object)
  , SiemensPlcOpcUaModel(new OpcUaModel(&object))
  , SiemensPlcErrorListModel(new qSlicerOpcUaRobotsMessagesListModel(&object))
  , SiemensPlcServiceListModel(new qSlicerOpcUaRobotsMessagesListModel(&object))
  , SiemensPlcMessagesListModel(new qSlicerOpcUaRobotsMessagesListModel(&object))
  , ProgressDialog(new QProgressDialog(&object))
  , ProgressTimer(new QTimer(&object))
{
}

// --------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidgetPrivate::setupUi(qSlicerSiemensPlcOpcUaWidget* widget)
{
  Q_Q(qSlicerSiemensPlcOpcUaWidget);

  this->Ui_qSlicerSiemensPlcOpcUaWidget::setupUi(widget);

  this->ProgressDialog->setWindowTitle(QObject::tr("Siemens PLC OPC UA"));
  this->ProgressDialog->setWindowModality(Qt::NonModal);
  this->ProgressDialog->setMinimumDuration(500);
  this->ProgressDialog->setAutoReset(false);
  this->ProgressDialog->setAutoClose(false);
  this->ProgressDialog->close();

  this->ProgressTimer->setSingleShot(true);
  this->ProgressTimer->setInterval(300);

  this->TreeView_OpcUaModel->setModel(SiemensPlcOpcUaModel.data());
  this->ListView_ErrorMessages->setModel(SiemensPlcErrorListModel.data());
  this->ListView_ServiceMessages->setModel(SiemensPlcServiceListModel.data());
  this->ListView_MiscMessages->setModel(SiemensPlcMessagesListModel.data());
  
  this->TreeView_OpcUaModel->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

//-----------------------------------------------------------------------------
bool qSlicerSiemensPlcOpcUaWidgetPrivate::ParseServerParentNode(const QString& serverNodeDispName)
{
  if (!this->ServerInterfacesNode)
  {
    return false;
  }
  if (serverNodeDispName.isEmpty())
  {
    return false;
  }

  QSharedPointer< QOpcUaClient > opcUaClient = this->OpcUaClient.toStrongRef();
  if (!opcUaClient)
  {
    return false;
  }
  OpcUaTreeItem* parentItem = nullptr;
  OpcUaTreeItem* rootItem = SiemensPlcOpcUaModel->getRootItem();
  if (rootItem)
  {
    parentItem = OpcUaTreeItem::findParentItemByName(rootItem, serverNodeDispName);
  }
  else
  {
    return false;
  }
  this->NodeNameDataMap.clear();
  if (parentItem)
  {
    QList< QPointer< OpcUaTreeItem > > items = OpcUaTreeItem::findLeafValueItems(parentItem);
    for (QPointer< OpcUaTreeItem > item : items)
    {
      QStringList parentNames = OpcUaTreeItem::findParentNamesForItem(item.data(), parentItem);

      if (parentNames.size())
      {
        QString hierNames;

        for (auto iter = parentNames.rbegin(); iter != parentNames.rend(); ++iter)
        {
          if (iter != parentNames.rend() - 1)
          {
            hierNames += *iter + QString(".");
          }
          else
          {
            hierNames += *iter;
          }
        }
        QOpcUaNode* itemNode = opcUaClient->node(item->getNodeId());

        NodeData data(itemNode, item->getNodeId(), hierNames, item->getNodeDescription());
        this->NodeNameDataMap[hierNames] = data;
      }
    }
  }
  return true;
}

//-----------------------------------------------------------------------------
QOpcUaNode* qSlicerSiemensPlcOpcUaWidgetPrivate::FindNodeFromFullDisplayName(const QString& nodeDisplayName)
{
  if (!this->NodeNameDataMap.size())
  {
    return nullptr;
  }
  for (auto& nodeData : this->NodeNameDataMap)
  {
    const QString& fullName = nodeData.getNodeName();
    if (fullName == nodeDisplayName)
    {
      return nodeData.getNode();
    }
  }
  return nullptr;
}

//-----------------------------------------------------------------------------
// qSlicerSiemensPlcOpcUaWidget methods

//-----------------------------------------------------------------------------
qSlicerSiemensPlcOpcUaWidget::qSlicerSiemensPlcOpcUaWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerSiemensPlcOpcUaWidgetPrivate(*this) )
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  d->setupUi(this);

  OpcUaModel* opcUaModel = d->SiemensPlcOpcUaModel.data();
  QTreeView* opcUaTreeView = d->TreeView_OpcUaModel;

  QObject::connect(d->ProgressTimer.data(), SIGNAL(timeout()), d->ProgressDialog.data(), SLOT(reset()));

  QObject::connect(opcUaModel, &QAbstractItemModel::rowsInserted, this,
    [this, opcUaModel, opcUaTreeView](const QModelIndex& parent, int first, int last)
    {
      opcUaTreeView->expand(parent);
      for (int row = first; row <= last; ++row)
      {
        QModelIndex childIndex = opcUaModel->index(row, 0, parent);
        opcUaTreeView->expand(childIndex);
        opcUaModel->rowCount(childIndex);
      }
    }
  );

  QProgressDialog* progressDialog = d->ProgressDialog.data();
  QTimer* progressTimer = d->ProgressTimer.data();
  QObject::connect(opcUaModel, &OpcUaModel::browseProgressChanged, this,
    [this, progressDialog, progressTimer](int finished, int started)
    {
      progressDialog->setLabelText(tr("Loading tree of nodes. Please, wait!"));
      progressDialog->setMaximum(started);
      progressDialog->setValue(finished);
      if (finished >= started)
      {
        progressTimer->start();
        progressDialog->close();
        emit this->nodesTreeLoadingFinished();
      }
      else
      {
        progressTimer->stop();
      }
    }
  );

  // Set coords from ASU buttons
  QObject::connect(d->PushButton_SetCoordX, SIGNAL(clicked()),
    this, SLOT(onSetCoordXClicked()));
  QObject::connect(d->PushButton_SetCoordY, SIGNAL(clicked()),
    this, SLOT(onSetCoordYClicked()));
  QObject::connect(d->PushButton_SetCoordZ, SIGNAL(clicked()),
    this, SLOT(onSetCoordZClicked()));
  QObject::connect(d->PushButton_SetAngleA, SIGNAL(clicked()),
    this, SLOT(onSetAngleAClicked())); 
  QObject::connect(d->PushButton_SetAngleB, SIGNAL(clicked()),
    this, SLOT(onSetAngleBClicked()));
  QObject::connect(d->PushButton_SetAngleC, SIGNAL(clicked()),
    this, SLOT(onSetAngleCClicked()));
  QObject::connect(d->PushButton_SetCoords, SIGNAL(clicked()),
    this, SLOT(onSetCoordsAndAnglesClicked()));
  QObject::connect(d->PushButton_SetTabletopPosition, SIGNAL(clicked()),
    this, SLOT(onSetTabletopPositionClicked()));

  // AutoManual R1 ToLoad button
  QObject::connect(d->PushButton_AMR1ToLoad, SIGNAL(pressed()),
    this, SLOT(onAutoManualR1ToLoadPressed()));
  QObject::connect(d->PushButton_AMR1ToLoad, SIGNAL(released()),
    this, SLOT(onAutoManualR1ToLoadReleased()));
  // AutoManual R1 LoadToIso button
  QObject::connect(d->PushButton_AMR1LoadToIso, SIGNAL(pressed()),
    this, SLOT(onAutoManualR1LoadToIsoPressed()));
  QObject::connect(d->PushButton_AMR1LoadToIso, SIGNAL(released()),
    this, SLOT(onAutoManualR1LoadToIsoReleased()));
  // AutoManual R1 ToNewCoords button
  QObject::connect(d->PushButton_AMR1ToNewCoords, SIGNAL(pressed()),
    this, SLOT(onAutoManualR1ToNewCoordsPressed()));
  QObject::connect(d->PushButton_AMR1ToNewCoords, SIGNAL(released()),
    this, SLOT(onAutoManualR1ToNewCoordsReleased()));

  // AutoManual R2 ToHome button
  QObject::connect(d->PushButton_AMR2ToHome, SIGNAL(pressed()),
    this, SLOT(onAutoManualR2ToHomePressed()));
  QObject::connect(d->PushButton_AMR2ToHome, SIGNAL(released()),
    this, SLOT(onAutoManualR2ToHomeReleased()));
  // AutoManual R2 ToPlane1 button
  QObject::connect(d->PushButton_AMR2ToPlane1, SIGNAL(pressed()),
    this, SLOT(onAutoManualR2ToPlane1Pressed()));
  QObject::connect(d->PushButton_AMR2ToPlane1, SIGNAL(released()),
    this, SLOT(onAutoManualR2ToPlane1Released()));
  // AutoManual R2 ToPlane2 button
  QObject::connect(d->PushButton_AMR2ToPlane2, SIGNAL(pressed()),
    this, SLOT(onAutoManualR2ToPlane2Pressed()));
  QObject::connect(d->PushButton_AMR2ToPlane2, SIGNAL(released()),
    this, SLOT(onAutoManualR2ToPlane2Released()));
  // AutoManual R1R2 EmerEvac button
  QObject::connect(d->PushButton_AMR1R2EmerEvac, SIGNAL(pressed()),
    this, SLOT(onAutoManualR1R2EmerEvacPressed()));
  QObject::connect(d->PushButton_AMR1R2EmerEvac, SIGNAL(released()),
    this, SLOT(onAutoManualR1R2EmerEvacReleased()));
  // AutoManual ApplyTableTopPosition
  QObject::connect(d->PushButton_AMApplyTableTopPosition, SIGNAL(pressed()),
    this, SLOT(onAutoManualApplyTablePositionPressed()));
  QObject::connect(d->PushButton_AMApplyTableTopPosition, SIGNAL(released()),
    this, SLOT(onAutoManualApplyTablePositionReleased()));
  // AutoManual ApplyNewCoords
  QObject::connect(d->PushButton_AMApplyNewCoords, SIGNAL(pressed()),
    this, SLOT(onAutoManualApplyNewCoordsPressed()));
  QObject::connect(d->PushButton_AMApplyNewCoords, SIGNAL(released()),
    this, SLOT(onAutoManualApplyNewCoordsReleased()));
/**
  // Service R1 BreakTest button
  QObject::connect(d->PushButton_SR1BrakeTest, SIGNAL(pressed()),
    this, SLOT(onServiceR1BreakTestPressed()));
  QObject::connect(d->PushButton_SR1BrakeTest, SIGNAL(released()),
    this, SLOT(onServiceR1BreakTestReleased()));
  // Service R1 MasterReferenceTest button
  QObject::connect(d->PushButton_SR1MasterRefTest, SIGNAL(pressed()),
    this, SLOT(onServiceR1MasterReferenceTestPressed()));
  QObject::connect(d->PushButton_SR1MasterRefTest, SIGNAL(released()),
    this, SLOT(onServiceR1MasterReferenceTestReleased()));
  // Service R1 ToLoad button
  QObject::connect(d->PushButton_SR1ToLoad, SIGNAL(pressed()),
    this, SLOT(onServiceR1LoadPressed()));
  QObject::connect(d->PushButton_SR1ToLoad, SIGNAL(released()),
    this, SLOT(onServiceR1LoadReleased()));
  // Service R1 ToServicePos1 button
  QObject::connect(d->PushButton_SMR1ServicePos1, SIGNAL(pressed()),
    this, SLOT(onServiceR1ServicePos1Pressed()));
  QObject::connect(d->PushButton_SMR1ServicePos1, SIGNAL(released()),
    this, SLOT(onServiceR1ServicePos1Released()));
  // Service R1 ToServicePos2 button
  QObject::connect(d->PushButton_SMR1ServicePos2, SIGNAL(pressed()),
    this, SLOT(onServiceR1ServicePos2Pressed()));
  QObject::connect(d->PushButton_SMR1ServicePos2, SIGNAL(released()),
    this, SLOT(onServiceR1ServicePos2Released()));
  // Service R1 ToServicePos3 button
  QObject::connect(d->PushButton_SMR1ServicePos3, SIGNAL(pressed()),
    this, SLOT(onServiceR1ServicePos3Pressed()));
  QObject::connect(d->PushButton_SMR1ServicePos3, SIGNAL(released()),
    this, SLOT(onServiceR1ServicePos3Released()));

  // Service R2 BreakTest button
  QObject::connect(d->PushButton_SR2BrakeTest, SIGNAL(pressed()),
    this, SLOT(onServiceR2BreakTestPressed()));
  QObject::connect(d->PushButton_SR2BrakeTest, SIGNAL(released()),
    this, SLOT(onServiceR2BreakTestReleased()));
  // Service R2 MasterReferenceTest button
  QObject::connect(d->PushButton_SR2MasterRefTest, SIGNAL(pressed()),
    this, SLOT(onServiceR2MasterReferenceTestPressed()));
  QObject::connect(d->PushButton_SR2MasterRefTest, SIGNAL(released()),
    this, SLOT(onServiceR2MasterReferenceTestReleased()));
  // Service R2 ToHome button
  QObject::connect(d->PushButton_SR2ToHome, SIGNAL(pressed()),
    this, SLOT(onServiceR2HomePressed()));
  QObject::connect(d->PushButton_SR2ToHome, SIGNAL(released()),
    this, SLOT(onServiceR2HomeReleased()));
  // Service R2 ToServicePos1 button
  QObject::connect(d->PushButton_SMR2ServicePos1, SIGNAL(pressed()),
    this, SLOT(onServiceR2ServicePos1Pressed()));
  QObject::connect(d->PushButton_SMR2ServicePos1, SIGNAL(released()),
    this, SLOT(onServiceR2ServicePos1Released()));
  // Service R2 ToServicePos2 button
  QObject::connect(d->PushButton_SMR2ServicePos2, SIGNAL(pressed()),
    this, SLOT(onServiceR2ServicePos2Pressed()));
  QObject::connect(d->PushButton_SMR2ServicePos2, SIGNAL(released()),
    this, SLOT(onServiceR2ServicePos2Released()));
  // Service R2 ToServicePos3 button
  QObject::connect(d->PushButton_SMR2ServicePos3, SIGNAL(pressed()),
    this, SLOT(onServiceR2ServicePos3Pressed()));
  QObject::connect(d->PushButton_SMR2ServicePos3, SIGNAL(released()),
    this, SLOT(onServiceR2ServicePos3Released()));
  // Service RestartSM button
  QObject::connect(d->PushButton_SMRestart, SIGNAL(pressed()),
    this, SLOT(onServiceRestartSmPressed()));
  QObject::connect(d->PushButton_SMRestart, SIGNAL(released()),
    this, SLOT(onServiceRestartSmReleased()));

  // KUKA controllers movement AllowT1 button
  QObject::connect(d->PushButton_KCMAllowT1, SIGNAL(pressed()),
    this, SLOT(onKukaAllowT1Pressed()));
  QObject::connect(d->PushButton_KCMAllowT1, SIGNAL(released()),
    this, SLOT(onKukaAllowT1Released()));
  // KUKA controllers movement AllowXRay button
  QObject::connect(d->PushButton_KCMAllowXRay, SIGNAL(pressed()),
    this, SLOT(onKukaAllowXrayPressed()));
  QObject::connect(d->PushButton_KCMAllowXRay, SIGNAL(released()),
    this, SLOT(onKukaAllowXrayReleased()));
  // KUKA controllers movement AllowBeam button
  QObject::connect(d->PushButton_KCMAllowBeam, SIGNAL(pressed()),
    this, SLOT(onKukaAllowBeamPressed()));
  QObject::connect(d->PushButton_KCMAllowBeam, SIGNAL(released()),
    this, SLOT(onKukaAllowBeamReleased()));
*/
  // Reset Errors button
  QObject::connect(d->PushButton_ResetErrors, SIGNAL(pressed()),
    this, SLOT(onResetErrorsPressed()));
  QObject::connect(d->PushButton_ResetErrors, SIGNAL(released()),
    this, SLOT(onResetErrorsReleased()));

  // MakeXRay button
//  QObject::connect(d->PushButton_MakeXRay, SIGNAL(pressed()),
//    this, SLOT(onMakeXrayPressed()));
//  QObject::connect(d->PushButton_MakeXRay, SIGNAL(released()),
//    this, SLOT(onMakeXrayReleased()));

  QObject::connect(d->ButtonGroup_TabletopPosition, SIGNAL(buttonClicked(QAbstractButton*)),
    this, SLOT(onTabletopPositionChanged(QAbstractButton*)));

  // Read nodes manually
  QObject::connect(d->PushButton_ReadStatusAndMessages, SIGNAL(clicked()),
    this, SLOT(onReadStatusAndMessagesClicked()));
}

//-----------------------------------------------------------------------------
qSlicerSiemensPlcOpcUaWidget::~qSlicerSiemensPlcOpcUaWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::setParameterNode(vtkMRMLNode* node)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);

  vtkMRMLSiemensPlcOpcUaNode* parameterNode = vtkMRMLSiemensPlcOpcUaNode::SafeDownCast(node);
  // Each time the node is modified, the UI widgets are updated
  qvtkReconnect(d->ParameterNode, parameterNode, vtkCommand::ModifiedEvent, 
    this, SLOT(updateWidgetFromMRML()));

  d->ParameterNode = parameterNode;

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::setSiemensPlcOpcUaClient(const QSharedPointer< QOpcUaClient >& sharedClient)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);

  d->OpcUaClient = sharedClient;
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  if (!d->ParameterNode)
  {
    return;
  }
  d->CollapsibleButton_AutomaticManualMovement->setEnabled(true);
  d->CollapsibleButton_ServiceMovement->setEnabled(true);
  d->CollapsibleButton_KukaControllersMovement->setEnabled(true);

  uint64_t errBits = d->ParameterNode->GetErrorMessages();
  uint64_t servBits = d->ParameterNode->GetServiceMessages();
  uint64_t miscBits = d->ParameterNode->GetMiscMessages();
  d->SiemensPlcErrorListModel->updateMessagesBits(errBits, 0, 0);
  d->SiemensPlcServiceListModel->updateMessagesBits(0, servBits, 0);
  d->SiemensPlcMessagesListModel->updateMessagesBits(0, 0, miscBits);

  bool buttonState[2] = { false, false };
  // AM
  d->ParameterNode->GetAutoManualR1ToLoad(buttonState);
  d->PushButton_AMR1ToLoad->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR1LoadToIso(buttonState);
  d->PushButton_AMR1LoadToIso->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR1ToNewCoords(buttonState);
  d->PushButton_AMR1ToNewCoords->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR2ToHome(buttonState);
  d->PushButton_AMR2ToHome->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR2ToPlane1(buttonState);
  d->PushButton_AMR2ToPlane1->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualR2ToPlane2(buttonState);
  d->PushButton_AMR2ToPlane2->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualEmergencyEvac(buttonState);
  d->PushButton_AMR1R2EmerEvac->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualApplyTabletopPosition(buttonState);
  d->PushButton_AMApplyTableTopPosition->setEnabled(buttonState[0]);

  d->ParameterNode->GetAutoManualApplyNewCoords(buttonState);
  d->PushButton_AMApplyNewCoords->setEnabled(buttonState[0]);

  // SM
  d->ParameterNode->GetServiceR1BreakTest(buttonState);
  d->PushButton_SR1BrakeTest->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR1MasterReferenceTest(buttonState);
  d->PushButton_SR1MasterRefTest->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR1Load(buttonState);
  d->PushButton_SR1ToLoad->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR1Position1(buttonState);
  d->PushButton_SMR1ServicePos1->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR1Position2(buttonState);
  d->PushButton_SMR1ServicePos2->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR1Position3(buttonState);
  d->PushButton_SMR1ServicePos3->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR2BreakTest(buttonState);
  d->PushButton_SR2BrakeTest->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR2MasterReferenceTest(buttonState);
  d->PushButton_SR2MasterRefTest->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR2Home(buttonState);
  d->PushButton_SR2ToHome->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR2Position1(buttonState);
  d->PushButton_SMR2ServicePos1->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR2Position2(buttonState);
  d->PushButton_SMR2ServicePos2->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceR2Position3(buttonState);
  d->PushButton_SMR2ServicePos3->setEnabled(buttonState[0]);

  d->ParameterNode->GetServiceRestart(buttonState);
  d->PushButton_SMRestart->setEnabled(buttonState[0]);

  // KCM
  d->ParameterNode->GetKukaAllowT1(buttonState);
  d->PushButton_KCMAllowT1->setEnabled(buttonState[0]);

  d->ParameterNode->GetKukaAllowXray(buttonState);
  d->PushButton_KCMAllowXRay->setEnabled(buttonState[0]);

  d->ParameterNode->GetKukaAllowBeam(buttonState);
  d->PushButton_KCMAllowBeam->setEnabled(buttonState[0]);

  // Misc.
  d->ParameterNode->GetResetErrors(buttonState);
  d->PushButton_ResetErrors->setEnabled(buttonState[0]);

  d->ParameterNode->GetMakeXray(buttonState);
  d->PushButton_MakeXRay->setEnabled(buttonState[0]);

  {
    QSignalBlocker block(d->CheckBox_HUMAN_ON_DEKA);
    bool flag = d->ParameterNode->GetPatientOnTableTop();
    d->CheckBox_HUMAN_ON_DEKA->setChecked(flag);
  }

  {
    QSignalBlocker block(d->CheckBox_HUMAN_ON_DEKA);
    bool flag = d->ParameterNode->GetPatientOnTableTop();
    d->CheckBox_HUMAN_ON_DEKA->setChecked(flag);
  }

  {
    QSignalBlocker block0(d->CheckBox_RobotsReadyForXray1);
    bool flag = d->ParameterNode->GetRobotsReadyForXray1();
    d->CheckBox_RobotsReadyForXray1->setChecked(flag);

    QSignalBlocker block1(d->CheckBox_RobotsReadyForXray2);
    flag = d->ParameterNode->GetRobotsReadyForXray2();
    d->CheckBox_RobotsReadyForXray2->setChecked(flag);

    QSignalBlocker block2(d->CheckBox_RobotsReadyForBeam);
    flag = d->ParameterNode->GetRobotsReadyForBeam();
    d->CheckBox_RobotsReadyForBeam->setChecked(flag);
  }

  {
    QSignalBlocker block0(d->CheckBox_DoorLockSensor1);
    bool flag = d->ParameterNode->GetDoorLockSensor1();
    d->CheckBox_DoorLockSensor1->setChecked(flag);

    QSignalBlocker block1(d->CheckBox_DoorLockSensor2);
    flag = d->ParameterNode->GetDoorLockSensor2();
    d->CheckBox_DoorLockSensor2->setChecked(flag);

    QSignalBlocker block2(d->CheckBox_EmergencyEvacuationFlag);
    flag = d->ParameterNode->GetEmergencyEvacSignal();
    d->CheckBox_EmergencyEvacuationFlag->setChecked(flag);
  }

  {
    QSignalBlocker block0(d->CheckBox_RobotsReadyForXray1);
    bool flag = d->ParameterNode->GetRobotsReadyForXray1();
    d->CheckBox_RobotsReadyForXray1->setChecked(flag);

    QSignalBlocker block1(d->CheckBox_RobotsReadyForXray2);
    flag = d->ParameterNode->GetRobotsReadyForXray2();
    d->CheckBox_RobotsReadyForXray2->setChecked(flag);

    QSignalBlocker block2(d->CheckBox_RobotsReadyForBeam);
    flag = d->ParameterNode->GetRobotsReadyForBeam();
    d->CheckBox_RobotsReadyForBeam->setChecked(flag);
  }

  {
    QSignalBlocker block0(d->RadioButton_PositionNotDefined);
    QSignalBlocker block1(d->RadioButton_Position1);
    QSignalBlocker block2(d->RadioButton_Position2);
    QSignalBlocker block3(d->RadioButton_Position3);
    uint8_t position = d->ParameterNode->GetTableTopPosition();
    switch (position)
    {
    case 0:
      d->RadioButton_PositionNotDefined->setChecked(true);
      break;
    case 1:
      d->RadioButton_Position1->setChecked(true);
      break;
    case 2:
      d->RadioButton_Position2->setChecked(true);
      break;
    case 3:
      d->RadioButton_Position3->setChecked(true);
      break;
    default:
      break;
    }
  }

  // Coordinate widgets
  double coordsAndAngles[6] = {}; // X,Y,Z,A,B,C
  d->ParameterNode->GetCoordsAndAngles(coordsAndAngles);
  d->DoubleSpinBox_CoordX->setValue(coordsAndAngles[0]);
  d->DoubleSpinBox_CoordY->setValue(coordsAndAngles[1]);
  d->DoubleSpinBox_CoordZ->setValue(coordsAndAngles[2]);
  d->DoubleSpinBox_AngleA->setValue(coordsAndAngles[3]);
  d->DoubleSpinBox_AngleB->setValue(coordsAndAngles[4]);
  d->DoubleSpinBox_AngleC->setValue(coordsAndAngles[5]);

  double r1Angles[6] = {};
  double r2Angles[6] = {};
  d->ParameterNode->GetCurrentAxisCoordsR1(r1Angles);
  d->ParameterNode->GetCurrentAxisCoordsR2(r2Angles);
  d->SliderWidget_R1A1->setValue(r1Angles[0]);
  d->SliderWidget_R1A2->setValue(r1Angles[1]);
  d->SliderWidget_R1A3->setValue(r1Angles[2]);
  d->SliderWidget_R1A4->setValue(r1Angles[3]);
  d->SliderWidget_R1A5->setValue(r1Angles[4]);
  d->SliderWidget_R1A6->setValue(r1Angles[5]);
  d->SliderWidget_R2A1->setValue(r2Angles[0]);
  d->SliderWidget_R2A2->setValue(r2Angles[1]);
  d->SliderWidget_R2A3->setValue(r2Angles[2]);
  d->SliderWidget_R2A4->setValue(r2Angles[3]);
  d->SliderWidget_R2A5->setValue(r2Angles[4]);
  d->SliderWidget_R2A6->setValue(r2Angles[5]);

  double r1KukaCoords[6] = {};
  d->ParameterNode->GetCurrentCoordsR1(r1KukaCoords);
  double* r1KukaAngles = r1KukaCoords + 3;
  d->CoordinatesWidget_CurrCoordsR1->setCoordinates(r1KukaCoords);
  d->CoordinatesWidget_CurrAnglesR1->setCoordinates(r1KukaAngles);

  double r2KukaCoords[6] = {};
  d->ParameterNode->GetCurrentCoordsR2(r2KukaCoords);
  double* r2KukaAngles = r2KukaCoords + 3;
  d->CoordinatesWidget_CurrCoordsR2->setCoordinates(r2KukaCoords);
  d->CoordinatesWidget_CurrAnglesR2->setCoordinates(r2KukaAngles);

  d->ParameterNode->GetConfirmedTargetCoordsR1(r1KukaCoords);
  d->CoordinatesWidget_ConfirmedTargetCoordsR1->setCoordinates(r1KukaCoords);
  d->CoordinatesWidget_ConfirmedTargetAnglesR1->setCoordinates(r1KukaAngles);

  d->ParameterNode->GetConfirmedTargetCoordsR2(r2KukaCoords);
  d->CoordinatesWidget_ConfirmedTargetCoordsR2->setCoordinates(r2KukaCoords);
  d->CoordinatesWidget_ConfirmedTargetAnglesR2->setCoordinates(r2KukaAngles);
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onOpcUaClientConnected()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  
  QSharedPointer< QOpcUaClient > sharedClient = d->OpcUaClient.toStrongRef();
  if (!sharedClient)
  {
    return;
  }

  d->SiemensPlcOpcUaModel->setOpcUaClient(sharedClient.data(), OpcUaTreeItem::SIEMENS_PLC_SERVER_INTERFACES_NODE_ID);
//  d->SiemensPlcOpcUaModel->setOpcUaClient(sharedClient.data(), QOpcUa::namespace0Id(QOpcUa::NodeIds::Namespace0::RootFolder));
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(0 /* BrowseName column*/, QHeaderView::Interactive);
  d->TreeView_OpcUaModel->header()->setSectionResizeMode(1 /* Value column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(2 /* NodeClass column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(3 /* DataType column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(4 /* NodeID column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(5 /* DisplayName column*/, QHeaderView::Interactive);
//  d->TreeView_OpcUaModel->header()->setSectionResizeMode(6 /* Description column*/, QHeaderView::Interactive);

  d->CurrentTimeNode.reset(sharedClient->node(QOpcUa::namespace0Id(QOpcUa::NodeIds::Namespace0::Server_ServerStatus_CurrentTime)));
  if (d->CurrentTimeNode)
  {
    // Subscribe to data changes
    d->CurrentTimeNode->enableMonitoring(QOpcUa::NodeAttribute::Value, QOpcUaMonitoringParameters(1000));
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onParseServerParentNodeClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  
  QSharedPointer< QOpcUaClient > sharedClient = d->OpcUaClient.toStrongRef();
  if (!sharedClient)
  {
    return;
  }

  d->ServerInterfacesNode.reset(sharedClient->node(OpcUaTreeItem::SIEMENS_PLC_SERVER_INTERFACES_NODE_ID));
  if (!d->ServerInterfacesNode)
  {
    return;
  }

  QObject::connect(d->ServerInterfacesNode.data(), &QOpcUaNode::attributeRead,
    this, &qSlicerSiemensPlcOpcUaWidget::onServerInterfacesRead);

  d->ServerInterfacesNode->readAttributes(QOpcUa::NodeAttribute::DisplayName);
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onOpcUaClientDisconnected()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);

  d->PushButton_ReadStatusAndMessages->setEnabled(false);
  if (d->CurrentTimeNode)
  {
    d->CurrentTimeNode->disableMonitoring(QOpcUa::NodeAttribute::Value);
  }
  d->SiemensPlcOpcUaModel->clear();
  d->SiemensPlcErrorListModel->clear();
  d->SiemensPlcServiceListModel->clear();
  d->SiemensPlcMessagesListModel->clear();
  
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onOpcUaModeChanged(vtkMRMLSiemensPlcOpcUaNode::ModeType mode)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteMode(mode);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": Set new mode:" << int(mode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServerInterfacesRead(QOpcUa::NodeAttributes attr)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);

  if (!d->ParameterNode)
  {
    return;
  }
  QHash< QString, QString > hashNodes;
  std::list< std::string > fullNamesList = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSiemensPlcFullNodeNamesList();

  if (attr & QOpcUa::NodeAttribute::DisplayName)
  { // Make sure the value attribute has been read
    QString nodeDisplayName = d->ServerInterfacesNode->attribute(QOpcUa::NodeAttribute::DisplayName).value<QOpcUaLocalizedText>().text();
    bool res = d->ParseServerParentNode(nodeDisplayName);
    if (res)
    {
      for (auto& nodeData : d->NodeNameDataMap)
      {
        if (nodeData.getNode())
        {
          hashNodes.insert(nodeData.getNodeName(), nodeData.getNodeId());
          auto iter = std::find(fullNamesList.begin(), fullNamesList.end(), nodeData.getNodeName().toStdString());
          if (iter != fullNamesList.end())
          {
//            qDebug() << Q_FUNC_INFO << "Node ID:" << nodeData.getNodeId()
//              << ", Name:" << nodeData.getNodeName()
//              << ", Description:" << nodeData.getNodeDescription() << Qt::endl;
          }
        }
      }
    }
  }
  d->PushButton_ReadStatusAndMessages->setEnabled(true);

  auto list = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->ServerConnected(d->OpcUaClient, hashNodes);
  QList< QOpcUaNode* > nodesList(list.begin(), list.end());

  d->ProgressDialog->setLabelText(tr("Reading nodes initial values, Please wait!"));
  d->ProgressDialog->setMaximum(nodesList.size());
  d->ProgressDialog->setValue(0);

  d->InitialValuesNodeLoader.reset(new qSlicerOpcUaInitialValueNodeLoader(nodesList, this));
  QObject::connect(d->InitialValuesNodeLoader.data(), &qSlicerOpcUaInitialValueNodeLoader::progress,
    this, [d](int done, int total)
  {
    d->ProgressDialog->setValue(done);
    d->ProgressTimer->stop();
  });
  QObject::connect(d->InitialValuesNodeLoader.data(), &qSlicerOpcUaInitialValueNodeLoader::finished,
    this, [this, d]()
  {
    if (d->ProgressTimer && d->ProgressDialog)
    {
      d->ProgressTimer->start();
      d->ProgressDialog->close();
    }
    d->InitialValuesNodeLoader.reset(nullptr);
    QMessageBox::information(this, tr("Siemens PLC OPC UA"), tr("Client is ready for data exchange with Siemens PLC!"));
  });
  d->ParameterNode->Modified();
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onReadStatusAndMessagesClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);

  std::list< std::string > fullNamesList = vtkSlicerU70RoomSiemensPlcOpcUaNodesNames::GetSiemensPlcFullNodeNamesList();
  QList< QOpcUaNode* > nodesList; 

  for (auto& nodeData : d->NodeNameDataMap)
  {
    if (nodeData.getNode())
    {
      auto iter = std::find(fullNamesList.begin(), fullNamesList.end(), nodeData.getNodeName().toStdString());
      if (iter != fullNamesList.end())
      {
        nodesList.push_back(nodeData.getNode());
      }
    }
  }

  d->ProgressDialog->setLabelText(tr("Reading nodes values, Please wait!"));
  d->ProgressDialog->setMaximum(nodesList.size());
  d->ProgressDialog->setValue(0);

  d->InitialValuesNodeReader.reset(new qSlicerOpcUaInitialValueNodeReader(nodesList, this));
  QObject::connect(d->InitialValuesNodeReader.data(), &qSlicerOpcUaInitialValueNodeReader::progress,
    this, [d](int done, int)
  {
    d->ProgressDialog->setValue(done);
    d->ProgressTimer->stop();
  });
  QObject::connect(d->InitialValuesNodeReader.data(), &qSlicerOpcUaInitialValueNodeReader::finished,
    this, [this, d]()
  {
    d->ProgressTimer->start();
    d->ProgressDialog->close();
    QMessageBox::information(this, tr("Siemens PLC OPC UA"), tr("Client is ready for data exchange with Siemens PLC!"));
  });

  d->InitialValuesNodeReader->start();
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1LoadToIsoPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 LoadToIso button pressed
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR1LoadToIsoPressed(true);
  double pos[6] = {};
  pos[0] = d->SliderWidget_R1A1->value();
  pos[1] = d->SliderWidget_R1A2->value();
  pos[2] = d->SliderWidget_R1A3->value();
  pos[3] = d->SliderWidget_R1A4->value();
  pos[4] = d->SliderWidget_R1A5->value();
  pos[5] = d->SliderWidget_R1A6->value();
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R1 LoadToIso button is pressed";
  }
  d->ParameterNode->SetLoadPositionAxisCoordsR1(pos);
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1LoadToIsoReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 LoadToIso button released
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR1LoadToIsoPressed(false);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R1 LoadToIso button is released";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1ToLoadPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 ToLoad button pressed
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR1ToLoadPressed(true);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R1 ToLoad button is pressed";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1ToLoadReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 ToLoad button released
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR1ToLoadPressed(false);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R1 ToLoad button is released";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1ToNewCoordsPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 ToNewCoords button pressed
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR1ToNewCoordsPressed(true);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R1 ToNewCoords button is pressed";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1ToNewCoordsReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 ToNewCoords button released
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR1ToNewCoordsPressed(false);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R1 ToNewCoords button is released";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToHomePressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R2 ToHome button pressed
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR2ToHomePressed(true);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R2 ToHome button is pressed";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToHomeReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R2 ToHome button released
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR2ToHomePressed(false);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R2 ToHome button is released";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToPlane1Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R2 ToPlane1 button pressed
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR2ToPlane1Pressed(true);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R2 ToPlane1 button is pressed";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToPlane1Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R2 ToPlane1 button released
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR2ToPlane1Pressed(false);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R2 ToPlane1 button is released";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToPlane2Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R2 ToPlane2 button pressed
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR2ToPlane2Pressed(true);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R2 ToPlane2 button is pressed";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR2ToPlane2Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R2 ToPlane2 button released
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR2ToPlane2Pressed(false);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R2 ToPlane2 button is released";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1R2EmerEvacPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1R2 EmergencyEvacuation button pressed
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualEmergencyEvacuationPressed(true);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R1R2 EmergencyEvacuation button is pressed";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualR1R2EmerEvacReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1R2 EmergencyEvacuation button released
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualEmergencyEvacuationPressed(false);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R1R2 EmergencyEvacuation button is released";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualApplyTablePositionPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual ApplyTablePosition button pressed
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualApplyTabletopOrientationPressed(true);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual ApplyTablePosition button is pressed";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualApplyTablePositionReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual ApplyTablePosition button released
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualApplyTabletopOrientationPressed(false);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual ApplyTablePosition button is released";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualApplyNewCoordsPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 ToNewCoords button pressed
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR1ToNewCoordsPressed(true);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R1 ToNewCoords button is pressed";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onAutoManualApplyNewCoordsReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // AutoManual R1 ToNewCoords button released
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteAutoManualR1ToNewCoordsPressed(false);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": AutoManual R1 ToNewCoords button is released";
  }
}

/*
//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1BreakTestPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 BreakTest button pressed node
  QOpcUaNode* smR1BeakTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_BREAKTEST_PRESSED_NODE_NAME);
  if (smR1BeakTestPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 BreakTest button pressed";
    smR1BeakTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1BreakTestReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 BreakTest button pressed node
  QOpcUaNode* smR1BeakTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_BREAKTEST_PRESSED_NODE_NAME);
  if (smR1BeakTestPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 BreakTest button released";
    smR1BeakTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1MasterReferenceTestPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 MasterReferenceTest button pressed node
  QOpcUaNode* smR1MasRefTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_MASREFTEST_PRESSED_NODE_NAME);
  if (smR1MasRefTestPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 MasterReferenceTest button pressed";
    smR1MasRefTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1MasterReferenceTestReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 MasterReferenceTest button pressed node
  QOpcUaNode* smR1MasRefTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_MASREFTEST_PRESSED_NODE_NAME);
  if (smR1MasRefTestPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 MasterReferenceTest button released";
    smR1MasRefTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1LoadPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 Load button pressed node
  QOpcUaNode* smR1LoadPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_LOAD_PRESSED_NODE_NAME);
  if (smR1LoadPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 Load button pressed";
    smR1LoadPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1LoadReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 Load button pressed node
  QOpcUaNode* smR1LoadPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_LOAD_PRESSED_NODE_NAME);
  if (smR1LoadPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 Load button released";
    smR1LoadPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos1Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition1 button pressed node
  QOpcUaNode* smR1ServicePos1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS1_PRESSED_NODE_NAME);
  if (smR1ServicePos1PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition1 button pressed";
    smR1ServicePos1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos1Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition1 button pressed node
  QOpcUaNode* smR1ServicePos1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS1_PRESSED_NODE_NAME);
  if (smR1ServicePos1PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition1 button released";
    smR1ServicePos1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos2Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition2 button pressed node
  QOpcUaNode* smR1ServicePos2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS2_PRESSED_NODE_NAME);
  if (smR1ServicePos2PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition2 button pressed";
    smR1ServicePos2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos2Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition2 button pressed node
  QOpcUaNode* smR1ServicePos2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS2_PRESSED_NODE_NAME);
  if (smR1ServicePos2PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition2 button released";
    smR1ServicePos2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos3Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition3 button pressed node
  QOpcUaNode* smR1ServicePos3PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS3_PRESSED_NODE_NAME);
  if (smR1ServicePos3PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition3 button pressed";
    smR1ServicePos3PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR1ServicePos3Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 ServicePosition3 button pressed node
  QOpcUaNode* smR1ServicePos3PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R1_SERVICEPOS3_PRESSED_NODE_NAME);
  if (smR1ServicePos3PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 ServicePosition3 button released";
    smR1ServicePos3PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2BreakTestPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 BreakTest button pressed node
  QOpcUaNode* smR2BeakTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_BREAKTEST_PRESSED_NODE_NAME);
  if (smR2BeakTestPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R2 BreakTest button pressed";
    smR2BeakTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2BreakTestReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 BreakTest button pressed node
  QOpcUaNode* smR2BeakTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_BREAKTEST_PRESSED_NODE_NAME);
  if (smR2BeakTestPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R2 BreakTest button released";
    smR2BeakTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2MasterReferenceTestPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 MasterReferenceTest button pressed node
  QOpcUaNode* smR2MasRefTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_MASREFTEST_PRESSED_NODE_NAME);
  if (smR2MasRefTestPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R1 MasterReferenceTest button pressed";
    smR2MasRefTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2MasterReferenceTestReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 MasterReferenceTest button pressed node
  QOpcUaNode* smR2MasRefTestPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_MASREFTEST_PRESSED_NODE_NAME);
  if (smR2MasRefTestPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R2 MasterReferenceTest button released";
    smR2MasRefTestPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2HomePressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 Home button pressed node
  QOpcUaNode* smR2HomePressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_HOME_PRESSED_NODE_NAME);
  if (smR2HomePressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R2 Home button pressed";
    smR2HomePressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2HomeReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R1 Load button pressed node
  QOpcUaNode* smR2HomePressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_HOME_PRESSED_NODE_NAME);
  if (smR2HomePressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R2 Home button released";
    smR2HomePressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos1Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition1 button pressed node
  QOpcUaNode* smR2ServicePos1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS1_PRESSED_NODE_NAME);
  if (smR2ServicePos1PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition1 button pressed";
    smR2ServicePos1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos1Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition1 button pressed node
  QOpcUaNode* smR2ServicePos1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS1_PRESSED_NODE_NAME);
  if (smR2ServicePos1PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition1 button released";
    smR2ServicePos1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos2Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition2 button pressed node
  QOpcUaNode* smR2ServicePos2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS2_PRESSED_NODE_NAME);
  if (smR2ServicePos2PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition2 button pressed";
    smR2ServicePos2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos2Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition2 button pressed node
  QOpcUaNode* smR2ServicePos2PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS2_PRESSED_NODE_NAME);
  if (smR2ServicePos2PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition2 button released";
    smR2ServicePos2PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos3Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition3 button pressed node
  QOpcUaNode* smR2ServicePos3PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS3_PRESSED_NODE_NAME);
  if (smR2ServicePos3PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition3 button pressed";
    smR2ServicePos3PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceR2ServicePos3Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service R2 ServicePosition3 button pressed node
  QOpcUaNode* smR2ServicePos3PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_R2_SERVICEPOS3_PRESSED_NODE_NAME);
  if (smR2ServicePos3PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": R2 ServicePosition3 button released";
    smR2ServicePos3PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceRestartSmPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service RestartServiceMode button pressed node
  QOpcUaNode* smRestartSmPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_RESTARTSM_PRESSED_NODE_NAME);
  if (smRestartSmPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": RestartServiceMode button pressed";
    smRestartSmPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onServiceRestartSmReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Service RestartServiceMode button pressed node
  QOpcUaNode* smRestartSmPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_SM_RESTARTSM_PRESSED_NODE_NAME);
  if (smRestartSmPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": RestartServiceMode button released";
    smRestartSmPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowT1Pressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowT1 button pressed node
  QOpcUaNode* kcmAllowT1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWT1_PRESSED_NODE_NAME);
  if (kcmAllowT1PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": AllowT1 button pressed";
    kcmAllowT1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowT1Released()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowT1 button pressed node
  QOpcUaNode* kcmAllowT1PressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWT1_PRESSED_NODE_NAME);
  if (kcmAllowT1PressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": AllowT1 button released";
    kcmAllowT1PressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowXrayPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowXRay button pressed node
  QOpcUaNode* kcmAllowXrayPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWXRAY_PRESSED_NODE_NAME);
  if (kcmAllowXrayPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": AllowXRay button pressed";
    kcmAllowXrayPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowXrayReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowXRay button pressed node
  QOpcUaNode* kcmAllowXrayPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWXRAY_PRESSED_NODE_NAME);
  if (kcmAllowXrayPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": AllowXRay button released";
    kcmAllowXrayPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}

void qSlicerSiemensPlcOpcUaWidget::onKukaAllowBeamPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowBeam button pressed node
  QOpcUaNode* kcmAllowBeamPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWBEAM_PRESSED_NODE_NAME);
  if (kcmAllowBeamPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": AllowBeam button pressed";
    kcmAllowBeamPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onKukaAllowBeamReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // KUKA Controllers AllowXRay button pressed node
  QOpcUaNode* kcmAllowBeamPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_KCM_ALLOWBEAM_PRESSED_NODE_NAME);
  if (kcmAllowBeamPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": AllowBeam button released";
    kcmAllowBeamPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}
*/
//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onResetErrorsPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // ResetErrors button pressed
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteResetErrorsPressed(true);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": ResetErrors button is pressed";
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onResetErrorsReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // ResetErrors button released
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteResetErrorsPressed(false);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": ResetErrors button is released";
  }
}
/*
//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onMakeXrayPressed()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // MakeXRay button pressed node
  QOpcUaNode* makeXrayPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_MAKEXRAY_PRESSED_NODE_NAME);
  if (makeXrayPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": MakeXRay pressed";
    makeXrayPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(true), QOpcUa::Types::Boolean);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onMakeXrayReleased()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // MakeXRay button pressed node
  QOpcUaNode* makeXrayPressedNode = d->FindNodeFromFullDisplayName(BUTTONS_MAKEXRAY_PRESSED_NODE_NAME);
  if (makeXrayPressedNode)
  {
//    qDebug() << Q_FUNC_INFO << ": MakeXRay released";
    makeXrayPressedNode->writeAttribute(QOpcUa::NodeAttribute::Value, QVariant(false), QOpcUa::Types::Boolean);
  }
}
*/

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetCoordXClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord from ASU X node
  int32_t x = static_cast< int32_t >(d->DoubleSpinBox_CoordX->value() * 1000.);
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteNewCoordX(x);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": SetCoordX button is pressed: " << x;
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetCoordYClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord from ASU Y node
  int32_t y = static_cast< int32_t >(d->DoubleSpinBox_CoordY->value() * 1000.);
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteNewCoordY(y);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": SetCoordY button is pressed: " << y;
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetCoordZClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord from ASU Z node
  int32_t z = static_cast< int32_t >(d->DoubleSpinBox_CoordZ->value() * 1000.);
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteNewCoordZ(z);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": SetCoordZ button is pressed: " << z;
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetAngleAClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord from ASU A node
  int32_t a = static_cast< int32_t >(d->DoubleSpinBox_AngleA->value() * 1000.);
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteNewCoordAngleA(a);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": SetAngleA button is pressed: " << a;
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetAngleBClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord from ASU B node
  int32_t b = static_cast< int32_t >(d->DoubleSpinBox_AngleB->value() * 1000.);
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteNewCoordAngleB(b);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": SetAngleB button is pressed: " << b;
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetAngleCClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord from ASU C node
  int32_t c = static_cast< int32_t >(d->DoubleSpinBox_AngleC->value() * 1000.);
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteNewCoordAngleC(c);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": SetAngleC button is pressed: " << c;
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetCoordsAndAnglesClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  // Coord X
  int32_t x = static_cast< int32_t >(d->DoubleSpinBox_CoordY->value() * 1000.);
  // Coord Y
  int32_t y = static_cast< int32_t >(d->DoubleSpinBox_CoordY->value() * 1000.);
  // Coord Z
  int32_t z = static_cast< int32_t >(d->DoubleSpinBox_CoordZ->value() * 1000.);
  // Angle A
  int32_t a = static_cast< int32_t >(d->DoubleSpinBox_AngleA->value() * 1000.);
  // Angle B
  int32_t b = static_cast< int32_t >(d->DoubleSpinBox_AngleB->value() * 1000.);
  // Angle C
  int32_t c = static_cast< int32_t >(d->DoubleSpinBox_AngleC->value() * 1000.);

  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteNewCoordY(x);
  res &= d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteNewCoordY(y);
  res &= d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteNewCoordZ(z);
  res &= d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteNewCoordAngleA(a);
  res &= d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteNewCoordAngleB(b);
  res &= d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteNewCoordAngleC(c);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": Set coords and angles button responce: " << res;
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onSetTabletopPositionClicked()
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  QRadioButton* rbutton = qobject_cast< QRadioButton* >(d->ButtonGroup_TabletopPosition->checkedButton());
  vtkMRMLSiemensPlcOpcUaNode::TabletopOrientation orient = vtkMRMLSiemensPlcOpcUaNode::UNKNOWN_ORIENTATION;
  if (rbutton == d->RadioButton_Position1)
  {
    orient = vtkMRMLSiemensPlcOpcUaNode::LEFT_RIGHT_ORIENTATION;
  }
  else if (rbutton == d->RadioButton_Position2)
  {
    orient = vtkMRMLSiemensPlcOpcUaNode::SUPERIOR_INFERIOR_ORIENTATION;
  }
  else if (rbutton == d->RadioButton_Position3)
  {
    orient = vtkMRMLSiemensPlcOpcUaNode::RIGHT_LEFT_ORIENTATION;
  }
  else if (rbutton == d->RadioButton_PositionNotDefined)
  {
    orient = vtkMRMLSiemensPlcOpcUaNode::UNKNOWN_ORIENTATION;
  }
  else
  {
    orient = vtkMRMLSiemensPlcOpcUaNode::UNKNOWN_ORIENTATION;
  }
  bool res = d->opcUaLogic->GetSiemensPlcOpcUaLogic()->WriteTabletopOrientation(orient);
  if (res)
  {
    qDebug() << Q_FUNC_INFO << ": Set new Tabletop Orientation:" << int(orient);
  }
}

//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::onTabletopPositionChanged(QAbstractButton* aButton)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  QRadioButton* rbutton = qobject_cast< QRadioButton* >(aButton);
  if (rbutton == d->RadioButton_PositionNotDefined)
  {
    qDebug() << Q_FUNC_INFO << "undef";
  }
  else if (rbutton == d->RadioButton_Position1)
  {
    qDebug() << Q_FUNC_INFO << "1";
  }
  else if (rbutton == d->RadioButton_Position2)
  {
    qDebug() << Q_FUNC_INFO << "2";
  }
  else if (rbutton == d->RadioButton_Position3)
  {
    qDebug() << Q_FUNC_INFO << "3";
  }
  else
  {
    qDebug() << Q_FUNC_INFO << "wrong";
  }
}


//-----------------------------------------------------------------------------
void qSlicerSiemensPlcOpcUaWidget::setOpcUaLogic(vtkSlicerU70RoomOpcUaLogic* logic)
{
  Q_D(qSlicerSiemensPlcOpcUaWidget);
  d->opcUaLogic = logic;
}
