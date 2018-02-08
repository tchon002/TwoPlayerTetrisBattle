#include <SoftwareSerial.h>
#include "Adafruit_GFX.h"   // Core graphics library
#include "Adafruit_LEDBackpack.h" // Hardware-specific library
#include <Wire.h>
#include <math.h>
#include <SPI.h>

SoftwareSerial atmega(10, 11); // RX, TX
Adafruit_8x16matrix matrix = Adafruit_8x16matrix();
bool positions[8][16] = {0};

static const uint8_t PROGMEM
  smile_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10100101,
    B10011001,
    B01000010,
    B00111100 },

  frown_bmp[] =
  { B00111100,
    B01000010,
    B10100101,
    B10000001,
    B10011001,
    B10100101,
    B01000010,
    B00111100 };

struct Coordinate{
  int x;
  int y;
};

struct Piece{
  Coordinate coords[4];
  int type;
};

bool game_over = false;
bool is_stored = false;
bool game_start = false;
bool incoming_powerup = false;
bool won_game = false;
Piece store;

void setup() {
  matrix.begin(0x70);
  randomSeed(analogRead(0));
  // Open serial communications and wait for port to open:
  Serial.begin(250000);
  while (!Serial);
  Serial.println("Starting up...");
 
  // set the data rate for the Atmega1284 port
  atmega.begin(9600); //Also set on the Atmega side
  while(!atmega.available());
  if(atmega.read() == 0xFF)
  {
    game_start = true;
  }
  Serial.println("starting game");
}


void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
  asm volatile ("  jmp 0");  
}  

unsigned char get_val() {
  unsigned char val;
  for(int i = 0; i < 1000; i++)
  {
    if(atmega.available())
    {
      val = atmega.read();
      break;
    }
    else
    {
      val = 0x00;
    }
  }
  Serial.print("Recieved: ");
  Serial.println(val);
  if(val == 0x21)
  {
    game_over = true;
    won_game = true;
  }
  if(val == 0x23)
  {
    incoming_powerup = true;
  }
  return val;
}

void make_piece(int num, Piece &active)
{
  Coordinate temp1, temp2, temp3, temp4;
    if(num == 0)
    {
      temp1.y = 8;
      temp1.x = 5;
      temp2.y = 9;
      temp2.x = 5;
      temp3.y = 9;
      temp3.x = 4;
      temp4.y = 9;
      temp4.x = 3;
      active.type = 0;
    }
    if(num == 1)
    {
      temp1.y = 9;
      temp1.x = 5;
      temp2.y = 8;
      temp2.x = 5;
      temp3.y = 8;
      temp3.x = 4;
      temp4.y = 8;
      temp4.x = 3;
      active.type = 1;      
    }
    if(num == 2)
    {
      temp1.y = 9;
      temp1.x = 4;
      temp2.y = 9;
      temp2.x = 3;
      temp3.y = 8;
      temp3.x = 4;
      temp4.y = 8;
      temp4.x = 3;
      active.type = 2;
    }
    if(num == 3)
    {
      temp1.y = 9;
      temp1.x = 5;
      temp2.y = 9;
      temp2.x = 4;
      temp3.y = 9;
      temp3.x = 3;
      temp4.y = 8;
      temp4.x = 4;
      active.type = 3;
    }
    if(num == 4)
    {
      temp1.y = 9;
      temp1.x = 4;
      temp2.y = 8;
      temp2.x = 4;
      temp3.y = 7;
      temp3.x = 4;
      temp4.y = 6;
      temp4.x = 4;
      active.type = 4;
    }
    if(num == 5)
    {
      temp1.y = 8;
      temp1.x = 5;
      temp2.y = 8;
      temp2.x = 4;
      temp3.y = 9;
      temp3.x = 4;
      temp4.y = 9;
      temp4.x = 3;
      active.type = 5;
    }
    if(num == 6)
    {
      temp1.y = 9;
      temp1.x = 5;
      temp2.y = 9;
      temp2.x = 4;
      temp3.y = 8;
      temp3.x = 4;
      temp4.y = 8;
      temp4.x = 3;
      active.type = 6;
    }
    active.coords[0] = temp1;
    active.coords[1] = temp2;
    active.coords[2] = temp3;
    active.coords[3] = temp4;
}

void read_input(int &counter, Piece &active, Piece &store, int &type)
{
  unsigned char input = get_val();
  if(input == 0x40) //store
  {
    if(!is_stored)
    {
      store = active;
      make_piece(random(7),active);
      for(int i = 0; i < 4; i++)
      {
        active.coords[i].y -= counter;
      }
    }
    else
    {
      Piece temp = store;
      store = active;
      active = temp;
    }
    is_stored = true;
  }
  else if(input == 0x41) //spin
  {
    if(active.type == 0)
    {
      if(active.coords[0].y == active.coords[1].y - 1)
      {
        if(active.coords[1].y < 9)
        {
          if(positions[active.coords[2].x + 1][active.coords[2].y + 1] == 0 &&
             positions[active.coords[2].x][active.coords[2].y + 1] == 0 &&
             positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
             active.coords[2].y - 1 >= 0)
          {
            active.coords[0].x = active.coords[2].x + 1;
            active.coords[0].y = active.coords[2].y + 1;
            active.coords[1].x = active.coords[2].x;
            active.coords[1].y = active.coords[2].y + 1;
            active.coords[3].x = active.coords[2].x;
            active.coords[3].y = active.coords[2].y - 1;
          }
        }
      }
      else if(active.coords[0].x == active.coords[1].x + 1)
      {
        if(active.coords[1].y <= 9)
        {
          if(positions[active.coords[2].x + 1][active.coords[2].y] == 0 &&
             positions[active.coords[2].x - 1][active.coords[2].y] == 0 &&
             positions[active.coords[2].x - 1][active.coords[2].y + 1] == 0 &&
             active.coords[2].x - 1 >= 0)
          {
            active.coords[0].x = active.coords[2].x - 1;
            active.coords[0].y = active.coords[2].y + 1;
            active.coords[1].x = active.coords[2].x - 1;
            active.coords[1].y = active.coords[2].y;
            active.coords[3].x = active.coords[2].x + 1;
            active.coords[3].y = active.coords[2].y;
          }
        }
      }
      else if(active.coords[0].y == active.coords[1].y + 1)
      {
        if(active.coords[0].y <= 9)
        {
          if(positions[active.coords[2].x][active.coords[2].y + 1] == 0 &&
             positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
             positions[active.coords[2].x - 1][active.coords[2].y - 1] == 0 &&
             active.coords[2].y - 1 >= 0)
          {
            active.coords[0].x = active.coords[2].x - 1;
            active.coords[0].y = active.coords[2].y - 1;
            active.coords[1].x = active.coords[2].x;
            active.coords[1].y = active.coords[2].y - 1;
            active.coords[3].x = active.coords[2].x;
            active.coords[3].y = active.coords[2].y + 1;
          }
        }
      }
      else if(active.coords[0].x == active.coords[1].x - 1)
      {
        if(active.coords[1].x + 1 <= 7)
        {
          if(positions[active.coords[2].x + 1][active.coords[2].y] == 0 &&
             positions[active.coords[2].x + 1][active.coords[2].y - 1] == 0 &&
             positions[active.coords[2].x - 1][active.coords[2].y] == 0 &&
             active.coords[2].x - 1 >= 0)
          {
            active.coords[0].x = active.coords[2].x + 1;
            active.coords[0].y = active.coords[2].y - 1;
            active.coords[1].x = active.coords[2].x + 1;
            active.coords[1].y = active.coords[2].y;
            active.coords[3].x = active.coords[2].x - 1;
            active.coords[3].y = active.coords[2].y;
          }
        }
      }
    }
    else if(active.type == 1)
    {
      if(active.coords[0].y == active.coords[1].y + 1)
      {
        if(positions[active.coords[2].x - 1][active.coords[2].y + 1] == 0 &&
           positions[active.coords[2].x][active.coords[2].y + 1] == 0 &&
           positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
           active.coords[2].y - 1 >= 0)
        {
          active.coords[0].x = active.coords[2].x - 1;
          active.coords[0].y = active.coords[2].y + 1;
          active.coords[1].x = active.coords[2].x;
          active.coords[1].y = active.coords[2].y + 1;
          active.coords[3].x = active.coords[2].x;
          active.coords[3].y = active.coords[2].y - 1;
        }
      }
      else if(active.coords[0].x == active.coords[1].x - 1)
      {
        if(positions[active.coords[2].x + 1][active.coords[2].y] == 0 &&
           positions[active.coords[2].x - 1][active.coords[2].y] == 0 &&
           positions[active.coords[2].x - 1][active.coords[2].y - 1] == 0 &&
           active.coords[2].x + 1 <= 7)
        {
          active.coords[0].x = active.coords[2].x - 1;
          active.coords[0].y = active.coords[2].y - 1;
          active.coords[1].x = active.coords[2].x - 1;
          active.coords[1].y = active.coords[2].y;
          active.coords[3].x = active.coords[2].x + 1;
          active.coords[3].y = active.coords[2].y;
        }
      }
      else if(active.coords[0].y == active.coords[1].y - 1)
      {
        if(positions[active.coords[2].x][active.coords[2].y + 1] == 0 &&
           positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
           positions[active.coords[2].x + 1][active.coords[2].y - 1] == 0 &&
           active.coords[2].y + 1 <= 9)
        {
          active.coords[0].x = active.coords[2].x + 1;
          active.coords[0].y = active.coords[2].y - 1;
          active.coords[1].x = active.coords[2].x;
          active.coords[1].y = active.coords[2].y - 1;
          active.coords[3].x = active.coords[2].x;
          active.coords[3].y = active.coords[2].y + 1;
        }
      }
      else if(active.coords[0].x == active.coords[1].x + 1)
      {
        if(positions[active.coords[2].x + 1][active.coords[2].y] == 0 &&
           positions[active.coords[2].x + 1][active.coords[2].y + 1] == 0 &&
           positions[active.coords[2].x - 1][active.coords[2].y] == 0 &&
           active.coords[2].x - 1 >= 0)
        {
          active.coords[0].x = active.coords[2].x + 1;
          active.coords[0].y = active.coords[2].y + 1;
          active.coords[1].x = active.coords[2].x + 1;
          active.coords[1].y = active.coords[2].y;
          active.coords[3].x = active.coords[2].x - 1;
          active.coords[3].y = active.coords[2].y;
        }
      }
    }
    else if(active.type == 3)
    {
      if(active.coords[0].x == active.coords[1].x + 1)
      {
        if(active.coords[1].y < 9)
        {
          if(positions[active.coords[1].x][active.coords[1].y + 1] == 0)
          {
            active.coords[0].x = active.coords[1].x;
            active.coords[0].y = active.coords[1].y + 1;
            active.coords[2].x = active.coords[1].x;
            active.coords[2].y = active.coords[1].y - 1;
            active.coords[3].x = active.coords[1].x + 1;
            active.coords[3].y = active.coords[1].y;
          }
        }
      }
      else if(active.coords[0].y == active.coords[1].y + 1)
      {
        if(active.coords[1].x - 1 >= 0)
        {
          if(positions[active.coords[1].x - 1][active.coords[1].y] == 0)
          {
            active.coords[0].x = active.coords[1].x - 1;
            active.coords[0].y = active.coords[1].y;
            active.coords[2].x = active.coords[1].x + 1;
            active.coords[2].y = active.coords[1].y;
            active.coords[3].x = active.coords[1].x;
            active.coords[3].y = active.coords[1].y + 1;
          }
        }
      }
      else if(active.coords[0].x == active.coords[1].x - 1)
      {
        if(active.coords[1].y - 1 >= 0)
        {
          if(positions[active.coords[1].x][active.coords[1].y - 1] == 0)
          {
            active.coords[0].x = active.coords[1].x;
            active.coords[0].y = active.coords[1].y - 1;
            active.coords[2].x = active.coords[1].x;
            active.coords[2].y = active.coords[1].y + 1;
            active.coords[3].x = active.coords[1].x - 1;
            active.coords[3].y = active.coords[1].y;
          }
        }
      }
      else if(active.coords[0].y == active.coords[1].y - 1)
      {
        if(active.coords[1].x + 1 <= 7)
        {
          if(positions[active.coords[1].x + 1][active.coords[1].y] == 0)
          {
            active.coords[0].x = active.coords[1].x + 1;
            active.coords[0].y = active.coords[1].y;
            active.coords[2].x = active.coords[1].x - 1;
            active.coords[2].y = active.coords[1].y;
            active.coords[3].x = active.coords[1].x;
            active.coords[3].y = active.coords[1].y - 1;
          }
        }
      }
    }
    else if(active.type == 4)
    {
      if(active.coords[0].x == active.coords[1].x + 1)
      {
        if(positions[active.coords[2].x][active.coords[2].y + 1] == 0 && positions[active.coords[2].x][active.coords[2].y - 1] == 0 && positions[active.coords[2].x][active.coords[2].y - 2] == 0
           && active.coords[2].y - 2 >= 0 && active.coords[2].y - 1 > 0)
        {
          active.coords[0].x = active.coords[2].x;
          active.coords[0].y = active.coords[2].y + 1;
          active.coords[1].x = active.coords[2].x;
          active.coords[1].y = active.coords[2].y;
          active.coords[2].y = active.coords[2].y - 1;
          active.coords[3].x = active.coords[2].x;
          active.coords[3].y = active.coords[2].y - 1;
        }
      }
      else
      {
        if(positions[active.coords[1].x + 2][active.coords[1].y] == 0 && positions[active.coords[1].x + 1][active.coords[1].y] == 0 && positions[active.coords[1].x - 1][active.coords[1].y] == 0
           && active.coords[1].x + 2 <= 7 && active.coords[1].x - 1 >= 0)
        {
          active.coords[0].x = active.coords[1].x + 2;
          active.coords[0].y = active.coords[1].y;
          active.coords[1].x = active.coords[1].x + 1;
          active.coords[2].x = active.coords[1].x - 1;
          active.coords[2].y = active.coords[1].y;
          active.coords[3].x = active.coords[1].x - 2;
          active.coords[3].y = active.coords[1].y;
        }
      }
    }
    else if(active.type == 5)
    {
      if(active.coords[0].x - 1 == active.coords[1].x)
      {
        if(positions[active.coords[1].x - 1][active.coords[1].y] == 0 && positions[active.coords[1].x - 1][active.coords[1].y - 1] == 0
           && active.coords[1].y - 1 >= 0)
        {
          active.coords[0].x = active.coords[1].x;
          active.coords[0].y = active.coords[1].y + 1;
          active.coords[2].x = active.coords[1].x - 1;
          active.coords[2].y = active.coords[1].y;
          active.coords[3].y = active.coords[1].y - 1;
        }
      }
      else
      {
        if(positions[active.coords[1].x + 1][active.coords[1].y] == 0 && positions[active.coords[1].x + 1][active.coords[1].y + 1] == 0
           && active.coords[1].x + 1 <= 7)
        {
          active.coords[0].x = active.coords[1].x + 1;
          active.coords[0].y = active.coords[1].y;
          active.coords[2].x = active.coords[1].x;
          active.coords[2].y = active.coords[1].y + 1;
          active.coords[3].y = active.coords[1].y + 1;
        }
      }
    }
    else if(active.type == 6)
    {
      if(active.coords[0].x - 1 == active.coords[1].x)
      {
        if(positions[active.coords[2].x - 1][active.coords[2].y + 1] == 0 && positions[active.coords[2].x][active.coords[2].y - 1] == 0
           && active.coords[2].y - 1 >= 0)
        {
          active.coords[0].x = active.coords[2].x - 1;
          active.coords[1].x = active.coords[2].x - 1;
          active.coords[1].y = active.coords[2].y;
          active.coords[3].x = active.coords[2].x;
          active.coords[3].y = active.coords[2].y - 1;
         }
       }
       else
       {
         if(positions[active.coords[2].x + 1][active.coords[2].y + 1] == 0 && positions[active.coords[2].x][active.coords[2].y + 1] == 0
            && active.coords[2].x + 1 <= 7)
         {
           active.coords[0].x = active.coords[2].x + 1;
           active.coords[1].x = active.coords[2].x;
           active.coords[1].y = active.coords[2].y + 1;
           active.coords[3].x = active.coords[2].x - 1;
           active.coords[3].y = active.coords[2].y;
         }
        }
      }
    }
    else if(input == 0x42) //left
    {
       if(positions[active.coords[0].x + 1][active.coords[0].y] == 0 && 
          positions[active.coords[1].x + 1][active.coords[1].y] == 0 &&
          positions[active.coords[2].x + 1][active.coords[2].y] == 0 &&
          positions[active.coords[3].x + 1][active.coords[3].y] == 0 &&
          active.coords[0].x < 7 &&
          active.coords[1].x < 7 &&
          active.coords[2].x < 7 &&
          active.coords[3].x < 7)
       {
          active.coords[0].x += 1;
          active.coords[1].x += 1;
          active.coords[2].x += 1;
          active.coords[3].x += 1;       
       }
    }
    else if(input == 0x43) //right
    {
      if(positions[active.coords[0].x - 1][active.coords[0].y] == 0 && 
         positions[active.coords[1].x - 1][active.coords[1].y] == 0 &&
         positions[active.coords[2].x - 1][active.coords[2].y] == 0 &&
         positions[active.coords[3].x - 1][active.coords[3].y] == 0 &&
         active.coords[0].x > 0 &&
         active.coords[1].x > 0 &&
         active.coords[2].x > 0 &&
         active.coords[3].x > 0)
      {
         active.coords[0].x -= 1;
         active.coords[1].x -= 1;
         active.coords[2].x -= 1;
         active.coords[3].x -= 1;          
      }
    }
    else if(input == 0x44) //down
    {
       if(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
          positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
          positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
          positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
          active.coords[0].y > 0 &&
          active.coords[1].y > 0 &&
          active.coords[2].y > 0 &&
          active.coords[3].y > 0)
       {          
         active.coords[0].y -= 1;
         active.coords[1].y -= 1;
         active.coords[2].y -= 1;
         active.coords[3].y -= 1;         
       }
    }
    else if(input == 0x21)
    {
      game_over = true;
      won_game = true;
    }
}

//void read_swapped_input(int &counter, Piece &active, Piece &store, int &type)
//{
//  unsigned char input = get_val();
//  if(input == 0x40) //store
//  {
//    if(!is_stored)
//    {
//      store = active;
//      make_piece(random(7),active);
//      for(int i = 0; i < 4; i++)
//      {
//        active.coords[i].y -= counter;
//      }
//    }
//    else
//    {
//      Piece temp = store;
//      store = active;
//      active = temp;
//    }
//    is_stored = true;
//  }
//  else if(input == 0x44) //spin
//  {
//    if(active.type == 0)
//    {
//      if(active.coords[0].y == active.coords[1].y - 1)
//      {
//        if(active.coords[1].y < 9)
//        {
//          if(positions[active.coords[2].x + 1][active.coords[2].y + 1] == 0 &&
//             positions[active.coords[2].x][active.coords[2].y + 1] == 0 &&
//             positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//             active.coords[2].y - 1 >= 0)
//          {
//            active.coords[0].x = active.coords[2].x + 1;
//            active.coords[0].y = active.coords[2].y + 1;
//            active.coords[1].x = active.coords[2].x;
//            active.coords[1].y = active.coords[2].y + 1;
//            active.coords[3].x = active.coords[2].x;
//            active.coords[3].y = active.coords[2].y - 1;
//          }
//        }
//      }
//      else if(active.coords[0].x == active.coords[1].x + 1)
//      {
//        if(active.coords[1].y <= 9)
//        {
//          if(positions[active.coords[2].x + 1][active.coords[2].y] == 0 &&
//             positions[active.coords[2].x - 1][active.coords[2].y] == 0 &&
//             positions[active.coords[2].x - 1][active.coords[2].y + 1] == 0 &&
//             active.coords[2].x - 1 >= 0)
//          {
//            active.coords[0].x = active.coords[2].x - 1;
//            active.coords[0].y = active.coords[2].y + 1;
//            active.coords[1].x = active.coords[2].x - 1;
//            active.coords[1].y = active.coords[2].y;
//            active.coords[3].x = active.coords[2].x + 1;
//            active.coords[3].y = active.coords[2].y;
//          }
//        }
//      }
//      else if(active.coords[0].y == active.coords[1].y + 1)
//      {
//        if(active.coords[0].y <= 9)
//        {
//          if(positions[active.coords[2].x][active.coords[2].y + 1] == 0 &&
//             positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//             positions[active.coords[2].x - 1][active.coords[2].y - 1] == 0 &&
//             active.coords[2].y - 1 >= 0)
//          {
//            active.coords[0].x = active.coords[2].x - 1;
//            active.coords[0].y = active.coords[2].y - 1;
//            active.coords[1].x = active.coords[2].x;
//            active.coords[1].y = active.coords[2].y - 1;
//            active.coords[3].x = active.coords[2].x;
//            active.coords[3].y = active.coords[2].y + 1;
//          }
//        }
//      }
//      else if(active.coords[0].x == active.coords[1].x - 1)
//      {
//        if(active.coords[1].x + 1 <= 7)
//        {
//          if(positions[active.coords[2].x + 1][active.coords[2].y] == 0 &&
//             positions[active.coords[2].x + 1][active.coords[2].y - 1] == 0 &&
//             positions[active.coords[2].x - 1][active.coords[2].y] == 0 &&
//             active.coords[2].x - 1 >= 0)
//          {
//            active.coords[0].x = active.coords[2].x + 1;
//            active.coords[0].y = active.coords[2].y - 1;
//            active.coords[1].x = active.coords[2].x + 1;
//            active.coords[1].y = active.coords[2].y;
//            active.coords[3].x = active.coords[2].x - 1;
//            active.coords[3].y = active.coords[2].y;
//          }
//        }
//      }
//    }
//    else if(active.type == 1)
//    {
//      if(active.coords[0].y == active.coords[1].y + 1)
//      {
//        if(positions[active.coords[2].x - 1][active.coords[2].y + 1] == 0 &&
//           positions[active.coords[2].x][active.coords[2].y + 1] == 0 &&
//           positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//           active.coords[2].y - 1 >= 0)
//        {
//          active.coords[0].x = active.coords[2].x - 1;
//          active.coords[0].y = active.coords[2].y + 1;
//          active.coords[1].x = active.coords[2].x;
//          active.coords[1].y = active.coords[2].y + 1;
//          active.coords[3].x = active.coords[2].x;
//          active.coords[3].y = active.coords[2].y - 1;
//        }
//      }
//      else if(active.coords[0].x == active.coords[1].x - 1)
//      {
//        if(positions[active.coords[2].x + 1][active.coords[2].y] == 0 &&
//           positions[active.coords[2].x - 1][active.coords[2].y] == 0 &&
//           positions[active.coords[2].x - 1][active.coords[2].y - 1] == 0 &&
//           active.coords[2].x + 1 <= 7)
//        {
//          active.coords[0].x = active.coords[2].x - 1;
//          active.coords[0].y = active.coords[2].y - 1;
//          active.coords[1].x = active.coords[2].x - 1;
//          active.coords[1].y = active.coords[2].y;
//          active.coords[3].x = active.coords[2].x + 1;
//          active.coords[3].y = active.coords[2].y;
//        }
//      }
//      else if(active.coords[0].y == active.coords[1].y - 1)
//      {
//        if(positions[active.coords[2].x][active.coords[2].y + 1] == 0 &&
//           positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//           positions[active.coords[2].x + 1][active.coords[2].y - 1] == 0 &&
//           active.coords[2].y + 1 <= 9)
//        {
//          active.coords[0].x = active.coords[2].x + 1;
//          active.coords[0].y = active.coords[2].y - 1;
//          active.coords[1].x = active.coords[2].x;
//          active.coords[1].y = active.coords[2].y - 1;
//          active.coords[3].x = active.coords[2].x;
//          active.coords[3].y = active.coords[2].y + 1;
//        }
//      }
//      else if(active.coords[0].x == active.coords[1].x + 1)
//      {
//        if(positions[active.coords[2].x + 1][active.coords[2].y] == 0 &&
//           positions[active.coords[2].x + 1][active.coords[2].y + 1] == 0 &&
//           positions[active.coords[2].x - 1][active.coords[2].y] == 0 &&
//           active.coords[2].x - 1 >= 0)
//        {
//          active.coords[0].x = active.coords[2].x + 1;
//          active.coords[0].y = active.coords[2].y + 1;
//          active.coords[1].x = active.coords[2].x + 1;
//          active.coords[1].y = active.coords[2].y;
//          active.coords[3].x = active.coords[2].x - 1;
//          active.coords[3].y = active.coords[2].y;
//        }
//      }
//    }
//    else if(active.type == 3)
//    {
//      if(active.coords[0].x == active.coords[1].x + 1)
//      {
//        if(active.coords[1].y < 9)
//        {
//          if(positions[active.coords[1].x][active.coords[1].y + 1] == 0)
//          {
//            active.coords[0].x = active.coords[1].x;
//            active.coords[0].y = active.coords[1].y + 1;
//            active.coords[2].x = active.coords[1].x;
//            active.coords[2].y = active.coords[1].y - 1;
//            active.coords[3].x = active.coords[1].x + 1;
//            active.coords[3].y = active.coords[1].y;
//          }
//        }
//      }
//      else if(active.coords[0].y == active.coords[1].y + 1)
//      {
//        if(active.coords[1].x - 1 >= 0)
//        {
//          if(positions[active.coords[1].x - 1][active.coords[1].y] == 0)
//          {
//            active.coords[0].x = active.coords[1].x - 1;
//            active.coords[0].y = active.coords[1].y;
//            active.coords[2].x = active.coords[1].x + 1;
//            active.coords[2].y = active.coords[1].y;
//            active.coords[3].x = active.coords[1].x;
//            active.coords[3].y = active.coords[1].y + 1;
//          }
//        }
//      }
//      else if(active.coords[0].x == active.coords[1].x - 1)
//      {
//        if(active.coords[1].y - 1 >= 0)
//        {
//          if(positions[active.coords[1].x][active.coords[1].y - 1] == 0)
//          {
//            active.coords[0].x = active.coords[1].x;
//            active.coords[0].y = active.coords[1].y - 1;
//            active.coords[2].x = active.coords[1].x;
//            active.coords[2].y = active.coords[1].y + 1;
//            active.coords[3].x = active.coords[1].x - 1;
//            active.coords[3].y = active.coords[1].y;
//          }
//        }
//      }
//      else if(active.coords[0].y == active.coords[1].y - 1)
//      {
//        if(active.coords[1].x + 1 <= 7)
//        {
//          if(positions[active.coords[1].x + 1][active.coords[1].y] == 0)
//          {
//            active.coords[0].x = active.coords[1].x + 1;
//            active.coords[0].y = active.coords[1].y;
//            active.coords[2].x = active.coords[1].x - 1;
//            active.coords[2].y = active.coords[1].y;
//            active.coords[3].x = active.coords[1].x;
//            active.coords[3].y = active.coords[1].y - 1;
//          }
//        }
//      }
//    }
//    else if(active.type == 4)
//    {
//      if(active.coords[0].x == active.coords[1].x + 1)
//      {
//        if(positions[active.coords[2].x][active.coords[2].y + 1] == 0 && positions[active.coords[2].x][active.coords[2].y - 1] == 0 && positions[active.coords[2].x][active.coords[2].y - 2] == 0
//           && active.coords[2].y - 2 >= 0 && active.coords[2].y - 1 > 0)
//        {
//          active.coords[0].x = active.coords[2].x;
//          active.coords[0].y = active.coords[2].y + 1;
//          active.coords[1].x = active.coords[2].x;
//          active.coords[1].y = active.coords[2].y;
//          active.coords[2].y = active.coords[2].y - 1;
//          active.coords[3].x = active.coords[2].x;
//          active.coords[3].y = active.coords[2].y - 1;
//        }
//      }
//      else
//      {
//        if(positions[active.coords[1].x + 2][active.coords[1].y] == 0 && positions[active.coords[1].x + 1][active.coords[1].y] == 0 && positions[active.coords[1].x - 1][active.coords[1].y] == 0
//           && active.coords[1].x + 2 <= 7 && active.coords[1].x - 1 >= 0)
//        {
//          active.coords[0].x = active.coords[1].x + 2;
//          active.coords[0].y = active.coords[1].y;
//          active.coords[1].x = active.coords[1].x + 1;
//          active.coords[2].x = active.coords[1].x - 1;
//          active.coords[2].y = active.coords[1].y;
//          active.coords[3].x = active.coords[1].x - 2;
//          active.coords[3].y = active.coords[1].y;
//        }
//      }
//    }
//    else if(active.type == 5)
//    {
//      if(active.coords[0].x - 1 == active.coords[1].x)
//      {
//        if(positions[active.coords[1].x - 1][active.coords[1].y] == 0 && positions[active.coords[1].x - 1][active.coords[1].y - 1] == 0
//           && active.coords[1].y - 1 >= 0)
//        {
//          active.coords[0].x = active.coords[1].x;
//          active.coords[0].y = active.coords[1].y + 1;
//          active.coords[2].x = active.coords[1].x - 1;
//          active.coords[2].y = active.coords[1].y;
//          active.coords[3].y = active.coords[1].y - 1;
//        }
//      }
//      else
//      {
//        if(positions[active.coords[1].x + 1][active.coords[1].y] == 0 && positions[active.coords[1].x + 1][active.coords[1].y + 1] == 0
//           && active.coords[1].x + 1 <= 7)
//        {
//          active.coords[0].x = active.coords[1].x + 1;
//          active.coords[0].y = active.coords[1].y;
//          active.coords[2].x = active.coords[1].x;
//          active.coords[2].y = active.coords[1].y + 1;
//          active.coords[3].y = active.coords[1].y + 1;
//        }
//      }
//    }
//    else if(active.type == 6)
//    {
//      if(active.coords[0].x - 1 == active.coords[1].x)
//      {
//        if(positions[active.coords[2].x - 1][active.coords[2].y + 1] == 0 && positions[active.coords[2].x][active.coords[2].y - 1] == 0
//           && active.coords[2].y - 1 >= 0)
//        {
//          active.coords[0].x = active.coords[2].x - 1;
//          active.coords[1].x = active.coords[2].x - 1;
//          active.coords[1].y = active.coords[2].y;
//          active.coords[3].x = active.coords[2].x;
//          active.coords[3].y = active.coords[2].y - 1;
//         }
//       }
//       else
//       {
//         if(positions[active.coords[2].x + 1][active.coords[2].y + 1] == 0 && positions[active.coords[2].x][active.coords[2].y + 1] == 0
//            && active.coords[2].x + 1 <= 7)
//         {
//           active.coords[0].x = active.coords[2].x + 1;
//           active.coords[1].x = active.coords[2].x;
//           active.coords[1].y = active.coords[2].y + 1;
//           active.coords[3].x = active.coords[2].x - 1;
//           active.coords[3].y = active.coords[2].y;
//         }
//        }
//      }
//    }
//    else if(input == 0x43) //left
//    {
//       if(positions[active.coords[0].x + 1][active.coords[0].y] == 0 && 
//          positions[active.coords[1].x + 1][active.coords[1].y] == 0 &&
//          positions[active.coords[2].x + 1][active.coords[2].y] == 0 &&
//          positions[active.coords[3].x + 1][active.coords[3].y] == 0 &&
//          active.coords[0].x < 7 &&
//          active.coords[1].x < 7 &&
//          active.coords[2].x < 7 &&
//          active.coords[3].x < 7)
//       {
//          active.coords[0].x += 1;
//          active.coords[1].x += 1;
//          active.coords[2].x += 1;
//          active.coords[3].x += 1;       
//       }
//    }
//    else if(input == 0x42) //right
//    {
//      if(positions[active.coords[0].x - 1][active.coords[0].y] == 0 && 
//         positions[active.coords[1].x - 1][active.coords[1].y] == 0 &&
//         positions[active.coords[2].x - 1][active.coords[2].y] == 0 &&
//         positions[active.coords[3].x - 1][active.coords[3].y] == 0 &&
//         active.coords[0].x > 0 &&
//         active.coords[1].x > 0 &&
//         active.coords[2].x > 0 &&
//         active.coords[3].x > 0)
//      {
//         active.coords[0].x -= 1;
//         active.coords[1].x -= 1;
//         active.coords[2].x -= 1;
//         active.coords[3].x -= 1;          
//      }
//    }
//    else if(input == 0x41) //down
//    {
//       if(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
//          positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
//          positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//          positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
//          active.coords[0].y > 0 &&
//          active.coords[1].y > 0 &&
//          active.coords[2].y > 0 &&
//          active.coords[3].y > 0)
//       {          
//         active.coords[0].y -= 1;
//         active.coords[1].y -= 1;
//         active.coords[2].y -= 1;
//         active.coords[3].y -= 1;         
//       }
//    }
//    else if(input == 0x21)
//    {
//      game_over = true;
//      won_game = true;
//    }
//}

void writeMatrix(Piece &active, Piece &store)
{  
   matrix.clear();
   matrix.writeDisplay();
   if(is_stored)
   {
     if(store.type == 0)
     {
       matrix.drawPixel(7,14,LED_ON);
       matrix.drawPixel(7,15,LED_ON);
       matrix.drawPixel(6,15,LED_ON);
       matrix.drawPixel(5,15,LED_ON);
     }
     if(store.type == 1)
     {
       matrix.drawPixel(7,15,LED_ON);
       matrix.drawPixel(7,14,LED_ON);
       matrix.drawPixel(6,14,LED_ON);
       matrix.drawPixel(5,14,LED_ON);
     }
     if(store.type == 2)
     {
       matrix.drawPixel(6,15,LED_ON);
       matrix.drawPixel(5,15,LED_ON);
       matrix.drawPixel(6,14,LED_ON);
       matrix.drawPixel(5,14,LED_ON);
     }
     if(store.type == 3)
     {
       matrix.drawPixel(7,15,LED_ON);
       matrix.drawPixel(6,15,LED_ON);
       matrix.drawPixel(5,15,LED_ON);
       matrix.drawPixel(6,14,LED_ON);
     }
     if(store.type == 4)
     {
       matrix.drawPixel(6,15,LED_ON);
       matrix.drawPixel(6,14,LED_ON);
       matrix.drawPixel(6,13,LED_ON);
       matrix.drawPixel(6,12,LED_ON);
     }
     if(store.type == 5)
     {
       matrix.drawPixel(7,14,LED_ON);
       matrix.drawPixel(6,14,LED_ON);
       matrix.drawPixel(6,15,LED_ON);
       matrix.drawPixel(5,15,LED_ON);
     }
     if(store.type == 6)
     {
       matrix.drawPixel(7,15,LED_ON);
       matrix.drawPixel(6,15,LED_ON);
       matrix.drawPixel(6,14,LED_ON);
       matrix.drawPixel(5,14,LED_ON);
     }
   }
   for(int i = 0; i < 4; i++)
   {
     matrix.drawPixel(active.coords[i].x, active.coords[i].y,LED_ON);
   }
   for(int i = 0; i < 8; i++)
   {
     for(int j = 0; j < 16; j++)
     {
       if(positions[i][j] == 1)
       {
         matrix.drawPixel(i,j,LED_ON);
       }
     }
   }
   matrix.writeDisplay();
}

void loop() { // run over and over
  if(!game_over && game_start)
  {
    unsigned char input;
    int type = random(7);
    Piece active;
    writeMatrix(active,store);
    make_piece(type, active);
    writeMatrix(active,store);
    matrix.writeDisplay();
    delay(500);
    int counter = 0;
    while(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
          positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
          positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
          positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
          active.coords[0].y > 0 &&
          active.coords[1].y > 0 &&
          active.coords[2].y > 0 &&
          active.coords[3].y > 0)
    {
      read_input(counter, active, store, type);
      writeMatrix(active, store);
      input = get_val();
      input = get_val();
      input = get_val(); 
      input = get_val();
      input = get_val(); 
      input = get_val();
      input = get_val();
      input = get_val();
      input = get_val(); 
      input = get_val();
      input = get_val(); 
      input = get_val();
      read_input(counter, active, store, type);
      writeMatrix(active, store); 
      input = get_val();
      input = get_val();
      input = get_val(); 
      input = get_val();
      input = get_val(); 
      input = get_val();
      input = get_val();
      input = get_val();
      input = get_val(); 
      input = get_val();
      input = get_val(); 
      input = get_val();
      read_input(counter, active, store, type);
      writeMatrix(active, store); 
      input = get_val();
      input = get_val();
      input = get_val(); 
      input = get_val();
      input = get_val(); 
      input = get_val();
      input = get_val();
      input = get_val();
      input = get_val(); 
      input = get_val();
      input = get_val(); 
      input = get_val();
      read_input(counter, active, store, type);
      writeMatrix(active, store);
      input = get_val(); 
      input = get_val();
      input = get_val(); 
      input = get_val();
      input = get_val(); 
      input = get_val();
      input = get_val();
      input = get_val();
      input = get_val(); 
      input = get_val();
      input = get_val(); 
      input = get_val();
      if(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
         positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
         positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
         positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
         active.coords[0].y > 0 &&
         active.coords[1].y > 0 &&
         active.coords[2].y > 0 &&
         active.coords[3].y > 0)
      {          
        active.coords[0].y -= 1;
        active.coords[1].y -= 1;
        active.coords[2].y -= 1;
        active.coords[3].y -= 1;         
      }
      
      writeMatrix(active,store);
      delay(500);
      counter++;
    }
    positions[active.coords[0].x][active.coords[0].y] = 1;
    positions[active.coords[1].x][active.coords[1].y] = 1;
    positions[active.coords[2].x][active.coords[2].y] = 1;
    positions[active.coords[3].x][active.coords[3].y] = 1;
    for(int i = 0; i <= 7; i++)
    {
      if(positions[i][9] == 1)
      {
        game_over = true;
      }
    }
    unsigned char row_completed = 1;
    unsigned char rows_cleared = 0;
    for(int i = 0; i < 9; i++)
    {
      row_completed = 1;
      for(int j = 0; j < 8; j++)
      {
        if(positions[j][i] == 0)
        {
          row_completed = 0;
        }
      }
      if(row_completed)
      {
        rows_cleared++;
        for(int k = i; k < 9; k++)
        {
          for(int l = 0; l < 8; l++)
          {
            positions[l][k] = positions[l][k+1];
          }
        }
        i--;
      }
    }
//    if(rows_cleared == 4)
//    {
//      atmega.write(0x23);
//    }
//    rows_cleared = 0;
//    if(incoming_powerup)
//    {
//      int powerup = 0;//random(5);
//      if(powerup == 0) //blind
//      {
//        matrix.clear();
//        matrix.writeDisplay();
//        if(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
//           positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
//           positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//           positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
//           active.coords[0].y > 0 &&
//           active.coords[1].y > 0 &&
//           active.coords[2].y > 0 &&
//           active.coords[3].y > 0)
//        {          
//          active.coords[0].y -= 1;
//          active.coords[1].y -= 1;
//          active.coords[2].y -= 1;
//          active.coords[3].y -= 1;         
//        }
//        read_input(counter, active, store, type);
//        input = get_val();
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val();
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        read_input(counter, active, store, type);
//        input = get_val();
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val();
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        read_input(counter, active, store, type);
//        input = get_val();
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val();
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        read_input(counter, active, store, type);
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val();
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        if(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
//           positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
//           positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//           positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
//           active.coords[0].y > 0 &&
//           active.coords[1].y > 0 &&
//           active.coords[2].y > 0 &&
//           active.coords[3].y > 0)
//        {          
//          active.coords[0].y -= 1;
//          active.coords[1].y -= 1;
//          active.coords[2].y -= 1;
//          active.coords[3].y -= 1;         
//        }
//        delay(500);
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val();
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//      }
//      else if(powerup == 1) //S or Z
//      {
//        int type = random(2) + 5;
//        Piece active;
//        writeMatrix(active,store);
//        make_piece(type, active);
//        writeMatrix(active,store);
//        matrix.writeDisplay();
//        delay(500);
//        int counter = 0;
//        while(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
//              positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
//              positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//              positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
//              active.coords[0].y > 0 &&
//              active.coords[1].y > 0 &&
//              active.coords[2].y > 0 &&
//              active.coords[3].y > 0)
//        {
//          read_input(counter, active, store, type);
//          writeMatrix(active, store);
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          read_input(counter, active, store, type);
//          writeMatrix(active, store); 
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          read_input(counter, active, store, type);
//          writeMatrix(active, store); 
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          read_input(counter, active, store, type);
//          writeMatrix(active, store);
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          if(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
//             positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
//             positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//             positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
//             active.coords[0].y > 0 &&
//             active.coords[1].y > 0 &&
//             active.coords[2].y > 0 &&
//             active.coords[3].y > 0)
//          {          
//            active.coords[0].y -= 1;
//            active.coords[1].y -= 1;
//            active.coords[2].y -= 1;
//            active.coords[3].y -= 1;         
//          }
//          
//          writeMatrix(active,store);
//          delay(500);
//          counter++;
//        }
//        positions[active.coords[0].x][active.coords[0].y] = 1;
//        positions[active.coords[1].x][active.coords[1].y] = 1;
//        positions[active.coords[2].x][active.coords[2].y] = 1;
//        positions[active.coords[3].x][active.coords[3].y] = 1;
//        for(int i = 0; i <= 7; i++)
//        {
//          if(positions[i][9] == 1)
//          {
//            game_over = true;
//          }
//        }
//        unsigned char row_completed = 1;
//        unsigned char rows_cleared = 0;
//        for(int i = 0; i < 9; i++)
//        {
//          row_completed = 1;
//          for(int j = 0; j < 8; j++)
//          {
//            if(positions[j][i] == 0)
//            {
//              row_completed = 0;
//            }
//          }
//          if(row_completed)
//          {
//            rows_cleared++;
//            for(int k = i; k < 9; k++)
//            {
//              for(int l = 0; l < 8; l++)
//              {
//                positions[l][k] = positions[l][k+1];
//              }
//            }
//            i--;
//          }
//        }
//        if(rows_cleared == 4)
//        {
//          atmega.write(0x23);
//        }
//        rows_cleared = 0;
//        delay(500);
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val();
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();        
//      }
//      else if(powerup == 2) //lock button
//      {
//        unsigned char input;
//        int type = random(7);
//        Piece active;
//        writeMatrix(active,store);
//        make_piece(type, active);
//        writeMatrix(active,store);
//        matrix.writeDisplay();
//        delay(500);
//        int counter = 0;
//        while(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
//              positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
//              positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//              positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
//              active.coords[0].y > 0 &&
//              active.coords[1].y > 0 &&
//              active.coords[2].y > 0 &&
//              active.coords[3].y > 0)
//        {
//          writeMatrix(active, store);
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          writeMatrix(active, store); 
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          writeMatrix(active, store); 
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          writeMatrix(active, store);
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          if(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
//             positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
//             positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//             positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
//             active.coords[0].y > 0 &&
//             active.coords[1].y > 0 &&
//             active.coords[2].y > 0 &&
//             active.coords[3].y > 0)
//          {          
//            active.coords[0].y -= 1;
//            active.coords[1].y -= 1;
//            active.coords[2].y -= 1;
//            active.coords[3].y -= 1;         
//          }
//          
//          writeMatrix(active,store);
//          delay(500);
//          counter++;
//        }
//        positions[active.coords[0].x][active.coords[0].y] = 1;
//        positions[active.coords[1].x][active.coords[1].y] = 1;
//        positions[active.coords[2].x][active.coords[2].y] = 1;
//        positions[active.coords[3].x][active.coords[3].y] = 1;
//        for(int i = 0; i <= 7; i++)
//        {
//          if(positions[i][9] == 1)
//          {
//            game_over = true;
//          }
//        }
//        unsigned char row_completed = 1;
//        unsigned char rows_cleared = 0;
//        for(int i = 0; i < 9; i++)
//        {
//          row_completed = 1;
//          for(int j = 0; j < 8; j++)
//          {
//            if(positions[j][i] == 0)
//            {
//              row_completed = 0;
//            }
//          }
//          if(row_completed)
//          {
//            rows_cleared++;
//            for(int k = i; k < 9; k++)
//            {
//              for(int l = 0; l < 8; l++)
//              {
//                positions[l][k] = positions[l][k+1];
//              }
//            }
//            i--;
//          }
//        }
//        if(rows_cleared == 4)
//        {
//          atmega.write(0x23);
//        }
//        rows_cleared = 0;
//        delay(500);
//      }
//      else if(powerup == 3) //speed drop
//      {
//        if(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
//             positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
//             positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//             positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
//             active.coords[0].y > 0 &&
//             active.coords[1].y > 0 &&
//             active.coords[2].y > 0 &&
//             active.coords[3].y > 0)
//          {          
//            active.coords[0].y -= 1;
//            active.coords[1].y -= 1;
//            active.coords[2].y -= 1;
//            active.coords[3].y -= 1;         
//          }
//          if(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
//             positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
//             positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//             positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
//             active.coords[0].y > 0 &&
//             active.coords[1].y > 0 &&
//             active.coords[2].y > 0 &&
//             active.coords[3].y > 0)
//          {          
//            active.coords[0].y -= 1;
//            active.coords[1].y -= 1;
//            active.coords[2].y -= 1;
//            active.coords[3].y -= 1;         
//          }
//      }
//      else if(powerup == 4) //switch left and right, spin and down
//      {
//        unsigned char input;
//        int type = random(7);
//        Piece active;
//        writeMatrix(active,store);
//        make_piece(type, active);
//        writeMatrix(active,store);
//        matrix.writeDisplay();
//        delay(500);
//        int counter = 0;
//        while(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
//              positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
//              positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//              positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
//              active.coords[0].y > 0 &&
//              active.coords[1].y > 0 &&
//              active.coords[2].y > 0 &&
//              active.coords[3].y > 0)
//        {
//          read_swapped_input(counter, active, store, type);
//          writeMatrix(active, store);
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          read_swapped_input(counter, active, store, type);
//          writeMatrix(active, store); 
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          read_swapped_input(counter, active, store, type);
//          writeMatrix(active, store); 
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          read_swapped_input(counter, active, store, type);
//          writeMatrix(active, store);
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val();
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          input = get_val(); 
//          input = get_val();
//          if(positions[active.coords[0].x][active.coords[0].y - 1] == 0 && 
//             positions[active.coords[1].x][active.coords[1].y - 1] == 0 &&
//             positions[active.coords[2].x][active.coords[2].y - 1] == 0 &&
//             positions[active.coords[3].x][active.coords[3].y - 1] == 0 &&
//             active.coords[0].y > 0 &&
//             active.coords[1].y > 0 &&
//             active.coords[2].y > 0 &&
//             active.coords[3].y > 0)
//          {          
//            active.coords[0].y -= 1;
//            active.coords[1].y -= 1;
//            active.coords[2].y -= 1;
//            active.coords[3].y -= 1;         
//          }
//          
//          writeMatrix(active,store);
//          delay(500);
//          counter++;
//        }
//        positions[active.coords[0].x][active.coords[0].y] = 1;
//        positions[active.coords[1].x][active.coords[1].y] = 1;
//        positions[active.coords[2].x][active.coords[2].y] = 1;
//        positions[active.coords[3].x][active.coords[3].y] = 1;
//        for(int i = 0; i <= 7; i++)
//        {
//          if(positions[i][9] == 1)
//          {
//            game_over = true;
//          }
//        }
//        unsigned char row_completed = 1;
//        unsigned char rows_cleared = 0;
//        for(int i = 0; i < 9; i++)
//        {
//          row_completed = 1;
//          for(int j = 0; j < 8; j++)
//          {
//            if(positions[j][i] == 0)
//            {
//              row_completed = 0;
//            }
//          }
//          if(row_completed)
//          {
//            rows_cleared++;
//            for(int k = i; k < 9; k++)
//            {
//              for(int l = 0; l < 8; l++)
//              {
//                positions[l][k] = positions[l][k+1];
//              }
//            }
//            i--;
//          }
//        }
//        if(rows_cleared == 4)
//        {
//          atmega.write(0x23);
//        }
//        rows_cleared = 0;
//        delay(500);
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val();
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//        input = get_val(); 
//        input = get_val();
//      }
//    }   
//    incoming_powerup = false;
    delay(500);
    input = get_val(); 
    input = get_val();
    input = get_val(); 
    input = get_val();
    input = get_val();
    input = get_val();
    input = get_val(); 
    input = get_val();
    input = get_val(); 
    input = get_val();
    input = get_val(); 
    input = get_val();
    input = get_val(); 
    input = get_val();
    input = get_val(); 
    input = get_val();
    input = get_val(); 
    input = get_val();
  }
  else if(game_over)
  {
    unsigned char input;
    if(!won_game)
    {
      atmega.write(0x21);
      matrix.setTextSize(1/2);
      matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
      matrix.setTextColor(LED_ON);
      matrix.setRotation(2);
      for (int8_t x=7; x>=-36; x--) {
        matrix.clear();
        matrix.setCursor(x,0);
        matrix.print("Loser");
        matrix.drawBitmap(0, 8, frown_bmp, 8, 15, LED_ON);
        matrix.writeDisplay();
        delay(100);
      }
    }
    else
    {
      matrix.setTextSize(1/2);
      matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
      matrix.setTextColor(LED_ON);
      matrix.setRotation(2);
      for (int8_t x=7; x>=-36; x--) {
        matrix.clear();
        matrix.setCursor(x,0);
        matrix.print("Winner");
        matrix.drawBitmap(0, 8, smile_bmp, 8, 15, LED_ON);
        matrix.writeDisplay();
        delay(100);
      }
    }
    software_Reset();
  }
}
