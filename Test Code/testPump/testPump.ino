int enA = 25;
int in1 = 26;
int in2 = 27;

void setup() {
  // Set all the motor control pins to outputs
	pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in1, OUTPUT);

  // Turn off motors - Initial state
	digitalWrite(in1, LOW);
	digitalWrite(in2, LOW);
}

void loop() {
  // Set motors to maximum speed
	// For PWM maximum possible values are 0 to 255
	analogWrite(enA, 0);


  // Turn on motor
	digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
}
