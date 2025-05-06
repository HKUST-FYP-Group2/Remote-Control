# Virtual Window Remote Control

The Remote Control component of the Virtual Window System is a hardware-based device designed to provide intuitive, tactile control over the virtual window experience.\
This ESP32-powered device connects via Bluetooth to the Virtual Window system, allowing users to adjust settings like brightness, volume, and ambient music directly from their workspace.

## Hardware Components

- **ESP32-S3-Touch-LCD-4.3**: Development board with integrated LCD touchscreen (https://www.waveshare.net/wiki/ESP32-S3-Touch-LCD-4.3)
- **18650 Battery**: Rechargeable lithium-ion battery for portable operation
- **Battry box with PH2.0 Connector**: For reliable battery connections
- **Power Switch**: To control power on/off
![Remote Control image](https://github.com/user-attachments/assets/234b1f67-48c2-4bcb-ae9a-1a9f479ebbf8)


## Software Stack

- **Arduino IDE**: For firmware development and deployment
- **LVGL (Light and Versatile Graphics Library)**: For creating the touchscreen interface
![image](https://github.com/user-attachments/assets/8c416ccc-7850-4647-9d5e-f0598e21dedf)

## Usage

1. **Power On**: Use the physical switch to activate the device
2. **Pairing**: Connect via Bluetooth using the bluetooth button in the projector app. Choose Virtual_Window_Control in the pairing panel.
3. **Adjust Settings**: Use touch screen to modify brightness, volume, and music selection


Here is a well-structured **Deployment** section for your `README.md` file, tailored to guide users through the deployment process of your Remote Control project using the **ESP32-S3-Touch-LCD-4.3** board:

---

## 🛠️ Deployment Instructions

Follow these steps to deploy the Remote Control firmware onto the **ESP32-S3-Touch-LCD-4.3** development board using the **Arduino IDE**.

### 🔧 Prerequisites

Before proceeding, ensure you have the following:

- **ESP32-S3-Touch-LCD-4.3** development board
- USB Type-C cable
- Arduino IDE installed (preferably the latest version)
- ESP32 board support installed in Arduino IDE (via Boards Manager)

---

### 📦 Step 1: Install Required Libraries

1. Open **Arduino IDE**.
2. Go to **Sketch > Include Library > Manage Libraries**.
3. Search and install the following libraries:
   - `LVGL` (Light and Versatile Graphics Library)
   - `ESP_IOExpander_Library` (for hardware control)

> 💡 For detailed installation instructions, refer to the official Waveshare guide:
> [Arduino 板管理教程](https://www.waveshare.net/wiki/Arduino_板管理教程) *(Chinese)*

---

### ⚙️ Step 2: Configure Board Settings

1. Go to **Tools > Board > ESP32 Arduino**.
2. Select your board: **ESP32-S3-Touch-LCD-4.3** or **ESP32S3 Dev Module** (or equivalent).
3. Set the following options in the Tools menu:

| Option | Value |
|--------|-------|
| **Upload Mode** | UART0 |
| **PSRAM** | OPI PSRAM |
| **Flash Mode** | QIO |
| **Flash Frequency** | 80MHz |
| **Flash Size** | 8MB |
| **Partition Scheme** | 8MB with spiffs |

---

### 🔌 Step 3: Connect the Device

1. Connect the **ESP32-S3-Touch-LCD-4.3** to your computer using a USB Type-C cable.
2. Ensure the board is recognized by your system:
   - Check the **Tools > Port** menu in Arduino IDE to confirm the correct COM port is selected.

---

### 📤 Step 4: Compile and Upload the Project

1. Open the project `.ino` file in Arduino IDE.
2. Review and adjust any configuration parameters if needed (e.g., BLE service UUIDs, default settings).
3. Click the **Verify** button (✔️) to compile the code.
4. Once compilation is successful, click the **Upload** button (➡️) to flash the firmware onto the device.

> ⏱️ The upload may take a minute. Once complete, the device will automatically reset and start running the firmware.

---

### 🧪 Step 5: Test the Remote Control

1. Power on the device (ensure the power switch is engaged).
2. The screen should display the **Connection Status Screen**.
3. Use the touchscreen to navigate to the **Main Control Screen**.
4. Pair the remote with the **Projector App** via Bluetooth.
5. Test brightness, volume, and background music controls to ensure they sync with the projector app.

---

### 📚 Resources

- [Waveshare ESP32-S3-Touch-LCD-4.3 Wiki](https://www.waveshare.net/wiki/ESP32-S3-Touch-LCD-4.3)
- [LVGL Official Documentation](https://docs.lvgl.io/latest/en/html/)
- [ESP-IDF SPIFFS Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/storage/spiffs.html)
