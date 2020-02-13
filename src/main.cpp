#include <ESP8266WiFi.h>
#include <Hash.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>

// TODO: Replace with correct values
const char *ssid = "";
const char *password = "";
const char *token = "";
const char *url = "";

#define DHTPIN 12    // Digital pin connected to the DHT sensor - GPIO12 = D6
#define ARRAY_MAX 60 // Max values to store in order to find an average and store in DB.

// Uncomment the type of sensor in use:
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// hourly temperature & humidity, updated in loop()
float hourly_temp = 0.0;
float hourly_hum = 0.0;

// minute array of temperature & humidity
float minute_temp[ARRAY_MAX];
float minute_hum[ARRAY_MAX];
unsigned int temp_arr_index = 0;
unsigned int hum_arr_index = 0;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0; // will store last time DHT was updated

// Updates DHT readings every 60 seconds
const long interval = 60000;

float average(const float v[], int n)
{
  float sum = 0.0f;

  for (int i = 0; i < n; i++)
  {
    sum += v[i]; //sum all the numbers in the vector v
  }

  return sum / n;
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println(".");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop()
{
  unsigned long currentMillis = millis();
  boolean sendData = false;

  if (currentMillis - previousMillis >= interval)
  {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;

    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();

    // if temperature read failed, don't change t value
    if (isnan(newT))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
      if (temp_arr_index < ARRAY_MAX)
      {
        minute_temp[temp_arr_index++] = newT;
        Serial.print("New Temperature ");
        Serial.println(newT);
      }
      else
      {
        // Handle a full array.
        temp_arr_index = 0;
        sendData = true;
        hourly_temp = average(minute_temp, ARRAY_MAX);
        Serial.print("Average Temperature ");
        Serial.println(hourly_temp);
      }
    }

    // Read Humidity
    float newH = dht.readHumidity();

    // if humidity read failed, don't change h value
    if (isnan(newH))
    {
      Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
      if (hum_arr_index < ARRAY_MAX)
      {
        minute_hum[hum_arr_index++] = newH;
        Serial.print("New Humidity ");
        Serial.println(newH);
      }
      else
      {
        // Handle a full array.
        hum_arr_index = 0;
        sendData = true;
        hourly_hum = average(minute_hum, ARRAY_MAX);
        Serial.print("Average Humidity ");
        Serial.println(hourly_hum);
      }
    }

    if (sendData)
    {
      HTTPClient http; //Declare object of class HTTPClient

      //Post Data
      String postData = "token=" + String(token) + "&date=" + currentMillis + "&temperature=" + hourly_temp + "&humidity=" + hourly_hum;
      http.begin(url);                                                     //Specify request destination
      http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header

      int httpCode = http.POST(postData); //Send the request
      String payload = http.getString();  //Get the response payload

      Serial.println(httpCode); //Print HTTP return code
      Serial.println(payload);  //Print request response payload

      http.end(); //Close connection
    }
  }
}