#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_VOLUME
#define LV_ATTRIBUTE_IMG_VOLUME
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_VOLUME uint8_t volume_map[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfc, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x80, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x01, 0xff, 0xe0, 0x00, 
  0x00, 0x00, 0x00, 0x01, 0xf8, 0x00, 0x00, 0x7f, 0xf0, 0x00, 
  0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x1f, 0xf8, 0x00, 
  0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x0f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x0f, 0xf8, 0x00, 0x00, 0x03, 0xfe, 0x00, 
  0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x01, 0xff, 0x00, 
  0x00, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x38, 0x00, 0xff, 0x00, 
  0x00, 0x00, 0x00, 0x7f, 0xf8, 0x00, 0x7e, 0x00, 0x7f, 0x80, 
  0x00, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x7f, 0x80, 0x3f, 0xc0, 
  0x00, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x7f, 0xc0, 0x1f, 0xc0, 
  0x00, 0x00, 0x03, 0xff, 0xf8, 0x00, 0x7f, 0xe0, 0x0f, 0xe0, 
  0x00, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x3f, 0xf0, 0x0f, 0xf0, 
  0x00, 0x00, 0x0f, 0xff, 0xf8, 0x00, 0x1f, 0xf8, 0x07, 0xf0, 
  0x00, 0x00, 0x1f, 0xff, 0xf8, 0x00, 0x07, 0xfc, 0x03, 0xf8, 
  0x00, 0x00, 0x3f, 0xff, 0xf8, 0x00, 0x03, 0xfe, 0x03, 0xf8, 
  0x00, 0x00, 0x7f, 0xff, 0xf8, 0x00, 0x00, 0xff, 0x01, 0xf8, 
  0x00, 0x00, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0x01, 0xfc, 
  0x7f, 0xff, 0xff, 0xff, 0xf8, 0x02, 0x00, 0x7f, 0x80, 0xfc, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0xc0, 0x3f, 0x80, 0xfc, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0xe0, 0x1f, 0xc0, 0xfe, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0xf0, 0x1f, 0xc0, 0x7e, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0xf8, 0x0f, 0xe0, 0x7e, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0xfc, 0x0f, 0xe0, 0x7e, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x03, 0xfc, 0x07, 0xe0, 0x7f, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x01, 0xfe, 0x07, 0xe0, 0x3f, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0xfe, 0x07, 0xe0, 0x3f, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x7e, 0x07, 0xf0, 0x3f, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x7e, 0x07, 0xf0, 0x3f, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x7e, 0x03, 0xf0, 0x3f, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x7e, 0x03, 0xf0, 0x3f, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x7e, 0x07, 0xf0, 0x3f, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x7e, 0x07, 0xf0, 0x3f, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0xfe, 0x07, 0xe0, 0x3f, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x01, 0xfc, 0x07, 0xe0, 0x3f, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x03, 0xfc, 0x07, 0xe0, 0x7f, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0xf8, 0x0f, 0xe0, 0x7e, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0xf8, 0x0f, 0xe0, 0x7e, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0xf0, 0x1f, 0xc0, 0x7e, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0xe0, 0x1f, 0xc0, 0xfe, 
  0xff, 0xff, 0xff, 0xff, 0xf8, 0x0f, 0x80, 0x3f, 0x80, 0xfc, 
  0x7f, 0xff, 0xff, 0xff, 0xf8, 0x02, 0x00, 0x7f, 0x80, 0xfc, 
  0x00, 0x00, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x7f, 0x01, 0xfc, 
  0x00, 0x00, 0x7f, 0xff, 0xf8, 0x00, 0x00, 0xff, 0x01, 0xf8, 
  0x00, 0x00, 0x3f, 0xff, 0xf8, 0x00, 0x03, 0xfe, 0x03, 0xf8, 
  0x00, 0x00, 0x1f, 0xff, 0xf8, 0x00, 0x07, 0xfc, 0x03, 0xf8, 
  0x00, 0x00, 0x0f, 0xff, 0xf8, 0x00, 0x1f, 0xf8, 0x07, 0xf0, 
  0x00, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x3f, 0xf0, 0x0f, 0xf0, 
  0x00, 0x00, 0x03, 0xff, 0xf8, 0x00, 0x7f, 0xe0, 0x0f, 0xe0, 
  0x00, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x7f, 0xc0, 0x1f, 0xc0, 
  0x00, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x7f, 0x80, 0x3f, 0xc0, 
  0x00, 0x00, 0x00, 0x7f, 0xf8, 0x00, 0x7e, 0x00, 0x7f, 0x80, 
  0x00, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x38, 0x00, 0xff, 0x00, 
  0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x01, 0xff, 0x00, 
  0x00, 0x00, 0x00, 0x0f, 0xf8, 0x00, 0x00, 0x03, 0xfe, 0x00, 
  0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x0f, 0xfc, 0x00, 
  0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x1f, 0xf8, 0x00, 
  0x00, 0x00, 0x00, 0x01, 0xf8, 0x00, 0x00, 0x7f, 0xf0, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x01, 0xff, 0xe0, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x80, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfc, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

const lv_img_dsc_t volume_img = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 80,
  .header.h = 80,
  .data_size = 800,
  .data = volume_map,
};
