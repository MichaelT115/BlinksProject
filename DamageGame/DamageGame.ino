#define ROLE_BASE     0
#define ROLE_RESOURCE 1
#define ROLE_HAZARD   2
byte _role = ROLE_BASE;

#define MAX_HEALTH 5
#define START_HEALTH 3
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
    switch (_role) {
      case ROLE_BASE:
        if (isAlone()) {
          _role = ROLE_HAZARD;
        } else {
          _role = ROLE_BASE;
        }
        break;
      case ROLE_HAZARD:
        _role = ROLE_BASE;
        break;
      case ROLE_RESOURCE:
        _role = ROLE_BASE;
        break;
    }
  }

  if (buttonSingleClicked()) {
    if (_isAlive) {
      messageState[POINTER_FACE] = Repair;
      damageHealth();
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
      damageHealth();
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
    case ROLE_BASE:
      displayBase();
      break;
    default:
      setColor(WHITE);
  }
}

void displayBase() {
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
