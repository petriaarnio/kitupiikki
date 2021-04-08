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
#include "finvoicetoimittaja.h"
#include "db/kirjanpito.h"
#include "rekisteri/asiakastoimittajadlg.h"
#include "pilvi/pilvikysely.h"
#include "maaritys/verkkolasku/verkkolaskumaaritys.h"
#include <QSettings>
#include "model/lasku.h"
#include "db/tositetyyppimodel.h"

FinvoiceToimittaja::FinvoiceToimittaja(QObject *parent) :
    AbstraktiToimittaja(parent)
{

}

void FinvoiceToimittaja::toimita()
{
    if( !kp()->asetukset()->luku("FinvoiceKaytossa") ) {
        virhe(tr("Verkkolaskutusta ei ole määritelty käyttöön kirjanpidon asetuksista"));
    } else if(!kp()->pilvi()->kayttajaPilvessa()) {
        virhe(tr("Verkkolaskujen toimittaminen edellyttää kirjautumista Kitsaan pilveen"));
    } else {
        if( init_.isEmpty())
            alustaInit();

        int kumppani = tositeMap().value("kumppani").toMap().value("id").toInt();
        if( kumppani ) {

        KpKysely *kysely = kpk(QString("/kumppanit/%1").arg(kumppani));
        connect( kysely, &KpKysely::vastaus, this, &FinvoiceToimittaja::kumppaniSaapuu);
        connect( kysely, &KpKysely::virhe, [this] {  this->virhe(tr("Asiakkaan tietojen noutaminen epäonnistui"));} );

        kysely->kysy();
        } else {
            virhe(tr("Verkkolaskulle ei ole määritelty asiakasta"));
        }
    }
}

void FinvoiceToimittaja::alustaInit()
{
    init_.insert("ovt", kp()->asetus("OvtTunnus"));
    init_.insert("operaattori", kp()->asetus("Operaattori"));
    init_.insert("nimi", kp()->asetus("Nimi"));
    init_.insert("alvtunnus", AsiakasToimittajaDlg::yToAlv( kp()->asetus("Ytunnus") ) );
    init_.insert("kotipaikka", kp()->asetus("Kotipaikka"));
    init_.insert("osoite", kp()->asetus("Katuosoite"));
    init_.insert("postinumero", kp()->asetus("Postinumero"));
    init_.insert("kaupunki", kp()->asetus("Kaupunki"));

    QVariantList tilit;
    for(const Iban iban : kp()->asetus("LaskuIbanit").split(',')) {
        QVariantMap tili;
        tili.insert("iban", iban.valeitta());
        tili.insert("bic", iban.bic());
        tilit.append(tili);
    }
    init_.insert("tilit", tilit);
}

void FinvoiceToimittaja::kumppaniSaapuu(QVariant *kumppani)
{
    QVariantMap asiakas = kumppani->toMap();

    QVariantMap pyynto;
    pyynto.insert("init", init_);
    pyynto.insert("asiakas", asiakas);

    Lasku lasku = tositeMap().value("lasku").toMap();
    int tyyppi = tositeMap().value("tyyppi").toInt();
    if( lasku.maksutapa() == Lasku::KATEINEN)
        lasku.set("tyyppi", "KUITTI");
    if( tyyppi == TositeTyyppi::HYVITYSLASKU)
        lasku.set("tyyppi", "HYVITYSLASKU");
    else if( tyyppi == TositeTyyppi::MAKSUMUISTUTUS)
        lasku.set("tyyppi", "MAKSUMUISTUTUS");
    else if( lasku.maksutapa() == Lasku::ENNAKKOLASKU)
        lasku.set("tyyppi", "ENNAKKOLASKU");
    else
        lasku.set("tyyppi", "LASKU");

    pyynto.insert("lasku", lasku.data() );
    pyynto.insert("rivit", tositeMap().value("rivit"));
    pyynto.insert("docid", tositeMap().value("id").toInt());

    if( kp()->asetukset()->luku("FinvoiceKaytossa") == VerkkolaskuMaaritys::PAIKALLINEN) {

        QString osoite = kp()->pilvi()->finvoiceOsoite() + "/create";
        PilviKysely *pk = new PilviKysely( kp()->pilvi(), KpKysely::POST,
                    osoite );
        connect( pk, &PilviKysely::vastaus, this, &FinvoiceToimittaja::laskuSaapuu);
        connect( pk, &KpKysely::virhe, [this] {  this->virhe(tr("Verkkolaskun muodostaminen epäonnistui"));} );

        pk->kysy(pyynto);

    } else if( kp()->asetukset()->luku("FinvoiceKaytossa") == VerkkolaskuMaaritys::MAVENTA) {
        QString osoite = kp()->pilvi()->finvoiceOsoite() + "/invoices/" + kp()->asetus("Ytunnus");

        PilviKysely *pk = new PilviKysely( kp()->pilvi(), KpKysely::POST,
                    osoite );
        connect( pk, &PilviKysely::vastaus, this, &FinvoiceToimittaja::maventaToimitettu);
        connect( pk, &KpKysely::virhe, [this] (int, QString selite) {  this->virhe(selite);} );

        pk->kysy(pyynto);
    }

}

void FinvoiceToimittaja::laskuSaapuu(QVariant *data)
{
    QByteArray lasku = data->toByteArray();

    QString hakemisto = kp()->settings()->value( QString("FinvoiceHakemisto/%1").arg(kp()->asetus("UID"))).toString();
    QString tnimi = QString("%1/lasku%2.xml")
            .arg(hakemisto)
            .arg(tositeMap().value("lasku").toMap().value("numero").toLongLong(),8,10,QChar('0'));
    QFile out(tnimi);
    if( !out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        virhe(tr("Laskutiedoston tallentaminen sijaintiin %1 epäonnistui").arg(hakemisto));
    } else {
        out.write(lasku);
        out.close();
    }
    merkkaaToimitetuksi();
}

void FinvoiceToimittaja::maventaToimitettu(QVariant *data)
{
    QVariantMap tulos = data->toMap();
    QString maventaId = tulos.value("id").toString();

    QVariantMap tosite(tositeMap());
    tosite.insert("maventaid", maventaId);
    tosite.insert("tila", Tosite::LAHETETTYLASKU);

    KpKysely *kysely = kpk(QString("/tositteet/%1").arg(tosite.value("id").toInt()), KpKysely::PUT);
    connect( kysely, &KpKysely::vastaus, this, &FinvoiceToimittaja::valmis);
    kysely->kysy(tosite);
}