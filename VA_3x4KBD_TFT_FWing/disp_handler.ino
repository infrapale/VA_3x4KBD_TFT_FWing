
#define NBR_DISP_ROWS  4

#include "sens_db.h"

extern sensor_entry_st collect_sens[NBR_COLLECTED_SENSORS];

void update_display(void)
{
  Serial.println("update_display");
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ILI9341_RED); 
  tft.setFont(&FreeSerif24pt7b); 
  tft.setTextColor(ILI9341_LIGHTGREY);
  tft.println("Villa Astrid");
  //setTextColor(uint16_t c, uint16_t bg)
  for (int i = 0; i < NBR_DISP_ROWS-1; i++){
    switch(i){
      case 0: tft.setTextColor(ILI9341_RED);
        break;
      case 1: tft.setTextColor(ILI9341_YELLOW);
        break;
      case 2: tft.setTextColor(ILI9341_CYAN);
        break;
        
    }
    tft.print(collect_sens[i].name);
    tft.print(" ");
    tft.print(String(collect_sens[i].value,collect_sens[i].nbr_decimals)); 
    tft.print(" ");
    tft.println(collect_sens[i].unit); 
  }
}

void printMsgLog(void){
  uint8_t i;
  uint8_t row;
  row = insert_at;
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);

  for(i=0;i< TEXT_VISIBLE_ROWS;i++){
    tft.println(text_buffer[row]);
    //Serial.println(text_buffer[row]);
    if(row == 0 ) {
      row = TEXT_VISIBLE_ROWS-1;
    }  
    else {
       row--;
    }   
  }
}

unsigned long testText() {
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(2);
  for (uint8_t row = 1;row < 100; row++){
     tft.print("Hello World! @ row ");
     tft.println(row);
  }
  
  return micros() - start;
}
