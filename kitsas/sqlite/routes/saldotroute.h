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
#ifndef SALDOTROUTE_H
#define SALDOTROUTE_H

#include "../sqliteroute.h"

class SaldotRoute : public SQLiteRoute
{
public:
    SaldotRoute(SQLiteModel* model);
    QVariant get(const QString &polku, const QUrlQuery &urlquery = QUrlQuery()) override;

protected:
    QVariant kustannuspaikat(const QDate& mista, const QDate& mihin, bool projektit = false, int kuuluu = -1);
};

#endif // SALDOTROUTE_H
