#pragma once

#ifndef DISPLAY_LCD_H
#define DISPLAY_LCD_H

#include <LiquidCrystal_I2C.h>

class DisplayLCD
{
private:
    int lcdColumns = 16;
    int lcdRows = 2;
    LiquidCrystal_I2C *lcd;

public:
    DisplayLCD();
    ~DisplayLCD();
    void Initialize();
    void printScreen(const char *row1, const char *row2 = "");
};

DisplayLCD::DisplayLCD(/* args */)
{
    lcd = new LiquidCrystal_I2C(0x27, lcdColumns, lcdRows);
}

DisplayLCD::~DisplayLCD()
{
    delete lcd;
}

void DisplayLCD::Initialize()
{
    lcd->init();
    lcd->backlight();
}

void DisplayLCD::printScreen(const char *row1, const char *row2)
{
    lcd->clear();
    lcd->setCursor(0, 0);
    lcd->print("  VSoftware S.R.L.  ");
    lcd->setCursor(0, 1);
    lcd->print("Ver: 1.0.0-beta");

    lcd->setCursor(0, 2);
    lcd->print(row1);
    lcd->setCursor(0, 3);
    lcd->print(row2);
}

#endif // DISPLAY_LCD_H