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
int TimeoutBZ = 0;
//////////////////////////////////

///// Moter //////////////////////
int se_a;
int se_a_s;
int output_A;
int GPIO14;
int state = 0;
int PLState = 1;
//////////////////////////////////

char readChar;
float VP, checkVolt;
int triggeredPin;
unsigned long previousMillis = 0, previousMillisCount = 0;
unsigned long previousMillisBZ = 0, previousMillisBZ02 = 0, previousMillisBZ03 = 0;

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
  Serial.println("IR 1 : ");
  Serial.println(IR_1);
  Serial.println("IR 2 : ");
  Serial.println(IR_2);

  state = 0;

  if (GPIO14 == 1 && IR_1 == 1 && IR_2 == 1) {
    Serial.println("Wake up goto Top");
    getWUL();
    while (state < 1) {
      getData();
      int IR_1 = digitalRead(32);
      int IR_2 = digitalRead(33);
      getVoltage();
      if (checkVolt >= 4.00) {
        PLState = 1;
        SendAllData();
        Moter_control();
      } else {
        Serial.println("Battery DOWN !!!");
        PLState = 1;
        SendAllData();
        delay(1000);
        Serial.println("Now Going to Sleep!");
        Goto_sleep_now();
      }
    }
  } else if (GPIO14 == 1 && IR_1 == 0 && IR_2 == 0) {
    Serial.println("Wake up stay on Top");
    PLState = 1;
    SendAllData();
  } else {
    Serial.println("Wake up stay DOWN");
    getWUL();
    getVoltage();
    PLState = 1;
    SendAllData();
    getData();
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
  //GPIO23 Trigger moter                       ////////////////////////////////
  //  pinMode(23, OUTPUT);
  //  digitalWrite(23, HIGH);
  //  delay(1000);

  //Serial config
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  //Print the Pins that triggered to wake up.
  print_GPIO_wake_up();

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
  /////////////////////////////////////////////////////////////////////////////

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  getData();

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

void pushStatePL () {
  Serial2.println('D');
}

void SendAllData () {
  String SendAll = 'V' + String(VP) + 'U' + String(distance) + 'S' + String(PLState) + '>';
  Serial2.println(SendAll);
}

void getData () {
  if (Serial2.available() > 0) {
    Serial.println("Wait command...");
    readChar = Serial2.read();
    Serial.println(readChar);
  }
  if (readChar == 'S') {
      Goto_sleep_now();
  }
}

void loop() {
  getData();
  wait_command();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    countLoop++;
  }
  if (triggeredPin == 2) {
    getData();
    countLoop = 0;
  } else if (triggeredPin == 15) {
    getData();
    countLoop = 0;
  }

  if (countLoop >= 2) {
    Goto_sleep_now();
  }
}

void wait_command () {
  if (triggeredPin == 2) {
    getData();
    unsigned long currentMillisBZ = millis();
    if (currentMillisBZ - previousMillisBZ >= 500) {
      previousMillisBZ = currentMillisBZ;
      getData();
      digitalWrite(22, HIGH);
      Serial.println("alert 1");
    }
    if (currentMillisBZ - previousMillisBZ02 >= 1000) {
      previousMillisBZ02 = currentMillisBZ;
      getData();
      digitalWrite(22, LOW);
      Serial.println("alert 2");
      TimeoutBZ++;
    }
    if (currentMillisBZ - previousMillisBZ03 >= 5000) {
      previousMillisBZ03 = currentMillisBZ;
      getData();
      digitalWrite(22, LOW);
      Serial.println("alert 3");
      TimeoutBZ++;
    }

    if (TimeoutBZ == 1) {
      PLState = 1;
      SendAllData();
    }
    
    if (TimeoutBZ == 180) {
      Goto_sleep_now();
    }

  } else if (triggeredPin == 15) {
    unsigned long currentMillisCount = millis();
    if (currentMillisCount - previousMillisCount >= 1000) {
      previousMillisCount = currentMillisCount;
      //Serial.println(count);
      count++;
    }

    int IR_1 = digitalRead(32);
    int IR_2 = digitalRead(33);

    getData();
    getVoltage();

    if (GPIO14 == 1) {
      if (IR_1 == 0 && IR_2 == 0) {
        getVoltage();
        getData();
        digitalWrite(19, LOW);
        digitalWrite(21, LOW);
      } else if (IR_1 == 1 && IR_2 == 0) {
        getVoltage();
        getData();
        if (checkVolt >= 4.00) {
          Moter_control();
        } else {
          Serial.println("Battery DOWN !!!");
          PLState = 1;
          SendAllData();
          Serial.println("Now Going to Sleep!");
          Goto_sleep_now();
        }
      } else if (IR_1 == 0 && IR_2 == 1) {
        getData();
        getVoltage();
        if (checkVolt >= 4.00) {
          Moter_control();
        } else {
          Serial.println("Battery DOWN !!!");
          PLState = 1;
          SendAllData();
          Serial.println("Now Going to Sleep!");
          Goto_sleep_now();
        }
      } else if (IR_1 == 1 && IR_2 == 1) {
        getWUL();
        getVoltage();
        getData();
        if (checkVolt >= 4.00) {
          Moter_control();
        } else {
          Serial.println("Battery DOWN !!!");
          PLState = 1;
          SendAllData();
          Serial.println("Now Going to Sleep!");
          Goto_sleep_now();
        }
      }
    } else {
      getWUL();
      getVoltage();
      getData();
      if (checkVolt >= 4.00) {
        Moter_control();
      }
    }

    if (count > 3) {
      count = 0;
      Goto_sleep_now();
    }
  }
}

void Moter_control () {
  int IR_1 = digitalRead(32);
  int IR_2 = digitalRead(33);
  Serial.println("IR 1 : ");
  Serial.println(IR_1);
  Serial.println("IR 2 : ");
  Serial.println(IR_2);
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
    Serial.print(se_a);
    Serial.print(",");
    Serial.println(se_a_s);
    state++;
  }
}

void getVoltage () {
  VP = analogRead(36);
  VP = (VP / 4095) * 6.6;
  checkVolt = VP;
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
  Serial.println("Going to sleep now");
  PLState = 0;
  SendAllData();
  delay(2000);
  esp_deep_sleep_start();
}
