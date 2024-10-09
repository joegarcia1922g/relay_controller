#include <WiFi.h>
#include <Preferences.h>

Preferences preferences;

String storedSSID;
String storedPassword;
bool credentialsExist = false;
int maxAttempts = 3;  // Maximum connection attempts
unsigned long timeout = 150000;  // 150-second timeout for the menu

// Static IP configuration (adjust these values based on your network)
IPAddress local_IP(192, 168, 1, 184);      // ESP32's static IP
IPAddress gateway(192, 168, 1, 1);         // Gateway IP (usually your router)
IPAddress subnet(255, 255, 255, 0);        // Subnet mask
IPAddress primaryDNS(8, 8, 8, 8);          // Optional: Set primary DNS (Google DNS)
IPAddress secondaryDNS(8, 8, 4, 4);        // Optional: Set secondary DNS

void setup() {
  Serial.begin(115200); // Start with the appropriate baud rate for ESP32
  Serial.println("Starting ESP32...");

  // Initialize stored WiFi preferences
  preferences.begin("wifi-config", false);
  storedSSID = preferences.getString("ssid", "");
  storedPassword = preferences.getString("password", "");

  // If credentials exist, try to connect automatically
  if (storedSSID != "" && storedPassword != "") {
    Serial.print("Automatically connecting to saved network: ");
    Serial.println(storedSSID);

    // Set static IP configuration before connecting to WiFi
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
      Serial.println("Failed to configure Static IP.");
    }

    // Try connecting with a maximum of 3 attempts
    for (int i = 1; i <= maxAttempts; i++) {
      Serial.printf("Attempt %d of %d...\n", i, maxAttempts);
      WiFi.begin(storedSSID.c_str(), storedPassword.c_str());

      if (connectToWiFi()) {
        Serial.println("Connection successful!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        return;  // Exit setup if successfully connected
      } else {
        Serial.println("Connection failed.");
      }

      // Wait 2 seconds before the next attempt
      delay(2000);
    }

    // If connection fails after 3 attempts, clear credentials
    Serial.println("Error: Could not connect after 3 attempts. Clearing credentials.");
    clearCredentials();
    showMenu();  // Show the menu for reconfiguration
  } else {
    Serial.println("No stored credentials.");
    showMenu();  // Show the menu if no credentials exist
  }
}

void showMenu() {
  while (true) {
    Serial.println("\n--- ESP32 MENU ---");
    Serial.println("S: Scan networks and connect to a new one");
    Serial.println("D: Diagnose current connection");
    Serial.println("-----------------\n");
    Serial.println("Select an option:");

    unsigned long startTime = millis();
    while (millis() - startTime < timeout) {
      if (Serial.available()) {
        String option = Serial.readStringUntil('\n');  // Read the entire line
        option.trim();  // Clean any whitespace or newlines

        handleOption(option);  // Process the selected option
        return;  // Exit after handling the option
      }
      delay(100);  // Small delay to avoid overloading the loop
    }

    // Timeout: show the menu again
    Serial.println("Timeout reached. Showing the menu again...");
  }
}

void handleOption(String option) {
  option.toUpperCase();  // Convert input to uppercase to handle both 's' and 'S'
  
  if (option == "S") {
    scanNetworks();  // Scan networks and allow selection
  } else if (option == "D") {
    checkConnectionStatus();  // Diagnose the current connection
  } else {
    Serial.println("Invalid option.");
    showMenu();  // Show the menu again if the option is invalid
  }
}

void scanNetworks() {
  Serial.println("Starting WiFi network scan...");

  // Ensure WiFi is enabled
  if (WiFi.getMode() == WIFI_MODE_NULL) {
    Serial.println("WiFi is disabled, enabling now...");
    WiFi.mode(WIFI_STA);  // Set WiFi mode to client
  }

  int n = WiFi.scanNetworks();  // Scan for networks
  Serial.print("Number of networks found: ");
  Serial.println(n);

  if (n == 0) {
    Serial.println("No WiFi networks found.");
  } else {
    Serial.println("Networks found:");
    for (int i = 0; i < n; ++i) {
      Serial.printf("%d: %s (Signal: %d)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
      delay(10);
    }

    Serial.println("Select the number of the network to connect to:");

    // 15-second timeout for network selection
    unsigned long startTime = millis();
    while (millis() - startTime < timeout) {
      if (Serial.available()) {
        String selectedNetworkStr = Serial.readStringUntil('\n');
        selectedNetworkStr.trim();  // Clean whitespace
        int selectedNetwork = selectedNetworkStr.toInt() - 1;  // Convert to index

        if (selectedNetwork >= 0 && selectedNetwork < n) {
          String wifiSSID = WiFi.SSID(selectedNetwork);
          Serial.print("You selected: ");
          Serial.println(wifiSSID);

          Serial.println("Enter password (leave blank if open network):");
          startTime = millis();
          while (millis() - startTime < timeout) {
            if (Serial.available()) {
              String wifiPassword = Serial.readStringUntil('\n');
              wifiPassword.trim();

              // Attempt to connect with the new credentials
              if (wifiPassword != "") {
                WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
              } else {
                WiFi.begin(wifiSSID.c_str());  // Open network, no password
              }

              if (connectToWiFi()) {
                Serial.println("Connection successful!");
                Serial.print("IP Address: ");
                Serial.println(WiFi.localIP());

                // Save credentials only after successful connection
                preferences.putString("ssid", wifiSSID);
                preferences.putString("password", wifiPassword);
              } else {
                Serial.println("Failed to connect. Check credentials.");
              }
              return;  // Exit once processed
            }
            delay(100);  // Avoid overloading the loop
          }

          Serial.println("Timeout reached. Showing networks again.");
          scanNetworks();  // If no selection, rescan networks
        } else {
          Serial.println("Invalid selection.");
        }
      }
      delay(100);  // Avoid overloading the loop
    }

    Serial.println("Timeout reached. Returning to the menu.");
    showMenu();  // Return to menu if no network is selected
  }
}

void checkConnectionStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Currently connected to the network:");
    Serial.println(WiFi.SSID());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Not connected to any network.");
  }
  showMenu();  // Return to menu after showing connection status
}

bool connectToWiFi() {
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);  // Short delay
    Serial.print(".");
  }
  Serial.println();  // New line after connection attempts
  return WiFi.status() == WL_CONNECTED;
}

void clearCredentials() {
  preferences.remove("ssid");
  preferences.remove("password");
  Serial.println("Credentials removed. Please restart to reconfigure.");
}

void loop() {
  // Empty because all logic is handled in setup and user interaction
}
