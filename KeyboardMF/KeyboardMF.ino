#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <SD_MMC.h>
#include "Matouch7.h"
#include "EspUsbHost.h"
#include <driver/i2s.h>

#define SCREEN_HD
#ifdef SCREEN_HD
#define SCREEN_W 1024
#define SCREEN_H 600
#endif

// microSD card
#define PIN_SD_CMD 11
#define PIN_SD_CLK 12
#define PIN_SD_D0 13

LGFX lcd;
LGFX_Sprite sprite(&lcd);
LGFX_Sprite spr(&lcd);

int brightnes[6] = { 50, 90, 120, 160, 200, 250 };
int b = 2;
int n = 0;

String fileName = "fileName.txt";
String copyright = "(c)2024-2026 Volos Projects x north-40";
String txt[16];
String spaces[16] = { "|", "|", "|", "|", "|", "|", "|", "|", "|", "|", "|", "|", "|", "|", "|", "|" };
int lineCount = 0;
bool mode = 0;  // 0 is esit  file name, 1 is edit text
bool openMenu = false;
unsigned short modeColor[2] = { TFT_RED, 0x0016 };
bool drawLine = true;
int line = 0;
int maxLine = 0;
int keyC;
String asc = "";
bool capsLock = false;
int lineLength = 0;

const int maxFiles = 32;     // Define maximum number of files to store
String fileNames[maxFiles];  // Array to store file names
int fileCount = 0;
int chosenFile;

void draw() {
  if (drawLine)
    sprite.fillSprite(1);
  else
    sprite.fillSprite(0);
  sprite.setTextSize(2);
  sprite.setTextColor(4);
  sprite.drawString("#" + String(line + 1) + ".", 4, 10);
  sprite.setTextColor(7);
  if (drawLine)
    sprite.drawString(spaces[line], 64, 10);
  sprite.setTextColor(2);
  sprite.drawString(txt[line], 64, 10);
  sprite.pushSprite(65, 70 + (line * 30));
}

void drawName() {

  spr.fillSprite(0);
  spr.setTextColor(1);
  spr.setTextSize(3);
  spr.drawString(fileName, 8, 8);
  spr.pushSprite(110, 6);
}

void listFilesInArray(const char* dirname) {
  File root = SD_MMC.open(dirname);
  fileCount = 0;
  if (!root || !root.isDirectory()) {
    Serial.println("Failed to open directory or not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory() && fileCount < maxFiles) {
      // Store the file name in the array
      fileNames[fileCount] = String(file.name());
      fileCount++;  // Increment file counter
    }
    file = root.openNextFile();  // Move to the next file
  }
}

class MyEspUsbHost : public EspUsbHost {
  void onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier) {
    // if F2
    if (keycode == 59) {
      openMenu = true;
      resetAll();
      listFilesInArray("/");
      lcd.setTextSize(2);
      for (int i = 0; i < fileCount; i++) {
        lcd.drawString(fileNames[i], 90, 80 + (i * 30));
      }
      lcd.fillRect(70, 83 + chosenFile * 30, 10, 10, TFT_YELLOW);
    }

    // if F1
    if (keycode == 58)
      resetAll();

    //IF CAPS LOCK
    if (keycode == 57) {
      capsLock = !capsLock;
      if (capsLock)
        lcd.fillRect(175, 575, 20, 15, TFT_RED);
      else
        lcd.fillRect(175, 575, 20, 15, TFT_SILVER);
    }

    if (keycode == 60) {
      mode = !mode;
      lcd.fillRect(30, 65, 8, 12, modeColor[!mode]);
      lcd.fillRect(30, 12, 8, 12, modeColor[mode]);
    }

    if (mode == 0 && openMenu == false) {
      if ('!' <= ascii && ascii <= '~' && fileName.length() < 18) {
        fileName = fileName + String((char)(ascii - capsLock * 32));
        //spaces[line]=" "+spaces[line];
      }

      if (keycode == 42) {
        fileName = fileName.substring(0, fileName.length() - 1);
        //spaces[line]=spaces[line].substring(1,spaces[line].length());
      }

      drawName();
    }

    if (openMenu) {
      if (keycode == 40) {
        resetAll();
        String ff = "/" + fileNames[chosenFile];
        fileName = fileNames[chosenFile];
        drawName();
        openFile(ff);
        openMenu = false;
      }

      //if down
      if (keycode == 81) {
        if (chosenFile < fileCount - 1) {
          lcd.fillRect(70, 83 + chosenFile * 30, 10, 10, TFT_BLACK);
          chosenFile++;
          lcd.fillRect(70, 83 + chosenFile * 30, 10, 10, TFT_YELLOW);
        }
      }
      // if up
      if (keycode == 82) {
        if (chosenFile > 0) {
          lcd.fillRect(70, 83 + chosenFile * 30, 10, 10, TFT_BLACK);
          chosenFile--;
          lcd.fillRect(70, 83 + chosenFile * 30, 10, 10, TFT_YELLOW);
        }
      }
    }

    if (mode == 1 && openMenu == false) {
      keyC = keycode;
      if ('!' <= ascii && ascii <= '~') {
        txt[line] = txt[line] + String((char)(ascii - capsLock * 32));
        spaces[line] = " " + spaces[line];
      }

      // if F4
      if (keycode == 61)
        saveFile();

      //IF F5
      if (keycode == 62) {
        b++;
        if (b == 6) b = 0;
        lcd.setBrightness(brightnes[b]);
      }

      //IF SPACE
      if (keycode == 44) {
        txt[line] = txt[line] + " ";
        spaces[line] = " " + spaces[line];
      }

      //IF TAB
      if (keycode == 43) {
        txt[line] = txt[line] + "   ";
        spaces[line] = "   " + spaces[line];
      }

      //IF BACKSPACE
      if (keycode == 42) {
        txt[line] = txt[line].substring(0, txt[line].length() - 1);
        spaces[line] = spaces[line].substring(1, spaces[line].length());
      }

      //IF DEL
      if (keycode == 76) {
        txt[line] = "";
        spaces[line] = "|";
      }

      // IF caps loc

      // if up
      if (keycode == 82) {
        if (line > 0) {
          drawLine = 0;
          draw();
          line--;
          drawLine = 1;
        }
      }

      //if down
      if (keycode == 81) {
        if (line < maxLine) {
          drawLine = 0;
          draw();
          line++;
          drawLine = 1;
        }
      }

      // IF ENTER
      if (keycode == 40 && line < 15) {
        drawLine = 0;
        draw();
        if (line == maxLine)
          maxLine++;
        line++;
        drawLine = 1;
      }

      draw();
    }
  };
};

void saveFile() {
  String ff = "/" + fileName;
  File file = SD_MMC.open(ff, FILE_WRITE);  // Open file.txt in write mode

  if (file) {
    for (int i = 0; i < maxLine + 1; i++)
      file.println(txt[i]);  // Write the first line from the String variable
    file.close();            // Don't forget to close the file
  }
}

void openFile(String fff) {

  File file = SD_MMC.open(fff, FILE_READ);

  if (file) {
    Serial.println("Reading file.txt...");

    // Read the file line by line
    while (file.available()) {
      if (lineCount < 16) {
        // Read a line from the file and store it in the array
        txt[maxLine] = file.readStringUntil('\n');  // Read until newline character
        lineLength = txt[maxLine].length();
        for (int j = 0; j < lineLength - 1; j++)
          spaces[maxLine] = " " + spaces[maxLine];
        maxLine++;  // Increment the line count
      }
    }
    file.close();  // Close the file
    Serial.println("File read successfully.");


  } else {
    Serial.println("Failed to open file.txt");
  }

  for (int i = 0; i < maxLine; i++) {
    line = i;
    if (i == maxLine - 1)
      drawLine = 1;
    else
      drawLine = 0;
    draw();
  }
}

MyEspUsbHost usbHost;
void setup(void) {
  //init USB HID
  usbHost.begin();
  usbHost.setHIDLocal(HID_LOCAL_Japan_Katakana);

  // screen init
  lcd.init();
  lcd.setRotation(2);
  lcd.setBrightness(brightnes[b]);

  lcd.fillScreen(0x0016);
  lcd.fillRect(55, 60, 920, 500, TFT_BLACK);
  lcd.fillRect(55, 570, 920, 26, TFT_BLACK);
  lcd.fillRect(55, 6, 403, 40, TFT_BLACK);
  lcd.drawRect(48, 4, 410, 44, TFT_SILVER);
  lcd.drawRect(48, 564, 932, 36, TFT_SILVER);
  lcd.fillRect(170, 570, 30, 24, TFT_SILVER);

  lcd.fillRect(24, 6, 20, 24, TFT_SILVER);
  lcd.fillRect(27, 9, 14, 18, 0x0016);
  lcd.fillRect(24, 14, 20, 8, 0x0016);

  lcd.fillRect(24, 59, 20, 24, TFT_SILVER);
  lcd.fillRect(27, 62, 14, 18, 0x0016);
  lcd.fillRect(24, 67, 20, 8, 0x0016);
  lcd.fillRect(30, 90, 9, 506, 0x8410);
  lcd.fillRect(30, 37, 9, 14, 0x8410);

  lcd.fillRect(30, 12, 8, 12, modeColor[mode]);

  lcd.fillRect(48, 53, 2, 510, TFT_SILVER);
  lcd.fillRect(48, 53, 932, 2, TFT_SILVER);
  lcd.fillRect(48, 563, 932, 2, TFT_SILVER);
  lcd.fillRect(978, 53, 2, 510, TFT_SILVER);

  // static text
  lcd.setTextSize(2);
  lcd.setTextColor(0x7BCF);
  lcd.drawString(copyright, 500, 576);
  lcd.drawString("CAPSLOCK:", 60, 576);

  //buttons
  lcd.fillRect(470, 6, 120, 40, TFT_SILVER);
  lcd.setTextSize(3);
  lcd.setTextColor(TFT_RED);
  lcd.drawString("F1", 480, 16);
  lcd.setTextColor(TFT_BLACK);
  lcd.drawString("NEW", 520, 16);

  lcd.fillRect(600, 6, 130, 40, TFT_SILVER);
  lcd.setTextColor(TFT_RED);
  lcd.drawString("F2", 610, 16);
  lcd.setTextColor(TFT_BLACK);
  lcd.drawString("OPEN", 650, 16);

  lcd.fillRect(740, 6, 130, 40, TFT_SILVER);
  lcd.setTextColor(TFT_RED);
  lcd.drawString("F4", 750, 16);
  lcd.setTextColor(TFT_BLACK);
  lcd.drawString("SAVE", 790, 16);

  lcd.fillRect(880, 6, 100, 40, TFT_SILVER);
  lcd.setTextColor(TFT_RED);
  lcd.drawString("F5", 890, 16);
  lcd.setTextColor(TFT_BLACK);
  lcd.drawString("*", 940, 16);

  lcd.setTextSize(3);
  lcd.setTextColor(TFT_YELLOW);
  lcd.drawString("F3 ", 70, 14);
  sprite.setColorDepth(4);
  sprite.createSprite(900, 30);
  sprite.setPaletteColor(2, 0x00FF00U);

  spr.setColorDepth(1);
  spr.createSprite(340, 38);

  SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
  if (!SD_MMC.begin("/sdcard", true, true)) {
    Serial.println("Card Mount Failed");
    return;
  }

  drawName();
}

void resetAll() {
  fileName = "fileName.txt";
  drawName();
  lcd.fillRect(55, 60, 920, 500, TFT_BLACK);
  line = 0;
  maxLine = 0;
  for (int i = 0; i < 16; i++) {
    txt[i] = "";
    spaces[i] = "|";
  }
}

void loop(void) {
  usbHost.task();
}
