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
#include "alvlaskelma.h"
#include "db/kirjanpito.h"
#include "db/verotyyppimodel.h"
#include "model/tosite.h"
#include "model/tositeviennit.h"
#include "db/tositetyyppimodel.h"
#include "model/tositeliitteet.h"
#include "alvsivu.h"

#include <QDebug>

AlvLaskelma::AlvLaskelma(QObject *parent) :
  Raportteri (parent),
  tosite_( new Tosite(this))
{

}

AlvLaskelma::~AlvLaskelma()
{
}

void AlvLaskelma::kirjoitaLaskelma()
{
    kirjoitaOtsikot();
    kirjoitaYhteenveto();
    kirjoitaErittely();
}

void AlvLaskelma::kirjoitaOtsikot()
{
    rk.asetaOtsikko( tr("ARVONLISÄVEROLASKELMA"));
    rk.asetaKausiteksti( QString("%1 - %2").arg(alkupvm_.toString("dd.MM.yyyy")).arg(loppupvm_.toString("dd.MM.yyyy") ) );

    rk.lisaaPvmSarake();
    rk.lisaaSarake("TOSITE12345");
    rk.lisaaVenyvaSarake();
    rk.lisaaVenyvaSarake();
    rk.lisaaSarake("24,00 ");
    rk.lisaaEurosarake();

    RaporttiRivi otsikko;
    otsikko.lisaa("Pvm");
    otsikko.lisaa("Tosite");
    otsikko.lisaa("Asiakas/Toimittaja");
    otsikko.lisaa("Selite");
    otsikko.lisaa("%",1,true);
    otsikko.lisaa("€",1,true);
    rk.lisaaOtsake(otsikko);
}

void AlvLaskelma::kirjoitaYhteenveto()
{

    RaporttiRivi otsikko;
    otsikko.lisaa(tr("Arvonlisäveroilmoituksen tiedot"),5);
    otsikko.lihavoi();
    otsikko.asetaKoko(14);

    rk.lisaaRivi(otsikko);
    rk.lisaaTyhjaRivi();

    if( kp()->onkoMaksuperusteinenAlv(loppupvm_)) {
        RaporttiRivi rivi;
        rivi.lisaa(tr("Maksuperusteinen arvonlisävero"),5);
        rk.lisaaRivi(rivi);
        rk.lisaaTyhjaRivi();
    }

    if( suhteutuskuukaudet_ ) {
        if( liikevaihto_ < 1000000)
            huojennus_ = verohuojennukseen_;
        else {
            huojennus_ = qRound64(verohuojennukseen_ - ( (liikevaihto_ - 1000000) * verohuojennukseen_ ) / 2000000.0);
        }
        if( huojennus_ > verohuojennukseen_)
            huojennus_ = verohuojennukseen_;
        if( huojennus_ < 0)
            huojennus_ = 0;
    }

    // Kotimaan myynti
    yvRivi(301, tr("Suoritettava 24%:n vero kotimaan myynnistä"), kotimaanmyyntivero(2400) );
    yvRivi(302, tr("Suoritettava 14%:n vero kotimaan myynnistä"), kotimaanmyyntivero(1400));
    yvRivi(303, tr("Suoritettava 10%:n vero kotimaan myynnistä"), kotimaanmyyntivero(1000));

    rk.lisaaTyhjaRivi();

    yvRivi(304, tr("Vero tavaroiden maahantuonnista EU:n ulkopuolelta"), taulu_.summa( AlvKoodi::MAAHANTUONTI + AlvKoodi::ALVKIRJAUS ));
    yvRivi(305, tr("Vero tavaraostoista muista EU-maista"),taulu_.summa(AlvKoodi::YHTEISOHANKINNAT_TAVARAT + AlvKoodi::ALVKIRJAUS));
    yvRivi(306,tr("Vero palveluostoista muista EU-maista"),taulu_.summa(AlvKoodi::YHTEISOHANKINNAT_PALVELUT + AlvKoodi::ALVKIRJAUS));

    rk.lisaaTyhjaRivi();

    yvRivi(307, tr("Verokauden vähennettävä vero"), taulu_.summa(200,299));
    maksettava_ = taulu_.summa(100,199) - taulu_.summa(200,299) - huojennus();
    yvRivi(308, tr("Maksettava vero / Palautukseen oikeuttava vero"), maksettava_);

    rk.lisaaTyhjaRivi();

    yvRivi(309, tr("0-verokannan alainen liikevaihto"), taulu_.summa(AlvKoodi::ALV0));
    yvRivi(310, tr("Tavaroiden maahantuonnit EU:n ulkopuolelta"), taulu_.summa(AlvKoodi::MAAHANTUONTI));
    yvRivi(311, tr("Tavaroiden myynnit muihin EU-maihin"), taulu_.summa(AlvKoodi::YHTEISOMYYNTI_TAVARAT));
    yvRivi(312, tr("Palveluiden myynnit muihin EU-maihin"), taulu_.summa(AlvKoodi::YHTEISOMYYNTI_PALVELUT));
    yvRivi(313, tr("Tavaraostot muista EU-maista"), taulu_.summa(AlvKoodi::YHTEISOHANKINNAT_TAVARAT));
    yvRivi(314, tr("Palveluostot muista EU-maista"), taulu_.summa(AlvKoodi::YHTEISOHANKINNAT_PALVELUT));

    yvRivi(318, tr("Vero rakentamispalveluiden ja metalliromun ostoista"), taulu_.summa(AlvKoodi::RAKENNUSPALVELU_OSTO + AlvKoodi::ALVKIRJAUS));
    yvRivi(319, tr("Rakentamispalveluiden ja metaalliromun myynnit"), taulu_.summa(AlvKoodi::RAKENNUSPALVELU_MYYNTI));
    yvRivi(320, tr("Rakentamispalveluiden ja metalliromun ostot"), taulu_.summa(AlvKoodi::RAKENNUSPALVELU_OSTO));

    yvRivi(315, tr("Alarajahuojennukseen oikeuttava liikevaihto"), liikevaihto_ );
    yvRivi(316, tr("Alarajahuojennukseen oikeuttava vero"), verohuojennukseen_);
    yvRivi(317, tr("Alarajahuojennuksen määrä"), huojennus());

    rk.lisaaTyhjaRivi();

}

void AlvLaskelma::kirjaaVerot()
{
    qlonglong vero = taulu_.summa(100,199);
    qlonglong vahennys = taulu_.summa(200,299);

    QString selite = tr("Arvonlisävero %1 - %2")
            .arg( alkupvm_.toString("dd.MM.yyyy") )
            .arg( loppupvm_.toString("dd.MM.yyyy"));

    if( vero ) {
        TositeVienti verot;
        verot.setSelite(selite);
        verot.setTili( kp()->tilit()->tiliTyypilla(TiliLaji::ALVVELKA).numero() );
        verot.setDebet( vero / 100.0 );
        verot.setAlvKoodi( AlvKoodi::TILITYS );

        lisaaKirjausVienti(verot);
    }
    if( vahennys )
    {
        TositeVienti vahennysRivi;
        vahennysRivi.setSelite(selite);
        vahennysRivi.setTili( kp()->tilit()->tiliTyypilla(TiliLaji::ALVSAATAVA).numero() );
        vahennysRivi.setKredit( vahennys / 100.0  );
        vahennysRivi.setAlvKoodi( AlvKoodi::TILITYS);

        lisaaKirjausVienti( vahennysRivi );
    }
    if( vero != vahennys) {
        TositeVienti maksu;
        maksu.setSelite( selite );
        maksu.setTili( kp()->tilit()->tiliTyypilla(TiliLaji::VEROVELKA).numero() );
        if( vahennys > vero && kp()->asetukset()->onko("AlvPalautusSaatavatilille"))
            maksu.setTili( kp()->tilit()->tiliTyypilla(TiliLaji::VEROSAATAVA).numero() );

        maksu.setAlvKoodi( AlvKoodi::TILITYS );
        if( vero > vahennys )
            maksu.setKredit(  vero - vahennys );
        else
            maksu.setDebet(  vahennys - vero );

        lisaaKirjausVienti( maksu );
    }
}

void AlvLaskelma::kirjoitaErittely()
{
    RaporttiRivi otsikko;
    otsikko.lisaa(tr("Erittely"),5);
    otsikko.lihavoi();
    otsikko.asetaKoko(14);

    rk.lisaaRivi(otsikko);
    rk.lisaaTyhjaRivi();


    QMapIterator<int, KoodiTaulu> koodiIter(taulu_.koodit);
    while( koodiIter.hasNext()) {
        koodiIter.next();
        int koodi = koodiIter.key();

        const KoodiTaulu &taulu = koodiIter.value();
        QMapIterator<int, KantaTaulu> kantaIter(taulu.kannat);
        while( kantaIter.hasNext()) {
            kantaIter.next();
            double verokanta = kantaIter.key() / 100.0;

            RaporttiRivi kantaOtsikko;
            kantaOtsikko.lisaa( kp()->alvTyypit()->yhdistelmaSeliteKoodilla(koodi), 4 );
            kantaOtsikko.lisaa( QString("%L1").arg(verokanta,0,'f',0));
            kantaOtsikko.lisaa( kantaIter.value().summa(debetistaKoodilla(koodi)) );
            kantaOtsikko.lihavoi();
            rk.lisaaRivi(kantaOtsikko);

            QMapIterator<int,TiliTaulu> tiliIter( kantaIter.value().tilit );
            while( tiliIter.hasNext()) {
                tiliIter.next();

                RaporttiRivi tiliOtsikko;
                tiliOtsikko.lisaa( kp()->tilit()->tiliNumerolla( tiliIter.key() ).nimiNumero(), 4 );
                rk.lisaaRivi(tiliOtsikko);

                for(auto vienti : tiliIter.value().viennit) {
                    RaporttiRivi rivi;
                    rivi.lisaa( vienti.value("pvm").toDate() );
                    QVariantMap tositeMap = vienti.value("tosite").toMap();
                    rivi.lisaa( kp()->tositeTunnus(tositeMap.value("tunniste").toInt(),
                                tositeMap.value("pvm").toDate(),
                                tositeMap.value("sarja").toString(),
                                true));

                    QString selite = vienti.value("selite").toString();
                    QString kumppani = vienti.value("kumppani").toMap().value("nimi").toString();

                    rivi.lisaa(kumppani);
                    rivi.lisaa(selite == kumppani ? "" : selite);

                    rivi.lisaa(  QString("%L1").arg(verokanta,0,'f',0) );

                    qlonglong debetsnt = qRound64(vienti.value("debet").toDouble() * 100);
                    qlonglong kreditsnt = qRound64( vienti.value("kredit").toDouble() * 100);

                    if( debetistaKoodilla( koodi ) )
                        rivi.lisaa( debetsnt - kreditsnt );
                    else
                        rivi.lisaa( kreditsnt - debetsnt );

                    rk.lisaaRivi( rivi );
                }
                // Tilin summa
                RaporttiRivi tiliSumma;
                tiliSumma.lisaa(QString(), 4);
                tiliSumma.lisaa(  QString("%L1").arg(verokanta,0,'f',0) );
                tiliSumma.lisaa( tiliIter.value().summa( debetistaKoodilla(koodi) ) );
                tiliSumma.viivaYlle();
                rk.lisaaRivi(tiliSumma);
                rk.lisaaTyhjaRivi();
            }
        }
    }
    // Marginaalierittely
    for( RaporttiRivi rivi : marginaaliRivit_)
        rk.lisaaRivi(rivi);
}

void AlvLaskelma::yvRivi(int koodi, const QString &selite, qlonglong sentit)
{
    if( sentit ) {
        RaporttiRivi rivi;
        rivi.lisaa(QString(), 1);
        rivi.lisaa( QString::number(koodi));
        rivi.lisaa(selite, 3);
        rivi.lisaa( sentit);
        rk.lisaaRivi(rivi);
    }
    if( koodi && sentit)
        koodattu_.insert(koodi, sentit);
}

qlonglong AlvLaskelma::kotimaanmyyntivero(int prosentinsadasosa)
{
    return taulu_.koodit.value(AlvKoodi::MYYNNIT_NETTO + AlvKoodi::ALVKIRJAUS).kannat.value(prosentinsadasosa).summa() +
            taulu_.koodit.value(AlvKoodi::MYYNNIT_BRUTTO + AlvKoodi::ALVKIRJAUS).kannat.value(prosentinsadasosa).summa() +
            taulu_.koodit.value(AlvKoodi::MAKSUPERUSTEINEN_MYYNTI + AlvKoodi::ALVKIRJAUS).kannat.value(prosentinsadasosa).summa() +
            taulu_.koodit.value(AlvKoodi::MYYNNIT_MARGINAALI + AlvKoodi::ALVKIRJAUS).kannat.value(prosentinsadasosa).summa() +
            taulu_.koodit.value(AlvKoodi::ENNAKKOLASKU_MYYNTI + AlvKoodi::ALVKIRJAUS).kannat.value(prosentinsadasosa).summa();
}



void AlvLaskelma::tilaaMaksuperusteisenTosite()
{
    if( !nollattavatErat_.isEmpty()) {
        // Ajan perusteella nollattavaa
        QPair<int,qlonglong> nollattava = nollattavatErat_.takeFirst();
        KpKysely *kysely = kpk("/tositteet");
        kysely->lisaaAttribuutti("vienti", nollattava.first);
        connect( kysely, &KpKysely::vastaus, [this, nollattava] (QVariant* var) { this->maksuperusteTositesaapuu(var, nollattava.second);});
        kysely->kysy();
        nollatutErat_.insert(nollattava.first);
    } else {
        int tosite = 0;
        int era = 0;
        qlonglong saldo = 0;

        while (!maksuperusteTositteet_.isEmpty()) {
            int tosite = maksuperusteTositteet_.takeLast();
            int era = maksuperusteiset_.value(tosite).first;
            qlonglong saldo = maksuperusteiset_.value(tosite).second;
            if( !nollatutErat_.contains(era) && qAbs(saldo) > 1e-5)
                break;
        }
        if( !tosite)
            viimeistele();
        else {

            KpKysely *kysely = kpk("/tositteet");
            kysely->lisaaAttribuutti("vienti",era);

            connect( kysely, &KpKysely::vastaus, [this, saldo] (QVariant* var) {this->maksuperusteTositesaapuu(var, saldo);});
            kysely->kysy();
        }
    }
}

void AlvLaskelma::kasitteleMaksuperusteinen(const QVariantMap &map)
{
    // Myyntisaatavat ja ostosaatavat, jotka eivät ole
    // aloita uutta tase-erää, saattavat olla maksuja

    Tili* tili = kp()->tilit()->tili(map.value("tili").toInt());
    if( tili ) {
        int tositeId = map.value("tosite").toMap().value("id").toInt();
        qlonglong debet = qRound64( map.value("debet").toDouble() * 100.0 );
        qlonglong kredit = qRound64( map.value("kredit").toDouble() * 100.0 );
        int era = map.value("era").toMap().value("id").toInt();

        if( tili->onko(TiliLaji::MYYNTISAATAVA) || tili->onko(TiliLaji::OSTOVELKA)) {
            maksuperusteiset_.insert( tositeId, qMakePair(era, debet - kredit));
        } else if( tili->onko(TiliLaji::RAHAVARAT)) {
            if( !maksuperusteTositteet_.contains(tositeId))
                maksuperusteTositteet_.append(tositeId);
        }
    }
}

void AlvLaskelma::maksuperusteTositesaapuu(QVariant *variant, qlonglong sentit)
{
    QVariantMap map = variant->toMap();
    QVariantList viennit = map.value("viennit").toList();
    TositeVienti vasta = viennit.value(0).toMap();
    qlonglong vastasentit = qRound64( vasta.kredit() * 100.0 ) - qRound64( vasta.debet() * 100.0);

    for(QVariant item : viennit) {
        TositeVienti vienti = item.toMap();
        if( qAbs(vienti.era().value("saldo").toDouble()) < 1e-5) {
            // Jos vero on jo maksettu, ei makseta uudelleen...
        } else if( vienti.alvKoodi() == AlvKoodi::MAKSUPERUSTEINEN_KOHDENTAMATON + AlvKoodi::MAKSUPERUSTEINEN_MYYNTI) {
            TositeVienti kohdentamattomasta;
            kohdentamattomasta.setTili(vienti.tili());
            double eurot = qRound64(vienti.kredit() * 100 * sentit / vastasentit) / 100.0;
            kohdentamattomasta.setDebet( eurot );
            kohdentamattomasta.setEra(vienti.era());
            kohdentamattomasta.setAlvProsentti(vienti.alvProsentti());
            kohdentamattomasta.setSelite("Maksuperusteinen alv " + vienti.pvm().toString("dd.MM.yyyy") + " " + vienti.selite() );
            kohdentamattomasta.setAlvKoodi(AlvKoodi::MAKSUPERUSTEINEN_KOHDENTAMATON + AlvKoodi::MAKSUPERUSTEINEN_MYYNTI);
            lisaaKirjausVienti(kohdentamattomasta);

            TositeVienti kohdennettuun;
            kohdennettuun.setTili(kp()->tilit()->tiliTyypilla(TiliLaji::ALVVELKA).numero());
            kohdennettuun.setKredit(eurot);
            kohdennettuun.setSelite("Maksuperusteinen alv " + vienti.pvm().toString("dd.MM.yyyy") + " " + vienti.selite() );
            kohdennettuun.setAlvProsentti(vienti.alvProsentti());
            kohdennettuun.setAlvKoodi(AlvKoodi::MAKSUPERUSTEINEN_MYYNTI + AlvKoodi::ALVKIRJAUS);
            lisaaKirjausVienti(kohdennettuun);
        } else if( vienti.alvKoodi() == AlvKoodi::MAKSUPERUSTEINEN_KOHDENTAMATON + AlvKoodi::MAKSUPERUSTEINEN_OSTO) {
            TositeVienti kohdentamattomasta;
            kohdentamattomasta.setTili(vienti.tili());
            double eurot = qRound64(vienti.debet() * 100 * sentit / vastasentit) / 100.0;
            kohdentamattomasta.setKredit( eurot );
            kohdentamattomasta.setEra(vienti.era());
            kohdentamattomasta.setAlvProsentti(vienti.alvProsentti());
            kohdentamattomasta.setSelite("Maksuperusteinen alv " + vienti.pvm().toString("dd.MM.yyyy") + " " + vienti.selite() );
            kohdentamattomasta.setAlvKoodi(AlvKoodi::MAKSUPERUSTEINEN_KOHDENTAMATON + AlvKoodi::MAKSUPERUSTEINEN_OSTO);
            lisaaKirjausVienti(kohdentamattomasta);

            TositeVienti kohdennettuun;
            kohdennettuun.setTili(kp()->tilit()->tiliTyypilla(TiliLaji::ALVSAATAVA).numero());
            kohdennettuun.setDebet(eurot);
            kohdennettuun.setSelite("Maksuperusteinen alv " + vienti.pvm().toString("dd.MM.yyyy") + " " + vienti.selite() );
            kohdennettuun.setAlvProsentti(vienti.alvProsentti());
            kohdennettuun.setAlvKoodi(AlvKoodi::MAKSUPERUSTEINEN_OSTO + AlvKoodi::ALVVAHENNYS);
            lisaaKirjausVienti(kohdennettuun);
        }

    }
    tilaaMaksuperusteisenTosite();
}

void AlvLaskelma::tilaaNollausLista(const QDate &pvm)
{
    KpKysely *kysely = kpk("/erat");
    kysely->lisaaAttribuutti("tili", kp()->tilit()->tiliTyypilla(TiliLaji::KOHDENTAMATONALVVELKA).numero());
    connect( kysely, &KpKysely::vastaus, [this,pvm] (QVariant *data) { this->nollaaMaksuperusteisetErat(data, pvm);} );
    kysely->kysy();

    kysely = kpk("/erat");
    kysely->lisaaAttribuutti("tili", kp()->tilit()->tiliTyypilla(TiliLaji::KOHDENTAMATONALVSAATAVA).numero());
    connect( kysely, &KpKysely::vastaus, [this,pvm] (QVariant *data) { this->nollaaMaksuperusteisetErat(data, pvm);} );
    kysely->kysy();
}

void AlvLaskelma::nollaaMaksuperusteisetErat(QVariant *variant, const QDate& pvm)
{
    QVariantList list = variant->toList();
    for( auto item : list) {
        QVariantMap map = item.toMap();
        if( map.value("pvm").toDate() > pvm)
            continue;
        int eraid = map.value("id").toInt();
        qlonglong saldo = qRound64(map.value("avoin").toDouble() * 100.0);
        nollattavatErat_.append(qMakePair(eraid, saldo));
    }
    nollattavatHaut_--;
    if( !nollattavatHaut_)
        hae();
}

void AlvLaskelma::laskeMarginaaliVerotus(int kanta)
{
    KoodiTaulu taulu = taulu_.koodit.value(AlvKoodi::MYYNNIT_MARGINAALI);
    KantaTaulu ktaulu = taulu.kannat.value(kanta);
    qlonglong myynti = ktaulu.summa();

    KoodiTaulu ostotaulu = taulu_.koodit.value(AlvKoodi::OSTOT_MARGINAALI);
    KantaTaulu oktaulu = ostotaulu.kannat.value(kanta);
    qlonglong ostot = oktaulu.summa(true);
    qlonglong alijaama = kp()->alvIlmoitukset()->marginaalialijaama(alkupvm_.addDays(-1), kanta);

    qlonglong marginaali = myynti - ostot - alijaama;      // TODO: Alijäämän lisäys
    qlonglong vero = qRound64(marginaali * 1.00 * kanta / (10000 + kanta));

    if( myynti || ostot ) {
        marginaaliRivit_.append(RaporttiRivi());
        marginaaliRivi(tr("Voittomarginaalimenettely myynti"),kanta,myynti);
        marginaaliRivi(tr("Voittomarginaalimenettely ostot"), kanta, ostot);
        if( alijaama )
            marginaaliRivi(tr("Aiempi alijäämä"), kanta, alijaama);
        if( marginaali > 0 || kp()->asetukset()->onko("AlvMatkatoimisto")) {
            marginaaliRivi(tr("Marginaali"), kanta, marginaali);
            marginaaliRivi(tr("Vero"),kanta,vero);
        } else if( marginaali < 0) {
            marginaaliRivi(tr("Alijäämä"), kanta, 0-marginaali);
        }
    }

    if( vero > 0 || (vero < 0 && kp()->asetukset()->onko("AlvMatkatoimisto"))) {
        // Marginaalivero kirjataan kaikille marginaalitileille
        QMapIterator<int,TiliTaulu> kirjausIter(ktaulu.tilit);
        while( kirjausIter.hasNext()) {
            kirjausIter.next();
            int tili = kirjausIter.key();
            qlonglong tilinmyynti = kirjausIter.value().summa();

            QString selite = tr("Voittomarginaalivero (Verokanta %1 %, osuus %2 %)")
                    .arg(kanta / 100.0, 0, 'f', 2)
                    .arg(tilinmyynti * 100.0 / myynti, 0, 'f', 2);
            TositeVienti tililta;
            tililta.setTili(tili);
            double eurot = qRound64(tilinmyynti * 1.0 / myynti * vero )/ 100.0;
            if( eurot > 0)
                tililta.setDebet( eurot );
            else
                tililta.setKredit( 0-eurot);

            tililta.setAlvProsentti(kanta / 100.0);
            tililta.setSelite(selite);
            tililta.setAlvKoodi(AlvKoodi::MYYNNIT_MARGINAALI + AlvKoodi::TILITYS);
            tililta.setTyyppi(TositeVienti::BRUTTOOIKAISU);
            lisaaKirjausVienti(tililta);

            TositeVienti tilille;
            tilille.setTili(kp()->tilit()->tiliTyypilla(TiliLaji::ALVVELKA).numero());
            if( eurot > 0)
                tilille.setKredit( eurot );
            else
                tilille.setDebet( 0-eurot);
            tilille.setAlvProsentti( kanta / 100.0);
            tilille.setSelite( selite );
            tilille.setAlvKoodi(AlvKoodi::MYYNNIT_MARGINAALI + AlvKoodi::ALVKIRJAUS);
            lisaaKirjausVienti(tilille);
        }

    } else if( marginaali < 0) {
        // Marginaaliveron alijäämä laitetaan muistiin
        marginaaliAlijaamat_.insert( QString::number(kanta / 100,'f',2), (0 - marginaali) / 100.0);
    }

}

void AlvLaskelma::marginaaliRivi(const QString selite, int kanta, qlonglong summa)
{
    RaporttiRivi rr;
    rr.lisaa("",2);
    rr.lisaa(selite);
    rr.lisaa(QString("%L1").arg(kanta / 100.0,0,'f',0) );
    rr.lisaa(summa);
    marginaaliRivit_.append(rr);

}

void AlvLaskelma::hae()
{
    // Jos maksuperusteinen nollaus, tehdään se ensin
    // Jos maksuperusteinen käytössä, ovat erääntyvät vuosi sitten tehdyt erät
    if( kp()->onkoMaksuperusteinenAlv(loppupvm_) && nollattavatHaut_)
        tilaaNollausLista(alkupvm_.addYears(-1));
    else if( kp()->asetukset()->pvm("MaksuAlvLoppuu") == alkupvm_ && nollattavatHaut_)
        // Kun maksuperusteinen alv päätetään, erääntyvät kaikki erät
        tilaaNollausLista( alkupvm_);
    else {

        KpKysely* kysely = kpk("/viennit");
        kysely->lisaaAttribuutti("alkupvm", alkupvm_);
        kysely->lisaaAttribuutti("loppupvm", loppupvm_);
        connect( kysely, &KpKysely::vastaus, this, &AlvLaskelma::viennitSaapuu);
        kysely->kysy();
    }
}

void AlvLaskelma::laske(const QDate &alkupvm, const QDate &loppupvm)
{
    alkupvm_ = alkupvm;
    loppupvm_ = loppupvm;
    hae();
}

void AlvLaskelma::tallennaViennit(const QVariantList &viennit, bool maksuperusteinen)
{
    taulu_.koodit.clear();
    QVariantList lista = viennit;
    for(auto item : lista) {
        QVariantMap map = item.toMap();
        if( map.value("alvkoodi").toInt() && map.value("tosite").toMap().value("tyyppi").toInt() != TositeTyyppi::ALVLASKELMA )
            taulu_.lisaa(map);
        if( maksuperusteinen )
            kasitteleMaksuperusteinen(map);
    }
}


void AlvLaskelma::viennitSaapuu(QVariant *viennit)
{
    bool maksuperusteinen = kp()->onkoMaksuperusteinenAlv(loppupvm_);
    tallennaViennit( viennit->toList(), maksuperusteinen);
    // Jos maksuperuste, käsitellään ko. tositteet
    tilaaMaksuperusteisenTosite();
}

void AlvLaskelma::haeHuojennusJosTarpeen()
{
    QDate huojennusalku;
    QDate huojennusloppu;


    if( loppupvm_ == kp()->tilikaudet()->tilikausiPaivalle(loppupvm_).paattyy() && alkupvm_.daysTo(loppupvm_) < 32 ) {
        huojennusalku = kp()->tilikaudet()->tilikausiPaivalle(loppupvm_).alkaa();
        huojennusloppu = loppupvm_;
    } else if( loppupvm_.month() == 12 && loppupvm_.day() == 31 && alkupvm_.daysTo(loppupvm_) > 31) {
        // Jos alv-kausi muu kuin kuukausi, lasketaan verovuoden mukaisesti
        huojennusloppu = loppupvm_;
        huojennusalku = QDate(loppupvm_.year(),1,1);
        QDate alvalkaa = kp()->asetukset()->pvm("AlvAlkaa");
        if( alvalkaa.isValid() && alvalkaa > huojennusalku)
            huojennusalku = alvalkaa;
    }

    if( huojennusalku.isValid()) {
        if( huojennusalku.day() == 1)
            suhteutuskuukaudet_ = 1;
        else
            suhteutuskuukaudet_ = 0;
        for( QDate pvm = huojennusalku.addMonths(1); pvm < loppupvm_; pvm = pvm.addMonths(1))
            suhteutuskuukaudet_++;

        if( loppupvm_.addDays(1).day() != 1)
            suhteutuskuukaudet_--;

        // Sitten tehdään huojennushaku
        KpKysely* kysely = kpk("/viennit");
        kysely->lisaaAttribuutti("alkupvm", huojennusalku);
        kysely->lisaaAttribuutti("loppupvm", huojennusloppu);
        connect( kysely, &KpKysely::vastaus, this, &AlvLaskelma::laskeHuojennus);
        kysely->kysy();

    } else {
        viimeViimeistely();
    }
}

void AlvLaskelma::laskeHuojennus(QVariant *viennit)
{
    liikevaihto_ = 0;  

    qlonglong veroon = 0;
    qlonglong vahennys = 0;

    QVariantList lista;
    for(QVariant var : viennit->toList()) {
        TositeVienti vienti = var.toMap();
        if( vienti.value("tosite").toMap().value("tyyppi").toInt() != TositeTyyppi::ALVLASKELMA || vienti.value("pvm").toDate() <= alkupvm_)
            lista.append(vienti);
    }

    lista.append(tosite_->viennit()->vientilLista());

    qDebug() << " =======HUOJENNUSLASKELMA==================";

    for( QVariant var : lista ) {
        TositeVienti vienti = var.toMap();

        if( vienti.tyyppi() == TositeVienti::BRUTTOOIKAISU)
            continue;

        int alvkoodi = vienti.alvKoodi();
        qlonglong debet = qRound64( vienti.debet() * 100);
        qlonglong kredit = qRound64( vienti.kredit() * 100);


        qDebug() << vienti.pvm().toString("dd.MM.yyyy ") << " D " << vienti.debet() << " K "  << vienti.kredit() << " ALV " << vienti.alvKoodi() << " " << vienti.selite();

        if( alvkoodi > 0 && alvkoodi < 100) {
            // Tämä on veron tai vähennyksen peruste
            if( alvkoodi == AlvKoodi::MYYNNIT_NETTO ||
                    alvkoodi == AlvKoodi::YHTEISOMYYNTI_TAVARAT ||
                    alvkoodi == AlvKoodi::ALV0 ||
                    alvkoodi == AlvKoodi::RAKENNUSPALVELU_MYYNTI ) {
                liikevaihto_ += kredit - debet;
                qDebug() << " A Liikevaihtoon " << kredit-debet;
            } else if( alvkoodi == AlvKoodi::MYYNNIT_BRUTTO) {
                qlonglong brutto = kredit - debet;
                qlonglong netto = qRound64( ( 100 * brutto / (100 + vienti.alvProsentti()) )) ;
                liikevaihto_ += netto;
                veroon += brutto - netto;
                qDebug() << " B Liikevaihtoon " << netto << " Veroon " << brutto-netto;
            } else if( alvkoodi == AlvKoodi::OSTOT_BRUTTO) {
                qlonglong brutto = debet - kredit;
                qlonglong netto = qRound64( ( 100 * brutto / (100 + vienti.alvProsentti()) )) ;
                vahennys += brutto - netto;
                qDebug() << " C Vähennys " << brutto-netto;
            } else if( alvkoodi == AlvKoodi::MYYNNIT_MARGINAALI) {
                liikevaihto_ += kredit - debet;
                qDebug() << " D Liikevaihtoon " << kredit-debet;
            }
        } else if( alvkoodi > 100 && alvkoodi < 200 && vienti.alvProsentti() > 1e-5) {
            // Tämä on maksettava vero
            if( alvkoodi == AlvKoodi::MYYNNIT_NETTO + AlvKoodi::ALVKIRJAUS ||
                alvkoodi == AlvKoodi::MYYNNIT_BRUTTO + AlvKoodi::ALVKIRJAUS ) {
                veroon += kredit - debet;
                qDebug() << " E Veroon " << kredit-debet;
            } else if( alvkoodi == AlvKoodi::MYYNNIT_MARGINAALI + AlvKoodi::ALVKIRJAUS) {
                qlonglong vero = kredit - debet;
                veroon += vero;
                liikevaihto_ -= vero;
                qDebug() << "F Liikevaihtoon " << 0 - vero << " Veroon " << vero;
            } else if( alvkoodi == AlvKoodi::MAKSUPERUSTEINEN_MYYNTI + AlvKoodi::ALVKIRJAUS ||
                       alvkoodi == AlvKoodi::ENNAKKOLASKU_MYYNTI + AlvKoodi::ALVKIRJAUS ) {
                // Käytettyjen tavaroiden sekä taide-, keräily- ja antiikkiesineiden marginaaliverojärjestelmää
                // ja matkatoimistopalvelujen marginaaliverojärjestelmää sovellettaessa liikevaihtoon
                // luetaan ostajalta veloitettu myyntihinta vähennettynä myynnistä suoritetun
                // arvonlisäveron osuudella.

                // Maksuperusteisessa alvissa veron peruste voi olla kirjattu toiselle verokaudelle, joten
                // se joudutaan laskemaan samoin verokirjauksesta

                qlonglong vero = kredit - debet;
                double veroprossa = vienti.alvProsentti();
                qlonglong liikevaihtoon = qRound64( 100 * vero /  veroprossa);
                veroon += vero;
                liikevaihto_ += liikevaihtoon;
                qDebug() << " G Liikevaihtoon " << liikevaihtoon << " Veroon " << vero;
            }
        } else if( alvkoodi > 200 && alvkoodi < 300) {
            // Kaikki ostojen alv-vähennykset lasketaan huojennukseen
            vahennys += debet - kredit;
            qDebug() << " H Vähennykseen " << debet-kredit;
        }
    }

    qDebug() << " ================================= ";
    qDebug() << " Oikeuttava Liikevaihto " << liikevaihto_;

    qDebug() << " Huomioitavat verot " << veroon;
    qDebug() << " Huomioitavat vähennykset " << vahennys;
    verohuojennukseen_ = veroon - vahennys;

    qDebug() << " Oikeuttava vero " << verohuojennukseen_;

    liikevaihto_ = liikevaihto_ * 12 / suhteutuskuukaudet_;

    qDebug() << " Suhteutettu "<< liikevaihto_;

    viimeViimeistely();

}

void AlvLaskelma::tallennusValmis()
{
    kp()->alvIlmoitukset()->lataa();
    emit tallennettu();
}

void AlvLaskelma::viimeistele()
{

    oikaiseBruttoKirjaukset();
    laskeMarginaaliVerotus(2400);
    laskeMarginaaliVerotus(1400);
    laskeMarginaaliVerotus(1000);
    haeHuojennusJosTarpeen();
}

void AlvLaskelma::viimeViimeistely()
{
    kirjoitaLaskelma();
    kirjaaVerot();
    emit valmis( rk );
}

void AlvLaskelma::kirjaaHuojennus()
{
    if( !huojennus() || !kp()->asetukset()->luku("AlvHuojennusTili"))
        return;

    QString selite = tr("Arvonlisäveron alarajahuojennus");

    TositeVienti huojennettava;
    huojennettava.setTili( kp()->tilit()->tiliTyypilla(TiliLaji::VEROVELKA).numero() );
    huojennettava.setSelite( selite );
    huojennettava.setDebet( huojennus() );
    lisaaKirjausVienti( huojennettava );

    TositeVienti huojentaja;
    huojentaja.setTili( kp()->asetukset()->luku("AlvHuojennusTili") );
    huojentaja.setSelite( selite );
    huojentaja.setKredit( huojennus() );
    lisaaKirjausVienti( huojentaja );
}

void AlvLaskelma::tallenna()
{
    tosite_->setData( Tosite::PVM, loppupvm_ );
    tosite_->setData( Tosite::OTSIKKO, tr("Arvonlisäveroilmoitus %1 - %2")
                     .arg(alkupvm_.toString("dd.MM.yyyy"))
                     .arg(loppupvm_.toString("dd.MM.yyyy")));
    tosite_->setData( Tosite::TYYPPI, TositeTyyppi::ALVLASKELMA  );

    tosite_->liitteet()->lisaa( rk.pdf(), "alv.pdf", "alv" );

    QVariantMap lisat;
    QVariantMap koodit;
    QMapIterator<int,qlonglong> iter(koodattu_);
    while( iter.hasNext()) {
        iter.next();
        koodit.insert( QString::number( iter.key() ), iter.value() / 100.0 );
    }
    if( kp()->onkoMaksuperusteinenAlv(loppupvm_))
        koodit.insert("337",1);

    lisat.insert("koodit", koodit);
    lisat.insert("kausialkaa", alkupvm_);
    lisat.insert("kausipaattyy", loppupvm_);
    lisat.insert("erapvm", AlvIlmoitustenModel::erapaiva(loppupvm_));
    lisat.insert("maksettava", maksettava() / 100.0);    
    if( !marginaaliAlijaamat_.isEmpty() )
        lisat.insert("marginaalialijaama", marginaaliAlijaamat_);
    tosite_->setData( Tosite::ALV, lisat);


    connect( tosite_, &Tosite::talletettu, this, &AlvLaskelma::tallennusValmis);
    tosite_->tallenna();
}

void AlvLaskelma::lisaaKirjausVienti(TositeVienti vienti)
{
    vienti.setPvm( loppupvm_ );
    taulu_.lisaa(vienti);

    tosite_->viennit()->lisaa(vienti);
}

void AlvLaskelma::oikaiseBruttoKirjaukset()
{
    QMapIterator<int,KantaTaulu> myyntiIter( taulu_.koodit.value( AlvKoodi::MYYNNIT_BRUTTO ).kannat );
    while( myyntiIter.hasNext())
    {
        myyntiIter.next();
        QMapIterator<int,TiliTaulu> tiliIter( myyntiIter.value().tilit );
        while( tiliIter.hasNext())
        {
            tiliIter.next();
            int tili = tiliIter.key();
            qlonglong brutto = tiliIter.value().summa();
            int sadasosaprosentti = myyntiIter.key();

            qlonglong netto = qRound64( brutto * 10000.0 / ( 10000.0 + sadasosaprosentti) );
            qlonglong vero = brutto - netto;

            QString selite = tr("Bruttomyyntien oikaisu %3 BRUTTO %L1, NETTO %L2")
                    .arg(brutto / 100.0, 0, 'f', 2 )
                    .arg(netto/100.0, 0, 'f', 2)
                    .arg( kp()->tilit()->tiliNumerolla(tili).nimiNumero() );

            TositeVienti pois;
            pois.setTili(tili);
            pois.setDebet( vero / 100.0 );
            pois.setAlvKoodi( AlvKoodi::MYYNNIT_BRUTTO );
            pois.setAlvProsentti( sadasosaprosentti / 100.0 );
            pois.setTyyppi(TositeVienti::BRUTTOOIKAISU);
            pois.setSelite(selite);
            lisaaKirjausVienti( pois );

            TositeVienti veroon;
            veroon.setTili( kp()->tilit()->tiliTyypilla( TiliLaji::ALVVELKA ).numero() );
            veroon.setKredit( vero / 100.0);
            veroon.setAlvKoodi( AlvKoodi::ALVKIRJAUS + AlvKoodi::MYYNNIT_BRUTTO);
            veroon.setTyyppi( TositeVienti::BRUTTOOIKAISU );
            veroon.setAlvProsentti( sadasosaprosentti / 100.0);
            veroon.setSelite(selite);
            lisaaKirjausVienti( veroon );
        }
    }

    QMapIterator<int,KantaTaulu> ostoIter( taulu_.koodit.value( AlvKoodi::OSTOT_BRUTTO ).kannat );
    while( ostoIter.hasNext())
    {
        ostoIter.next();
        QMapIterator<int,TiliTaulu> tiliIter( ostoIter.value().tilit );
        while( tiliIter.hasNext())
        {
            tiliIter.next();
            int tili = tiliIter.key();
            qlonglong brutto = tiliIter.value().summa(true);
            int sadasosaprosentti = ostoIter.key();

            qlonglong netto = qRound64(brutto * 10000.0 / ( 10000 + sadasosaprosentti));
            qlonglong vero = brutto - netto;

            QString selite = tr("Brutto-ostojen oikaisu %3 BRUTTO %L1, NETTO %L2")
                    .arg(brutto / 100.0, 0, 'f', 2 )
                    .arg(netto/100.0, 0, 'f', 2)
                    .arg( kp()->tilit()->tiliNumerolla(tili).nimiNumero() );

            TositeVienti pois;
            pois.setTili(tili);
            pois.setKredit( vero / 100.0 );
            pois.setAlvKoodi( AlvKoodi::OSTOT_BRUTTO );
            pois.setAlvProsentti( sadasosaprosentti / 100.0 );
            pois.setSelite(selite);
            pois.setTyyppi( TositeVienti::BRUTTOOIKAISU );
            lisaaKirjausVienti( pois );

            TositeVienti veroon;
            veroon.setTili( kp()->tilit()->tiliTyypilla( TiliLaji::ALVSAATAVA ).numero() );
            veroon.setDebet( vero / 100.0);
            veroon.setAlvKoodi( AlvKoodi::ALVVAHENNYS + AlvKoodi::OSTOT_BRUTTO);
            veroon.setAlvProsentti( sadasosaprosentti / 100.0);
            veroon.setSelite(selite);
            veroon.setTyyppi( TositeVienti::BRUTTOOIKAISU );
            lisaaKirjausVienti( veroon );
        }
    }
}

bool AlvLaskelma::debetistaKoodilla(int alvkoodi)
{
    return  (( alvkoodi / 100 == 0 || alvkoodi / 100 == 4 ) && alvkoodi % 20 / 10 == 0  ) ||  ( alvkoodi / 100 == 2 ) ;
}

void AlvLaskelma::AlvTaulu::lisaa(const QVariantMap &rivi)
{
    int koodi = rivi.value("alvkoodi").toInt();
    if( !koodit.contains(koodi))
        koodit.insert(koodi, KoodiTaulu());
    koodit[koodi].lisaa(rivi);
}

qlonglong AlvLaskelma::AlvTaulu::summa(int koodista, int koodiin)
{
    if( !koodiin)
        koodiin = koodista;

    qlonglong s = 0;
    QMapIterator<int,KoodiTaulu> iter(koodit);
    while( iter.hasNext()) {
        iter.next();
        if( iter.key() >= koodista && iter.key() <= koodiin )
            s += iter.value().summa( debetistaKoodilla( iter.key() ) );
    }
    return s;
}

void AlvLaskelma::KoodiTaulu::lisaa(const QVariantMap &rivi)
{
    int kanta = qRound( rivi.value("alvprosentti").toDouble() * 100 );
    if( !kannat.contains(kanta))
        kannat.insert(kanta, KantaTaulu());
    kannat[kanta].lisaa(rivi);
}

qlonglong AlvLaskelma::KoodiTaulu::summa(bool debetista) const
{
    qlonglong s = 0;
    QMapIterator<int,KantaTaulu> iter(kannat);
    while( iter.hasNext()) {
        iter.next();
        s += iter.value().summa(debetista);
    }
    return s;
}

void AlvLaskelma::KantaTaulu::lisaa(const QVariantMap &rivi)
{
    int tili = rivi.value("tili").toInt();
    if( !tilit.contains(tili))
        tilit.insert(tili, TiliTaulu());
    tilit[tili].lisaa(rivi);
}

qlonglong AlvLaskelma::KantaTaulu::summa(bool debetista) const
{
    qlonglong s = 0;
    QMapIterator<int,TiliTaulu> iter(tilit);
    while( iter.hasNext()) {
        iter.next();
        s += iter.value().summa(debetista);
    }
    return s;
}

void AlvLaskelma::TiliTaulu::lisaa(const QVariantMap &rivi)
{
    viennit.append(rivi);
}

qlonglong AlvLaskelma::TiliTaulu::summa(bool debetista) const
{
    qlonglong s = 0;
    for( auto vienti : viennit ) {
        if( debetista ) {
            s += qRound64( vienti.value("debet").toDouble() * 100.0 );
            s -= qRound64( vienti.value("kredit").toDouble() * 100.0);
        } else {
            s -= qRound64( vienti.value("debet").toDouble() * 100.0 );
            s += qRound64( vienti.value("kredit").toDouble() * 100.0);
        }
    }
    return s;
}
