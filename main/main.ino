#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Arduino.h>
#include <ESP_Panel_Library.h>
#include <ESP_IOExpander_Library.h>

#include <lvgl.h>
#include "lvgl_port_v8.h"
#include <examples/lv_examples.h>

// Extend IO Pin define
#define TP_RST 1
#define LCD_BL 2
#define LCD_RST 3
#define SD_CS 4
#define USB_SEL 5

static lv_obj_t *ble_status_label;
static lv_obj_t *home_screen; // Object for the home page screen
static lv_obj_t *ble_screen;  // Object for the BLE status page
bool navigate_to_home = false; // Flag to indicate navigation should occur
unsigned long connected_time = 0; // Timestamp of when the device connected


BLEServer* Server = NULL;
BLECharacteristic* Characteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

#define SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"

class ServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* Server) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* Server) {
    deviceConnected = false;
  }
};

class CharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* Characteristic) {
    String value = Characteristic->getValue();
    if (value.length() > 0) {
      Serial.print("Characteristic event, written: ");
      Serial.println(value); // Print the integer value
    }
  }
};

void ble_init(){
  BLEDevice::init("Virtual_Window_Control");

  Server = BLEDevice::createServer();
  Server->setCallbacks(new ServerCallbacks());

  BLEService *Service = Server->createService(SERVICE_UUID);

  Characteristic = Service->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
                    
  Characteristic->setCallbacks(new CharacteristicCallbacks());

  Characteristic->addDescriptor(new BLE2902());

  Service->start();
  
  BLEAdvertising *Advertising = BLEDevice::getAdvertising();
  Advertising->addServiceUUID(SERVICE_UUID);
  Advertising->setScanResponse(false);
  Advertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void lvgl_init(){
  Serial.begin(115200);
  pinMode(GPIO_INPUT_IO_4, OUTPUT);
  /**
   * These development boards require the use of an IO expander to configure the screen,
   * so it needs to be initialized in advance and registered with the panel for use.
   *
   */
  Serial.println("Initialize IO expander");
  /* Initialize IO expander */
  ESP_IOExpander *expander = new ESP_IOExpander_CH422G((i2c_port_t)I2C_MASTER_NUM, ESP_IO_EXPANDER_I2C_CH422G_ADDRESS_000, I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO);
  // ESP_IOExpander *expander = new ESP_IOExpander_CH422G(I2C_MASTER_NUM, ESP_IO_EXPANDER_I2C_CH422G_ADDRESS_000);
  expander->init();
  expander->begin();
  expander->multiPinMode(TP_RST | LCD_BL | LCD_RST | SD_CS | USB_SEL, OUTPUT);
  expander->multiDigitalWrite(TP_RST | LCD_BL | LCD_RST, HIGH);
  delay(100);
  //gt911 initialization, must be added, otherwise the touch screen will not be recognized  
  //initialization begin
  expander->multiDigitalWrite(TP_RST | LCD_RST, LOW);
  delay(100);
  digitalWrite(GPIO_INPUT_IO_4, LOW);
  delay(100);
  expander->multiDigitalWrite(TP_RST | LCD_RST, HIGH);
  delay(200);
  //initialization end
  

  Serial.println("Initialize panel device");
  ESP_Panel *panel = new ESP_Panel();
  panel->init();
  #if LVGL_PORT_AVOID_TEAR
    // When avoid tearing function is enabled, configure the RGB bus according to the LVGL configuration
    ESP_PanelBus_RGB *rgb_bus = static_cast<ESP_PanelBus_RGB *>(panel->getLcd()->getBus());
    rgb_bus->configRgbFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
    rgb_bus->configRgbBounceBufferSize(LVGL_PORT_RGB_BOUNCE_BUFFER_SIZE);
  #endif
  panel->begin();

  Serial.println("Initialize LVGL");
  lvgl_port_init(panel->getLcd(), panel->getTouch());
}



void create_ui(void) {
    lvgl_port_lock(-1);

    // Create BLE screen
    ble_screen = lv_obj_create(NULL); // Create the BLE screen object
    lv_obj_set_style_bg_color(ble_screen, lv_color_hex(0x003366), 0);
    lv_obj_set_style_bg_opa(ble_screen, LV_OPA_COVER, 0);

    // Create BLE status label
    ble_status_label = lv_label_create(ble_screen);
    lv_label_set_text(ble_status_label, "Raspberry Pi is not connected"); // Initial label text
    lv_obj_set_style_text_font(ble_status_label, &lv_font_montserrat_16, 0); // Use size 16 for font
    lv_obj_set_style_text_color(ble_status_label, lv_color_hex(0xFFFFFF), 0); // Set text color to white
    lv_obj_align(ble_status_label, LV_ALIGN_CENTER, 0, 0); // Center the label

    lv_scr_load(ble_screen); // Load the BLE screen as the initial screen

    // Create the home screen (buttons will be added in the next step)
    home_screen = lv_obj_create(NULL); // Create a separate screen for the home page
    lv_obj_set_style_bg_color(home_screen, lv_color_hex(0x003366), 0);
    lv_obj_set_style_bg_opa(home_screen, LV_OPA_COVER, 0);

    lvgl_port_unlock();
}

void create_home_page() {
    lvgl_port_lock(-1);

    // Create "Weather" button
    lv_obj_t *btn_weather = lv_btn_create(home_screen);
    lv_obj_set_size(btn_weather, 120, 80); // Button size
    lv_obj_set_style_bg_color(btn_weather, lv_color_hex(0xCC9900), 0); // Set button color
    lv_obj_align(btn_weather, LV_ALIGN_CENTER, -150, 0); // Left position
    lv_obj_t *label_weather = lv_label_create(btn_weather);
    lv_label_set_text(label_weather, "Weather");
    lv_obj_center(label_weather);

    // Create "Brightness" button
    lv_obj_t *btn_brightness = lv_btn_create(home_screen);
    lv_obj_set_size(btn_brightness, 120, 80);
    lv_obj_set_style_bg_color(btn_brightness, lv_color_hex(0xCC9900), 0);
    lv_obj_align(btn_brightness, LV_ALIGN_CENTER, 0, 0); // Center position
    lv_obj_t *label_brightness = lv_label_create(btn_brightness);
    lv_label_set_text(label_brightness, "Brightness");
    lv_obj_center(label_brightness);

    // Create "Volume" button
    lv_obj_t *btn_volume = lv_btn_create(home_screen);
    lv_obj_set_size(btn_volume, 120, 80);
    lv_obj_set_style_bg_color(btn_volume, lv_color_hex(0xCC9900), 0);
    lv_obj_align(btn_volume, LV_ALIGN_CENTER, 150, 0); // Right position
    lv_obj_t *label_volume = lv_label_create(btn_volume);
    lv_label_set_text(label_volume, "Volume");
    lv_obj_center(label_volume);

    lvgl_port_unlock();
}



void setup() {
  Serial.begin(115200);
  ble_init();
  lvgl_init();
  create_ui();
  create_home_page();
}

void loop() {
    // Notify changed value if the device is connected
    if (deviceConnected) {
        Characteristic->setValue(String(value).c_str());
        Characteristic->notify();
        value++;

        // Show "Connected" and wait for 1 second before navigating to the home page
        Serial.println("New value notified: ");
        Serial.println(value);

        lvgl_port_lock(-1);
        lv_label_set_text(ble_status_label, "Connected");
        lv_obj_align(ble_status_label, LV_ALIGN_CENTER, 0, 0); // Ensure the label is centered
        lvgl_port_unlock();

        // Wait for 1 second
        delay(1000);

        // Navigate to the home page
        lvgl_port_lock(-1);
        lv_scr_load(home_screen); // Load the home screen
        lvgl_port_unlock();

        delay(3000); // Optionally delay before the next action to avoid rapid BLE notifications
    }

    // Handle disconnections
    if (!deviceConnected && oldDeviceConnected) {
        Serial.println("Device disconnected.");
        delay(500); // Give the Bluetooth stack the chance to get things ready
        Server->startAdvertising(); // Restart advertising
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;

        // Update BLE status label to show "Raspberry Pi is not connected"
        lvgl_port_lock(-1);
        lv_label_set_text(ble_status_label, "Raspberry Pi is not connected");
        lv_obj_align(ble_status_label, LV_ALIGN_CENTER, 0, 0); // Re-center the label
        lv_scr_load(ble_screen); // Return to BLE status screen
        lvgl_port_unlock();
    }

    // Handle initial connection
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
        Serial.println("Device Connected");
    }
}
