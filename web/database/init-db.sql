create database if not exists ece140;

use ece140;

create table if not exists users (
  id         integer auto_increment primary key,
  first_name varchar(64) not null,
  last_name  varchar(64) not null,
  username   varchar(64) not null unique,
  password   varchar(64) not null,
  created_at timestamp not null default current_timestamp
);

create table if not exists sessions (
  session_id varchar(64) primary key,
  session_data json not null,
  created_at timestamp not null default current_timestamp
);



create table if not exists sensor_data (
  id integer auto_increment primary key,
  user_id integer not null,
  location varchar(255) not null,
  created_at timestamp not null default current_timestamp,
  temperature float not null,
  humidity float not null
);
