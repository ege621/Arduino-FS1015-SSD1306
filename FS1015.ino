//This code combines the FS1015 flow sensor with SSD1306 OLED display.
//The sensor values are mapped to a rectangle length for visualization.

//You can modify this template to suit your visualization needs.


//Imorting the necessary libraries
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
byte data[5];
int i , summ , cksm;
int16_t raw_flow;
const int addr = 0x50;
float flow_ms , filtered_flow,prev, alpha = 0.1;

//Define OLED variables
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32 //do NOT trust this. mine was 128x64 with 0x3C address.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//creating a rectangle with variable length based on sensor data
void testfillrect(float in) 
  {
  display.clearDisplay();
  float length_rect = map(in,0,12.5,0,display.width()); //The third argument is your sensor's maximum data. Mine was 12.5 m/s.
  display.fillRect(1, 1, length_rect, 10, SSD1306_INVERSE);
  }

void setup() 
{
  Wire.begin();
  Serial.begin(9600);

//The display won't work without the display.begin() command.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) //initialize the display
    { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
    }
}


void loop() 
{
  
  Wire.requestFrom(addr,5); //request 5 bytes from the slave
  
  for (i=0;i<5;i++) //assign the bytes to an array 
    {
    data[i] = Wire.read();  
    }
  
  summ = data[1] + data[2] + data[3] + data[4]; //calculate checksum and take two's compliment
  cksm = data[0] + summ;
  cksm = (~cksm) + 1;
  
  
  if (cksm == 0xFFFFFF00) //if the data is valid, take the measurement and print
    {
    raw_flow = data[1]<<8 | data[2]; // Raw Pressure Data  
    //this equation is based from the FS1015-1005 datasheet
    flow_ms = 0.8024 - 0.000344*raw_flow + 0.000001*raw_flow*raw_flow;
    flow_ms -= 0.8024; //subtract the constant offset so the sensor reads 0 at 0 m/s flow
    filtered_flow = alpha*flow_ms + (1-alpha)*prev; //apply exponential filtering
    prev = filtered_flow; 
    //Serial.println(filtered_flow);
    display.clearDisplay();
    
    testfillrect(filtered_flow);
  
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(14, 23);
    display.print(filtered_flow);
    display.print(" m/s");
    display.display();  
    }
}
