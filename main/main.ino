#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Arduino.h>
#include <ESP_Panel_Library.h>
#include <ESP_IOExpander_Library.h>

#include <lvgl.h>
#include "lvgl_port_v8.h"

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
static lv_fs_drv_t drv; 

bool navigate_to_home = false; // Flag to indicate navigation should occur
unsigned long connected_time = 0; // Timestamp of when the device connected
static uint32_t last_btn_click = 0;
static const uint32_t DEBOUNCE_TIME = 300;
int brightness = 100;
int volume = 0;

BLEServer* Server = NULL;
BLECharacteristic* Characteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool is_screen_init = false;
uint32_t value = 0;

#define SERVICE_UUID        "419d7afd-9d84-4387-bdd6-54428c9aabbb"
#define CHARACTERISTIC_UUID "0f2441e6-7094-4561-b9c3-59f3690eb052"

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

void ble_init() {
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

void ble_send() {
  String s = "brightness-" + String(brightness) + "|" + "volume-" + String(volume);
  Characteristic->setValue(s);
  Characteristic->notify();
}

void lvgl_init() {
  Serial.begin(115200);
  pinMode(GPIO_INPUT_IO_4, OUTPUT);
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
  if (current_time - last_btn_click < DEBOUNCE_TIME) {
    return;
  }
  last_btn_click = current_time;

  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lvgl_port_lock(-1);
    lv_scr_load(brightness_screen);
    lvgl_port_unlock();
  }
}

static void btn_volume_cb(lv_event_t *e) {
  uint32_t current_time = millis();
  if (current_time - last_btn_click < DEBOUNCE_TIME) {
    return;
  }
  last_btn_click = current_time;

  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
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
  lv_obj_set_style_text_font(ble_status_label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(ble_status_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(ble_status_label, LV_ALIGN_CENTER, 0, 0); // Center the label
  
  lv_scr_load(ble_screen); // Load the BLE screen as the initial screen

  // Create the home screen (buttons will be added in the next step)
  home_screen = lv_obj_create(NULL); // Create a separate screen for the home page
  lv_obj_set_style_bg_color(home_screen, lv_color_hex(0x003366), 0);
  lv_obj_set_style_bg_opa(home_screen, LV_OPA_COVER, 0);

  lvgl_port_unlock();
}

static void btn_event_cb(lv_event_t *e) {
    if (millis() - last_btn_click < DEBOUNCE_TIME) return;
    last_btn_click = millis();

    lv_obj_t *btn = lv_event_get_target(e);
    
    if(btn == lv_obj_get_child(home_screen, 0)) {
        lv_scr_load(brightness_screen);
    }
    else if(btn == lv_obj_get_child(home_screen, 1)) {
        lv_scr_load(volume_screen);
    }
    else if(btn == lv_obj_get_child(home_screen, 2)) {
        String s = "bgm";
        Characteristic->setValue(s);
        Characteristic->notify();
    }
}


void create_home_page() {
    lvgl_port_lock(-1);

    // Create home screen
    home_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(home_screen, lv_color_hex(0x003366), LV_PART_MAIN);
    lv_obj_set_size(home_screen, LV_HOR_RES, LV_VER_RES);

    // Create button style
    static lv_style_t btn_style;
    lv_style_init(&btn_style);
    lv_style_set_border_opa(&btn_style, LV_OPA_COVER);
    lv_style_set_border_color(&btn_style, lv_color_white());
    lv_style_set_border_width(&btn_style, 2);

    // Create image style
    static lv_style_t img_style;
    lv_style_init(&img_style);
    lv_style_set_img_recolor(&img_style, lv_color_white());
    lv_style_set_img_recolor_opa(&img_style, LV_OPA_COVER);

    // Brightness Button
    lv_obj_t *btn_brightness = lv_btn_create(home_screen);
    lv_obj_set_size(btn_brightness, 120, 150);
    lv_obj_align(btn_brightness, LV_ALIGN_CENTER, -150, 0);
    lv_obj_add_style(btn_brightness, &btn_style, LV_PART_MAIN);
    
    // Brightness Image
    LV_IMG_DECLARE(brightness_img);
    lv_obj_t *img_brightness = lv_img_create(btn_brightness);
    lv_img_set_src(img_brightness, &brightness_img);
    lv_obj_add_style(img_brightness, &img_style, LV_PART_MAIN);
    lv_obj_align(img_brightness, LV_ALIGN_CENTER, 0, -20);

    // Brightness Label
    lv_obj_t *label_brightness = lv_label_create(btn_brightness);
    lv_label_set_text(label_brightness, "Brightness");
    lv_obj_set_style_text_font(label_brightness, &lv_font_montserrat_16, 0);
    lv_obj_align(label_brightness, LV_ALIGN_CENTER, 0, 40);

    // Volume Button
    lv_obj_t *btn_volume = lv_btn_create(home_screen);
    lv_obj_set_size(btn_volume, 120, 150);
    lv_obj_align(btn_volume, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(btn_volume, &btn_style, LV_PART_MAIN);
    
    // Volume Image
    LV_IMG_DECLARE(volume_img);
    lv_obj_t *img_volume = lv_img_create(btn_volume);
    lv_img_set_src(img_volume, &volume_img);
    lv_obj_add_style(img_volume, &img_style, LV_PART_MAIN);
    lv_obj_align(img_volume, LV_ALIGN_CENTER, 0, -20);

    // Volume Label
    lv_obj_t *label_volume = lv_label_create(btn_volume);
    lv_label_set_text(label_volume, "Volume");
    lv_obj_set_style_text_font(label_volume, &lv_font_montserrat_16, 0);
    lv_obj_align(label_volume, LV_ALIGN_CENTER, 0, 40);

    // Music Button
    lv_obj_t *btn_music = lv_btn_create(home_screen);
    lv_obj_set_size(btn_music, 120, 150);
    lv_obj_align(btn_music, LV_ALIGN_CENTER, 150, 0);
    lv_obj_add_style(btn_music, &btn_style, LV_PART_MAIN);
    
    // Music Image
    LV_IMG_DECLARE(music_img);
    lv_obj_t *img_music = lv_img_create(btn_music);
    lv_img_set_src(img_music, &music_img);
    lv_obj_add_style(img_music, &img_style, LV_PART_MAIN);
    lv_obj_align(img_music, LV_ALIGN_CENTER, 0, -20);

    // Music Label
    lv_obj_t *label_music = lv_label_create(btn_music);
    lv_label_set_text(label_music, "Switch BGM");
    lv_obj_set_style_text_font(label_music, &lv_font_montserrat_16, 0);
    lv_obj_align(label_music, LV_ALIGN_CENTER, 0, 40);

    // Add event callbacks
    lv_obj_add_event_cb(btn_brightness, btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_volume, btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_music, btn_event_cb, LV_EVENT_CLICKED, NULL);
    
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

  // Create title label
  lv_obj_t *label = lv_label_create(brightness_screen);
  lv_label_set_text(label, "BRIGHTNESS");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 30);

  // Create slider container
  lv_obj_t *slider_cont = lv_obj_create(brightness_screen);
  lv_obj_remove_style_all(slider_cont);
  lv_obj_set_size(slider_cont, 320, 100);
  lv_obj_align(slider_cont, LV_ALIGN_CENTER, 0, 0);

  // Create slider with better styling
  brightness_slider = lv_slider_create(slider_cont);
  lv_obj_set_size(brightness_slider, 300, 20);
  lv_obj_center(brightness_slider);
  lv_slider_set_range(brightness_slider, 10, 100);
  lv_slider_set_value(brightness_slider, brightness, LV_ANIM_ON);
  
  // Style the slider
  lv_obj_set_style_bg_color(brightness_slider, lv_color_hex(0x555555), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(brightness_slider, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_radius(brightness_slider, 10, LV_PART_MAIN);
  
  // Style the knob
  lv_obj_set_style_bg_color(brightness_slider, lv_color_hex(0xFFFFFF), LV_PART_KNOB);  
  lv_obj_set_style_bg_opa(brightness_slider, LV_OPA_COVER, LV_PART_KNOB);
  lv_obj_set_style_radius(brightness_slider, 10, LV_PART_KNOB);
  lv_obj_set_style_shadow_width(brightness_slider, 10, LV_PART_KNOB);
  lv_obj_set_style_shadow_color(brightness_slider, lv_color_hex(0x888888), LV_PART_KNOB);
  
  // Style the indicator
  lv_obj_set_style_bg_color(brightness_slider, lv_color_hex(0xFFAA00), LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(brightness_slider, LV_OPA_COVER, LV_PART_INDICATOR);
  
  // Add event callback
  lv_obj_add_event_cb(brightness_slider, slider_brightness_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Create brightness value display label
  lv_obj_t *value_label = lv_label_create(slider_cont);
  lv_label_set_text_fmt(value_label, "%d%%", brightness);
  lv_obj_set_style_text_font(value_label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(value_label, lv_color_white(), 0);
  lv_obj_align(value_label, LV_ALIGN_BOTTOM_MID, 0, 40);
  
  // Update value label when slider changes
  lv_obj_add_event_cb(brightness_slider, [](lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    int val = lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "%d%%", val);
  }, LV_EVENT_VALUE_CHANGED, value_label);

  // Create styled back button
  lv_obj_t *btn_back = lv_btn_create(brightness_screen);
  lv_obj_set_size(btn_back, 100, 50);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_LEFT, 20, -20);
  lv_obj_set_style_radius(btn_back, 10, 0);
  lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x005599), 0);
  lv_obj_set_style_shadow_width(btn_back, 5, 0);
  lv_obj_set_style_shadow_ofs_y(btn_back, 3, 0);
  
  lv_obj_t *label_back = lv_label_create(btn_back);
  lv_label_set_text(label_back, "Back");
  lv_obj_center(label_back);
  
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e) {
    lv_scr_load(home_screen);
  }, LV_EVENT_CLICKED, NULL);

  // Add icons
  lv_obj_t *icon_low = lv_label_create(brightness_screen);
  lv_label_set_text(icon_low, LV_SYMBOL_MINUS);
  lv_obj_set_style_text_font(icon_low, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(icon_low, lv_color_white(), 0);
  lv_obj_align(icon_low, LV_ALIGN_CENTER, -170, 0);
  
  lv_obj_t *icon_high = lv_label_create(brightness_screen);
  lv_label_set_text(icon_high, LV_SYMBOL_PLUS);
  lv_obj_set_style_text_font(icon_high, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(icon_high, lv_color_white(), 0);
  lv_obj_align(icon_high, LV_ALIGN_CENTER, 170, 0);

  lvgl_port_unlock();
}

void create_volume_page() {
  lvgl_port_lock(-1);

  // Create volume screen
  volume_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(volume_screen, lv_color_hex(0x003366), 0);
  lv_obj_set_style_bg_opa(volume_screen, LV_OPA_COVER, 0);

  // Create title label
  lv_obj_t *label = lv_label_create(volume_screen);
  lv_label_set_text(label, "VOLUME");
  lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(label, lv_color_white(), 0);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 30);

  // Create slider container
  lv_obj_t *slider_cont = lv_obj_create(volume_screen);
  lv_obj_remove_style_all(slider_cont);
  lv_obj_set_size(slider_cont, 320, 100);
  lv_obj_align(slider_cont, LV_ALIGN_CENTER, 0, 0);

  // Create slider with better styling
  volume_slider = lv_slider_create(slider_cont);
  lv_obj_set_size(volume_slider, 300, 20);
  lv_obj_center(volume_slider);
  lv_slider_set_range(volume_slider, 0, 100);
  lv_slider_set_value(volume_slider, volume, LV_ANIM_ON);
  
  // Style the slider
  lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x555555), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(volume_slider, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_radius(volume_slider, 10, LV_PART_MAIN);
  
  // Style the knob
  lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0xFFFFFF), LV_PART_KNOB);  
  lv_obj_set_style_bg_opa(volume_slider, LV_OPA_COVER, LV_PART_KNOB);
  lv_obj_set_style_radius(volume_slider, 10, LV_PART_KNOB);
  lv_obj_set_style_shadow_width(volume_slider, 10, LV_PART_KNOB);
  lv_obj_set_style_shadow_color(volume_slider, lv_color_hex(0x888888), LV_PART_KNOB);
  
  // Style the indicator
  lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x00AAFF), LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(volume_slider, LV_OPA_COVER, LV_PART_INDICATOR);
  
  // Add event callback
  lv_obj_add_event_cb(volume_slider, slider_volume_cb, LV_EVENT_VALUE_CHANGED, NULL);

  // Create volume value display label
  lv_obj_t *value_label = lv_label_create(slider_cont);
  lv_label_set_text_fmt(value_label, "%d%%", volume);
  lv_obj_set_style_text_font(value_label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(value_label, lv_color_white(), 0);
  lv_obj_align(value_label, LV_ALIGN_BOTTOM_MID, 0, 40);
  
  // Update value label when slider changes
  lv_obj_add_event_cb(volume_slider, [](lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    int val = lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "%d%%", val);
  }, LV_EVENT_VALUE_CHANGED, value_label);

  // Create styled back button
  lv_obj_t *btn_back = lv_btn_create(volume_screen);
  lv_obj_set_size(btn_back, 100, 50);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_LEFT, 20, -20);
  lv_obj_set_style_radius(btn_back, 10, 0);
  lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x005599), 0);
  lv_obj_set_style_shadow_width(btn_back, 5, 0);
  lv_obj_set_style_shadow_ofs_y(btn_back, 3, 0);
  
  lv_obj_t *label_back = lv_label_create(btn_back);
  lv_label_set_text(label_back, "Back");
  lv_obj_center(label_back);
  
  lv_obj_add_event_cb(btn_back, [](lv_event_t *e) {
    lv_scr_load(home_screen);
  }, LV_EVENT_CLICKED, NULL);

  // Add icons
  lv_obj_t *icon_low = lv_label_create(volume_screen);
  lv_label_set_text(icon_low, LV_SYMBOL_VOLUME_MID);
  lv_obj_set_style_text_font(icon_low, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(icon_low, lv_color_white(), 0);
  lv_obj_align(icon_low, LV_ALIGN_CENTER, -170, 0);
  
  lv_obj_t *icon_high = lv_label_create(volume_screen);
  lv_label_set_text(icon_high, LV_SYMBOL_VOLUME_MAX);
  lv_obj_set_style_text_font(icon_high, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(icon_high, lv_color_white(), 0);
  lv_obj_align(icon_high, LV_ALIGN_CENTER, 170, 0);

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
    if (!is_screen_init) {
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
    lv_label_set_text(ble_status_label, "Projector App is not connected");
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
