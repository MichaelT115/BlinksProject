#define ROLE_HULL     0
#define ROLE_HAZARD   1
byte _role = ROLE_HULL;

#define MAX_HEALTH 5
#define START_HEALTH 5
byte _health = START_HEALTH;
bool _isAlive = true;

void damageHealth() {
  if (_health == 0)
    return;

  --_health;

  _isAlive = _health;
}

void repairHealth() {
  ++_health;

  if (_health > MAX_HEALTH) {
    _health = MAX_HEALTH;
  }
}

#define POINTER_FACE 5

byte _hazardOffset = 0;

void setup() {
}

void loop() {
  logic();

  display();
}

// Messages
enum messages { None, Recieved, Damage, Repair };
byte messageState[6] = { None, None, None, None, None, None };

bool getNeighborIsAlive(byte face) {
  return getLastValueReceivedOnFace(face) >> 5;
}

byte getNeighborMessage(byte face) {
  return getLastValueReceivedOnFace(face) & 0b00011111;
}

byte createMessageData(byte face) {
  return _isAlive << 5 | messageState[face];
}

void logic() {
  // Controls
  if (buttonDoubleClicked()) {
    if (isAlone()) {
      _role = _role == ROLE_HULL ? ROLE_HAZARD : ROLE_HULL;
    }
  }

  if (buttonSingleClicked()) {
    switch (_role) {
      case ROLE_HULL:
        if (_isAlive) {
          messageState[POINTER_FACE] = Repair;
          damageHealth();
        }
        break;
      case ROLE_HAZARD:
        messageState[(_hazardOffset + 0) % FACE_COUNT] = Damage;
        messageState[(_hazardOffset + 2) % FACE_COUNT] = Damage;
        messageState[(_hazardOffset + 4) % FACE_COUNT] = Damage;

        _hazardOffset = (_hazardOffset + 1) % FACE_COUNT;
        break;
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
    setValueSentOnFace(createMessageData(f), f);
  }
}

void standardLoop(byte face) {
  // Do we have a neighbor
  if (isValueReceivedOnFaceExpired(face))
    return;

  // Did the neighbor send a real message.
  byte neighborMessage = getNeighborMessage(face);
  if (neighborMessage == None || neighborMessage == Recieved)
    return;

  // Mark that we recieved the message
  messageState[face] = Recieved;

  // Handle Message
  switch (neighborMessage) {
    case Damage:
      if (_role == ROLE_HULL) {
        damageHealth();
      }
      messageState[(face + 3) % 6] = Damage;
      break;
    case Repair:
      repairHealth();
      break;
  }
}

void listeningLoop(byte face) {
  // The channel is clear.
  if (isValueReceivedOnFaceExpired(face) || getNeighborMessage(face) == None) {
    messageState[face] = None;
  }
}

void activeLoop(byte face) {
  // Our message is recieved or there is no recipient
  if (isValueReceivedOnFaceExpired(face) || getNeighborMessage(face) == Recieved) {
    messageState[face] = None;
  }
}

// Display
void display() {
  switch (_role) {
    case ROLE_HULL:
      displayHull();
      break;
    case ROLE_HAZARD:
      displayHazard();
      break;
    default:
      setColor(WHITE);
  }
}

void displayHull() {
  byte faceIndex = 0;

  if (!_isAlive) {
    setColor(OFF);
    return;
  }
  
  while (faceIndex < _health) {
    setColorOnFace(GREEN, faceIndex);
    ++faceIndex;
  }
  while (faceIndex < POINTER_FACE) {
    setColorOnFace(OFF, faceIndex);
    ++faceIndex;
  }

  setColorOnFace(CYAN, POINTER_FACE);
}


#define DAMAGE_COLOR ORANGE
#define NONDAMAGE_COLOR OFF
void displayHazard() {
  setColorOnFace(DAMAGE_COLOR,    (_hazardOffset + 0) % FACE_COUNT);
  setColorOnFace(NONDAMAGE_COLOR, (_hazardOffset + 1) % FACE_COUNT);
  setColorOnFace(DAMAGE_COLOR,    (_hazardOffset + 2) % FACE_COUNT);
  setColorOnFace(NONDAMAGE_COLOR, (_hazardOffset + 3) % FACE_COUNT);
  setColorOnFace(DAMAGE_COLOR,    (_hazardOffset + 4) % FACE_COUNT);
  setColorOnFace(NONDAMAGE_COLOR, (_hazardOffset + 5) % FACE_COUNT);
}
