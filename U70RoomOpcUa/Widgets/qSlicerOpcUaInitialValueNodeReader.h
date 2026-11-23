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

#ifndef __qSlicerOpcUaInitialValueNodeReader_h
#define __qSlicerOpcUaInitialValueNodeReader_h

// Qt includes
#include <QObject>

// QtOpcUa includes
#include <QOpcUaNode>

// U70RoomOpcUaModule Widgets includes
#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

class qSlicerScadaOpcUaWidgetPrivate;

class Q_SLICER_MODULE_U70ROOMOPCUA_WIDGETS_EXPORT qSlicerOpcUaInitialValueNodeReader : public QObject {
  Q_OBJECT
public:
  qSlicerOpcUaInitialValueNodeReader(const QList< QOpcUaNode* >& nodes, QObject* parent = nullptr)
    : QObject(parent), m_nodes(nodes), m_total(nodes.size())
  {
    for (QOpcUaNode* node : nodes)
    {
      QObject::connect(node, &QOpcUaNode::attributeRead, this, [this](QOpcUa::NodeAttributes)
      {
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

#endif
