#include "LCD_ST7032.h"
#include "pin_assign.h"

LCD::LCD(uint8_t pin_rst, uint8_t pin_rs, uint8_t pin_cs) :
  _rst(pin_rst), _rs(pin_rs), _cs(pin_cs)
{
  _spi_settings = SPISettings(1000000, MSBFIRST, SPI_MODE0);
}

void LCD::init(){
  pinMode(_rst, OUTPUT);
  pinMode(_cs, OUTPUT);
  pinMode(_rs, OUTPUT);

  digitalWrite(_rst, HIGH);
  digitalWrite(_rs, HIGH);
  digitalWrite(_cs, HIGH);

  digitalWrite(_rst, LOW);
  delay(2);
  send(0x30, COMMAND); // wakeup
  delay(2);
  digitalWrite(_rst, HIGH);
  
  send(0x30, COMMAND); // wakeup
  send(0x30, COMMAND); // wakeup

  send(0x39, COMMAND); // function set
  send(0x14, COMMAND);   // internal osc frequency
  send(0x56, COMMAND);   // power control
  send(0x6D, COMMAND);   // follower control

  send(0x70, COMMAND);   // contrast
  send(0x0C, COMMAND);   // display on
  send(0x06, COMMAND);   // entry mode
  send(0x01, COMMAND);   // clear

  delay(10);
}

void LCD::begin(uint8_t rows, uint8_t cols, uint8_t dotsize){
  SPI.begin();
  init();
  // do we need to call parent begin??
  LCD_BASE::begin(rows, cols, dotsize);
  clear();
  setCursor(0,0);
}

void LCD::send(uint8_t value, uint8_t mode){
  // send mode => RS pin
  // LOW: command; HIGH: data
  bool send_mode = ( mode == DATA );

  noInterrupts();
  SPI.beginTransaction(_spi_settings);
  digitalWrite(_cs, LOW);
  digitalWrite(_rs, send_mode);
  delayMicroseconds(150); // do we need this?
  SPI.transfer(value);
  delayMicroseconds(150); // do we need this?
  SPI.endTransaction();
  digitalWrite(_cs, HIGH);
  interrupts();
}
void LCD::printer(const char * s){
  char printed[17];
  strcpy(printed,s);
  if (strlen(s) < 17){
    for(unsigned int i = 0; i < 17-strlen(s); i++){
      strcat(printed," ");
    }
  }
  print(printed);
}
LCD lcd(LCD_RST, LCD_RS, LCD_CS);