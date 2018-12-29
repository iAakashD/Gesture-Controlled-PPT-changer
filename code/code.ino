//**for MPU-6050**
#include <MPU6050_tockn.h>
#include <Wire.h>
MPU6050 mpu6050(Wire);

//**software serial for HID Bluetooth module**
#include <SoftwareSerial.h>
SoftwareSerial bluetooth_HID(19, 18); // RX, TX

//**to store data in EEPROM**
#include <EEPROM.h>   //save 0-49 for master-0-x, 50-99 for master-0-y, 100-149 for master-0-z ..... 

//**for OLED display**
//#include <Wire.h> //already added
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define SSD1306_LCDHEIGHT 64
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//**for user interface**
#define down_button 17
#define select_button 16
#define down_button_pressed digitalRead(down_button)==LOW
#define select_button_pressed digitalRead(select_button)==LOW
int line=0;

#define display_init display.clearDisplay();display.setTextSize(1);display.setTextColor(WHITE);display.setCursor(0,0)

//**to store data**
#define num_masters 2
#define DOF 3
#define sample_size 50
#define avg_lenght 2

int master[num_masters][DOF][sample_size];       //array to store master gesture
int temp_values[DOF][sample_size];               //array to store temp. values from take_reading function

void do_action(int a)      
{     //set actions to do if the gestures matches
  /*
   * Some commands in PowerPoint Presentation
   * n - go to next slide
   * p - go to the previous slide
   * w - display white screen
   * b - display black scrren
  */
  if(a==0)
  {
    //go to next slide
    bluetooth_HID.write("n");
  }
  else if(a==1)
  {
    //go to previous slide
    bluetooth_HID.write("p");
  }
}

void display_set_cursor(int line)     
{     //function display cursor on particular line of OLED display
  if(line==0)
  {
    display.drawPixel(1, 10, WHITE);
    display.drawPixel(2, 10, WHITE);
    
    display.drawPixel(3, 11, WHITE);
    display.drawPixel(4, 11, WHITE);
    
    display.drawPixel(5, 12, WHITE);
    display.drawPixel(6, 12, WHITE);
    display.drawPixel(5, 12, WHITE);
    
    display.drawPixel(4, 13, WHITE);
    display.drawPixel(3, 13, WHITE);
    
    display.drawPixel(2, 14, WHITE);
    display.drawPixel(1, 14, WHITE); 
  }
  else if(line==1)
  {
    display.drawPixel(1, 17, WHITE);
    display.drawPixel(2, 17, WHITE);
    
    display.drawPixel(3, 18, WHITE);
    display.drawPixel(4, 18, WHITE);
    
    display.drawPixel(5, 19, WHITE);
    display.drawPixel(6, 19, WHITE);
    display.drawPixel(5, 19, WHITE);
    
    display.drawPixel(4, 20, WHITE);
    display.drawPixel(3, 20, WHITE);
    
    display.drawPixel(2, 21, WHITE);
    display.drawPixel(1, 21, WHITE); 
  }
  else if(line==2)
  {
    display.drawPixel(1, 25, WHITE);
    display.drawPixel(2, 25, WHITE);
    
    display.drawPixel(3, 26, WHITE);
    display.drawPixel(4, 26, WHITE);
    
    display.drawPixel(5, 27, WHITE);
    display.drawPixel(6, 27, WHITE);
    display.drawPixel(5, 27, WHITE);
    
    display.drawPixel(4, 28, WHITE);
    display.drawPixel(3, 28, WHITE);
    
    display.drawPixel(2, 29, WHITE);
    display.drawPixel(1, 29, WHITE); 
  }
}

void EEPROM_write(int master_select)    
{     //store master gestures in EEPROM of arduino
  int i, j, addr;
    for(i=0; i<DOF; i++)
    {
      for(j=0; j<sample_size; j++)
      {
        addr=(master_select*sample_size)+(i*DOF)+j;
        EEPROM.write(addr, master[master_select][i][j]);
        delay(5);
      }
    }
}

void EEPROM_read(int master_select)     
{     //retrieve master gestures from EEPROM
  int i, j, addr;
    for(i=0; i<DOF; i++)
    {
      for(j=0; j<sample_size; j++)
      {
        addr=(master_select*sample_size)+(i*DOF)+j;
        master[master_select][i][j]=EEPROM.read(addr);
        delay(5);
      }
    }
}

void copy_reading(int temp_values[DOF][sample_size], int master[num_masters][DOF][sample_size], int master_select)
{     //copy readings from temp_values to selected master
  int i, j;
  for(i=0; i<DOF; i++)
  {
    for(j=0; j<sample_size; j++)
    {
       master[master_select][i][j]=temp_values[i][j];
    }
  }
}

void take_reading(void)
{     //take sample_size*avg_lenght readings from MPU-6050, average every avg_lenght and give "sample_size" number of elements out
    int debug=0;
    int i, j, k, sum=1;
    int reading[DOF][avg_lenght*sample_size];
   
    //take readings
    for(i=0; i<avg_lenght*sample_size; i++)
    {
        mpu6050.update();
        
        reading[0][i]=mpu6050.getAccX()*50+50;    //taking readings of acceleraton in g's, 1g, 1.2g
        if(reading[0][i]<0)
          reading[0][i]=0;
        else if(reading[0][i]>100)
          reading[0][i]=100;
          
        reading[1][i]=mpu6050.getAccY()*50+50;
        if(reading[1][i]<0)
          reading[1][i]=0;
        else if(reading[1][i]>100)
          reading[1][i]=100;
          
        reading[2][i]=mpu6050.getAccZ()*50+50;
        if(reading[2][i]<0)
          reading[2][i]=0;
        else if(reading[2][i]>100)
          reading[2][i]=100;

        if(debug)
        {
          Serial.print(reading[0][i]);
          Serial.print("    "); Serial.print(reading[1][i]);
          Serial.print("    "); Serial.println(reading[2][i]);
        }
        delay(20);
    }

    if(avg_lenght>1)
    {
        for(i=0; i<DOF; i++)
        {
            for(j=0; j<(sample_size); j++)
            {
                for(k=0; k<avg_lenght; k++)
                {
                    sum=sum+reading[i][avg_lenght*j+k];
                }
                temp_values[i][j]=sum/avg_lenght;
                sum=0;
            }
        }
    }
    else if(avg_lenght==1)
    {
        for(i=0; i<DOF; i++)
        {
            for(j=0; j<sample_size; j++)
                temp_values[i][j]=reading[i][j];
        }
    }
}

unsigned int calc_DTW_score(int test[DOF][sample_size], int master[num_masters][DOF][sample_size], int master_select)
{     //calculates DTW score between 2 time series
    int print_DTW=0;         //1 to print DTW matrix, else 0
    int i;
    //creating variables to store input parameters
    int matrix_size=sample_size+1;
    unsigned int a[matrix_size][matrix_size];    //matrix of DTW
    unsigned int DTW_score[DOF];
    unsigned int final_DTW_score=0;

    for(i=0; i<DOF; i++)
    {
      int x=0, y=0;
  
      //moving input series to DTW matrix
      for(x=0; x<sample_size; x++)
          a[x+1][0]=master[master_select][i][x];
      for(y=0; y<sample_size; y++)
          a[0][y+1]=test[i][y];
  
      //**computing DTW matrix**
  
      /*
      DTW matrix calculation
      reference video
      https://www.youtube.com/watch?v=_K1OsqCicBY
      */
  
      a[0][0]=0;
      a[1][1]=abs_sub(a[1][0], a[0][1]);      //as no previously computed values
      a[2][1]=abs_sub(a[2][0], a[0][1]);
      a[1][2]=abs_sub(a[1][0], a[0][2]);
  
      x=1;
      for(y=2; y<matrix_size; y++)
      {
          a[x][y]=abs_sub(a[x][0], a[0][y]) + a[x][y-1];
      }
  
      y=1;
      for(x=2; x<matrix_size; x++)
      {
          a[x][y]=abs_sub(a[x][0], a[0][y]) + a[x-1][y];
      }
  
      for(x=2; x<matrix_size; x++)
      {
          for(y=2; y<matrix_size; y++)
          {
              a[x][y]=abs_sub(a[x][0], a[0][y]) + Min(a[x][y-1], a[x-1][y], a[x-1][y-1]);
          }
  
      }
  
      
      //**calculating DTW score**
  
      x=sample_size;
      y=sample_size;
      DTW_score[i]=a[x][y];
  
      while(x!=0 && y!=0)
      {
          if(a[x-1][y-1]<=a[x][y-1] && a[x-1][y-1]<=a[x-1][y])
          {
              DTW_score[i]=DTW_score[i]+a[x-1][y-1];
              x--;
              y--;
          }
  
          else if(a[x][y-1]<=a[x-1][y-1] && a[x][y-1]<=a[x-1][y])
          {
              DTW_score[i]=DTW_score[i]+a[x][y-1];
              y--;
          }
          else
          {
              DTW_score[i]=DTW_score[i]+a[x-1][y];
              x--;
          }
      }
    }
    for(i=0; i<DOF; i++)
      final_DTW_score=final_DTW_score+DTW_score[i];
    final_DTW_score=final_DTW_score/DOF;
    return final_DTW_score;
}

int abs_sub(int a, int b)
{     //finds the absolute subtraction (difference) of 2 entered numbers
    int c=a-b;
    if(c<0)
        c=-c;
    return c;
}

int Min(int a, int b, int c)
{     //finds the minimum of 3 numbers entered
    if(a<b && a<c)
        return a;
    else if(b<a && b<c)
        return b;
    else
        return c;
}

void setup() {
  //start serial monitor, required for debugging purposes
  Serial.begin(9600);

  //for MPU-6050
  Wire.begin();
  mpu6050.begin();

  //software serial for HID Bluetooth module
  bluetooth_HID.begin(115200);
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(down_button, INPUT_PULLUP);  //move down
  pinMode(select_button, INPUT_PULLUP);  //select

  //for OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  //for user interface
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(35,10);
  display.println("Hello");
  display.display();
  
  delay(500);   //display Hello for .5 second
}

void loop() {
  while(1)
  {
    display_init;
    display.println("What?");
    display.println("  a. test gesture");
    display.println("  b. record master");
    display.println("  c. update EEPROM");
    display_set_cursor(line);
    display.display();
  
    if(down_button_pressed)
    {
      delay(10);    //to remove button bouncing
      line++;
      if(line>2)
        line=0;
    }
    if(select_button_pressed)
    {
      delay(10);
      break;
    }
  }

  if(line==0)     //test gesture
  {
   int i, DTW_score[num_masters], min_score, location;
   
   /*display_init;
   display.println("recording gesture in 50 uS");
   display.display();

   delay(50);
   */
   
   display_init;
   display.println("recording gesture");
   display.display();
   
   digitalWrite(13, HIGH);
   take_reading();
   digitalWrite(13, LOW);

   display_init;
   display.println("done recording gesture");
   display.display();

   for(int i=0; i<num_masters; i++)   //calculate DTW_score for each master
     DTW_score[i]=calc_DTW_score(temp_values, master, i);
     
   min_score=DTW_score[0];    //finding minimum of DTW_scores
   for(i=1; i<num_masters; i++)
   {
     if(DTW_score[i]<min_score){
       min_score=DTW_score[i];
     }
   }
   for(i=0; i<num_masters; ++i)
   {
     if(min_score==DTW_score[i]){
      break;   
     }
   }
   display_init;
   display.print("master is: ");
   display.println(i);
   display.display();
   do_action(i);
   delay(2000);
  }
  else if(line==1)    //record master
  {
    line=0;
    while(1)
    {
      display_init;
      display.println("record which master?");
      display.println("  a. master-0");
      display.println("  b. master-1");
      display_set_cursor(line);
      display.display();
      if(down_button_pressed)
      {
        delay(5);    //to remove button bouncing
        line++;
        if(line>1)
          line=0;
      }
      if(select_button_pressed)
        break;
    }

    if(line==0)       //record master 0
    {
      /*display_init;
      display.println("recording in 50uS");
      display.display();
      delay(50);*/
      
      display_init;
      display.println("recording gesture 0");
      display.display();
      
      digitalWrite(13, HIGH);
      take_reading();
      digitalWrite(13, LOW);

      display_init;
      display.println("done recording gesture");
      display.display();
      
      copy_reading(temp_values, master, 0);
    }
    else if(line==1)  //record master 1
    {
      /*display_init;
      display.println("recording in 50uS");
      delay(50);
      */
      display_init;
      display.println("recording gesture 1");
      display.display();
      
      digitalWrite(13, HIGH);
      take_reading();
      digitalWrite(13, LOW);
      
      display_init;
      display.println("done recording gesture");
      display.display();
      
      copy_reading(temp_values, master, 1);
    }
  }
  else if(line==2)    //update EEPROM
  {
    line=0;
    while(1)
    {
      display_init;
      display.println("save or retrieve");
      display.println("  a. save master");
      display.println("  b. retrieve master");
      display_set_cursor(line);
      display.display();
      if(down_button_pressed)
      {
        delay(5);    //to remove button bouncing
        line++;
        if(line>1)
          line=0;
      }
      if(select_button_pressed)
      {
        delay(5);
        break;
      }
    }
    if(line==0)   //save master
    {
      line=0;
      while(1)
      {
        display_init;
        display.println("save which master");
        display.println("  a. master-0");
        display.println("  b. master-1");
        display_set_cursor(line);
        display.display();
        
        if(down_button_pressed)
        {
          delay(5);    //to remove button bouncing
          line++;
          if(line>1)
            line=0;
        }
        if(select_button_pressed)
        {
          delay(5);
          break;
        }   
      }
      display_init;
      display.println("writing to EEPROM");
      display.display();
      
      EEPROM_write(line);

      display_init;
      display.println("done writing to EEPROM");
      display.display();
    }
    else if(line==1)    //retrieve master
    {
      line=0;
      while(1)
      {
        display_init;
        display.println("which master?");
        display.println("  a. master-0");
        display.println("  b. master-1");
        display_set_cursor(line);
        display.display();
        if(down_button_pressed)
        {
          delay(5);    //to remove button bouncing
          line++;
          if(line>1)
            line=0;
        }
        if(select_button_pressed)
        {
          delay(5);
          break;
        }   
      }
      display_init;
      display.println("retrieving master");
      display.display();
      
      EEPROM_read(line);
      
      display_init;
      display.println("done retrieving maser");
      display.display();
    }
  }
}
