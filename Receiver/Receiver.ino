/*
  NOTE:
  ======
  Only RTC IO can be used as a source for external wake
  source. They are pins: 0,2,4,12-15,25-27,32-39.
*/

// RX TX pins
#define RXD2 17
#define TXD2 18

// Wake up pin
#define BUTTON_PIN_BITMASK 0x8004 // 2 and 15

RTC_DATA_ATTR int bootCount = 0;
// Wake up time set
#define uS_TO_S_FACTOR 500000
#define Time_Sleep_S 5

///// Ultrasonic /////////////////
#define echoPin 26
#define trigPin 25
int duration, distance;
int count = 0, countLoop = 0;
//////////////////////////////////

///// Moter //////////////////////
int se_a;
int se_a_s;
int output_A;
int GPIO14;
int state;
//////////////////////////////////

char readChar;
float VP;
int triggeredPin;
unsigned long previousMillis = 0;

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }

  getVoltage();
  GPIO14 = digitalRead(14);
  int IR_1 = digitalRead(32);
  int IR_2 = digitalRead(33);
  state = 0;

  if (GPIO14 == 1 && IR_1 == 1 && IR_2 == 1) {
    getWUL();
    getData();
    while (state < 1) {
      GPIO14 = digitalRead(14);
      int IR_1 = digitalRead(32);
      int IR_2 = digitalRead(33);
      Moter_control();
    }
  } else if (GPIO14 == 1 && IR_1 == 0 && IR_2 == 0) {
    getData();
  } else {
    getData();
    while (state < 1) {
      GPIO14 = digitalRead(14);
      int IR_1 = digitalRead(32);
      int IR_2 = digitalRead(33);
      Moter_control();
    }
  }
}

void print_GPIO_wake_up() {
  triggeredPin = 0;
  int GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  triggeredPin = (log(GPIO_reason)) / log(2);
  Serial.print("GPIO that triggered to wake up: GPIO ");
  Serial.println((log(GPIO_reason)) / log(2), 0);
}

void setup() {
  //Serial config
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  //Ultrasonic pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //ADC pin
  pinMode(36, INPUT);

  //Buzzer pin
  pinMode(22, OUTPUT);

  //Trigger WUL pin
  pinMode(27, OUTPUT);

  ////////////////////////////// Moter Control ////////////////////////////////
  //GPIO14 Status                              ////////////////////////////////
  pinMode(14, INPUT);                          ////////////////////////////////
  //IR Sensor pins                             ////////////////////////////////
  pinMode(32, INPUT_PULLUP);                   ////////////////////////////////
  pinMode(33, INPUT_PULLUP);                   ////////////////////////////////
  //Moter pins                                 ////////////////////////////////
  pinMode(19, OUTPUT);                         ////////////////////////////////
  pinMode(21, OUTPUT);                         ////////////////////////////////
  //GPIO23 Trigger moter                       ////////////////////////////////
  pinMode(23, OUTPUT);                         ////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  //Print the Pins that triggered to wake up.
  print_GPIO_wake_up();

  //Multi pins
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  //Wake up by timer
  //esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * Time_Sleep_S);

}

void pushDataV () {
  Serial2.println('V');
  Serial2.println(VP);
}

void pushDataWUL () {
  Serial2.println('U');
  Serial2.println(distance);
}

void getData () {
  if (Serial2.available() > 0) {
    readChar = Serial2.read();
    Serial.println("Wait commnad...");
    if (readChar == 'V') {
      getVoltage();
      pushDataV();
    }

    if (readChar == 'U') {
      getWUL();
      pushDataWUL();
    }

    if (readChar == 'S') {
      Goto_sleep_now();
    }
  }
}

void loop() {
  //Serial.println("Loop ! ! ! ");

  //Multi pins
  //esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  getData();
  wait_command();

  //GPIO14 = digitalRead(14);
  //Moter_control();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 500) {
    previousMillis = currentMillis;
    countLoop++;
  }
  if (triggeredPin == 2) {
    countLoop = 0;
  } else if (triggeredPin == 15) {
    countLoop = 0;
  }

  if (countLoop > 10) {
    Goto_sleep_now();
  }
}

void wait_command () {
  if (triggeredPin == 2) {
    getData();

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 500) {
      previousMillis = currentMillis;
      digitalWrite(22, HIGH);
      Serial.println("alert ");
    }
    if (currentMillis - previousMillis >= 1000) {
      previousMillis = currentMillis;
      digitalWrite(22, LOW);
    }
  } else if (triggeredPin == 15) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 100) {
      previousMillis = currentMillis;
      //Serial.println(count);
      count++;
    }

    GPIO14 = digitalRead(14);
    int IR_1 = digitalRead(32);
    int IR_2 = digitalRead(33);

    getData();
    getVoltage();

    if (GPIO14 == 1) {
      if (IR_1 == 0 && IR_2 == 0) {
        getData();
      } else if (IR_1 == 1 && IR_2 == 0) {
        Moter_control();
      } else if (IR_1 == 0 && IR_2 == 1) {
        Moter_control();
      } else if (IR_1 == 1 && IR_2 == 1) {
        getWUL();
        Moter_control();
      }
    } else {
      getWUL();
      getData();
      Moter_control();
    }

    if (count > 100) {
      count = 0;
      Goto_sleep_now();
    }
  }
}

void Moter_control () {
  int IR_1 = digitalRead(32);
  int IR_2 = digitalRead(33);

  // Trigger GPIO 23
  digitalWrite(23, HIGH);
  //  Serial.print(IR_1);
  //  Serial.println(IR_2);

  if (GPIO14 == 1 && distance > 50) {
    se_a_s = 0;
  } else if (GPIO14 == 1 && distance < 50) {
    se_a_s = 2;
  } else if (GPIO14 == 0) {
    se_a_s = 2;
  }

  if (IR_1 == 0 && IR_2 == 0) {
    se_a = 0;
  } else if (IR_1 == 0 && IR_2 == 1) {
    se_a = 1;
  } else if (IR_1 == 1 && IR_2 == 0) {
    se_a = 1;
  } else if (IR_1 == 1 && IR_2 == 1) {
    se_a = 2;
  }
  output_A = se_a - se_a_s;

  if (output_A < 0) {
    /// เปิดขึ้น
    digitalWrite(19, LOW);
    digitalWrite(21, HIGH);
    count = 0;
  } else if (output_A > 0) {
    /// ปิดลง
    digitalWrite(19, HIGH);
    digitalWrite(21, LOW);
    count = 0;
  } else {
    digitalWrite(19, LOW);
    digitalWrite(21, LOW);
    digitalWrite(23, LOW);
    Serial.print(se_a);
    Serial.print(",");
    Serial.println(se_a_s);
    state++;
  }
}

void getVoltage () {
  VP = analogRead(36);
  VP = (VP / 4096) * 6.6;
  Serial.print("Your battery voltage : ");
  Serial.print(VP);
  Serial.println("v");
}

void getWUL () {
  digitalWrite(27, HIGH);

  digitalWrite(trigPin, LOW);
  delayMicroseconds(10);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(50);

  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  //Serial.println(duration);
  distance = duration / 58.2;
  if (distance == 0) {

  }
  Serial.print("Measuring distance : ");
  Serial.print(distance);
  Serial.println(" cm");
}

void Goto_sleep_now () {
  //Go to sleep now
  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
}
