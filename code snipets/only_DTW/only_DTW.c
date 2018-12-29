#include <stdio.h>

#define sample_size 5
#define print_DTW_matrix 1         //1 to print DTW matrix, else 0

int master[sample_size];       //array to store master gesture
int temp_values[sample_size];               //array to store temp. values from take_reading function

void calc_DTW_score(int master[sample_size], int test[sample_size])
{
    int i;
    //creating variables to store input parameters
    int matrix_size=sample_size+1;
    unsigned int a[matrix_size][matrix_size];    //matrix of DTW
    unsigned int DTW_score=0;
    int x=0, y=0;

    //moving input series to DTW matrix
    for(x=0; x<sample_size; x++)
        a[x+1][0]=master[x];
    for(y=0; y<sample_size; y++)
        a[0][y+1]=test[y];

    //**computing DTW matrix**

    /*
    DTW matrix calculation
    cell = difference of corresponding time series + minimum of previously computed 3 values
    reference video
    https://www.youtube.com/watch?v=_K1OsqCicBY
    */

    a[0][0]=0;      //no need

    a[1][1]=abs_sub(a[1][0], a[0][1]);      //first element
    a[2][1]=abs_sub(a[2][0], a[0][1]);
    a[1][2]=abs_sub(a[1][0], a[0][2]);

    x=1;        //first row remaining elements
    for(y=2; y<matrix_size; y++)
        a[x][y]=abs_sub(a[x][0], a[0][y]) + a[x][y-1];

    y=1;        //first column remaining elements
    for(x=2; x<matrix_size; x++)
        a[x][y]=abs_sub(a[x][0], a[0][y]) + a[x-1][y];

    for(x=2; x<matrix_size; x++)        //rest of the matrix
    {
        for(y=2; y<matrix_size; y++)
        {
            a[x][y]=abs_sub(a[x][0], a[0][y]) + Min(a[x][y-1], a[x-1][y], a[x-1][y-1]);
        }

    }

    //**printing DTW matrix**

    if(print_DTW_matrix)       //to print DTW matrix
    {
        printf("\n");
        for(y=0; y<matrix_size; y++)
        {
            for(x=0; x<matrix_size; x++)
            {
                printf("%d\t", a[x][y]);
            }
            printf("\n");
            printf("\n");
        }
    }

    //**calculating DTW score**

    x=sample_size;      //going to bottom right most element of matrix
    y=sample_size;
    DTW_score=a[x][y];      //adding from there

    while(x!=0 && y!=0)     //till top left is reached
    {
        if(a[x-1][y-1]<=a[x][y-1] && a[x-1][y-1]<=a[x-1][y])    //if diagonal is minimum
        {
            DTW_score=DTW_score+a[x-1][y-1];    //add it
            x--;        //go to its position
            y--;
        }

        else if(a[x][y-1]<=a[x-1][y-1] && a[x][y-1]<=a[x-1][y])         //if top is minimum
        {
            DTW_score=DTW_score+a[x][y-1];
            y--;
        }
        else        //add its side
        {
            DTW_score=DTW_score+a[x-1][y];
            x--;
        }
    }

    printf("\nDTW score is: ");
    printf("%d", DTW_score);
}

int abs_sub(int a, int b)
{
    //finds the absolute subtraction (difference) of 2 entered numbers
    int c=a-b;
    if(c<0)
        c=-c;
    return c;
}

int Min(int a, int b, int c)
{
    //finds the minimum of 3 numbers entered
    if(a<b && a<c)
        return a;
    else if(b<a && b<c)
        return b;
    else
        return c;
}


int main(void)
{
    int i;
    for(i=0; i<sample_size; i++)
        master[i]=i;
    for(i=0; i<sample_size; i++)
        temp_values[i]=i*2;
    calc_DTW_score(master, temp_values);
}
