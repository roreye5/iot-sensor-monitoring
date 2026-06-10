''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
# Necessary Imports
import os
from fastapi import FastAPI, HTTPException
from fastapi import FastAPI, File, UploadFile
from fastapi import FastAPI, Request, Response    # The main FastAPI import and Request/Response objects
from fastapi.responses import RedirectResponse    # Used to redirect to another route
from pydantic import BaseModel                    # Used to define the model matching the DB Schema
from fastapi.responses import HTMLResponse        # Used for returning HTML responses (JSON is default)
from fastapi.templating import Jinja2Templates    # Used for generating HTML from templatized files
from fastapi.staticfiles import StaticFiles       # Used for making static resources available to server
from pathlib import Path
import uvicorn                                    # Used for running the app directly through Python
from server import dbutils as db                              # Import helper module of database functions!

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
# Configuration
BASE_DIR = Path(__file__).resolve().parent
app = FastAPI()                                   # Specify the "app" that will run the routing
@app.on_event("startup")
def startup_event():
    db.initialize_database()
views = Jinja2Templates(directory=BASE_DIR / "views")        # Specify where the HTML files are located
static_files = StaticFiles(directory=BASE_DIR / "public")    # Specify where the static files are located
app.mount('/public', static_files, name='public') # Mount the static files directory to /public

# Use MySQL for storing session data
from server.sessiondb import Sessions
sessions = Sessions(db.db_config, expiry=600)

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
# Define a User class that matches the SQL schema we defined for our users
class User(BaseModel):
  first_name: str
  last_name: str
  username: str
  password: str

class Visitor(BaseModel):
  username: str
  password: str

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
# A function to authenticate users when trying to login or use protected routes
# returns the user data
def authenticate_user(username:str, password:str) -> bool | dict | None:
  if db.check_user_password(username, password):
     return db.get_user_by_username(username) 
  return False

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
TEMP_DIRECTORY = "temp"

@app.post('/api/audio')
async def handle_audio(audio_file: UploadFile = File(...)):
    # Make sure the temporary directory exists
    os.makedirs(TEMP_DIRECTORY, exist_ok=True)
    
    # Full path for the uploaded file
    file_location = os.path.join(TEMP_DIRECTORY, audio_file.filename)

    # Write the uploaded file to the filesystem
    with open(file_location, "wb+") as file_object:
        file_object.write(audio_file.file.read())

    # Check if the file was saved correctly by verifying the file size
    file_size = os.path.getsize(file_location)

    if file_size > 0:
        # Return success message with file details
        return {
            "info": "Audio received and saved.",
            "filename": audio_file.filename,
            "size_bytes": file_size
        }
    else:
        # If file is empty, return an error message
        return {
            "error": "The file was not saved correctly.",
            "filename": audio_file.filename
        }

@app.get('/record', response_class=HTMLResponse)
def record_audio_page(request:Request) -> HTMLResponse:
    session = sessions.get_session(request)
    if len(session) > 0 and session.get('logged_in'):
      with open(BASE_DIR /"views/display.html") as html:
        return HTMLResponse(content=html.read())

    else:
      return RedirectResponse(url="/login", status_code=302)
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''


# GET /users/{user_id}
@app.get('/users/{user_id}')
def get_user(user_id:int) -> dict:
  user = db.select_users(user_id)
  if user:
    return {'id':user[0], 'first_name':user[1], 'last_name':user[2], 'username':user[3]}
  return {}

# POST /users
# Used to create a new user
@app.post("/users")
def post_user(user:User) -> dict:
  new_id = db.create_user(user.first_name, user.last_name, user.username, user.password)
  return get_user(new_id)   # probably need to change this
# because we don't need to dynamically load html page
# right now it : After creating the user, it retrieves and returns the newly created user's information.



''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
# Authentication routes (login, logout, and a protected route for testing)

# get route: return a static HTML page
@app.get("/login", response_class=HTMLResponse)
def get_login(request: Request) -> HTMLResponse:
    return views.TemplateResponse(
      request,
      "login.html",
      context = {"request": request}
    )
    
@app.post('/login')
def post_login(visitor:Visitor, request:Request, response:Response) -> dict:
  username = visitor.username
  password = visitor.password

  # Invalidate previous session if logged in
  session = sessions.get_session(request)
  if len(session) > 0:
    sessions.end_session(request, response)

  # Authenticate the user
  user = authenticate_user(username, password)
  if user:
    session_data = {'username': username, "first_name": user["first_name"], 'logged_in': True}
    session_id = sessions.create_session(response, session_data)
    return {'message': 'Login successful', 'session_id': session_id}
  else:
    return {'message': 'Invalid username or password', 'session_id': 0}
  

@app.post('/logout')
def post_logout(request:Request, response:Response) -> dict:
  sessions.end_session(request, response)
  return {'message': 'Logout successful', 'session_id': 0}

# get route: return dashboard HTML page if logged in
# or redirect to login page if not logged in
@app.get("/dashboard", response_class=HTMLResponse)
def get_dashboard(request:Request) -> HTMLResponse:
    
    session = sessions.get_session(request)
    if len(session) > 0 and session.get('logged_in'):
      return views.TemplateResponse(
        request,
        "dashboard.html",
        context = {"request": request, "username": str(session.get('first_name')).capitalize()}
      )
    else:
      return RedirectResponse(url="/login", status_code=302)

# GET /sessions
@app.get('/sessions')
def get_sessions(request:Request) -> dict:
  return sessions.get_session(request)

''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
# Index route to load the main page in a templatized fashion
# GET /
@app.get('/', response_class=HTMLResponse)
def get_home(request:Request) -> HTMLResponse:
    return views.TemplateResponse(
       request,
       "homepage.html",
       context = {"request": request}
    )


''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''



# If running the server directly from Python as a module
if __name__ == "__main__":
  uvicorn.run("main:app", host="127.0.0.1", port=8000, reload=True)
