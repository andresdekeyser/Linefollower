/* DRV8833 test:
   Laat motor A en B vooruit, achteruit en stoppen.
*/

const int AIN1 = 2;
const int AIN2 = 3;
const int BIN1 = 4;
const int BIN2 = 5;

void setup() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  Serial.begin(9600);
  Serial.println("DRV8833 test start");
}

void stopMotors() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
}

void forward() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
}

void backward() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
}

void left() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
}

void right() {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
}

void loop() {

  Serial.println("Vooruit");
  forward();
  delay(2000);

  Serial.println("Stop");
  stopMotors();
  delay(1000);

  Serial.println("Achteruit");
  backward();
  delay(2000);

  Serial.println("Stop");
  stopMotors();
  delay(1000);

  Serial.println("Links draaien");
  left();
  delay(2000);

  Serial.println("Rechts draaien");
  right();
  delay(2000);

  Serial.println("Stop");
  stopMotors();
  delay(1000);
}
