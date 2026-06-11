''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
# Necessary Imports
import mysql.connector as mysql                   # Used for interacting with the MySQL database
import os                                         # Used for interacting with the system environment
from pathlib import Path                             # Used for handling file paths
from dotenv import load_dotenv                    # Used to read the credentials
from datetime import datetime
import bcrypt
from pathlib import Path

BASE_DIR = Path(__file__).resolve().parent
SQL_INIT_FILE = BASE_DIR.parent / "database" / "init-db.sql"
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
# Configuration
BASE_DIR = Path(__file__).resolve().parent
load_dotenv(BASE_DIR.parent / 'credentials.env')  # Read in the environment variables for MySQL
db_config = {
  "host": os.environ['MYSQL_HOST'],
  "user": os.environ['MYSQL_USER'],
  "password": os.environ['MYSQL_PASSWORD'],
  "database": os.environ['MYSQL_DATABASE']
}

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
def initialize_database() -> None:
  db = mysql.connect(**db_config)
  cursor = db.cursor()

  with open(SQL_INIT_FILE, "r") as file:
    sql_script = file.read()

  for statement in sql_script.split(";"):
    statement = statement.strip()
    if statement:
      cursor.execute(statement)

  db.commit()
  cursor.close()
  db.close()
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
# Define helper functions for CRUD operations
# CREATE SQL query
def create_user(first_name:str, last_name:str, username:str, password:str) -> int:
  password = bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt())

  db = mysql.connect(**db_config)
  cursor = db.cursor()
  query = "insert into users (first_name, last_name, username, password) values (%s, %s, %s, %s)"
  values = (first_name, last_name, username, password)
  cursor.execute(query, values)
  db.commit()
  db.close()
  return cursor.lastrowid

# SELECT SQL query
def select_users(user_id:int=None) -> list:
  db = mysql.connect(**db_config)
  cursor = db.cursor()
  if user_id == None:
    query = f"select id, first_name, last_name, username from users;"
    cursor.execute(query)
    result = cursor.fetchall()
  else:
    query = f"select id, first_name, last_name, username from users where id={user_id};"
    cursor.execute(query)
    result = cursor.fetchone()
  db.close()
  return result


# update user

# delete user



# SELECT query to verify hashed password of users
def check_user_password(username:str, password:str) -> bool:
  db = mysql.connect(**db_config)
  cursor = db.cursor()
  query = 'select password from users where username=%s'
  cursor.execute(query, (username,))
  result = cursor.fetchone()
  cursor.close()
  db.close()

  if result is not None:
    return bcrypt.checkpw(password.encode('utf-8'), result[0].encode('utf-8'))
  return False


def get_user_by_username(username: str) -> dict | None:
    query = "SELECT id, first_name, last_name, username FROM users WHERE username = %s"

    connection = mysql.connect(**db_config)
    cursor = connection.cursor(dictionary=True)

    cursor.execute(query, (username,))
    user = cursor.fetchone()

    cursor.close()
    connection.close()

    return user

 
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
# CREATE query to insert sensor data
def insert_sensor_data(user_id:int = 2, location:str = "Default Location", temperature:float = 0.0, humidity:float = 0.0) -> int:
  db = mysql.connect(**db_config)
  cursor = db.cursor()
  query = "insert into sensor_data (user_id, location, temperature, humidity) values (%s, %s, %s, %s)"
  values = (user_id, location, temperature, humidity)
  cursor.execute(query, values)
  db.commit()
  db.close()
  return cursor.lastrowid

# SELECT query to get sensor data
def get_sensor_data(user_id:int=None, location:str=None, start_time:str=None, end_time:str=None, sensor_type:str='temperature') -> list:
  db = mysql.connect(**db_config)
  cursor = db.cursor(dictionary=True)

  field = 'temperature' if sensor_type == 'temperature' else 'humidity'
  unit = 'Temperature (°F)' if field == 'temperature' else 'Humidity (%)'

  conditions = []
  values = []

  if user_id is not None:
    conditions.append('user_id = %s')
    values.append(user_id)
  if location is not None:
    conditions.append('location = %s')
    values.append(location)
  if start_time is not None and end_time is not None:
    conditions.append('created_at BETWEEN %s AND %s')
    values.append(start_time.replace('T', ' ').replace('Z', ''))
    values.append(end_time.replace('T', ' ').replace('Z', ''))

  query = f"SELECT id, user_id, location, created_at AS time, {field} AS value FROM sensor_data"
  if conditions:
    query += ' WHERE ' + ' AND '.join(conditions)

  cursor.execute(query, tuple(values))
  result = cursor.fetchall()
  db.close()

  for row in result:
    if isinstance(row['time'], datetime):
      row['time'] = row['time'].strftime('%Y-%m-%dT%H:%M:%SZ')
    else:
      row['time'] = str(row['time'])
    row['unit'] = unit

  return result