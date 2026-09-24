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

#ifndef __qSlicerOpcUaRobotsMessagesListModel_h
#define __qSlicerOpcUaRobotsMessagesListModel_h

#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

// SlicerQt includes
#include "qSlicerObject.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// SlicerRT ScadaOpcUa MRML includes
#include <vtkMRMLScadaOpcUaNode.h>

// Qt
#include <QAbstractListModel>
#include <QIcon>
#include <QString>
#include <QVector>

// FooBar Widgets includes
#include "qSlicerU70RoomOpcUaModuleWidgetsExport.h"

class Q_SLICER_MODULE_U70ROOMOPCUA_WIDGETS_EXPORT qSlicerOpcUaRobotsMessagesListModel : public QAbstractListModel
{
  Q_OBJECT
public:
  static constexpr size_t MESSAGES_SIZE = 64;
  struct ListItem
  {
    QIcon icon;
    QString text;
  };

  explicit qSlicerOpcUaRobotsMessagesListModel(QObject *parent = nullptr) : QAbstractListModel(parent) {}

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;
  // Public API to fill/modify the model
  void addItem(const QIcon &icon, const QString &text);
  void updateErrorBits(quint64 errorMessages);
  void updateServiceBits(quint64 serviceMessages);
  void updateMiscBits(quint64 miscMessages);
  void updateMessagesBits(quint64 errorMessages, quint64 serviceMessages, quint64 miscMessages);
  void clear();

private:
  void updateItemsList();
  QVector< ListItem > messages;
  quint64 errorMessages;
  quint64 serviceMessages;
  quint64 miscMessages;
};

#endif
