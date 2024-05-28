#include <lvgl.h>
#include <ui.h>
#include <Arduino.h>
#include "lv_drv_conf.h"

#ifdef LGFX_DRIVER
#define LGFX_USE_V1

#ifdef ESP32_2432S032C
#include "LGFX_custom/LGFX_ESP32_2432S032C.h"
#endif

#ifdef WT32SC01PLUS
#include "LGFX_custom/conf_WT32SCO1-Plus.h"
#endif

#ifdef ESP32_2432S032C_ESPI
#include <TFT_eSPI.h>
#include <TFT_Touch.h>

#define TFT_PWM_CHANNEL_BL 7
#define TFT_PWM_FREQ_BL 5000
#define TFT_PWM_BITS_BL 8
#define TFT_PWM_MAX_BL ((1 << TFT_PWM_BITS_BL) - 1)
#endif

#ifdef ESP32_2432S032C_ESPI

TFT_eSPI tft = TFT_eSPI();
TFT_Touch touch = TFT_Touch(XPT2046_CS, XPT2046_CLK, XPT2046_MOSI, XPT2046_MISO);

#else
LGFX gfx;
#ifdef __cplusplus
extern "C" {
#endif
void display_drv_sleep(void);
void display_drv_wakeup(void);
#ifdef __cplusplus
}
#endif

void display_drv_sleep(void){
   gfx.sleep();
}
void display_drv_wakeup(void){
   gfx.wakeup();
}
#endif





/* Display flushing */
void display_drv_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

#ifdef ESP32_2432S032C_ESPI
    uint32_t wh = w * h;
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    while (wh--)
      tft.pushColor(color_p++->full);

#else 
    gfx.startWrite();
    gfx.setAddrWindow(area->x1, area->y1, w, h);
    gfx.writePixelsDMA((lgfx::rgb565_t *)&color_p->full, w * h);
#endif

    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void touch_drv_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    data->state = LV_INDEV_STATE_REL;
    
#ifdef ESP32_2432S032C_ESPI
    if (touch.Pressed()) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x =  touch.X();
        data->point.y = touch.Y();
    }
#else
    if( gfx.getTouch( &touchX, &touchY ) )
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;
    }
#endif
}

void display_drv_init()
{
#ifdef ESP32_2432S032C_ESPI
    tft.init();
    tft.setRotation(1);
    touch.setRotation(1);
    touch.setCal(496, 3421, 700, 3428, 320, 240, 1);

   pinMode(TFT_BL, OUTPUT);
   digitalWrite(TFT_BL, HIGH);
   ledcSetup(TFT_PWM_CHANNEL_BL, TFT_PWM_FREQ_BL, TFT_PWM_BITS_BL);
   ledcAttachPin(TFT_BL, TFT_PWM_CHANNEL_BL);
   ledcWrite(TFT_PWM_CHANNEL_BL, TFT_PWM_MAX_BL);

#else

    gfx.init();
    gfx.initDMA();

#endif
#ifdef SCREEN_ROTATION    
    gfx.setRotation(SCREEN_ROTATION);
#endif
}
void touch_drv_init(){}

#endif //#ifdef LGFX_DRIVER