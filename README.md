# rogueAP_ESP8266
This is a simple proof of concept Rogue Access Point for ESP8266. ESP8266 is an extremely low cost, low powered, Wifi capable microcontroller with decent range.

With this sketch it will pose as a free public Wifi network and entice victims into entering an email address and an often reused password. The credentials are stored on flash and are printed to serial on startup. They are also accessible on 192.168.4.1/credentials. This AP also intercepts DNS requests to act as a captive portal. Most modern operating systems will recognize this and direct the user straight to a login page for phishing attacks.

Recommended configuration would be 80MHz mode with 3MB flash. Since no HTTPS is used, 80MHz should be sufficient and reduce power consumption. OTA is not supported, so the extra flash space can be used to store more credentials.
