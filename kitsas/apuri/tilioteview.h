/*
   Copyright (C) 2019 Arto Hyvättinen

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TILIOTEVIEW_H
#define TILIOTEVIEW_H

#include <QTableView>

class TilioteView : public QTableView
{
    Q_OBJECT
public:
    TilioteView(QWidget *parent);
    ~TilioteView();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint) override;
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

    void lisaaTyhjaRivi();

private:
    QModelIndex lastValidIndex_;
};

#endif // TILIOTEVIEW_H
