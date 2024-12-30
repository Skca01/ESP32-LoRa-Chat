#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <LoRa.h>
#include <SPIFFS.h>

// WiFi credentials
const char* ssid = "ESP32-LoRa-Chat2";
const char* password = "chatpass123";

// LoRa Pins
#define LORA_SS   5     
#define LORA_SCK  18    
#define LORA_MISO 19    
#define LORA_MOSI 23    

// LoRa Settings
byte localAddress = 0xBB;    
byte destination = 0xAA;     

WebServer server(80);

#define MAX_MESSAGES 50
String messageBuffer[MAX_MESSAGES];
int messageCount = 0;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 LoRa Chat</title>
    <style>
        :root {
            --primary-color: #3498db;
            --secondary-color: #2ecc71;
            --background-gradient-1: #667eea;
            --background-gradient-2: #764ba2;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: Arial, sans-serif;
            background: linear-gradient(135deg, var(--background-gradient-1), var(--background-gradient-2));
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            padding: 20px 0;
        }

        #username-modal {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.8);
            display: flex;
            align-items: center;
            justify-content: center;
            z-index: 1000;
        }

        .modal-content {
            background: white;
            padding: 20px;
            border-radius: 10px;
            text-align: center;
        }

        #username-input {
            padding: 10px;
            margin: 10px 0;
            border: 1px solid #ddd;
            border-radius: 5px;
            width: 200px;
        }

        #status-bar {
            background: rgba(255, 255, 255, 0.1);
            padding: 10px;
            border-radius: 10px;
            margin-bottom: 10px;
            color: white;
            font-size: 0.9em;
            text-align: center;
            width: 90%;
            max-width: 400px;
        }

        #chat-container {
            width: 90%;
            max-width: 400px;
            height: 65vh;
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.2);
            display: flex;
            flex-direction: column;
            overflow: hidden;
        }

        #message-area {
            flex-grow: 1;
            overflow-y: auto;
            padding: 15px;
            display: flex;
            flex-direction: column;
            gap: 10px;
        }

        .message {
            max-width: 80%;
            padding: 10px 15px;
            border-radius: 15px;
            margin-bottom: 10px;
            word-wrap: break-word;
        }

        .sent-message {
            background: linear-gradient(135deg, #3498db, #2980b9);
            color: white;
            align-self: flex-end;
            border-bottom-right-radius: 5px;
        }

        .received-message {
            background: linear-gradient(135deg, #ecf0f1, #bdc3c7);
            color: #2c3e50;
            align-self: flex-start;
            border-bottom-left-radius: 5px;
        }

        .username {
            font-weight: bold;
            font-size: 0.8em;
            margin-bottom: 4px;
        }

        .message-info {
            font-size: 0.7em;
            opacity: 0.7;
            margin-top: 4px;
        }

        .input-area {
            display: flex;
            padding: 15px;
            background: rgba(255, 255, 255, 0.1);
            border-top: 1px solid rgba(255, 255, 255, 0.2);
        }

        #message-input {
            flex-grow: 1;
            padding: 10px 15px;
            background: rgba(255, 255, 255, 0.2);
            border: none;
            border-radius: 30px;
            margin-right: 10px;
            color: white;
        }

        #message-input::placeholder {
            color: rgba(255, 255, 255, 0.6);
        }

        button {
            background: linear-gradient(135deg, #3498db, #2ecc71);
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 30px;
            cursor: pointer;
            transition: transform 0.3s ease;
        }

        button:hover {
            transform: scale(1.05);
        }

        button:disabled {
            opacity: 0.5;
            cursor: not-allowed;
        }
    </style>
</head>
<body>
    <div id="username-modal">
        <div class="modal-content">
            <h2>Enter your username</h2>
            <input type="text" id="username-input" placeholder="Username">
            <button id="username-submit">Join Chat</button>
        </div>
    </div>

    <div id="status-bar">
        Connecting...
    </div>
    
    <div id="chat-container">
        <div id="message-area"></div>
        <div class="input-area">
            <input type="text" id="message-input" placeholder="Type a message..." disabled>
            <button id="send-btn" disabled>Send</button>
        </div>
    </div>

    <script>
        const statusBar = document.getElementById('status-bar');
        const messageArea = document.getElementById('message-area');
        const messageInput = document.getElementById('message-input');
        const sendBtn = document.getElementById('send-btn');
        const usernameModal = document.getElementById('username-modal');
        const usernameInput = document.getElementById('username-input');
        const usernameSubmit = document.getElementById('username-submit');
        
        let username = '';
        let messageCheckInterval;

        usernameSubmit.addEventListener('click', () => {
            username = usernameInput.value.trim();
            if (username) {
                usernameModal.style.display = 'none';
                initChat();
            }
        });

        function initChat() {
            statusBar.textContent = 'Connected';
            statusBar.style.background = 'rgba(46, 204, 113, 0.2)';
            messageInput.disabled = false;
            sendBtn.disabled = false;
            
            // Start checking for new messages
            messageCheckInterval = setInterval(checkNewMessages, 1000);
        }

        function checkNewMessages() {
            fetch('/messages')
                .then(response => response.json())
                .then(data => {
                    messageArea.innerHTML = ''; // Clear existing messages
                    data.messages.forEach(msg => {
                        const [sender, content] = msg.split(': ');
                        const type = sender === username ? 'sent' : 'received';
                        addMessage(content, type, sender);
                    });
                })
                .catch(error => {
                    console.error('Error fetching messages:', error);
                    statusBar.textContent = 'Connection error';
                    statusBar.style.background = 'rgba(231, 76, 60, 0.2)';
                });
        }

        function addMessage(content, type, sender) {
            const messageDiv = document.createElement('div');
            messageDiv.classList.add('message', `${type}-message`);
            
            const usernameDiv = document.createElement('div');
            usernameDiv.classList.add('username');
            usernameDiv.textContent = sender;
            messageDiv.appendChild(usernameDiv);
            
            const messageContent = document.createElement('div');
            messageContent.textContent = content;
            messageDiv.appendChild(messageContent);
            
            const messageInfo = document.createElement('div');
            messageInfo.classList.add('message-info');
            messageInfo.textContent = new Date().toLocaleTimeString();
            messageDiv.appendChild(messageInfo);
            
            messageArea.appendChild(messageDiv);
            messageArea.scrollTop = messageArea.scrollHeight;
        }

        function sendMessage() {
            const message = messageInput.value.trim();
            if (message) {
                fetch('/send', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                    },
                    body: JSON.stringify({
                        message: message,
                        sender: username
                    })
                })
                .then(response => response.json())
                .then(data => {
                    if (data.status === 'sent') {
                        messageInput.value = '';
                    }
                })
                .catch(error => {
                    console.error('Error sending message:', error);
                    statusBar.textContent = 'Error sending message';
                    statusBar.style.background = 'rgba(231, 76, 60, 0.2)';
                });
            }
        }

        sendBtn.addEventListener('click', sendMessage);
        messageInput.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') {
                sendMessage();
            }
        });
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    
    if(!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed!");
        return;
    }

    Serial.println("Initializing LoRa...");
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
    LoRa.setPins(LORA_SS, -1, -1);
    
    if (!LoRa.begin(915E6)) {
        Serial.println("LoRa initialization failed!");
        return;
    }
    
    // Configure LoRa parameters
    // Optimized settings for Philippines region
    LoRa.setSpreadingFactor(10);           // Increased for better range
    LoRa.setSignalBandwidth(125E3);        // Standard for 915MHz
    LoRa.setCodingRate4(5);                // 4/5 coding rate
    LoRa.setSyncWord(0xF3);                // Network identifier
    LoRa.setTxPower(20, PA_OUTPUT_PA_BOOST_PIN); // Max power for better range
    LoRa.enableCrc();                      // Enable CRC checking

    WiFi.softAP(ssid, password);
    Serial.println("Access Point Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", HTTP_GET, handleRoot);
    server.on("/send", HTTP_POST, handleSendMessage);
    server.on("/messages", HTTP_GET, handleGetMessages);

    server.begin();
    Serial.println("Web Server Started");
}

void handleRoot() {
    server.send(200, "text/html", index_html);
}

void handleSendMessage() {
    if (server.hasArg("plain")) {
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, server.arg("plain"));
        
        if (!error) {
            String message = doc["message"].as<String>();
            String sender = doc["sender"].as<String>();
            
            LoRa.beginPacket();
            LoRa.write(destination);
            LoRa.write(localAddress);
            LoRa.print(sender + ": " + message);
            LoRa.endPacket();
            
            addMessageToBuffer(sender + ": " + message);
            
            server.send(200, "application/json", "{\"status\":\"sent\"}");
        } else {
            server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        }
    }
}

void handleGetMessages() {
    StaticJsonDocument<2048> doc;
    JsonArray array = doc.createNestedArray("messages");
    
    for(int i = 0; i < messageCount; i++) {
        array.add(messageBuffer[i]);
    }
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void addMessageToBuffer(String message) {
    if (messageCount >= MAX_MESSAGES) {
        for(int i = 0; i < MAX_MESSAGES - 1; i++) {
            messageBuffer[i] = messageBuffer[i + 1];
        }
        messageCount--;
    }
    messageBuffer[messageCount++] = message;
}

void loop() {
    server.handleClient();
    
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        byte recipient = LoRa.read();
        byte sender = LoRa.read();
        
        String incoming = "";
        while (LoRa.available()) {
            incoming += (char)LoRa.read();
        }
        
        if (recipient == localAddress) {
            addMessageToBuffer(incoming);
        }
    }
}