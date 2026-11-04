/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "ScadaOpcUaTreeItem.h"
#include "ScadaOpcUaModel.h"

#include <QOpcUaClient>
#include <QOpcUaArgument>
#include <QOpcUaAxisInformation>
#include <QOpcUaComplexNumber>
#include <QOpcUaDoubleComplexNumber>
#include <QOpcUaEUInformation>
#include <QOpcUaExtensionObject>
#include <QOpcUaLocalizedText>
#include <QOpcUaQualifiedName>
#include <QOpcUaRange>
#include <QOpcUaXValue>
#include <QMetaEnum>
#include <QPixmap>

#include <vtkMRMLScadaOpcUaNode.h>

#include <bitset>

namespace {

constexpr int numberOfDisplayColumns = 7; // NodeId, Value, NodeClass, DataType, BrowseName, DisplayName, Description

}

ScadaOpcUaTreeItem::ScadaOpcUaTreeItem(ScadaOpcUaModel *model)
  :
    QObject(nullptr)
  , mModel(model)
{
}

ScadaOpcUaTreeItem::ScadaOpcUaTreeItem(QOpcUaNode *node, ScadaOpcUaModel *model, ScadaOpcUaTreeItem *parent) 
  :
    QObject(parent)
  , mOpcNode(node)
  , mModel(model)
  , mParentItem(parent)
{
  QObject::connect(mOpcNode.get(), &QOpcUaNode::attributeRead, this, &ScadaOpcUaTreeItem::handleAttributes);
  QObject::connect(mOpcNode.get(), &QOpcUaNode::browseFinished, this, &ScadaOpcUaTreeItem::browseFinished);

  if (!mOpcNode->readAttributes(QOpcUa::NodeAttribute::Value
    | QOpcUa::NodeAttribute::NodeClass
    | QOpcUa::NodeAttribute::Description
    | QOpcUa::NodeAttribute::DataType
    | QOpcUa::NodeAttribute::BrowseName
    | QOpcUa::NodeAttribute::DisplayName))
  {
    qWarning() << tr("Reading attributes") << mOpcNode->nodeId() << tr("failed");
  }
}

ScadaOpcUaTreeItem::ScadaOpcUaTreeItem(QOpcUaNode *node, ScadaOpcUaModel *model, const QOpcUaReferenceDescription &browsingData, ScadaOpcUaTreeItem *parent)
  :
  ScadaOpcUaTreeItem(node, model, parent)
{
  mNodeBrowseName = browsingData.browseName().name();
  mNodeClass = browsingData.nodeClass();
  mNodeId = browsingData.targetNodeId().nodeId();
  mNodeDisplayName = browsingData.displayName().text();
}

ScadaOpcUaTreeItem::~ScadaOpcUaTreeItem()
{
  qDeleteAll(mChildItems);
}

ScadaOpcUaTreeItem* ScadaOpcUaTreeItem::child(int row)
{
  if (row >= mChildItems.size())
  {
    qCritical() << tr("TreeItem in row") << row << tr("does not exist.");
  }
  return mChildItems[row];
}

int ScadaOpcUaTreeItem::childIndex(const ScadaOpcUaTreeItem *child) const
{
  return mChildItems.indexOf(const_cast<ScadaOpcUaTreeItem *>(child));
}

int ScadaOpcUaTreeItem::childCount()
{
  startBrowsing();
  return mChildItems.size();
}

int ScadaOpcUaTreeItem::columnCount() const
{
  return numberOfDisplayColumns;
}

QVariant ScadaOpcUaTreeItem::data(int column)
{
  if (column == 0)
  {
    return mNodeBrowseName;
  }
  if (column == 1)
  {
      if (!mAttributesReady)
      {
        return tr("Loading ...");
      }
      const auto type = mOpcNode->attribute(QOpcUa::NodeAttribute::DataType).toString();
      const auto value = mOpcNode->attribute(QOpcUa::NodeAttribute::Value);

      return variantToString(value, type);
  }
  if (column == 2)
  {
    QMetaEnum metaEnum = QMetaEnum::fromType<QOpcUa::NodeClass>();
    QString name = metaEnum.valueToKey(int(mNodeClass));
    return name + " (" + QString::number(int(mNodeClass)) + ')';
  }
  if (column == 3)
  {
    if (!mAttributesReady)
    {
      return tr("Loading ...");
    }
    const QString typeId = mOpcNode->attribute(QOpcUa::NodeAttribute::DataType).toString();
    auto enumEntry = QOpcUa::namespace0IdFromNodeId(typeId);
    if (enumEntry == QOpcUa::NodeIds::Namespace0::Unknown)
    {
      return typeId;
    }
    return QOpcUa::namespace0IdName(enumEntry) + " (" + typeId + ")";
  }
  if (column == 4)
  {
    return mNodeId;
  }
  if (column == 5)
  {
    return mNodeDisplayName;
  }
  if (column == 6)
  {
    return mAttributesReady ? \
      mOpcNode->attribute(QOpcUa::NodeAttribute::Description).value<QOpcUaLocalizedText>().text() : \
      tr("Loading ...");
  }
  return QVariant();
}

int ScadaOpcUaTreeItem::row() const
{
  if (!mParentItem)
  {
    return 0;
  }
  return mParentItem->childIndex(this);
}

ScadaOpcUaTreeItem *ScadaOpcUaTreeItem::parentItem()
{
  return mParentItem;
}

void ScadaOpcUaTreeItem::appendChild(ScadaOpcUaTreeItem *child)
{
  if (!child)
  {
    return;
  }
  if (!hasChildNodeItem(child->mNodeId))
  {
    mChildItems.append(child);
    mChildNodeIds.insert(child->mNodeId);
  }
  else
  {
    child->deleteLater();
  }
}

static QPixmap createPixmap(const QColor &c)
{
  QPixmap p(10,10);
  p.fill(c);
  return p;
}

QPixmap ScadaOpcUaTreeItem::icon(int column) const
{
  if (column != 0 || !mOpcNode)
  {
    return QPixmap();
  }

  static const QPixmap objectPixmap = createPixmap(Qt::darkGreen);
  static const QPixmap variablePixmap = createPixmap(Qt::darkBlue);
  static const QPixmap methodPixmap = createPixmap(Qt::darkRed);
  static const QPixmap defaultPixmap = createPixmap(Qt::gray);

  switch (mNodeClass)
  {
  case QOpcUa::NodeClass::Object:
    return objectPixmap;
  case QOpcUa::NodeClass::Variable:
    return variablePixmap;
  case QOpcUa::NodeClass::Method:
    return methodPixmap;
  default:
    break;
  }
  return defaultPixmap;
}

bool ScadaOpcUaTreeItem::hasChildNodeItem(const QString &nodeId) const
{
  return mChildNodeIds.contains(nodeId);
}

void ScadaOpcUaTreeItem::startBrowsing()
{
  if (mBrowseStarted)
  {
    return;
  }
  if (!mOpcNode->browseChildren())
  {
    qWarning() << tr("Browsing node") << mOpcNode->nodeId() << tr("failed");
  }
  else
  {
    mModel->browseStarted();
    mBrowseStarted = true;
  }
}

void ScadaOpcUaTreeItem::handleAttributes(QOpcUa::NodeAttributes attr)
{
  if (attr & QOpcUa::NodeAttribute::NodeClass)
  {
    mNodeClass = mOpcNode->attribute(QOpcUa::NodeAttribute::NodeClass).value<QOpcUa::NodeClass>();
  }
  if (attr & QOpcUa::NodeAttribute::BrowseName)
  {
    mNodeBrowseName = mOpcNode->attribute(QOpcUa::NodeAttribute::BrowseName).value<QOpcUaQualifiedName>().name();
  }
  if (attr & QOpcUa::NodeAttribute::DisplayName)
  {
    mNodeDisplayName = mOpcNode->attribute(QOpcUa::NodeAttribute::DisplayName).value<QOpcUaLocalizedText>().text();
  }
  if (attr & QOpcUa::NodeAttribute::Description)
  {
    mNodeDescription = mOpcNode->attribute(QOpcUa::NodeAttribute::Description).value<QOpcUaLocalizedText>().text();
  }
  mAttributesReady = true;
  emit mModel->dataChanged(mModel->createIndex(row(), 0, this), mModel->createIndex(row(), numberOfDisplayColumns - 1, this));
}

void ScadaOpcUaTreeItem::browseFinished(const QVector<QOpcUaReferenceDescription> &children, QOpcUa::UaStatusCode statusCode)
{
  struct GuaranteedFinish {
    GuaranteedFinish(ScadaOpcUaModel* model) : m(model) {}
    ~GuaranteedFinish() { m->browseFinished(); }
    ScadaOpcUaModel* m{ nullptr };
  } dummyGuard(mModel);

//  mModel->browseFinished();

  if (statusCode != QOpcUa::Good)
  {
    qWarning() << tr("Browsing node") << mOpcNode->nodeId() << tr("finally failed:") << statusCode;
    return;
  }

  auto index = mModel->createIndex(row(), 0, this);

  for (const auto &item : children)
  {
    if (hasChildNodeItem(item.targetNodeId().nodeId()))
    {
      continue;
    }
    auto node = mModel->opcUaClient()->node(item.targetNodeId());
    if (!node)
    {
      qWarning() << tr("Failed to instantiate node:") << item.targetNodeId().nodeId();
      continue;
    }

    mModel->beginInsertRows(index, mChildItems.size(), mChildItems.size() + 1);
    this->appendChild(new ScadaOpcUaTreeItem(node, mModel, item, this));
    mModel->endInsertRows();
  }

  emit mModel->dataChanged(mModel->createIndex(row(), 0, this), mModel->createIndex(row(), numberOfDisplayColumns - 1, this));
}

QString ScadaOpcUaTreeItem::variantToString(const QVariant &value, const QString &typeNodeId) const
{
  if (value.type() == QVariant::List)
  {
    QString concat;
    const auto list = value.toList();
    for (int i = 0, size = list.size(); i < size; ++i)
    {
      if (i)
      {
        concat.append(QLatin1Char('\n'));
      }
      concat.append(variantToString(list.at(i), typeNodeId));
    }
    return concat;
  }

  if (typeNodeId == QLatin1String("ns=0;i=19"))
  { // StatusCode
    const char *name = QMetaEnum::fromType<QOpcUa::UaStatusCode>().valueToKey(value.toInt());
    return name ? QLatin1String(name) : QLatin1String("Unknown StatusCode");
  }
  if (typeNodeId == QLatin1String("ns=0;i=2"))
  { // Char
    return QString::number(value.toInt());
  }
  if (typeNodeId == QLatin1String("ns=0;i=3"))
  { // SChar
    return QString::number(value.toUInt());
  }
  if (typeNodeId == QLatin1String("ns=0;i=4"))
  { // Int16
    return QString::number(value.toInt());
  }
  if (typeNodeId == QLatin1String("ns=0;i=5"))
  {  // UInt16
    return QString::number(value.toUInt());
  }
  if (value.type() == QVariant::ByteArray)
  {
    return QLatin1String("0x") + value.toByteArray().toHex();
  }
  if (value.type() == QVariant::DateTime)
  {
    return value.toDateTime().toString(Qt::ISODate);
  }
  if (value.canConvert<QOpcUaQualifiedName>())
  {
    const auto name = value.value<QOpcUaQualifiedName>();
    return QStringLiteral("[NamespaceIndex: %1, Name: \"%2\"]").arg(name.namespaceIndex()).arg(name.name());
  }
  if (value.canConvert<QOpcUaLocalizedText>())
  {
    const auto text = value.value<QOpcUaLocalizedText>();
    return localizedTextToString(text);
  }
  if (value.canConvert<QOpcUaRange>())
  {
    const auto range = value.value<QOpcUaRange>();
    return rangeToString(range);
  }
  if (value.canConvert<QOpcUaComplexNumber>())
  {
    const auto complex = value.value<QOpcUaComplexNumber>();
    return QStringLiteral("[Real: %1, Imaginary: %2]").arg(complex.real()).arg(complex.imaginary());
  }
  if (value.canConvert<QOpcUaDoubleComplexNumber>())
  {
    const auto complex = value.value<QOpcUaDoubleComplexNumber>();
    return QStringLiteral("[Real: %1, Imaginary: %2]").arg(complex.real()).arg(complex.imaginary());
  }
  if (value.canConvert<QOpcUaXValue>())
  {
    const auto xv = value.value<QOpcUaXValue>();
    return QStringLiteral("[X: %1, Value: %2]").arg(xv.x()).arg(xv.value());
  }
  if (value.canConvert<QOpcUaEUInformation>())
  {
    const auto info = value.value<QOpcUaEUInformation>();
    return euInformationToString(info);
  }
  if (value.canConvert<QOpcUaAxisInformation>())
  {
    const auto info = value.value<QOpcUaAxisInformation>();
    return QStringLiteral("[EUInformation: %1, EURange: %2, Title: %3 , AxisScaleType: %4, AxisSteps: %5]").arg(
      euInformationToString(info.engineeringUnits())).arg(rangeToString(info.eURange())).arg(localizedTextToString(info.title())).arg(
      info.axisScaleType() == QOpcUa::AxisScale::Linear ? "Linear" : (info.axisScaleType() == QOpcUa::AxisScale::Ln) ? "Ln" : "Log").arg(
      numberArrayToString(info.axisSteps()));
  }
  if (value.canConvert<QOpcUaExpandedNodeId>())
  {
    const auto id = value.value<QOpcUaExpandedNodeId>();
    return QStringLiteral("[NodeId: \"%1\", ServerIndex: \"%2\", NamespaceUri: \"%3\"]").arg(
      id.nodeId()).arg(id.serverIndex()).arg(id.namespaceUri());
  }
  if (value.canConvert<QOpcUaArgument>())
  {
    const auto a = value.value<QOpcUaArgument>();
    return QStringLiteral("[Name: \"%1\", DataType: \"%2\", ValueRank: \"%3\", ArrayDimensions: %4, Description: %5]").arg(
      a.name(), a.dataTypeId()).arg(a.valueRank()).arg(numberArrayToString(a.arrayDimensions()),
      localizedTextToString(a.description()));
  }
  if (value.canConvert<QOpcUaExtensionObject>())
  {
    const auto obj = value.value<QOpcUaExtensionObject>();
    return QStringLiteral("[TypeId: \"%1\", Encoding: %2, Body: 0x%3]").arg(obj.encodingTypeId(),
      obj.encoding() == QOpcUaExtensionObject::Encoding::NoBody ?
      "NoBody" : (obj.encoding() == QOpcUaExtensionObject::Encoding::ByteString ?
      "ByteString" : "XML")).arg(obj.encodedBody().isEmpty() ? "0" : QString(obj.encodedBody().toHex()));
  }

  if (value.canConvert<QString>())
  {
    return value.toString();
  }
  return QString();
}

QString ScadaOpcUaTreeItem::localizedTextToString(const QOpcUaLocalizedText &text) const
{
  return QStringLiteral("[Locale: \"%1\", Text: \"%2\"]").arg(text.locale()).arg(text.text());
}

QString ScadaOpcUaTreeItem::rangeToString(const QOpcUaRange &range) const
{
  return QStringLiteral("[Low: %1, High: %2]").arg(range.low()).arg(range.high());
}

QString ScadaOpcUaTreeItem::euInformationToString(const QOpcUaEUInformation &info) const
{
  return QStringLiteral("[UnitId: %1, NamespaceUri: \"%2\", DisplayName: %3, Description: %4]").arg(info.unitId()).arg(
    info.namespaceUri()).arg(localizedTextToString(info.displayName())).arg(localizedTextToString(info.description()));
}

ScadaOpcUaTreeItem* ScadaOpcUaTreeItem::findParentItemByName(ScadaOpcUaTreeItem* parentItem, const QString& displayedName)
{
  if (!parentItem)
  {
    return nullptr;
  }
  if (parentItem->getNodeDisplayName() == displayedName)
  {
    return parentItem;
  }
  for (int i = 0; i < parentItem->childCount(); ++i)
  {
    ScadaOpcUaTreeItem* item = ScadaOpcUaTreeItem::findParentItemByName(parentItem->child(i), displayedName);
    if (item)
    {
      return item;
    }
  }
  return nullptr;
}

QList< QPointer< ScadaOpcUaTreeItem > > ScadaOpcUaTreeItem::findLeafValueItems(ScadaOpcUaTreeItem* item)
{
  QList< QPointer< ScadaOpcUaTreeItem > > finalLeaves;
  if (!item)
  {
    return finalLeaves;
  }

  // Since messages are list of flags, we get the item of the messages, rather than
  // all the children (items of the list ) items of the parent message item
  // Since messages are list of flags, we get the item of the messages, rather than
  // all the children (items of the list ) items of the parent message item
  bool nodesFlag = \
    (item->getNodeDisplayName() == QString(SCADA_SYSTIME_NODE)) || \
    (item->getNodeDisplayName() == QString(SCADA_EVENT_STRING_NODE)) || \
    (item->getNodeDisplayName() == QString(SCADA_ERROR_STRING_NODE));

  if (!item->currentChildCount() || nodesFlag)
  {
//    if (item->getNodeClass() == QOpcUa::NodeClass::Variable)
//    {
    finalLeaves.append(QPointer(item));
//    }
  }
  else
  {
    for (int i = 0; i < item->currentChildCount(); ++i)
    {
      finalLeaves += ScadaOpcUaTreeItem::findLeafValueItems(item->child(i));
    }
  }
  return finalLeaves;
}

QStringList ScadaOpcUaTreeItem::findParentNamesForItem(ScadaOpcUaTreeItem* currentItem, ScadaOpcUaTreeItem* rootItem)
{
  QStringList parentNames;
  if (!currentItem || !rootItem)
  {
    return parentNames;
  }
  if (currentItem == rootItem)
  {
    parentNames.push_back(rootItem->getNodeDisplayName());
    return parentNames;
  }
  if (ScadaOpcUaTreeItem* parent = currentItem->parentItem())
  {
    parentNames.push_back(currentItem->getNodeDisplayName());
    parentNames += ScadaOpcUaTreeItem::findParentNamesForItem(parent, rootItem);
  }
  return parentNames;
}
