#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include <ESP32Servo.h>

// =====================================================
// KHAI BAO CHAN
// =====================================================

// AS608
#define FINGER_RX 16
#define FINGER_TX 17

// OLED
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_ADDR 0x3C

// SERVO
#define SERVO_PIN 18

// NUT NHAN
#define BUTTON_PIN 27

// COI
#define BUZZER_PIN 26


// =====================================================
// CAU HINH SERVO
// =====================================================
//
// Theo hien tuong servo cua ban dang quay lien tuc,
// ta dieu khien no nhu servo 360 do.
//
// 1500 us = DUNG
//  >1500 us = quay mot chieu
//  <1500 us = quay chieu nguoc lai
//
// Neu chieu quay nguoc, doi OPEN_SPEED va CLOSE_SPEED.
//

#define SERVO_STOP       1500

#define OPEN_SPEED       1300
#define CLOSE_SPEED      1700

// Thoi gian servo quay de mo/khoa
// Can chinh tuy theo co cau khoa cua ban.
#define OPEN_TIME        3000
#define CLOSE_TIME       3000

// Thoi gian cua mo
#define DOOR_OPEN_TIME   5000


// =====================================================
// OLED
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);
// =====================================================
// AS608
// =====================================================

HardwareSerial FingerSerial(2);

Adafruit_Fingerprint finger(
  &FingerSerial
);
// =====================================================
// SERVO
// =====================================================
Servo doorServo;
// =====================================================
// BIEN
// =====================================================
bool scanEnabled = true;
int wrongCount = 0;
unsigned long lastButtonTime = 0;
const unsigned long DEBOUNCE_TIME = 300;
// =====================================================
// HIEN THI OLED
// =====================================================
void showOLED(String line1, String line2 = "")
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 10);
  display.println(line1);

  display.setCursor(0, 30);
  display.println(line2);

  display.display();
}


// =====================================================
// COI
// =====================================================

void buzzer(int time_ms)
{
  digitalWrite(
    BUZZER_PIN,
    HIGH
  );

  delay(time_ms);

  digitalWrite(
    BUZZER_PIN,
    LOW
  );
}


// =====================================================
// SERVO DUNG
// =====================================================

void servoStop()
{
  doorServo.writeMicroseconds(
    SERVO_STOP
  );

  delay(100);
}


// =====================================================
// MO KHOA
// =====================================================

void unlockDoor()
{
  Serial.println();
  Serial.println("==============================");
  Serial.println("VAN TAY DUNG!");
  Serial.println("DANG MO KHOA");
  Serial.println("==============================");


  // ---------------------------------------------------
  // QUAY SERVO DE MO KHOA
  // ---------------------------------------------------

  showOLED(
    "VAN TAY DUNG!",
    "DANG MO KHOA"
  );

  Serial.println("Servo dang mo...");

  doorServo.writeMicroseconds(
    OPEN_SPEED
  );

  delay(OPEN_TIME);


  // ---------------------------------------------------
  // DUNG SERVO
  // ---------------------------------------------------

  servoStop();

  Serial.println("DA MO KHOA");


  // ---------------------------------------------------
  // GIU CUA MO
  // ---------------------------------------------------

  showOLED(
    "DA MO KHOA",
    "xin mời thầy vào ..."
  );

  delay(DOOR_OPEN_TIME);


  // ---------------------------------------------------
  // QUAY SERVO DE KHOA
  // ---------------------------------------------------

  showOLED(
    "DANG KHOA",
    "Vui long cho"
  );

  Serial.println("Dang khoa cua...");

  doorServo.writeMicroseconds(
    CLOSE_SPEED
  );

  delay(CLOSE_TIME);


  // ---------------------------------------------------
  // DUNG SERVO
  // ---------------------------------------------------

  servoStop();

  Serial.println("CUA DA KHOA");


  showOLED(
    "DA KHOA",
    "Dat ngon tay"
  );

  delay(1000);
}


// =====================================================
// NUT NHAN
// =====================================================

void checkButton()
{
  static bool lastButtonState = HIGH;

  bool currentButtonState =
    digitalRead(BUTTON_PIN);


  // Phat hien nhan nut
  if (
    lastButtonState == HIGH &&
    currentButtonState == LOW
  )
  {
    if (
      millis() - lastButtonTime >
      DEBOUNCE_TIME
    )
    {
      lastButtonTime = millis();

      // Dao trang thai quet
      scanEnabled = !scanEnabled;


      if (scanEnabled)
      {
        Serial.println();
        Serial.println(
          "QUET VAN TAY: BAT"
        );

        showOLED(
          "QUET VAN TAY",
          "DANG BAT"
        );

        delay(500);

        showOLED(
          "SAN SANG",
          "Dat ngon tay"
        );
      }
      else
      {
        Serial.println();
        Serial.println(
          "QUET VAN TAY: TAT"
        );

        showOLED(
          "QUET VAN TAY",
          "DANG TAT"
        );
      }
    }
  }

  lastButtonState =
    currentButtonState;
}


// =====================================================
// KIEM TRA VAN TAY
// =====================================================

void checkFingerprint()
{
  uint8_t p;


  // ---------------------------------------------------
  // LAY ANH
  // ---------------------------------------------------

  p = finger.getImage();


  if (p == FINGERPRINT_NOFINGER)
  {
    return;
  }


  if (p != FINGERPRINT_OK)
  {
    Serial.println(
      "LOI LAY ANH VAN TAY"
    );

    return;
  }


  Serial.println(
    "DA NHAN VAN TAY"
  );


  // ---------------------------------------------------
  // CHUYEN ANH THANH MAU
  // ---------------------------------------------------

  p = finger.image2Tz();


  if (p != FINGERPRINT_OK)
  {
    Serial.println(
      "LOI XU LY VAN TAY"
    );

    showOLED(
      "LOI VAN TAY",
      "THU LAI"
    );

    delay(1000);

    showOLED(
      "SAN SANG",
      "Dat ngon tay"
    );

    return;
  }


  // ---------------------------------------------------
  // TIM VAN TAY
  // ---------------------------------------------------

  p = finger.fingerSearch();


  // ===================================================
  // VAN TAY DUNG
  // ===================================================

  if (p == FINGERPRINT_OK)
  {
    Serial.println();
    Serial.println("==============================");
    Serial.println("VAN TAY DUNG!");

    Serial.print("ID: ");
    Serial.println(
      finger.fingerID
    );

    Serial.print("DO TIN CAY: ");
    Serial.println(
      finger.confidence
    );

    Serial.println("==============================");


    // Reset so lan sai
    wrongCount = 0;


    // Mo cua
    unlockDoor();


    return;
  }


  // ===================================================
  // VAN TAY SAI
  // ===================================================

  if (p == FINGERPRINT_NOTFOUND)
  {
    wrongCount++;


    Serial.println();
    Serial.println(
      "VAN TAY SAI"
    );

    Serial.print(
      "LAN SAI: "
    );

    Serial.println(
      wrongCount
    );


    // -------------------------------------------------
    // SAI LAN 1
    // -------------------------------------------------

    if (wrongCount == 1)
    {
      showOLED(
        "VAN TAY SAI!",
        "LAN 1"
      );

      delay(1500);
    }


    // -------------------------------------------------
    // SAI LAN 2
    // -------------------------------------------------

    else if (wrongCount == 2)
    {
      showOLED(
        "VAN TAY SAI!",
        "LAN 2"
      );

      delay(1500);
    }


    // -------------------------------------------------
    // SAI 3 LAN
    // -------------------------------------------------

    else if (wrongCount >= 3)
    {
      Serial.println(
        "CANH BAO SAI 3 LAN!"
      );


      showOLED(
        "CANH BAO!",
        "SAI 3 LAN"
      );


      // Coi keu 2 giay
      buzzer(2000);


      // Reset
      wrongCount = 0;


      showOLED(
        "DA RESET",
        "Dat ngon tay"
      );

      delay(1500);
    }


    showOLED(
      "SAN SANG",
      "Dat ngon tay"
    );

    return;
  }
}


// =====================================================
// MENU SERIAL
// =====================================================

void printMenu()
{
  Serial.println();
  Serial.println("================================");
  Serial.println("          MENU AS608");
  Serial.println("================================");

  Serial.println(
    "1 - NAP VAN TAY"
  );

  Serial.println(
    "2 - XOA VAN TAY THEO ID"
  );

  Serial.println(
    "3 - XOA TAT CA VAN TAY"
  );

  Serial.println(
    "4 - XEM SO VAN TAY"
  );

  Serial.println(
    "================================"
  );

  Serial.println(
    "Nhap lua chon:"
  );
}


// =====================================================
// XEM SO VAN TAY
// =====================================================

void showTemplateCount()
{
  uint8_t p =
    finger.getTemplateCount();


  if (p == FINGERPRINT_OK)
  {
    Serial.println();

    Serial.println(
      "=============================="
    );

    Serial.print(
      "SO VAN TAY DA LUU: "
    );

    Serial.println(
      finger.templateCount
    );

    Serial.println(
      "=============================="
    );
  }
  else
  {
    Serial.println(
      "Khong doc duoc bo nho AS608!"
    );
  }
}


// =====================================================
// NAP VAN TAY
// =====================================================

bool enrollFingerprint(
  uint8_t id
)
{
  int p = -1;


  Serial.println();
  Serial.println(
    "=============================="
  );

  Serial.print(
    "DANG NAP ID: "
  );

  Serial.println(id);

  Serial.println(
    "=============================="
  );


  // ---------------------------------------------------
  // LAN 1
  // ---------------------------------------------------

  Serial.println(
    "Dat ngon tay lan 1..."
  );

  showOLED(
    "NAP VAN TAY",
    "Dat ngon tay"
  );


  while (
    p != FINGERPRINT_OK
  )
  {
    p =
      finger.getImage();


    if (
      p == FINGERPRINT_NOFINGER
    )
    {
      delay(100);
      continue;
    }


    if (
      p == FINGERPRINT_OK
    )
    {
      Serial.println(
        "Da chup lan 1!"
      );
    }
    else
    {
      Serial.println(
        "Loi chup anh lan 1!"
      );

      return false;
    }
  }


  p =
    finger.image2Tz(1);


  if (
    p != FINGERPRINT_OK
  )
  {
    Serial.println(
      "Loi xu ly anh lan 1!"
    );

    return false;
  }


  // ---------------------------------------------------
  // NHAC NGON TAY
  // ---------------------------------------------------

  Serial.println(
    "Hay nhac ngon tay ra..."
  );

  showOLED(
    "NAP VAN TAY",
    "Nhat ngon tay ra"
  );

  delay(1500);


  p = 0;


  while (
    p != FINGERPRINT_NOFINGER
  )
  {
    p =
      finger.getImage();

    delay(100);
  }


  // ---------------------------------------------------
  // LAN 2
  // ---------------------------------------------------

  Serial.println(
    "Dat lai cung ngon tay..."
  );

  showOLED(
    "NAP VAN TAY",
    "Dat lai ngon tay"
  );


  p = -1;


  while (
    p != FINGERPRINT_OK
  )
  {
    p =
      finger.getImage();


    if (
      p == FINGERPRINT_NOFINGER
    )
    {
      delay(100);
      continue;
    }


    if (
      p == FINGERPRINT_OK
    )
    {
      Serial.println(
        "Da chup lan 2!"
      );
    }
    else
    {
      Serial.println(
        "Loi chup anh lan 2!"
      );

      return false;
    }
  }


  p =
    finger.image2Tz(2);


  if (
    p != FINGERPRINT_OK
  )
  {
    Serial.println(
      "Loi xu ly anh lan 2!"
    );

    return false;
  }


  // ---------------------------------------------------
  // SO SANH
  // ---------------------------------------------------

  Serial.println(
    "Dang so sanh..."
  );


  p =
    finger.createModel();


  if (
    p != FINGERPRINT_OK
  )
  {
    Serial.println(
      "HAI LAN QUET KHONG GIONG!"
    );

    showOLED(
      "NAP THAT BAI",
      "Khong trung khop"
    );

    delay(1500);

    return false;
  }


  Serial.println(
    "Hai lan quet trung khop!"
  );


  // ---------------------------------------------------
  // LUU
  // ---------------------------------------------------

  p =
    finger.storeModel(id);


  if (
    p == FINGERPRINT_OK
  )
  {
    Serial.println();
    Serial.println(
      "NAP VAN TAY THANH CONG!"
    );

    Serial.print(
      "ID = "
    );

    Serial.println(id);


    showOLED(
      "NAP THANH CONG!",
      "ID: " + String(id)
    );

    delay(2000);

    return true;
  }
  else
  {
    Serial.println(
      "LOI KHI LUU VAN TAY!"
    );

    showOLED(
      "NAP THAT BAI",
      "Loi luu"
    );

    delay(1500);

    return false;
  }
}


// =====================================================
// XOA 1 VAN TAY
// =====================================================

void deleteFingerprint(
  uint8_t id
)
{
  Serial.println();

  Serial.print(
    "Dang xoa ID = "
  );

  Serial.println(id);


  uint8_t p =
    finger.deleteModel(id);


  if (
    p == FINGERPRINT_OK
  )
  {
    Serial.println(
      "XOA THANH CONG!"
    );

    showOLED(
      "XOA THANH CONG",
      "ID: " + String(id)
    );

    delay(1500);
  }
  else
  {
    Serial.println(
      "XOA THAT BAI!"
    );

    showOLED(
      "XOA THAT BAI",
      "ID: " + String(id)
    );

    delay(1500);
  }
}


// =====================================================
// XOA TAT CA VAN TAY
// =====================================================

void deleteAllFingerprints()
{
  Serial.println();
  Serial.println(
    "=============================="
  );

  Serial.println(
    "CANH BAO!"
  );

  Serial.println(
    "XOA TOAN BO VAN TAY"
  );

  Serial.println(
    "Nhap Y de xac nhan"
  );

  Serial.println(
    "Nhap N de huy"
  );


  showOLED(
    "XOA TAT CA?",
    "Y = Xoa / N = Huy"
  );


  while (true)
  {
    if (Serial.available())
    {
      char c =
        Serial.read();


      if (
        c == 'Y' ||
        c == 'y'
      )
      {
        uint8_t p =
          finger.emptyDatabase();


        if (
          p == FINGERPRINT_OK
        )
        {
          Serial.println(
            "DA XOA TOAN BO VAN TAY!"
          );

          showOLED(
            "DA XOA TAT CA",
            "Bo nho trong"
          );
        }
        else
        {
          Serial.println(
            "XOA THAT BAI!"
          );

          showOLED(
            "XOA THAT BAI",
            "Thu lai"
          );
        }


        delay(2000);

        break;
      }


      if (
        c == 'N' ||
        c == 'n'
      )
      {
        Serial.println(
          "DA HUY XOA"
        );

        showOLED(
          "DA HUY",
          "Khong xoa"
        );

        delay(1000);

        break;
      }
    }

    delay(20);
  }
}


// =====================================================
// XU LY MENU
// =====================================================

void serialMenu()
{
  if (!Serial.available())
  {
    return;
  }


  char command =
    Serial.read();


  // ---------------------------------------------------
  // 1 = NAP
  // ---------------------------------------------------

  if (
    command == '1'
  )
  {
    scanEnabled = false;


    Serial.println();
    Serial.println(
      "Nhap ID muon nap (1-127):"
    );


    while (
      !Serial.available()
    )
    {
      delay(20);
    }


    int id =
      Serial.parseInt();


    if (
      id < 1 ||
      id > 127
    )
    {
      Serial.println(
        "ID khong hop le!"
      );

      scanEnabled = true;

      printMenu();

      return;
    }


    enrollFingerprint(
      id
    );


    scanEnabled = true;


    showOLED(
      "SAN SANG",
      "Dat ngon tay"
    );


    printMenu();
  }


  // ---------------------------------------------------
  // 2 = XOA ID
  // ---------------------------------------------------

  else if (
    command == '2'
  )
  {
    scanEnabled = false;


    Serial.println();
    Serial.println(
      "Nhap ID muon xoa (1-127):"
    );


    while (
      !Serial.available()
    )
    {
      delay(20);
    }


    int id =
      Serial.parseInt();


    if (
      id < 1 ||
      id > 127
    )
    {
      Serial.println(
        "ID khong hop le!"
      );

      scanEnabled = true;

      printMenu();

      return;
    }


    deleteFingerprint(
      id
    );


    scanEnabled = true;


    showOLED(
      "SAN SANG",
      "Dat ngon tay"
    );


    printMenu();
  }


  // ---------------------------------------------------
  // 3 = XOA TAT CA
  // ---------------------------------------------------

  else if (
    command == '3'
  )
  {
    scanEnabled = false;


    deleteAllFingerprints();


    scanEnabled = true;


    showOLED(
      "SAN SANG",
      "Dat ngon tay"
    );


    printMenu();
  }


  // ---------------------------------------------------
  // 4 = XEM SO
  // ---------------------------------------------------

  else if (
    command == '4'
  )
  {
    showTemplateCount();

    printMenu();
  }
}


// =====================================================
// SETUP
// =====================================================

void setup()
{
  // ---------------------------------------------------
  // SERIAL
  // ---------------------------------------------------

  Serial.begin(115200);

  delay(1000);


  Serial.println();
  Serial.println(
    "=============================="
  );

  Serial.println(
    "      KHOA CUA VAN TAY"
  );

  Serial.println(
    "=============================="
  );


  // ---------------------------------------------------
  // OLED
  // ---------------------------------------------------

  Wire.begin(
    OLED_SDA,
    OLED_SCL
  );


  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      OLED_ADDR
    )
  )
  {
    Serial.println(
      "LOI: KHONG TIM THAY OLED!"
    );


    while (1)
    {
      delay(100);
    }
  }


  showOLED(
    "KHOA VAN TAY",
    "DANG KHOI DONG"
  );


  delay(1000);


  // ---------------------------------------------------
  // NUT NHAN
  // ---------------------------------------------------

  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );


  // ---------------------------------------------------
  // BUZZER
  // ---------------------------------------------------

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );


  digitalWrite(
    BUZZER_PIN,
    LOW
  );


  // ---------------------------------------------------
  // SERVO
  // ---------------------------------------------------

  doorServo.setPeriodHertz(
    50
  );


  doorServo.attach(
    SERVO_PIN,
    500,
    2500
  );


  // Quan trong:
  // Cho servo dung ngay khi khoi dong
  doorServo.writeMicroseconds(
    SERVO_STOP
  );


  delay(1000);


  // ---------------------------------------------------
  // AS608
  // ---------------------------------------------------

  Serial.println(
    "Dang ket noi AS608..."
  );


  FingerSerial.begin(
    57600,
    SERIAL_8N1,
    FINGER_RX,
    FINGER_TX
  );


  finger.begin(57600);


  delay(1000);


  if (
    finger.verifyPassword()
  )
  {
    Serial.println(
      "AS608 KET NOI THANH CONG!"
    );


    showOLED(
      "AS608 OK",
      "He thong san sang"
    );
  }
  else
  {
    Serial.println(
      "AS608 KHONG KET NOI!"
    );


    Serial.println(
      "Kiem tra TX/RX/VCC/GND"
    );


    showOLED(
      "AS608 LOI!",
      "Kiem tra day"
    );
  }


  // ---------------------------------------------------
  // SO VAN TAY
  // ---------------------------------------------------

  showTemplateCount();


  delay(1000);


  showOLED(
    "SAN SANG",
    "Dat ngon tay"
  );


  printMenu();
}


// =====================================================
// LOOP
// =====================================================

void loop()
{
  // Nút bật/tắt quét
  checkButton();


  // Menu Serial
  serialMenu();


  // Quét vân tay
  if (scanEnabled)
  {
    checkFingerprint();
  }


  delay(50);
}