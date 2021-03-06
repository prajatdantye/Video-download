/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PLUGINVIEW_H
#define PLUGINVIEW_H

#include <QWidget>

class PluginNavModel;
class ListView;
class QModelIndex;
class QVBoxLayout;

class PluginView : public QWidget
{
    Q_OBJECT
    
public:
    explicit PluginView(const QString &service, QWidget *parent = 0);
    
public Q_SLOTS:
    void search(const QString &query, const QString &type, const QString &order);
    void showResource(const QString &type, const QString &id);
    
private:
    void showCategories(const QString &name, const QString &id);
    void showPlaylists(const QString &name, const QString &id);
    void showSearchDialog();
    void showUsers(const QString &name, const QString &id);
    void showVideos(const QString &name, const QString &id);
    
private Q_SLOTS:
    void onItemActivated(const QModelIndex &index);
    
private:
    PluginNavModel *m_model;
    
    ListView *m_view;
    QVBoxLayout *m_layout;
};

#endif // PLUGINVIEW_H
