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

#ifndef __qSlicerScadaOpcUaWidget_h
#define __qSlicerScadaOpcUaWidget_h

// Qt includes
#include <QWidget>
#include <QSharedPointer>

// QtOpcUa includes
#include <QOpcUaNode>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// U70RoomOpcUaModule Widgets includes
#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

#include <vtkMRMLScadaOpcUaNode.h>

class qSlicerScadaOpcUaWidgetPrivate;
class QOpcUaClient;
class QAbstractButton;

class vtkMRMLNode;

class vtkSlicerU70RoomOpcUaLogic;
/*
class NodeReader : public QObject {
  Q_OBJECT
public:
  NodeReader(const QList< QOpcUaNode* >& nodes, QObject* parent = nullptr)
    : QObject(parent), m_nodes(nodes), m_total(nodes.size())
  {
    for (QOpcUaNode* node : nodes)
    {
      QObject::connect(node, &QOpcUaNode::attributeRead, this, [this, node](QOpcUa::NodeAttributes attrs)
      {
        Q_UNUSED(attrs);
        ++m_done;
        emit progress(m_done, m_total);
        if (m_done == m_total)
        {
          emit finished();
        }
      });
    }
  }
  void start()
  {
    for (QOpcUaNode* node : m_nodes)
    {
      if (node)
      {
        node->readAttributes(QOpcUa::NodeAttribute::Value);
      }
    }
  }
signals:
  void progress(int done, int total);
  void finished();
private:
  QList< QOpcUaNode* > m_nodes;
  int m_total{ 0 };
  int m_done{ 0 };
};
*/
class Q_SLICER_MODULE_U70ROOMOPCUA_WIDGETS_EXPORT qSlicerScadaOpcUaWidget
  : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
    
public:
  typedef QWidget Superclass;
  qSlicerScadaOpcUaWidget(QWidget *parent=0);
  ~qSlicerScadaOpcUaWidget() override;

signals:
  void nodesTreeLoadingFinished(void);

public slots:
  /// Set ScadaOpcUa MRML node (Parameter node)
  void setParameterNode(vtkMRMLNode* node);
  void setOpcUaLogic(vtkSlicerU70RoomOpcUaLogic* logic);
  /// Set ScadaOpcUa client
  void setOpcUaClient(const QSharedPointer< QOpcUaClient >& opcUaClient);
  /// Update widget GUI from ScadaOpcUa parameters node
  void updateWidgetFromMRML();
  
  void onParseServerParentNodeClicked();
  void onOpcUaClientConnected();
  void onOpcUaClientDisconnected();

  void onPositionModuleNodesRead(QOpcUa::NodeAttributes attr);

  void onReadNodesValuesClicked();
  void onScadaExpiredTimeout();

protected:
  QScopedPointer<qSlicerScadaOpcUaWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerScadaOpcUaWidget);
  Q_DISABLE_COPY(qSlicerScadaOpcUaWidget);
};

#endif
