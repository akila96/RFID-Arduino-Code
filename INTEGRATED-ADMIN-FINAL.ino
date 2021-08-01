#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>


#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D2
#define RST_PIN D1
MFRC522 mfrc522(SS_PIN, RST_PIN);

#define ON_Board_LED 2


const char* ssid = "JioFi3_EADD46";
const char* password = "hdv66ynvny";

ESP8266WebServer server(80);

int readsuccess;
byte readcard[4];
char str[32] = "";
String StrUID;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(D3, OUTPUT);
  digitalWrite(D3, LOW);
  delay(500);

  WiFi.begin(ssid, password);
  Serial.println("");

  pinMode(ON_Board_LED, OUTPUT);
  digitalWrite(ON_Board_LED, HIGH);


  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");

    digitalWrite(ON_Board_LED, LOW);
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
  }
  digitalWrite(ON_Board_LED, HIGH);

  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Please tag a card to see the UID ");
  Serial.println("");
}

const char *host = "http://192.168.225.238/";

void loop() {

  readsuccess = getid();

  if (readsuccess) {
    digitalWrite(ON_Board_LED, LOW);
    HTTPClient http;

    String UIDresultSend, postData;
    UIDresultSend = StrUID;


    postData = "UIDresult=" + UIDresultSend;

    http.begin("http://192.168.225.238/NodeMCU_Final/getUID.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpCode = http.POST(postData);
    String payload = http.getString();

    Serial.println(UIDresultSend);
    Serial.println(httpCode);
    Serial.println(payload);

    http.end();
    delay(1000);
    digitalWrite(ON_Board_LED, HIGH);

    String GetAddress, LinkGet, getData;
    int id = 1; //--> ID in Database
    GetAddress = "NodeMCU_Final/GetData.php";
    LinkGet = host + GetAddress; //--> Make a Specify request destination
    getData = "ID=" + String(id);
    //getData = "Stat";
    Serial.println("----------------Connect to Server-----------------");
    Serial.println("Get ID from Database");
    Serial.print("Request Link : ");
    Serial.println(LinkGet);
    http.begin(LinkGet); //--> Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");    
    int httpCodeGet = http.POST(getData); 
    String payloadGet = http.getString(); 
    Serial.print("Response Code : "); 
    Serial.println(httpCodeGet); 
    Serial.print("Returned data from Server : ");
    Serial.println(payloadGet); 

    Serial.println("----------------Closing Connection----------------");
    http.end(); 

    if (payloadGet.indexOf(StrUID) > 0) {
      digitalWrite(D3, HIGH);
      delay(1000);
      digitalWrite(D3, LOW);
      Serial.print(": Registered User, Access Granted!");
      Serial.println();

    } else {
      digitalWrite(D3, LOW);
      Serial.print(": User is not registered, Access Denied!");
      Serial.println();
    }

  }
}

int getid() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }

  Serial.print("THE UID OF THE SCANNED CARD IS : ");

  for (int i = 0; i < 4; i++) {
    readcard[i] = mfrc522.uid.uidByte[i]; //storing the UID of the tag in readcard
    array_to_string(readcard, 4, str);
    StrUID = str;
  }
  mfrc522.PICC_HaltA();
  return 1;
}


void array_to_string(byte array[], unsigned int len, char buffer[]) {  
  for (unsigned int i = 0; i < len; i++)
  {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len * 2] = '\0';
}
