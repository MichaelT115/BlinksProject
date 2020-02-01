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
  if (_health == 0)
    return;

  --_health;
}

void repairHealth() {
  ++_health;

  if (_health > 6) {
    _health = 6;
  }
}


void setup() {
  setValueSentOnAllFaces(0);
  _health = 6;
}

void loop() {
  logic();

  display();
}

// Messages
enum messages { None, Recieved, Damage, Repair };
byte messageState[6] = { None, None, None, None, None, None };

void logic() {
  // Controls
  if (buttonDoubleClicked()) {
    switch (_role) {
      case ROLE_BASE:
        if (isAlone()) {
          setRole(ROLE_HAZARD);
        } else {
          setRole(ROLE_BASE);
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

  if (buttonSingleClicked()) {
    FOREACH_FACE(f) {
      messageState[f] = Damage;
    }
  }
  
  // Listen for messages
  FOREACH_FACE(f) {
    switch (messageState[f]) {
      case None:  // Message is listening
        standardLoop(f);
        break;
      case Recieved:  // We have recieved a message and are waiting for the channel to clear.
        listeningLoop(f);
        break;
      default:  // We have sent a message and are waiting for confirmation.
        activeLoop(f);
        break;
    }
  }

  // Set messages
  FOREACH_FACE(f) {
    setValueSentOnFace(messageState[f], f);
  }
}

void standardLoop(byte face) {
  // Do we have a neighbor
  if (isValueReceivedOnFaceExpired(face))
    return;

  // Did the neighbor send a real message.
  byte neighborMessage = getLastValueReceivedOnFace(face);
  if (neighborMessage == None || neighborMessage == Recieved)
    return;

  // Mark that we recieved the message
  messageState[face] = Recieved;

  // Handle Message
  switch (neighborMessage) {
    case Damage:
      damageHealth();
      messageState[(face + 3) % 6] = Damage;
      break;
  }
}

void listeningLoop(byte face) {
  // The channel is clear.
  if (isValueReceivedOnFaceExpired(face) || getLastValueReceivedOnFace(face) == None) {
    messageState[face] = None;
  }
}

void activeLoop(byte face) {
  // Our message is recieved or there is no recipient
  if (isValueReceivedOnFaceExpired(face) || getLastValueReceivedOnFace(face) == Recieved) {
    messageState[face] = None;
  }
}


// Display
void display() {
  displayHealth();
}

void displayHealth() {
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
