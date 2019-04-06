
// commented and edited by Alaa Amed
// Wi-Fi network, password, & API keys have been omitted from the code

#include <ESP8266WiFi.h>      // Include the Wi-Fi library
#include <ESP8266HTTPClient.h> // Include this library, which provides methods to send HTTP requests.
#include <ArduinoJson.h> //provides the ability to parse and construct JSON objects

const char* ssid = "";  // The SSID (name) of the Wi-Fi network I am connected to 
const char* pass = ""; // The password of the Wi-Fi network
const char* key = ""; // API key 
String weatherKey = ""; // API key for open weather map

typedef struct { // here we create a new data type definition, a box to hold other data types
  String ip;    // structure declaration: type string 
  String cc;    //for each name:value pair coming in from the service, we will create a slot
  String cn;    //in our structure to hold our data
  String rc;
  String rn;
  String cy;
  String ln;
  String lt;
} GeoData;     //then we give our new data structure a name so we can use it in our code

typedef struct { // here we create a new data type definition, a box to hold other data types
  String tp;
  String pr;
  String hd;
  String ws;
  String wd;
  String cd;
} MetData;     //then we give our new data structure a name so we can use it in our code

GeoData location; //we have created a GeoData type, but not an instance of that type,
                  //so we create the variable 'location' of type GeoData

MetData conditions; //we have created a MetData type, but not an instance of that type,
                   //so we create the variable 'conditions' of type MetData

void setup() { // Code runs only once or whenever we restart the board
  Serial.begin(115200); // Start the Serial communication to send messages to the computer
  delay(10); // Wait 1/10 of a second 
  Serial.print("This board is running: "); // print this message 
  Serial.println(F(__FILE__)); // print this message to the serial monitor
  Serial.print("Compiled: "); // print this message to the serial monitor
  Serial.println(F(__DATE__ " " __TIME__)); // print this message to the serial monitor
  
  Serial.print("Connecting to "); Serial.println(ssid); // print this message to the serial monitor + Wi-Fi network name

  WiFi.mode(WIFI_STA); // switch Wi-Fi mode
  WiFi.begin(ssid, pass); // Connect to the network

  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(500); // Wait 1/2 a second
    Serial.print("."); // Print loading dots to indicate an action is in progress
  }

  Serial.println(); Serial.println("WiFi connected"); Serial.println(); // if connected to the Wi-Fi, print this message 
  Serial.print("Your ESP has been assigned the internal IP address ");
  Serial.println(WiFi.localIP()); // Print IP address to the serial monitor

  getGeo(); // Function call: go run the code in getGeo
  
  // Print GeoData obtained to the serial monitor
  Serial.println("Your external IP address is " + location.ip);
  Serial.print("Your ESP is currently in " + location.cn + " (" + location.cc + "),");
  Serial.println(" in or near " + location.cy + ", " + location.rc + ".");
  Serial.println("and located at (roughly) ");
  Serial.println(location.lt + " latitude by " + location.ln + " longitude."); 

  getMet(location.cy); // Function call: go run the code in getMet

  // Print weather data obtained to the serial monitor
  Serial.println();
  Serial.println("With " + conditions.cd + ", the temperature in " + location.cy + ", " + location.rc);
  Serial.println("is " + conditions.tp + "F, with a humidity of " + conditions.hd + "%. The winds are blowing");
  Serial.println(conditions.wd + " at " + conditions.ws + " miles per hour, and the ");
  Serial.println("barometric pressure is at " + conditions.pr + " millibars.");
}

void loop() {
  // we don't want to repeat anything so leave the loop empty 
}

String getIP() { // get IP address
  HTTPClient theClient;
  String ipAddress;

  theClient.begin("http://api.ipify.org/?format=json");
  int httpCode = theClient.GET();

  if (httpCode > 0) {
    if (httpCode == 200) {

      DynamicJsonBuffer jsonBuffer;

      String payload = theClient.getString();
      JsonObject& root = jsonBuffer.parse(payload);
      ipAddress = root["ip"].as<String>();

    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
      return "error";
    }
  }
  return ipAddress; // return IP address
}

void getGeo() {   // function called getGeo that provides geo data 
  HTTPClient theClient;  // Use HttpClient object to send requests
  Serial.println("Making HTTP request"); // print this message to the serial monitor
  theClient.begin("http://api.ipstack.com/" + getIP() + "?access_key=" + key); //return IP as .json object
  int httpCode = theClient.GET();

  if (httpCode > 0) { // If we get something back
    if (httpCode == 200) {
      Serial.println("Received HTTP payload.");
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      Serial.println("Parsing...");
      JsonObject& root = jsonBuffer.parse(payload);

      // Test if parsing succeeds.
      if (!root.success()) { // If parsing doesn't successed 
        Serial.println("parseObject() failed"); // Print this to serial monitor 
        Serial.println(payload); // Print the actual information or message in transmitted data, as opposed to automatically generated metadata.
        return;
      }

      //Some debugging lines below:
           Serial.println(payload);
           root.printTo(Serial);

      //Using .dot syntax, we refer to the variable "location" which is of
      //type GeoData, and place our data into the data structure.

      location.ip = root["ip"].as<String>();            //we cast the values as Strings b/c
      location.cc = root["country_code"].as<String>();  //the 'slots' in GeoData are Strings
      location.cn = root["country_name"].as<String>();  
      location.rc = root["region_code"].as<String>();
      location.rn = root["region_name"].as<String>();
      location.cy = root["city"].as<String>();
      location.lt = root["latitude"].as<String>();
      location.ln = root["longitude"].as<String>();

    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
    }
  }
}

void getMet(String city) { // function called getMet that provides weather data
  HTTPClient theClient; // Use HttpClient object to send requests
  
  String apiCall = "http://api.openweathermap.org/data/2.5/weather?q=Seattle"; // Construct API call
  apiCall += "&units=imperial&appid=";
  apiCall += weatherKey; // Add the API key to the call 
  
  theClient.begin(apiCall);
  int httpCode = theClient.GET();
  if (httpCode > 0) {

    if (httpCode == HTTP_CODE_OK) {
      String payload = theClient.getString();
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(payload);
      
      if (!root.success()) { // Test if parsing succeeds.
        Serial.println("parseObject() failed in getMet()."); // If parsing doesn't successed, print that to serial monitor 
        return;
      }
      conditions.tp = root["main"]["temp"].as<String>();     //we cast the values as Strings b/c
      conditions.pr = root["main"]["pressure"].as<String>(); //the 'slots' in GeoData are Strings
      conditions.hd = root["main"]["humidity"].as<String>();
      conditions.cd = root["weather"][0]["description"].as<String>();
      conditions.ws = root["wind"]["speed"].as<String>();
      int deg = root["wind"]["deg"].as<int>();
      conditions.wd = getNSEW(deg);
    }
  }
  else {
    Serial.printf("Something went wrong with connecting to the endpoint in getMet().");
  }
}

String getNSEW(int d) {
  String direct;

  //Conversion based upon http://climate.umn.edu/snow_fence/Components/winddirectionanddegreeswithouttable3.htm
  if (d > 348.75 && d < 360 || d >= 0  && d < 11.25) {
    direct = "north";
  };
  if (d > 11.25 && d < 33.75) {
    direct = "north northeast";
  };
  if (d > 33.75 && d < 56.25) {
    direct = "northeast";
  };
  if (d > 56.25 && d < 78.75) {
    direct = "east northeast";
  };
  if (d < 78.75 && d < 101.25) {
    direct = "east";
  };
  if (d < 101.25 && d < 123.75) {
    direct = "east southeast";
  };
  if (d < 123.75 && d < 146.25) {
    direct = "southeast";
  };
  if (d < 146.25 && d < 168.75) {
    direct = "south southeast";
  };
  if (d < 168.75 && d < 191.25) {
    direct = "south";
  };
  if (d < 191.25 && d < 213.75) {
    direct = "south southwest";
  };
  if (d < 213.25 && d < 236.25) {
    direct = "southwest";
  };
  if (d < 236.25 && d < 258.75) {
    direct = "west southwest";
  };
  if (d < 258.75 && d < 281.25) {
    direct = "west";
  };
  if (d < 281.25 && d < 303.75) {
    direct = "west northwest";
  };
  if (d < 303.75 && d < 326.25) {
    direct = "south southeast";
  };
  if (d < 326.25 && d < 348.75) {
    direct = "north northwest";
  };
  return direct;
}
