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
static lv_obj_t *brightness_screen;  // Screen for brightness control 
static lv_obj_t *volume_screen;      // Screen for volume control
static lv_obj_t *brightness_slider;
static lv_obj_t *volume_slider;
bool navigate_to_home = false; // Flag to indicate navigation should occur
unsigned long connected_time = 0; // Timestamp of when the device connected
static uint32_t last_btn_click = 0;
static const uint32_t DEBOUNCE_TIME = 300;
int brightness = 0;
int volume = 0;

BLEServer* Server = NULL;
BLECharacteristic* Characteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool is_screen_init = false;
uint32_t value = 0;

#define SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"

void update_sliders();

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
      String value = Characteristic->getValue().c_str();
      
      int delimiterPos = value.indexOf('|');
      if (delimiterPos != -1) {
          String brightnessStr = value.substring(0, delimiterPos);
          String volumeStr = value.substring(delimiterPos + 1);
          brightness = brightnessStr.substring(brightnessStr.indexOf('-') + 1).toInt();
          volume = volumeStr.substring(volumeStr.indexOf('-') + 1).toInt();
          update_sliders();
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

void ble_send(){
  String s = "brightness-" + String(brightness) + "|" + "volume-" + String(volume);
  Characteristic->setValue(s);
  Characteristic->notify();
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

static void btn_brightness_cb(lv_event_t *e) {
    uint32_t current_time = millis();
    if(current_time - last_btn_click < DEBOUNCE_TIME) {
        return;
    }
    last_btn_click = current_time;

    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        lvgl_port_lock(-1);
        lv_scr_load(brightness_screen);
        lvgl_port_unlock();
    }
}

static void btn_volume_cb(lv_event_t *e) {
    uint32_t current_time = millis();
    if(current_time - last_btn_click < DEBOUNCE_TIME) {
        return;
    }
    last_btn_click = current_time;

    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        lvgl_port_lock(-1);
        lv_scr_load(volume_screen);
        lvgl_port_unlock();
    }
}


static void slider_brightness_cb(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    brightness = lv_slider_get_value(slider);
    ble_send();
}

static void slider_volume_cb(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    volume = lv_slider_get_value(slider);
    ble_send();
}

void create_ui(void) {
    lvgl_port_lock(-1);

    // Create BLE screen
    ble_screen = lv_obj_create(NULL); // Create the BLE screen object
    lv_obj_set_style_bg_color(ble_screen, lv_color_hex(0x003366), 0);
    lv_obj_set_style_bg_opa(ble_screen, LV_OPA_COVER, 0);

    // Create BLE status label
    ble_status_label = lv_label_create(ble_screen);
    lv_label_set_text(ble_status_label, "Raspberry Pi is not connected");
    lv_obj_set_style_text_font(ble_status_label, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(ble_status_label, lv_color_hex(0xFFFFFF), 0);
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

    // "Weather" button
    lv_obj_t *btn_weather = lv_btn_create(home_screen);
    lv_obj_set_size(btn_weather, 120, 80); // Button size
    lv_obj_set_style_bg_color(btn_weather, lv_color_hex(0xCC9900), 0); // Set button color
    lv_obj_align(btn_weather, LV_ALIGN_CENTER, -150, 0); // Left position
    lv_obj_t *label_weather = lv_label_create(btn_weather);
    lv_label_set_text(label_weather, "Weather");
    lv_obj_center(label_weather);

    // "Brightness" button
    lv_obj_t *btn_brightness = lv_btn_create(home_screen);
    lv_obj_set_size(btn_brightness, 120, 80);
    lv_obj_set_style_bg_color(btn_brightness, lv_color_hex(0xCC9900), 0);
    lv_obj_align(btn_brightness, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_brightness, btn_brightness_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_brightness = lv_label_create(btn_brightness);
    lv_label_set_text(label_brightness, "Brightness");
    lv_obj_center(label_brightness);

    // "Volume" button
    lv_obj_t *btn_volume = lv_btn_create(home_screen);
    lv_obj_set_size(btn_volume, 120, 80);
    lv_obj_set_style_bg_color(btn_volume, lv_color_hex(0xCC9900), 0);
    lv_obj_align(btn_volume, LV_ALIGN_CENTER, 150, 0);
    lv_obj_add_event_cb(btn_volume, btn_volume_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label_volume = lv_label_create(btn_volume);
    lv_label_set_text(label_volume, "Volume");
    lv_obj_center(label_volume);

    lvgl_port_unlock();
}

void update_sliders() {
    if (brightness_slider) {
        lv_slider_set_value(brightness_slider, brightness, LV_ANIM_ON);
    }
    if (volume_slider) {
        lv_slider_set_value(volume_slider, volume, LV_ANIM_ON);
    }
}


void create_brightness_page() {
    lvgl_port_lock(-1);
    
    // Create brightness screen
    brightness_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(brightness_screen, lv_color_hex(0x003366), 0);
    lv_obj_set_style_bg_opa(brightness_screen, LV_OPA_COVER, 0);

    // Create slider
    brightness_slider = lv_slider_create(brightness_screen);
    lv_obj_set_size(brightness_slider, 300, 20);
    lv_obj_align(brightness_slider, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(brightness_slider, 0, 100);
    lv_slider_set_value(brightness_slider, brightness, LV_ANIM_OFF);
    lv_obj_add_event_cb(brightness_slider, slider_brightness_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Create label
    lv_obj_t *label = lv_label_create(brightness_screen);
    lv_label_set_text(label, "Brightness");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -40);

    // Create back button
    lv_obj_t *btn_back = lv_btn_create(brightness_screen);
    lv_obj_set_size(btn_back, 70, 50);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_t *label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "Back");
    lv_obj_center(label_back);
    lv_obj_add_event_cb(btn_back, [](lv_event_t *e) {
        lv_scr_load(home_screen);
    }, LV_EVENT_CLICKED, NULL);

    lvgl_port_unlock();
}

void create_volume_page() {
    lvgl_port_lock(-1);
    
    // Create volume screen
    volume_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(volume_screen, lv_color_hex(0x003366), 0);
    lv_obj_set_style_bg_opa(volume_screen, LV_OPA_COVER, 0);

    // Create slider
    volume_slider = lv_slider_create(volume_screen);
    lv_obj_set_size(volume_slider, 300, 20);
    lv_obj_align(volume_slider, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(volume_slider, 0, 100);
    lv_slider_set_value(volume_slider, volume, LV_ANIM_OFF);
    lv_obj_add_event_cb(volume_slider, slider_volume_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Create label
    lv_obj_t *label = lv_label_create(volume_screen);
    lv_label_set_text(label, "Volume");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -40);

    // Create back button
    lv_obj_t *btn_back = lv_btn_create(volume_screen);
    lv_obj_set_size(btn_back, 70, 50);
    lv_obj_align(btn_back, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_t *label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "Back");
    lv_obj_center(label_back);
    lv_obj_add_event_cb(btn_back, [](lv_event_t *e) {
        lv_scr_load(home_screen);
    }, LV_EVENT_CLICKED, NULL);

    lvgl_port_unlock();
}


void setup() {
  Serial.begin(115200);
  ble_init();
  lvgl_init();
  create_ui();
  create_home_page();
  create_brightness_page();
  create_volume_page();
}

void loop() {
    // connected
    if (deviceConnected) {
      if(!is_screen_init){
        lvgl_port_lock(-1);
        lv_label_set_text(ble_status_label, "Connected");
        lvgl_port_unlock();

        // Wait for 1 second
        delay(1000);

        // Navigate to the home page
        lvgl_port_lock(-1);
        lv_scr_load(home_screen); // Load the home screen
        lvgl_port_unlock();
        
        is_screen_init = true;
      }
    }

    // disconnections
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        Server->startAdvertising();
        oldDeviceConnected = deviceConnected;

        lvgl_port_lock(-1);
        lv_label_set_text(ble_status_label, "Raspberry Pi is not connected");
        lv_scr_load(ble_screen);
        lvgl_port_unlock();
        is_screen_init = false;
    }

    // initial connection
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
        Serial.println("Device Connected");
    }
}
