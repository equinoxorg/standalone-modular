void setup()
{
  Serial.begin(9600);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(10, INPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
}

void loop()
{
  if (digitalRead(10) == LOW) //payment = pin 10
  {
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
  }
  else if (analogRead(1) < 512) // voltage divider check
  {
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
  }
  else if (digitalRead(2) == HIGH) //USB jack 1 check, pin 2 and 5
  {
    digitalWrite(5, LOW);
  }
  else if (digitalRead(3) == HIGH) //USB jack 2 check, pin 3 and 6
  {
    digitalWrite(6, LOW);
  }
  else if (digitalRead(4) == HIGH) // DC output, pin 4 and 7
  {
    digitalWrite(7, LOW);
  }
  else //after all checks are ok, set all output on
  {
    digitalWrite(5, HIGH);
    digitalWrite(6, HIGH);
    digitalWrite(7, HIGH);
  }
}
