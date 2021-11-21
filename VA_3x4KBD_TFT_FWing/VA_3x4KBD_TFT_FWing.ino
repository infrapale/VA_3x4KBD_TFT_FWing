/*
 * Villa Astrid Wall Terminal 
 * 3x4 character 14segment display elements
 * RFM69 434MHz Radio module
 * Tom Höglund 2020
 * 
 * https://github.com/infrapale/VA_wall_3x4x14_term
 */

#include <stdio.h>
#include <string.h>
//#include <avr/dtostrf.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeSerif18pt7b.h>
#include <Fonts/FreeSerif24pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include "secrets.h"
//#include "sens_db.h"
#include <rfm69_support.h>
#include "light_msg.h"
#include "akbd.h"
#include "TaHa.h" 


#ifdef ESP8266
   #define STMPE_CS 16
   #define TFT_CS   0
   #define TFT_DC   15
   #define SD_CS    2
#endif
#ifdef ESP32
   #define STMPE_CS 32
   #define TFT_CS   15
   #define TFT_DC   33
   #define SD_CS    14
#endif
#ifdef TEENSYDUINO
   #define TFT_DC   10
   #define TFT_CS   4
   #define STMPE_CS 3
   #define SD_CS    8
#endif
#ifdef ARDUINO_STM32_FEATHER
   #define TFT_DC   PB4
   #define TFT_CS   PA15
   #define STMPE_CS PC7
   #define SD_CS    PC5
#endif
#ifdef ARDUINO_NRF52_FEATHER /* BSP 0.6.5 and higher! */
   #define TFT_DC   11
   #define TFT_CS   31
   #define STMPE_CS 30
   #define SD_CS    27
#endif
#if defined(ARDUINO_MAX32620FTHR) || defined(ARDUINO_MAX32630FTHR)
   #define TFT_DC   P5_4
   #define TFT_CS   P5_3
   #define STMPE_CS P3_3
   #define SD_CS    P3_2
#endif


// Anything else!
#if defined (__AVR_ATmega32U4__) || defined(ARDUINO_SAMD_FEATHER_M0) || defined (__AVR_ATmega328P__) || defined(ARDUINO_SAMD_ZERO) || defined(__SAMD51__) || defined(__SAM3X8E__)
   #define STMPE_CS 6
   #define TFT_CS   9
   #define TFT_DC   10
   #define SD_CS    5
#endif


/*
 * RFM69 Definitions for the M0+RFM69 Feather
 */
#define FREQUENCY     RF69_434MHZ
#define RFM69_CS      8
#define RFM69_INT     3
#define RFM69_RST     4
#define RFM69_FREQ    434.0 
#define RFM69_TX_IVAL_100ms  20;

   
#define KBD_NBR_KEYS       12
#define BTN_NBR_BTNS       3
#define LDR_PIN            A2


#define TEXT_SIZE 1
#define TEXT_VISIBLE_ROWS 25
#define TEXT_VISIBLE_CHAR 44

#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

char text_buffer[TEXT_VISIBLE_ROWS][TEXT_VISIBLE_CHAR];
uint8_t show_from=0;
uint8_t insert_at=0;
unsigned long show_menu_millis;
boolean menu_is_active;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
// Singleton instance of the radio driver
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);
int16_t packetnum = 0;  // packet counter, we increment per xmission


struct sensors_struct {
    uint8_t light_val;
};

sensors_struct sensor;

int16_t dec_div[] = {1,10,100,1000,10000};

char buf[16];
akbd kbd3x4(A0);
uint16_t kbd_values[KBD_NBR_KEYS] = {
  52,92,132,198,312,413,550,699,788,871,936,967
};
uint16_t btn_values[BTN_NBR_BTNS] = {707,917,420};
float sensor_value[3] = { 24.1, -5.5, 4.5 };  //indoor -outdoor -water

// Task handler definitions
TaHa taha_kbd_scan;
TaHa taha_btn_scan;
TaHa taha_menu;
TaHa radio_send_handle;
TaHa radio_receive_handle;
//TaHa display_handle;
TaHa local_sensor_handle;
/**
 * @brief  Scan Analog Keyboard, pressed keys are stored in object buffer
 * @param  -
 * @retval -
 */
void scan_kbd (void)
{
  kbd3x4.scan();
}


/**
 * @brief  Control menu using a rotaty encoder
 *         if menu is changed display menu position
 * @param  -
 * @retval -
 */

/*
void read_local_sensors(void)
{
     sensor.light_val = map(analogRead(LDR_PIN),0,1023,0,100); 
     Serial.println(sensor.light_val);    
}
*/
/**
 * @brief Arduino setup function
 * @param -
 * @retval -
 */
void setup() {
  delay(3000);
  Serial.begin(9600);
  Serial.println("VA_3x4KBD_TFT_FWing");
  //menu_init();
  radio_init(RFM69_CS,RFM69_INT,RFM69_RST, RFM69_FREQ);
  radio_send_msg("VA_3x4KBD_TFT_FWing");
  init_light_msg();
  
  kbd3x4.set_nbr_keys(KBD_NBR_KEYS);
  kbd3x4.enable_on_off(true);
  
  for(uint8_t i = 0;i < KBD_NBR_KEYS; i++){ 
      kbd3x4.set_aval(i, kbd_values[i]);
  }
  tft.begin();
  if (!ts.begin()) { 
    Serial.println("Unable to start touchscreen.");
  } 
  else { 
    Serial.println("Touchscreen started."); 
  }
  show_menu_millis = millis();
  menu_is_active = false;
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 24);
  tft.setTextColor(ILI9341_WHITE);  
  //tft.setTextSize(1);
  tft.setFont(&FreeSerif24pt7b);
  //tft.setFont(&FreeMonoBoldOblique12pt7b);
  tft.println("VA_3x4KBD_");
  tft.println("TFT_FWing");
  tft.print("2021");
  
  //tft.print("RST pin = "); tft.println(RFM69_RST,DEC);
  //tft.print("CS pin = ");tft.println(RFM69_CS,DEC);
  //tft.print("INT pin = ");tft.println(RFM69_INT,DEC);
  
  
  /*
   * Define task execution
   */
   
  taha_kbd_scan.set_interval(10,RUN_RECURRING, scan_kbd);
  radio_send_handle.set_interval(500,RUN_RECURRING, radio_tx_handler);
  //radio_receive_handle.set_interval(500,RUN_RECURRING, radio_rx_handler);
  //local_sensor_handle.set_interval(5000,RUN_RECURRING, read_local_sensors);

 
}
/**
 * @brief Run tasks  via scheduler 
 * @param -
 * @retval -
 */
void loop() {
  char btn;
  
  taha_kbd_scan.run();
  taha_btn_scan.run();
  taha_menu.run();
  radio_send_handle.run();
  radio_receive_handle.run();
  local_sensor_handle.run();
  
  btn = kbd3x4.read();
  if (btn) {
    //Serial.println(btn);uint16_t aval = kbd3x4.rd_analog(); Serial.println(aval);
    act_on_kbd3x4(btn);
  }

     if (radio_check_available_msg()) {
    // Should be a message for us now   
    char buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    len = radio_read_msg(buf, RH_RF69_MAX_MESSAGE_LEN);
    //if (rf69.recv(buf, &len)) {
    if (len>0)
      if (!len) return;
      buf[len] = 0;
      Serial.print("Received [");
      Serial.print(len);
      Serial.print("]: ");
      Serial.println((char*)buf);
      //Serial.print("RSSI: ");
      //Serial.println(rfm69.lastRssi(), DEC);
      AddRow((char*)buf);
      parse_msg((char*)buf);
      //printMsgLog();
      if( show_menu_millis > millis()){
        //show_menu();
      }
      else {
        update_display();
      }

      if (strstr((char *)buf, "Hello World")) {
        // Send a reply!
        uint8_t data[] = "And hello back to you";
        //rfm69.send(data, sizeof(data));
        //rfm69.waitPacketSent();
        Serial.println("Sent a reply");
        Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks
      }
    } else {
      Serial.println("Receive failed");
    }

}
