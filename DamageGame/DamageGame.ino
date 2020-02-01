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
  // Double Click Sets Role
  if (buttonDoubleClicked()) {
    switch (_role) {
      case ROLE_BASE:
        if (isConnectedOnOneOrMoreFaces()) {
          setRole(ROLE_HAZARD);
        } else {
          setRole(ROLE_RESOURCE);
        }
        break;
      case ROLE_HAZARD:
        setRole(ROLE_BASE);
        break;
      case ROLE_RESOURCE:
        setRole(ROLE_BASE);
        break;
    }
  }


  communication();

  display();
}

// Messages
#define MESSAGE_NONE      0
#define MESSAGE_RECIEVED  1
#define MESSAGE_DAMAGE    2

#define MESSAGE_TIMER_LENGTH    300
#define LISTENING_TIMER_LENGTH  300

Timer eventTimer;
Timer listeningTimer;
void communication() {
  // Sending Events
  bool canSendEvents = eventTimer.isExpired();
  if (canSendEvents) {
    if(buttonSingleClicked()) {
      setValueSentOnAllFaces(MESSAGE_DAMAGE);
      eventTimer.set(MESSAGE_TIMER_LENGTH);
    } else {
      setValueSentOnAllFaces(MESSAGE_NONE); // We want to explicitly send none by default.
    }
  }

  // Stop sending events when eve
  FOREACH_FACE(face) {
    if (getLastValueReceivedOnFace(face) == MESSAGE_RECIEVED) {
      setValueSentOnFace(MESSAGE_NONE, face);
    }
  }

  // Listening for events
  bool isListening = listeningTimer.isExpired();
  if (isListening) {
    FOREACH_FACE(face) {
      if (isValueReceivedOnFaceExpired(face))
        continue;
      
      if (getLastValueReceivedOnFace(face) == MESSAGE_DAMAGE) {
        damageHealth();
        setValueSentOnFace(MESSAGE_RECIEVED, face);
        listeningTimer.set(LISTENING_TIMER_LENGTH);
      }
    }
  }
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
