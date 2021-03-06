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
#ifndef INITROUTE_H
#define INITROUTE_H

#include "../sqliteroute.h"

class InitRoute : public SQLiteRoute
{
public:
    InitRoute(SQLiteModel *model);

    QVariant get(const QString &polku, const QUrlQuery& urlquery = QUrlQuery()) override;

    QVariant patch(const QString &polku, const QVariant &data) override;

protected:
    void paivitaAsetukset(const QVariantMap& map);
    void paivitaTilit(const QVariantList &list);
};

#endif // INITROUTE_H
