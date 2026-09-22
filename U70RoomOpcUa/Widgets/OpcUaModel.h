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

#ifndef __OpcUaModel_h
#define __OpcUaModel_h

#include "OpcUaTreeItem.h"

#include <QAbstractItemModel>
#include <QOpcUaNode>
#include <QScopedPointer>
#include <QPointer>

class QOpcUaClient;
class OpcUaTreeItem;

// U70RoomOpcUaModule Widgets includes
#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

class Q_SLICER_MODULE_U70ROOMOPCUA_WIDGETS_EXPORT OpcUaModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  OpcUaModel(QObject *parent = nullptr);

  void setOpcUaClient(QOpcUaClient *);
  QOpcUaClient* opcUaClient() const;

  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  OpcUaTreeItem* getRootItem() const { return this->mRootItem.data(); }

private:
  QPointer< QOpcUaClient > mOpcUaClient;
  QScopedPointer< OpcUaTreeItem > mRootItem;

  friend class OpcUaTreeItem;
};

#endif
