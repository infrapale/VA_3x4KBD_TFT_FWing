#include "arduino.h"

#include "light_msg.h"

#define MSG_BUF_LEN 32
#define MSG_BUF_LEN_MASK 0b00011111;

struct msg_buf_struct {
  char code[CODE_LEN];  // ring buffer
  char zone[ZONE_LEN];  // ring buffer
  char func[FUNC_LEN];
} msg_buf[MSG_BUF_LEN];  

byte buf_wr_indx;
byte buf_rd_indx;

/**
 * @brief
 * @param
 * @retval
 */
void init_light_msg(void){
     // clear code and zone buffers
    for(uint8_t i=0;i<MSG_BUF_LEN; i++){
        msg_buf[i].zone[0] = 0;
        msg_buf[i].code[0] = 0;
        msg_buf[i].func[0] = 0;
    }
    buf_wr_indx = 0;
    buf_rd_indx = 0;

}
/**
 * @brief Run action based on key
 * @param
 * @retval
 */
void act_on_kbd3x4(char btn){
    char func[2] ="1";
    if (btn & 0b10000000)
    {
       func[0] = '0';
       btn = btn & 0b01111111; 
    }
    switch(btn){
       case '0': 
          add_code("TK1","RWC_1",func);
          add_code("MH2","RWC_2",func); 
          break;        
       case '1': 
          add_code("MH2","RET_1",func); 
          break;
       case '2': 
          add_code("TK1","RPOLK",func); 
          break;
       case '3': 
          add_code("MH2","RMH21",func); 
          add_code("MH2","RMH22",func); 
          break;     
       case '4': 
          add_code("TK1","RPARV",func); 
          break;   
       case '5': 
          add_code("MH1","RMH11",func); 
          add_code("MH1","RMH12",func); 
          add_code("MH1","RMH13",func); 
          break;   
       case '6': 
          add_code("MH2","RKHH2",func); 
          add_code("MH2","RPSH1",func);
          add_code("TK1","RKHH1",func);
          add_code("TK1","SAUN1",func);
          break;   
       case '7': 
          add_code("TK1","RTUP1",func); 
          add_code("TK1","RTUP2",func);
                 break;   
       case '8': 
          add_code("MH1","RKOK1",func); 
          add_code("MH1","RKOK2",func); 
          add_code("MH1","RKOK3",func); 
          add_code("MH1","RKOK4",func); 
          add_code("MH1","RKOK5",func); 
          break;   
       case '9': 
          add_code("MH1","xxxxx","1"); 
          break;   
       case '*': 
          add_code("MH1","*.OFF","0"); 
          add_code("MH2","*.OFF","0");
          add_code("TK1","*.OFF","0");
          break;   
       case '#': 
          add_code("MH1","RKOK3",func); 
          add_code("MH1","RKOK4",func); 
          add_code("MH1","RKOK5",func); 
          break;   
    }  
}


/**
 * @brief  Add light control codes to buffer
 * @param
 * @retval
 */
 void add_code(const char *new_zone, const char *new_code, const char *new_func){
    int i;
    for(i = 0; i < CODE_LEN; i++) {
        if (new_code[i] != 0) { 
            msg_buf[buf_wr_indx].code[i] = new_code[i];
        } 
        else {
            msg_buf[buf_wr_indx].code[i] =0;
        }   
    }
    for(i = 0; i < ZONE_LEN; i++) {
        if (new_code[i] != 0) { 
             msg_buf[buf_wr_indx].zone[i] = new_zone[i];
        } 
        else {
            msg_buf[buf_wr_indx].zone[i] =0;
        }   
    }
    for(i = 0; i < FUNC_LEN; i++) {
        if (new_func[i] != 0) { 
             msg_buf[buf_wr_indx].func[i] = new_func[i];
        } 
        else {
            msg_buf[buf_wr_indx].func[i] =0;
        }   
    }
    buf_wr_indx = ++buf_wr_indx & MSG_BUF_LEN_MASK;   
}

/**
 * @brief Send light control message over radio
 * @param
 * @retval
 */
void radiate_msg( const char *zone, const char *relay_addr, char *func ) {
    char rf69_packet[RH_RF69_MAX_MESSAGE_LEN];
    if (json_char_array(rf69_packet, RH_RF69_MAX_MESSAGE_LEN, zone, relay_addr, func, "") > 0)
    {
      radio_send_msg(rf69_packet);
      Serial.println(rf69_packet);
      
     
    }
    else 
    {
      Serial.println("RFM69 message length was exceeded"); 
    }
}
/**
 * @brief  Task activating radio transmission
 * @param
 * @retval
 */
void radio_tx_handler(void){
    if (msg_buf[buf_rd_indx].code[0] != 0){
        radiate_msg( msg_buf[buf_rd_indx].zone, msg_buf[buf_rd_indx].code, msg_buf[buf_rd_indx].func);
        Serial.print(msg_buf[buf_rd_indx].zone); 
        Serial.println(msg_buf[buf_rd_indx].code);
        msg_buf[buf_rd_indx].zone[0] = 0;
        msg_buf[buf_rd_indx].code[0] = 0;
        msg_buf[buf_rd_indx].code[0] = 0;
        buf_rd_indx = ++buf_rd_indx & MSG_BUF_LEN_MASK; 
        radio_send_handle.delay_task(2000);
    }
}

void show_menu(){
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ILI9341_RED); 
  tft.setFont(&FreeSerif24pt7b); 
  tft.setTextColor(ILI9341_LIGHTGREY);
  tft.println("Menu is under");
  tft.println("construction");
  
}

void AddRow( char *txt){
   uint8_t i;
   if(++insert_at > TEXT_VISIBLE_ROWS-1){
      insert_at = 0;      
   } 
   if (insert_at < TEXT_VISIBLE_ROWS){
      for (i=0;txt[i]!=0 && i< TEXT_VISIBLE_CHAR-1;i++){
         text_buffer[insert_at][i]=txt[i];
      }   
      text_buffer[insert_at][++i] = 0;
   }   
   
 
   Serial.println(txt);
}

void update_display(void){
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ILI9341_RED); 
  tft.setFont(&FreeSerif24pt7b); 
  tft.setTextColor(ILI9341_LIGHTGREY);
  tft.println("Villa Astrid");
  //setTextColor(uint16_t c, uint16_t bg)
  for (int i = 0; i < NBR_COLLECTED_SENSORS; i++){
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
    tft.println(collect_sens[i].value);  
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