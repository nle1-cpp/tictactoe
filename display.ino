// Code for displaying wins and losses
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#define SDA 14
#define SCL 13

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup_display()
{
  Wire.begin(SDA, SCL);
  if (!i2CAddrTest(0x27)) {
    lcd = LiquidCrystal_I2C(0x3F, 16, 2);
}
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
}

bool i2CAddrTest(uint8_t addr) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    return true;
  }
  return false;
}

void lcd_print_ln1(const char* string)
{
  lcd.setCursor(0,0);
  lcd.print(string);
}

void lcd_print_ln2(const char* string)
{
  lcd.setCursor(0,1);
  lcd.print(string);
}
