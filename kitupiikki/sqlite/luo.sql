CREATE TABLE Asetus
(
	avain varchar(128) PRIMARY KEY NOT NULL,
	arvo text,
	muokattu timestamp
);

CREATE TABLE Tili
(
	numero integer PRIMARY KEY NOT NULL,
	tyyppi varchar(10) NOT NULL,
	json text
);

CREATE TABLE Otsikko
(
	numero integer NOT NULL,
	taso integer NOT NULL,
	json text,
	PRIMARY KEY (numero,taso)
);

CREATE TABLE Tilikausi
(
	alkaa date PRIMARY KEY NOT NULL,
	loppuu date UNIQUE NOT NULL,
	json text
);

CREATE TABLE Kohdennus
(
	id serial PRIMARY KEY NOT NULL,
	tyyppi kohdennustyyppi NOT NULL,
	json text
);

INSERT INTO Kohdennus (id, tyyppi, json ) VALUES
( 0, 'oletus', '{"nimi":{"fi":"Yleinen","se":"Allmän", "en":"General"}}' );

CREATE TABLE Budjetti
(
	tilikausi DATE REFERENCES Tilikausi(alkaa),
	kohdennus INTEGER REFERENCES Kohdennus(id) ON DELETE CASCADE,
	tili INTEGER REFERENCES Tili(numero) ON DELETE CASCADE,
	euro NUMERIC (10,2),
	PRIMARY KEY (tilikausi, kohdennus, tili)
);

CREATE TABLE Kumppani
(
	id SERIAL PRIMARY KEY NOT NULL,
	nimi VARCHAR(255),
	alvtunnus VARCHAR(20),
	ryhmat integer[],
	json text
);

CREATE INDEX kumppani_nimi_index ON Kumppani(nimi);

CREATE TABLE KumppaniIban
(
	iban VARCHAR(30) PRIMARY KEY NOT NULL,
	kumppani INTEGER REFERENCES Kumppani(id) ON DELETE CASCADE
);

CREATE TABLE Tosite
(
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	pvm date,
	tyyppi integer,
	tila integer DEFAULT 100,
	tunniste integer,
	sarja VARCHAR(10),
	otsikko TEXT,
	kumppani integer REFERENCES Kumppani(id),
	json text
);

CREATE INDEX tosite_pvm ON Tosite (pvm);
CREATE INDEX tosite_tyyppi ON Tosite (tyyppi);
CREATE INDEX tosite_tila ON Tosite (tila);

CREATE TABLE Tositeloki
(
	id integer PRIMARY KEY AUTOINCREMENT NOT NULL,
	tosite integer REFERENCES Tosite(id),
	aika timestamp DEFAULT current_timestamp,
	data jsonb,
	userid integer,
	tila integer
);

CREATE INDEX tositeloki_tosite ON Tositeloki (tosite);

CREATE TABLE Vienti
(
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	rivi integer NOT NULL,
	tosite integer REFERENCES Tosite(id) ON DELETE CASCADE,
	tyyppi integer DEFAULT (0),
	pvm date,
	tili integer REFERENCES Tili(numero) ON DELETE RESTRICT,
	kohdennus integer DEFAULT(0) REFERENCES Kohdennus(id) ON DELETE RESTRICT,
	merkkaukset integer[],
	selite text,
	debet numeric(10,2),
	kredit numeric(10,2),
	eraid integer,
	alvprosentti numeric(5,2),
	alvkoodi integer,
	kumppani integer REFERENCES Kumppani(id),
	jaksoalkaa DATE,
	jaksoloppuu DATE,
	json text,
	CHECK (debet > 0 OR kredit > 0),
	CHECK (debet = 0 OR kredit = 0)
);

CREATE INDEX vienti_tosite ON Vienti (tosite);
CREATE INDEX vienti_pvm ON Vienti (pvm);
CREATE INDEX vienti_tili ON Vienti (tili);
CREATE INDEX vienti_kohdennus ON Vienti (kohdennus);

CREATE TABLE Liite
(
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	tosite integer REFERENCES Tosite (id) ON DELETE CASCADE,
	nimi text,
	tyyppi text,
	sha text,
	data bytea,
	luotu timestamp DEFAULT current_timestamp,
	json text,
	UNIQUE(tosite,nimi)
);

CREATE INDEX liite_tosite ON Liite (tosite);

CREATE TABLE Tuote
(
	id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	nimike VARCHAR(255),
	json text
);

CREATE TABLE Rivi
(
	tosite integer REFERENCES Tosite(id) ON DELETE CASCADE,
	rivi integer,
	tuote integer,
	myyntikpl real,
	ostokpl real,
	ahinta real DEFAULT(0.0),
	json text,
	PRIMARY KEY(tosite, rivi)
);

CREATE INDEX rivi_tosite ON Rivi (tosite);