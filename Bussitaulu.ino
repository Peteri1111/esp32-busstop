#include <HTTPClient.h>
#include <LiquidCrystal.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "config.h"


// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
unsigned long timerDelay = 0;

struct ValuePair {
  int lineNumber;
  int timeToArrival;
};

// First is line 10, second is line 6
ValuePair arrayIidesranta[5] = {
  {-1, -1},{-1, -1},{-1, -1},{-1, -1},{-1, -1}
  };


int arrayViinikanliittyma[5] = {-1};
int arrayNekala[5] = {-1};
int arraySaarioinen[5] = {-1};
int arrayIkea[5] = {-1};


unsigned long lastTime = 0;
void resetIidesrantaArray() {
  for (int i = 0; i < 5; i++) {
    arrayIidesranta[i] = {-1, -1};
}
}
void resetBusArray(int arr[], int size) {
  for (int i = 0; i < size; i++) {
    arr[i] = -1; // Set each element to 0
  }
}

void reset_lcd() {
  lcd.setCursor(0,0);
  lcd.clear();
}
void reset_lcd_up() {
  lcd.setCursor(0,0);
  lcd.print("               ");
  lcd.setCursor(0,0);

}
void reset_lcd_down() {
  lcd.setCursor(0,1);
  lcd.print("               ");
  lcd.setCursor(0,1);
}
void lcd_cursor_up() {
  lcd.setCursor(0,0);
}
void lcd_cursor_down() {
  lcd.setCursor(0,1);
}

void wifi_connect() {
  lcd.print("Connecting...");
  Serial.println("Connecting to wifi...");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd_cursor_down();
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected:");
  lcd_cursor_down();
  Serial.println(WiFi.localIP());
  lcd_cursor_up();
}

DynamicJsonDocument http_request(const char* request, DynamicJsonDocument filter) {
  HTTPClient http;
  
  http.useHTTP10(true);
  http.begin(request);
  Serial.println(request);
  
  int httpResponseCode = http.GET();
  Serial.println(httpResponseCode);

  // Check the HTTP response code
  if (httpResponseCode != HTTP_CODE_OK) {
    // Handle the error here
    Serial.print("HTTP request failed with error code: ");
    Serial.println(httpResponseCode);
    return DynamicJsonDocument(0);
  }

  // Read the response and deserialize into JSON document
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));

  // Check if deserialization was successful
  if (error) {
    // Handle the error here
    Serial.print("JSON deserialization failed with error: ");
    Serial.println(error.c_str());
    return DynamicJsonDocument(0);
  }

  // Cleanup and return the JSON document
  http.end();
  return doc;
}


void get_substring(const char* source, char* destination, int startIndex, int endIndex) {
  int length = endIndex - startIndex + 1;
  strncpy(destination, &source[startIndex], length);
  destination[length] = '\0';
}


unsigned long get_substring_iso(char t, const char* timestamp) {
  unsigned long result;
  char subst[3];
  switch (t) {
    case 'h': //11,12
      get_substring(timestamp, subst, 11, 12);
      break;
    case 'm': //14,15
      get_substring(timestamp, subst, 14, 15);
      break;
    case 's': //17,18
    get_substring(timestamp, subst, 17, 18);
      break;
     default:
      break;  
  }
  return atol(subst);
  
}
unsigned long get_time_total(const char* timestamp) {
  unsigned long t1 = get_substring_iso('h', timestamp);
  unsigned long t2 = get_substring_iso('m', timestamp);
  unsigned long t3 = get_substring_iso('s', timestamp);
  return ((t1*60*60)+(t2*60)+t3);
}

unsigned long compare_times(unsigned long timeNow, unsigned long timeFuture) {
  /*
  Serial.print("Time is now: ");
  Serial.println(timeNow);
  Serial.print("Next bus arrives at time: ");
  Serial.println(timeFuture);
  */
  
  if (timeFuture < timeNow) {
    return (timeFuture + 24 * 60 * 60) - timeNow;
  }
  return timeFuture - timeNow;
}

bool get_time(char* timestamp) {
  StaticJsonDocument<200> filter;
  filter["datetime"] = true;
  DynamicJsonDocument d = http_request(timeRequest, filter);
  
  // Check if the doc size is 0
  if (d.size() == 0) {
    // Handle the error here
    Serial.println("Error: Failed to get the time.");
    return false;
  }
  
  const char* t = d["datetime"];
  strncpy(timestamp, t, 19); // Copy the first 19 characters of the timestamp
  timestamp[19] = '\0'; // Null-terminate the string
  return true;
}

int minutes_to_arrival(const char* timestamp, const char* timeNow) {
  /*
  Serial.println(timestamp);
  Serial.println("");
  Serial.println(timeNow);
  */
  unsigned long t1 = get_time_total(timestamp);
  unsigned long t2 = get_time_total(timeNow);
  return compare_times(t2,t1);
  
}

void resetArrays() {
  resetIidesrantaArray();
  resetBusArray(arrayViinikanliittyma,5);
  resetBusArray(arrayNekala,5);
  resetBusArray(arraySaarioinen,5);
  resetBusArray(arrayIkea,5);
}

bool get_busses() {
  StaticJsonDocument<200> filter;
  filter["body"]["3500"][0]["lineRef"] = true;
  filter["body"]["3500"][0]["call"]["expectedArrivalTime"] = true;
  filter["body"]["3557"][0]["lineRef"] = true;
  filter["body"]["3557"][0]["call"]["expectedArrivalTime"] = true;
  filter["body"]["3000"][0]["lineRef"] = true;
  filter["body"]["3000"][0]["call"]["expectedArrivalTime"] = true;
  filter["body"]["3062"][0]["lineRef"] = true;
  filter["body"]["3062"][0]["call"]["expectedArrivalTime"] = true;
  filter["body"]["3557"][0]["lineRef"] = true;
  filter["body"]["3557"][0]["call"]["expectedArrivalTime"] = true;
  DynamicJsonDocument d = http_request(busStopRequest, filter);
  if (d.size() == 0) {
    return false;
  }
  JsonObject body = d["body"];
  char timeNow[20];
  bool got_time = get_time(timeNow);
  if (!got_time) { return false; }
  
  //Found data and time, so lets reset the arrays
  resetArrays();
  
  for (JsonPair keyValue : body) {
    const char* stopIdStr = keyValue.key().c_str();
    const int stopId = atoi(stopIdStr);
    JsonArray arrivals = keyValue.value().as<JsonArray>();
    Serial.print("Stop ID: ");
    Serial.println(stopId);
    
    int iterator = 0;
    for (JsonObject arrival : arrivals) {
      // Our array is full
      if (iterator >= 4) continue;
      const char* lineRef = arrival["lineRef"];
      const char* expectedArrivalTime = arrival["call"]["expectedArrivalTime"];
      unsigned long arrival_expected = minutes_to_arrival(expectedArrivalTime, timeNow);
      int expectedTime = arrival_expected/60;

      Serial.println();
      switch (stopId) {
        case 3500:
          Serial.println("Viinikan liittymä");
          arrayViinikanliittyma[iterator] = expectedTime;
          break;
        case 3557:
          Serial.println("Iidesranta");
          arrayIidesranta[iterator].lineNumber =  atoi(lineRef);
          arrayIidesranta[iterator].timeToArrival = expectedTime;
          break;
        case 3533:
          Serial.println("Saarioinen");
          arraySaarioinen[iterator] = expectedTime;
          break;
        case 3062:
          Serial.println("Nekala");
          arrayNekala[iterator] = expectedTime;
          break;
        case 3000:
          Serial.println("Viinikan liittymä");
          arrayIkea[iterator] = expectedTime;
          break;
        default: 
          break;
      }
      Serial.println("*************************************");
      Serial.print("Bus number: ");
      Serial.println(lineRef);

      Serial.print("Bus expected arrival time: ");
      Serial.println(expectedArrivalTime);

      Serial.print("Bus arrives in ");
      Serial.print(expectedTime);
      Serial.println(" min.");
      Serial.println("*************************************");
      iterator++;
    }
    Serial.println();
  }
  
  return true;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid,password);
  lcd.begin(16, 2);
  wifi_connect();

  reset_lcd();  
}
 
void loop() {
 if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if (WiFi.status()== WL_CONNECTED){
      bool success = get_busses();
      if (success) {
        Serial.println("Succesfully printed bus data!");
        reset_lcd();
        lcd.print("Iiidesranta 6&10");
        lcd_cursor_down();
        for (int i = 0; i < 5 ; i++) {
          int line = arrayIidesranta[i].lineNumber;
          int arrival = arrayIidesranta[i].timeToArrival;
          if (line == -1) continue;
          reset_lcd_down();
          
          Serial.print("Bussi numero "); 
          Serial.print(line);
          Serial.print(" Saapuu ");
          Serial.print(arrival);
          Serial.println(" min kuluttua.");
          lcd.print(line);
          lcd.print(": ");
          lcd.print(arrival);
          lcd.print(" min.");
          delay(3000);
        }
      } else {
        reset_lcd();
        lcd.print("API conn down");
        lcd_cursor_down();
        lcd.print("Retrying....");

        Serial.println("Something went wrong!");
        timerDelay = 2000;
        return;
      }
    }
    else {
      wifi_connect();
    }
    lastTime = millis();
    timerDelay = 60000;
  }
}
