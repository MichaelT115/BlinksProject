#define ROLE_SHIP     0
#define ROLE_HAZARD   1
byte _role = ROLE_SHIP;

//
// Ship Properties
//
#define POINTER_FACE 5

#define MAX_HEALTH 5
#define START_HEALTH 5
byte _health = START_HEALTH;

void damageHealth() {
  if (_health == 0)
    return;

  --_health;
}

void repairHealth() {
  ++_health;

  if (_health > MAX_HEALTH) {
    _health = MAX_HEALTH;
  }
}

//
// Hazard Properties
//
#define START_OFFSET 0
byte _hazardOffset = START_OFFSET;

//
// Communications
//
enum messages { 
  None = 0,     // Placeholder when messages are not being set.
  Recieved = 1, // A message telling neighbor that their message has been recieved and processed.
  Damage = 2,   // A message telling neighbor to take damage
  Repair = 3,   // A message telling neighbor to repair.
};
byte messageState[6] = { None, None, None, None, None, None }; 

bool getNeighborIsAlive(byte face)  { return getLastValueReceivedOnFace(face) >> 5; }
byte getNeighborMessage(byte face)  { return getLastValueReceivedOnFace(face) & 0b00011111; }
byte createMessageData(byte face)   { return (_health != 0) << 5 | messageState[face]; }

//
// Gameplay
//
void gameplay() {
  // Toggle Role
  if (buttonLongPressed() && isAlone()) {
    _role = _role == ROLE_SHIP ? ROLE_HAZARD : ROLE_SHIP;
    _health = MAX_HEALTH;
  }

  if (buttonMultiClicked()) {
     _health = MAX_HEALTH;
  }

  // Trigger Funtion
  if (buttonSingleClicked()) {
    switch (_role) {
      case ROLE_SHIP:
        // Repair pointed at node.
        if (_health) {
          messageState[POINTER_FACE] = Repair;
          damageHealth();
        }
        break;
      case ROLE_HAZARD:
        // Damage in three directions.
        messageState[_hazardOffset + 0] = Damage;
        messageState[_hazardOffset + 2] = Damage;
        messageState[_hazardOffset + 4] = Damage;

        // Update hazard directions
        _hazardOffset = (_hazardOffset + 1) % 2;
        break;
    }
  }

  // Listen for messages
  FOREACH_FACE(face) {
    // There is no nieghbor, set message to none.
    if (isValueReceivedOnFaceExpired(face)) {
       messageState[face] = None;
       continue;
    }

    byte message = getNeighborMessage(face);
    
    switch (messageState[face]) {
      case None:  // Face is listening for a message
        // If there is a valid message.
        if (message != None && message != Recieved) {
          handleMessage(face, message);
        }
        break;
      case Recieved:  // We have recieved a message and are waiting for the channel to clear.
        if (message == None) {
          messageState[face] = None;  // Channel is clear. Wait for future message
        }
        break;
      default:  // We have sent a message and are waiting for confirmation.
        if (message == Recieved) {
          messageState[face] = None;  // Our message is confirmed. Clear the channel.
        }
        break;
    }
  }

  // Send messages
  FOREACH_FACE(f) {
    setValueSentOnFace(createMessageData(f), f);
  }
}

void handleMessage(byte face, byte message) {
  // Mark that we recieved the message
  messageState[face] = Recieved;
  
  switch (message) {
    case Damage:
      // Damage node on opposite side.
      messageState[(face + 3) % 6] = Damage;
    
      if (_role == ROLE_SHIP) {
        damageHealth();
      }
      return;
    case Repair:
      if (_role == ROLE_SHIP) {
        repairHealth();
      }
      return;
  }
}

//
// Display
//

void display() {
  switch (_role) {
    case ROLE_SHIP:
      displayShip();
      break;
    case ROLE_HAZARD:
      displayHazard();
      break;
    default:
      setColor(WHITE);
  }
}

#define DEAD_COLOR RED
#define HEALTH_COLOR GREEN
#define DAMAGED_COLOR OFF
#define POINTER_COLOR MAGENTA
void displayShip() {
  if (!_health) {
    setColorOnFace(dim(DEAD_COLOR, 255 * (((millis() - 167 * 0) % 1000) / 1000.0f)), 0);
    setColorOnFace(dim(DEAD_COLOR, 255 * (((millis() - 167 * 1) % 1000) / 1000.0f)), 1);
    setColorOnFace(dim(DEAD_COLOR, 255 * (((millis() - 167 * 2) % 1000) / 1000.0f)), 2);
    setColorOnFace(dim(DEAD_COLOR, 255 * (((millis() - 167 * 3) % 1000) / 1000.0f)), 3);
    setColorOnFace(dim(DEAD_COLOR, 255 * (((millis() - 167 * 4) % 1000) / 1000.0f)), 4);
    setColorOnFace(dim(DEAD_COLOR, 255 * (((millis() - 167 * 5) % 1000) / 1000.0f)), 5);    
    return;
  }

  byte faceIndex = 0;
  
  while (faceIndex < _health) {
    setColorOnFace(HEALTH_COLOR, faceIndex);
    ++faceIndex;
  }
  while (faceIndex < POINTER_FACE) {
    setColorOnFace(DAMAGED_COLOR, faceIndex);
    ++faceIndex;
  }

  setColorOnFace(POINTER_COLOR, POINTER_FACE);
}

#define HAZARD_DAMAGE_COLOR ORANGE
#define HAZARD_NONDAMAGE_COLOR OFF
void displayHazard() {
  setColorOnFace(HAZARD_DAMAGE_COLOR,    (_hazardOffset + 0));
  setColorOnFace(HAZARD_NONDAMAGE_COLOR, (_hazardOffset + 1));
  setColorOnFace(HAZARD_DAMAGE_COLOR,    (_hazardOffset + 2));
  setColorOnFace(HAZARD_NONDAMAGE_COLOR, (_hazardOffset + 3));
  setColorOnFace(HAZARD_DAMAGE_COLOR,    (_hazardOffset + 4));
  setColorOnFace(HAZARD_NONDAMAGE_COLOR, (_hazardOffset + 5) % FACE_COUNT);
}

// Event Functions:

void setup() { }

void loop() {
  gameplay();
  display();
}
