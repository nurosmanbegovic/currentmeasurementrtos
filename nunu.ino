#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <LiquidCrystal.h> //library for LCD
#include <SD.h> //library for SD CARD
#include <SPI.h> //library for communication with sd card
 
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
 
//Measuring Current Using ACS712
 
const int analogchannel = 0; //Connect current sensor with A0 of Arduino
const int CS = 53; //Connect sd card with pin 53 of Arduino
int sensitivity = 100; // use 100 for 20A Module, 66 for 30A Module and 185 for 05A Module
float adcvalue= 0;
int offsetvoltage = 2500; 
double Voltage = 0; //voltage measuring
double ecurrent = 0;// Current measuring
File myFile;


SemaphoreHandle_t xSemaphore;

/*create reference handler for Tasks */
TaskHandle_t TaskReadValues_t;
TaskHandle_t TaskWriteValues_t;
TaskHandle_t Initialize_t;
TaskHandle_t TaskWriteCard_t;




void setup()
{
  Serial.begin(9600); // Enable serial communication library.
  lcd.begin(20, 4);
  Serial.println(F("In Setup function"));
 
  while (!Serial){}

  Serial.print("Initializing SD card...");
  
  if (!SD.begin(CS)) 
  {
    Serial.println("initialization failed!");
    while (1);
  }  
  Serial.println("initialization done.");

  
  Serial.println(F("Creating semaphore and giving it"));
  // Create and release binary semaphore
   vSemaphoreCreateBinary(xSemaphore);
  xSemaphoreGive(xSemaphore);
  xTaskCreate(TaskInitialize, "Task0", 500, NULL, 0, &Initialize_t); 
 vTaskStartScheduler();
}

void loop()
{
  // put your main code here, to run repeatedly:
}

static void TaskInitialize(void* pvParameters) {
  
 
    xSemaphoreTake( xSemaphore, portMAX_DELAY );
  Serial.println(F("TaskInitialize"));    
   //LCD order
   lcd.clear();//clearing the LCD display
   lcd.display();//Turning on the display again
   lcd.setCursor(1,0);//setting LCD cursor
   lcd.print("Reading Values from");//prints on LCD
   lcd.setCursor(1,1);
   lcd.print("DC Current Sensor");
   lcd.setCursor(5,2);
   lcd.print("ACS 712");
  //delay for 2 sec
  delay(2000); 
  
    Serial.println(F("Creating task for reading values"));    
   delay(1000);
  xTaskCreate(TaskReadValues, "Task1", 500, NULL, 1, &TaskReadValues_t);
 
 // 
xSemaphoreGive(xSemaphore);
  vTaskDelete(Initialize_t);
}

/* Task1 with priority 1 */
 void TaskReadValues(void* pvParameters)
{
  
  while (1)
  {

    //Take semaphore
   xSemaphoreTake( xSemaphore, portMAX_DELAY );
    Serial.println(F("In TaskReadValues"));    
    delay(1000);
    Serial.println(F("TaskReadValues acquired semaphore"));    
    delay(1000);

    unsigned int temp=0;
     float maxpoint = 0;
     int i=0;
     for(i=0;i<500;i++)
     {
     if(temp = analogRead(analogchannel),temp>maxpoint)
     {
     maxpoint = temp;
     
     }
     }
    // maxpoint = analogRead(analogchannel);
     adcvalue = maxpoint; 
     Voltage = (adcvalue / 1023.0) * 5000; // Gets you mV
     ecurrent = ((Voltage - offsetvoltage) / sensitivity);
     ecurrent = ( ecurrent ) / ( sqrt(2) );    

    Serial.println(F("Creating TaskWriteValues"));    
    
    delay(1000);
    xTaskCreate(TaskWriteValues, "Task2", 500, NULL, 1, &TaskWriteValues_t);
    xSemaphoreGive(xSemaphore); //release the semaphore
    vTaskDelete(TaskReadValues_t);    // Delete the task using the TaskReadValues
   
  }

}

static void TaskWriteCard(void* pvParameters){
  myFile = SD.open("example2.txt", FILE_WRITE);
  if (SD.exists("example2.txt")) {

    Serial.println("example2.txt exists.");

  } else {

    Serial.println("example2.txt doesn't exist.");

  }
  if(myFile) {
    Serial.print("Writing to example2.txt...");
    myFile.println("adc Value = ");
    myFile.println(adcvalue);
    myFile.println("Voltage = ");
    myFile.println(Voltage);
    myFile.println("ac Current = ");
    myFile.println(ecurrent);
    myFile.close();
    Serial.println("done.");
  }
  vTaskDelete(NULL);
  
}

/* Task2 with priority 1 */
static void TaskWriteValues(void* pvParameters)
{
  while (1)
  {
   xSemaphoreTake( xSemaphore, portMAX_DELAY );
   
    //Prints on the serial port
     Serial.println("Raw Value = " ); // prints on the serial monitor
    Serial.print(adcvalue); //prints the results on the serial monitor
     
    lcd.clear();//clears the display of LCD
    //delay(1000);//delay of 1 sec
     lcd.display();
     lcd.setCursor(1,0);
     lcd.print("adc Value = ");
     lcd.setCursor(13,0);
     lcd.print(adcvalue);
    
     Serial.print(F("\t mV = ")); // shows the voltage measured 
    Serial.print(Voltage,3); // the '3' after voltage allows you to display 3 digits after decimal point
     
     lcd.setCursor(1,1);
     lcd.print("Voltage = ");
     lcd.setCursor(11,1);
     lcd.print(Voltage,3);
     lcd.setCursor(17,1);
     lcd.print("mV");//Unit for the voltages to be measured
    
     Serial.print(F("\t ecurrent = ")); // shows the voltage measured 
     Serial.println(ecurrent,3);// the '3' after voltage allows you to display 3 digits after decimal point
   
     lcd.setCursor(1,2);
     lcd.print("ac Current = ");
     lcd.setCursor(11,2);
     lcd.print(ecurrent,3);
     lcd.setCursor(16,2);
     lcd.print("A"); //unit for the current to be measured
   // delay(2500); //delay of 2.5 sec
    xTaskCreate(TaskWriteCard, "Task4", 500, NULL, 2, &TaskWriteCard_t);
    xTaskCreate(TaskReadValues, "Task1", 500, NULL, 1, &TaskReadValues_t);
    xSemaphoreGive(xSemaphore); 
    vTaskDelete(TaskWriteValues_t);    // Delete the task using the TaskWriteValues
    
  }
}
