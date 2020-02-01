#define ROLE_BASE     0
#define ROLE_RESOURCE 1
#define ROLE_HAZARD   2
byte _role = ROLE_BASE;

void setRole(byte role) {
  _role = role;
}

Color getRoleColor() {
  switch (_role) {
    case ROLE_BASE:
      return GREEN;
    case ROLE_RESOURCE:
      return BLUE;
    case ROLE_HAZARD:
      return RED;
    default:
      return WHITE;
  }
}

byte _health;
void damageHealth() {
  if(_health == 0)
    return;

   --_health;
}

void repairHealth() {
  ++_health;

  if(_health > 6) {
    _health = 6;
  }
}


bool isConnectedOnOneOrMoreFaces() {
  FOREACH_FACE(face) {
    if (!isValueReceivedOnFaceExpired(face)) {
      return true;
    }
  }
  return false;
}


void setup() {
  setValueSentOnAllFaces(0);
  _health = 6;
}

void loop() {
  if (buttonSingleClicked()) {
    if (isConnectedOnOneOrMoreFaces()) {
      setRole(ROLE_BASE);
    } else {
      setRole(ROLE_RESOURCE);
    }
  }

  if (buttonDoubleClicked()){
    if (_role == ROLE_BASE) {
      damageHealth();
    } else if (_role == ROLE_RESOURCE) {
      repairHealth();
    }
  }

  communication();

  display();
}

void communication() {
  setValueSentOnAllFaces(0);  
}

// Display
void display() {
  displayHealth();
}

void displayHealth(){
  Color color = getRoleColor();

  byte faceIndex = 0;
  while (faceIndex < _health) {
    setColorOnFace(color, faceIndex);
    ++faceIndex;
  }
  while (faceIndex < FACE_COUNT) {
    setColorOnFace(OFF, faceIndex);
    ++faceIndex;
  }
}
