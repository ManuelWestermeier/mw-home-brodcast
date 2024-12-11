#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>

#include "./secrits.hpp"
#include "./page.hpp"

// LCD setup (address 0x27, 4x20 LCD)
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Create WebServer object on port 80
WebServer server(80);

void renderScreen(String text) {
  lcd.clear();
  lcd.home();

  // Print the IP address
  IPAddress IP = WiFi.softAPIP();
  lcd.print("IP: ");
  lcd.print(IP);
  lcd.setCursor(0, 1);

  int strIndex = 0;
  int line = 1;  // Start at line 1 (2nd line on the LCD)
  int x = 0;     // Start at column 0

  while (strIndex < text.length() && line < 4) {  // Only display on the first 3 lines
    char currentChar = text.charAt(strIndex);
    strIndex++;

    // Skip carriage return characters
    if (currentChar == '\r') continue;

    // Handle tab character (use 4 spaces)
    if (currentChar == '\t') {
      x += 4;
      if (x >= 20) {  // If we go beyond the screen width, move to the next line
        x = 0;
        line++;
        if (line >= 4) break;  // Stop if we've filled the screen
      }
      continue;
    }

    // Handle newline character
    if (currentChar == '\n') {
      line++;
      x = 0;
      if (line >= 4) break;  // Stop if we've filled the screen
      continue;
    }

    // Print the character to the LCD
    lcd.setCursor(x, line);
    lcd.write(currentChar);
    x++;

    // Check if we reached the end of the line
    if (x >= 20) {
      x = 0;
      line++;
      if (line >= 4) break;  // Stop if we've filled the screen
    }
  }
}

String escapeHTML(String input) {
  input.replace("&", "&amp;");
  input.replace("<", "&lt;");
  input.replace(">", "&gt;");
  input.replace("\"", "&quot;");
  input.replace("'", "&#39;");
  input.replace("\n", "<br>");
  return input;
}

void setup() {
  // Initialize serial monitor
  Serial.begin(115200);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.noCursor();

  // Set up Wi-Fi Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Wi-Fi Access Point started");
  renderScreen("Input Text On the Website");

  // Define the root route
  server.on("/", HTTP_GET, []() {
    String inputMessage;

    // Check if "input" parameter is provided
    if (server.hasArg("input")) {
      inputMessage = server.arg("input");

      // Display the input on the LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Received Input:");
      lcd.setCursor(0, 1);
      lcd.print(inputMessage);

      // Print input to serial monitor
      Serial.println("Input received: " + inputMessage);
      renderScreen(inputMessage);
    } else {
      // Send response to client with form
      server.send(200, "text/html", htmlTop + R"(<form action='/' method='get' onsubmit='submitForm(event)'>
        <textarea name='input' placeholder='input text...'></textarea>
        <br>
        <button type='submit'>
          Send
        </button>
        <script>
          function submitForm(event) {
            const val = document.querySelector('textarea').value;
            if (val.length > 57) {
              event.preventDefault();
              alert('Text is longer than 57 characters');
            }
            if (val.split('\n').length > 3) {
              event.preventDefault();
              alert('Text has more than 3 lines');
            }
          }
        </script>
      </form>)" + htmlBottom);
      return;  // Important to return here so that the rest of the code doesn't run
    }

    // Construct the HTML response with escaped characters
    String response = htmlTop + "Input received: <br>";
    response += escapeHTML(inputMessage);
    response += "<script>setTimeout(()=>{window.location = '/'}, 3000)</script>" + htmlBottom;

    // Send the HTML response to the client
    server.send(200, "text/html", response);
    tone(12, 433, 300);
  });

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
  lcd.setCursor(0, 3);
  lcd.print("Server ready");
}

void loop() {
  // Nothing needed here for WebServer
  server.handleClient();
}
