int MESSAGE_LENGTH = 8;

int scl = 3;
int sda = 11;
int max_time = 8 * MESSAGE_LENGTH;
int curr_time = 0;
int data = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  
  // 0101010101
  if (curr_time < max_time) { // Write data
    digitalWrite(scl, HIGH); // Turn clock on

    if (data == 1) {
      digitalWrite(sda, HIGH);
    } else {
      digitalWrite(sda, LOW);
    }

    data = 1 - data; // 0->1, 1->0
  } else {
    digitalWrite(scl, LOW);
    digitalWrite(sda, LOW);
  }
  
  curr_time++;
  // 5 ms delay between signals, 
  delay(5);

}
