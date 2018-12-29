//**for MPU-6050**
#include <MPU6050_tockn.h>
#include <Wire.h>
MPU6050 mpu6050(Wire);

//**to store data**
#define num_masters 2
#define DOF 3
#define sample_size 50
#define avg_lenght 2

int master[num_masters][DOF][sample_size];       //array to store master gesture
int temp_values[DOF][sample_size];               //array to store temp. values from take_reading function

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
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  delay(500);   //display Hello for .5 second
}

void loop() {
  digitalWrite(13, HIGH);
  take_reading();     //take readings and save it as master gesture 0
  copy_reading(temp_values, master, 0);
  digitalWrite(13, LOW);

  delay(2000);
  
  digitalWrite(13, HIGH);
  take_reading();
  copy_reading(temp_values, master, 1);     //take readings and save it as master gesture 1
  digitalWrite(13, LOW);
  
  delay(2000);

  digitalWrite(13, HIGH);
  take_reading();
  digitalWrite(13, LOW);

  int DTW_0=calc_DTW_score(temp_values, master, 0);   //calculate DTW scores with available master getsures
  int DTW_1=calc_DTW_score(temp_values, master, 1);

  if(DTW_0<DTW_1)   //if score with gesture 0 is less, gesture 0 is matching
  {
    for(int i=0; i<10; i++)   //blink LED 10 times
    {
      digitalWrite(13, HIGH);
      delay(150);
      digitalWrite(13, LOW);
      delay(150);
    }
  }
  else    //else gesture 1 is matching
    digitalWrite(13, LOW);    //turn off LED for 3 seconds
    delay(3000);
}
