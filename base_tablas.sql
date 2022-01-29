DROP DATABASE IF EXISTS BBDD_GT9;
CREATE DATABASE BBDD_GT9;

USE BBDD_GT9;

CREATE TABLE jugador (
	username Varchar (60),
	password Varchar (60),
	id INTEGER
)ENGINE=InnoDB;

INSERT INTO jugador values ('Tomi', '123456789', 1);
INSERT INTO jugador values ('Irina', '987654321', 2);
INSERT INTO jugador values ('Oscar', 'abcdefghi', 3);
INSERT INTO jugador values ('Iron-MAN', 'supertraje',4);
INSERT INTO jugador values ('THOR', 'sistemes_operatius',5);
INSERT INTO jugador values ('HULK', 'APLASTAR!!!!!', 6);
INSERT INTO jugador values ('MAGNETO', 'mutantes_10', 7);
INSERT INTO jugador values ('INFERNO', 'fuego-fatuo', 8);

CREATE TABLE partida (
	id INTEGER,
	fecha TEXT,
	duracion_MINUTOS INTEGER,
	ganador Varchar (60)
)ENGINE=InnoDB;

INSERT INTO partida values (1, '29/05/2021', 5, 'Irina');
INSERT INTO partida values (2, '29/05/2021', 7, 'Oscar');
INSERT INTO partida values (3, '29/05/2021', 4, 'Tomi');
INSERT INTO partida values (2, '29/05/2021', 7, 'Oscar');
INSERT INTO partida values (4, '29/05/2021', 2, 'HULK');
INSERT INTO partida values (3, '29/05/2021', 4, 'Tomi');
INSERT INTO partida values (1, '29/05/2021', 5, 'Irina');
INSERT INTO partida values (4, '29/05/2021', 2, 'HULK');

CREATE TABLE participacion (
	puntos INTEGER,
	id_jugador INTEGER,
	id_partida INTEGER
)ENGINE=InnoDB;

INSERT INTO participacion values (140, 1, 2);
INSERT INTO participacion values (120, 2, 1);
INSERT INTO participacion values (150, 3, 3);
INSERT INTO participacion values (50, 4, 4);
INSERT INTO participacion values (50, 5, 1);
INSERT INTO participacion values (400, 6, 3);
INSERT INTO participacion values (50, 7, 4);
INSERT INTO participacion values (50, 8, 2);
