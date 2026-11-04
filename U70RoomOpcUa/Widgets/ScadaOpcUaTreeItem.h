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

#ifndef __ScadaOpcUaTreeItem_h
#define __ScadaOpcUaTreeItem_h

// U70RoomOpcUaModule Widgets includes
#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

#include <QObject>
#include <QOpcUaNode>
#include <QScopedPointer>
#include <QPointer>

class ScadaOpcUaModel;
class QOpcUaRange;
class QOpcUaEUInformation;

class Q_SLICER_MODULE_U70ROOMOPCUA_WIDGETS_EXPORT ScadaOpcUaTreeItem : public QObject
{
  Q_OBJECT
public:
  static constexpr const char* SCADA_POSITION_MODULE_NODE_ID = "ns=1;s=Модуль позиционирования";
  static constexpr const char* SCADA_SYSTIME_NODE = "SysTimeScada";
  static constexpr const char* SCADA_EVENT_STRING_NODE = "EventStr";
  static constexpr const char* SCADA_ERROR_STRING_NODE = "ErrStr";
  
  explicit ScadaOpcUaTreeItem(ScadaOpcUaModel *model);
  ScadaOpcUaTreeItem(QOpcUaNode *node, ScadaOpcUaModel *model, ScadaOpcUaTreeItem *parent);
  ScadaOpcUaTreeItem(QOpcUaNode *node, ScadaOpcUaModel *model, const QOpcUaReferenceDescription &browsingData, ScadaOpcUaTreeItem *parent);
  ~ScadaOpcUaTreeItem();
  ScadaOpcUaTreeItem *child(int row);

  int childIndex(const ScadaOpcUaTreeItem *child) const;
  int childCount();
  int columnCount() const;
  QVariant data(int column);
  int row() const;
  ScadaOpcUaTreeItem *parentItem();
  void appendChild(ScadaOpcUaTreeItem *child);
  QPixmap icon(int column) const;
  bool hasChildNodeItem(const QString &nodeId) const;
  QString getNodeBrowseName() const { return mNodeBrowseName; }
  QString getNodeId() const { return mNodeId; }
  QString getNodeDisplayName() const { return mNodeDisplayName; }
  QString getNodeDescription() const { return mNodeDescription; }
  QOpcUa::NodeClass getNodeClass() const { return mNodeClass; }
  const QList< QPointer< ScadaOpcUaTreeItem > >& getChildrenItems() const { return this->mChildItems; }
  const QSet< QString >& getChildrenNodesId() const { return this->mChildNodeIds; }
  int currentChildCount() const { return mChildItems.size(); }

  static ScadaOpcUaTreeItem* findParentItemByName(ScadaOpcUaTreeItem* parentItem, const QString& displayedName);
  static QMap< QString, QString > findAllChildrenForItem(ScadaOpcUaTreeItem* parentItem);
  static QList< QPointer< ScadaOpcUaTreeItem > > findLeafValueItems(ScadaOpcUaTreeItem* rootItem);
  static QStringList findAllItemsWithoutChildren(ScadaOpcUaTreeItem* parentItem);
  static QStringList findParentNamesForItem(ScadaOpcUaTreeItem* currentItem, ScadaOpcUaTreeItem* parentItem);

private slots:
  void startBrowsing();
  void handleAttributes(QOpcUa::NodeAttributes attr);
  void browseFinished(const QVector<QOpcUaReferenceDescription> &children, QOpcUa::UaStatusCode statusCode);

private:
  QString variantToString(const QVariant &value, const QString &typeNodeId = QString()) const;
  QString localizedTextToString(const QOpcUaLocalizedText &text) const;
  QString rangeToString(const QOpcUaRange &range) const;
  QString euInformationToString(const QOpcUaEUInformation &info) const;
  template <typename T> QString numberArrayToString(const QVector<T> &vec) const;

  QScopedPointer< QOpcUaNode > mOpcNode;
  QPointer< ScadaOpcUaModel > mModel;
  bool mAttributesReady = false;
  bool mBrowseStarted = false;
  QList< QPointer< ScadaOpcUaTreeItem > > mChildItems;
  QSet<QString> mChildNodeIds;
  QPointer< ScadaOpcUaTreeItem > mParentItem;

private:
  QString mNodeBrowseName;
  QString mNodeId;
  QString mNodeDisplayName;
  QString mNodeDescription;
  QOpcUa::NodeClass mNodeClass = QOpcUa::NodeClass::Undefined;
};

template <typename T>
QString ScadaOpcUaTreeItem::numberArrayToString(const QVector<T> &vec) const
{
  QString list(QLatin1Char('['));
  for (int i = 0, size = vec.size(); i < size; ++i)
  {
    if (i)
    {
      list.append(QLatin1Char(';'));
    }
    list.append(QString::number(vec.at(i)));
  }
  list.append(QLatin1Char(']'));
  return list;
}

#endif
