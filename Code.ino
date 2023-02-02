//*-----------------------------------------------*
//|-----------Groce Stack (Ysh) Ver 1.0-----------|
//*-----------------------------------------------*

//Defining the constants
//Designed and programmed by Yogeshwar G

#define RST_PIN D1 
#define SS_PIN D0  
#define FIREBASE_HOST "XXXXXXXX-XXXXX-XXXXXXX.firebaseio.com"
#define FIREBASE_AUTH "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
#define WIFI_SSID "XXX"
#define WIFI_PASSWORD "XXXXXXXXXXXXXXX"
const long utcOffsetInSeconds = 19080;  

//Header Files
#include <ESP8266WiFi.h>
#include <FB_Const.h>
#include <FB_Error.h>
#include <Firebase.h>
#include <FirebaseESP8266.h> 
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include<ctime>

//Creating Objects from the library

MFRC522 mfrc522(SS_PIN, RST_PIN); 
FirebaseData firebaseData;
FirebaseData statusData;
FirebaseJson json;
LiquidCrystal_I2C lcd(0x27,16,2);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

struct Data {
  String nameProduct;
  String startDate;
  String endDate;
  String quantity;
  String QPS;
  };

struct Date {
  int d, m, y;
};
const int monthDays[12] = {31, 28, 31, 30, 31, 30,
               31, 31, 30, 31, 30, 31};

String content = "";


void setup()
{
  //Setup of components, UDP, NTP
  Wire.begin(D3, D4);
  Serial.begin(115200);
  lcd.init();                 
  lcd.clear();
  lcd.backlight();   
  SPI.begin();        
  mfrc522.PCD_Init();
  Serial.println(F("RFID Ready"));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("*");
        delay(50);
    }
    Serial.println("");
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Serial.println("FIREBASE CONNECTED");
    timeClient.begin();
    timeClient.setTimeOffset(utcOffsetInSeconds);
    Serial.println();
    
}

int countLeapYears(Date d)
{
  int years = d.y;
  if (d.m <= 2)
    years--;

  return years / 4 - years / 100 + years / 400;
}

int getDifference(Date dt1, Date dt2)
{
  long int n1 = dt1.y * 365 + dt1.d;
  for (int i = 0; i < dt1.m - 1; i++)
    n1 += monthDays[i];
   n1 += countLeapYears(dt1);

  long int n2 = dt2.y * 365 + dt2.d;
  for (int i = 0; i < dt2.m - 1; i++)
    n2 += monthDays[i];
  n2 += countLeapYears(dt2);
  return (n2 - n1);
}

void LCDFlush() {
    lcd.setCursor(0,0);
    lcd.print("                      ");
    lcd.setCursor(0,1);
    lcd.print("                      ");
    delay(10);
}

void LCD(String placeHolder, String information) {
  lcd.setCursor(0, 0);  
  lcd.print(placeHolder);
  lcd.setCursor(0, 1);  
  lcd.print(information);
  delay(6000);
  LCDFlush();
}

void getData(String ID) {

  LCD("Reading Product Details","From Web Console");
  
  struct Data data;
  int readFlag = 0;
  int writeFlag = 0;
  if (Firebase.getString(statusData, "/Details/Name")){
      data.nameProduct = statusData.stringData();
      readFlag = 1;
  }
  if (Firebase.getString(statusData, "/Details/StartDate")){
      data.startDate = statusData.stringData();
      readFlag = 1;
  }
  if (Firebase.getString(statusData, "/Details/EndDate")){
      data.endDate = statusData.stringData();
      readFlag = 1;
  }
  if (Firebase.getString(statusData, "/Details/Quantity")){
      data.quantity = statusData.stringData();
      readFlag = 1;
  }
  if (Firebase.getString(statusData, "/Details/QPS")){      
      data.QPS = statusData.stringData();
      readFlag = 1;
  }

   LCD("Writing Details","To The Lid");
  if(readFlag == 0)
   Serial.println("Error Retriving Data");
   Serial.println("Retrived Data");
   Serial.println(data.nameProduct);
   Serial.println(data.startDate);
   Serial.println(data.endDate);
   Serial.println(data.quantity);
   Serial.println(data.QPS);
   String NameURL = "/" + ID + "/Name";
   String SDURL = "/" + ID + "/StartDate";
   String EDURL = "/" + ID + "/EndDate";
   String QURL = "/" + ID + "/Quantity";
   String QPSURL = "/" + ID + "/QPS";

   int QPS = data.quantity.toInt() * ((float)(data.QPS.toInt() / (float)(100)));
   Serial.println(QPS);
   data.QPS = String(QPS);
   
   if(Firebase.setString(firebaseData, NameURL, data.nameProduct)){
      writeFlag = 1;
   }
   if(Firebase.setString(firebaseData, SDURL, data.startDate)){
      writeFlag = 1;
   }
   if(Firebase.setString(firebaseData, EDURL, data.endDate)){
      writeFlag = 1;
   }
   if(Firebase.setString(firebaseData, QURL, data.quantity)){
      writeFlag = 1;
   }
   if(Firebase.setString(firebaseData, QPSURL, data.QPS)){
      writeFlag = 1;
   }

   if(writeFlag == 0)
      Serial.println("Error Writing Data");
    LCD("Write","Successful");
}

void checkStatus(String ID) {
  LCD("Reading Details","From Cloud DB");
  
  struct Data data;
  int readFlag = 0;
  int writeFlag = 0;

  String prefix = "/" + ID;
  if (Firebase.getString(statusData, prefix + "/Name")){
      data.nameProduct = statusData.stringData();
      readFlag = 1;
  }
  if (Firebase.getString(statusData, prefix + "/StartDate")){
      data.startDate = statusData.stringData();
      readFlag = 1;
  }
  if (Firebase.getString(statusData, prefix + "/EndDate")){
      data.endDate = statusData.stringData();
      readFlag = 1;
  }
  if (Firebase.getString(statusData, prefix + "/Quantity")){
      data.quantity = statusData.stringData();
      readFlag = 1;
  }
  if (Firebase.getString(statusData, prefix + "/QPS")){      
      data.QPS = statusData.stringData();
      readFlag = 1;
  }

   int quantity = data.quantity.toInt();
   int QPS = data.QPS.toInt();
   float percent = (float)QPS/100;
   
   float tempquantity = (float)(quantity - QPS);
   String finalQuantity = String(tempquantity);
   
   Serial.println("Retrived Data");
   Serial.print("Product Name");
   Serial.println(data.nameProduct);

   Serial.print("Expiry Date");
   Serial.println(data.endDate);
   
   Serial.print("Final Quantity");
   Serial.println(finalQuantity);
  struct Date d1;
  struct Date d2;

  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  
    d1.d =  ptm->tm_mday;
    d1.m = ptm->tm_mon+1;
    d1.y = ptm->tm_year+1900;

  Serial.print("Current Date :");
  Serial.print(d1.d);
  Serial.print(d1.m);
  Serial.println(d1.y);
  
  d2.y = data.endDate.substring(0,4).toInt();
  d2.m = data.endDate.substring(5.7).toInt();
  d2.d = data.endDate.substring(8,10).toInt();

  Serial.print("Exipry Date :");
  Serial.print(d2.d);
  Serial.print(d2.m);
  Serial.println(d2.y);

  if(d1.y == 0) {
    LCD("ERROR:","Wrong Lid");
  }
  
  else {
      int diff = getDifference(d1,d2);
      Serial.print("Number Of Days Left:");
      Serial.println(diff);
      
      LCD("Groce-Stack","Details");
      LCD("Product Name:",data.nameProduct);
      LCD("Date of Exp:",data.endDate);
      LCD("Quantity (g):",finalQuantity);

      if(diff > 365) {
          LCD("ALERT!","Wrong Date");  
      }
      else {
          LCD("Remaining Days",String(diff));
      }
    
      if(diff <= 15) {
          LCD("ALERT!","ABOUT TO EXP");
          LCD("ALERT!","ABOUT TO EXP");
          LCD("ALERT!","ABOUT TO EXP");
      }
           
      if(finalQuantity.toInt() <= (quantity * QPS * 0.01)) {
        LCD("ALERT!","LOW STOCK");
        LCD("ALERT!","LOW STOCK");
        LCD("ALERT!","LOW STOCK");
      }
  }
  
  int writeFlagg = 0;
  String QURL = "/" + ID + "/Quantity";
  if(Firebase.setString(firebaseData, QURL, finalQuantity)){
      writeFlagg = 1;
   }

   if(writeFlagg == 1)
    Serial.println("Fetch Updated Successfully");
}
void scanRFID(String *ID)
{
    content.clear();
    byte letter;
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return;
    }
    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
    {
        return;
    }
    Serial.println("UID tag :");

    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    *ID = content.substring(1);
    delay(100);
    Serial.println("");
}


int optionGet (String ID) {
  if(ID == "53 04 14 11")
    return 1;
   return 2;
}
void loop()
{
    String ID = "";
    String optionKey = "";
    int option = 0;
    ID.clear();
    
    Serial.println("\n\n");
    Serial.println("---------------GROCE-STACK---------------");
    Serial.println("-----------------------------------------");
    Serial.println("\n\n");

    LCD("Groce-Stack","Kitchen Partner");
    delay(1000);
    LCD("1.Write with Key","2.Fetch Details");
    while (optionKey == "")
    {
      scanRFID(&optionKey);
    }
    option = optionGet(optionKey);
    delay(5000);
    switch (option)
    {
    case 1:
    {
        LCD("Writing From:","Web Console");
        delay(1000);
        LCD("Scan The Top Of","GroceStack Lid");
        ID.clear();
        delay(400);
        scanRFID(&ID);
        delay(1000);
        getData(ID);
        delay(1000);
        break;
    }
    case 2:
    {
        LCD("Fetching GroceStack","Lid Details");      
        LCD("Scan The Top Of","GroceStack Lid");
        ID.clear();
        scanRFID(&ID);
        delay(1000);
        checkStatus(ID);
        break;
    }
    default:
        Serial.println("Wrong Choice");
    }
}
