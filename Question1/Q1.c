#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>
#include <string.h>
#include <math.h>

//This struct is used to group all the relevant values q1Function needs to return
typedef struct{
    float avg;
    float max;
    float min;
    float *arrEven;
    unsigned int arrEvenLen;
} ArrayInfo;


//This is an interface for memory allocation in order to let the calling code provide its prefered allocator,
//makes the memory allocation explicit for the caller and lets the caller optimize memory allocation if needed
typedef void* (*AllocatorFunction)(size_t, size_t);

//The main function of Question 1
ArrayInfo q1Function(float arr[], unsigned int len, AllocatorFunction allocatorFunction){

    ArrayInfo arrayInfo = {};

    //Stack allocated array used to store all the even values found in the original array
    unsigned int evenCount = 0;
    float evenNum[len];

    float sum = 0;
    arrayInfo.max = arrayInfo.min = arr[0];

    for(int i = 0; i < len; i += 1)
    {
        if(arr[i] > arrayInfo.max) arrayInfo.max = arr[i];
        else if(arr[i] < arrayInfo.min) arrayInfo.min = arr[i];
        sum += arr[i];

        //Checks if the float is a whole number and multiple of 2 (since even-number logic only applies to integers)
        if(floorf(arr[i]) == arr[i] && fmodf(arr[i], 2.0f) == 0.0f){

            //Stores the even values in the stack allocated array
            evenNum[evenCount] = arr[i];
            evenCount += 1;
        } 
    }

    //Allocates a new array with the user provided allocator and with the length of total even numbers inside the original array
    arrayInfo.arrEven = (float*) allocatorFunction(sizeof(float) * evenCount, alignof(float));
    arrayInfo.arrEvenLen = evenCount;

    //Copies memory from the stack allocated array to the array that will be returned to the caller
    memcpy(arrayInfo.arrEven, &(evenNum[0]), evenCount * sizeof(float));

    arrayInfo.avg = sum / len;
    return arrayInfo;
}

int main(){

    float arr[] = {1, 2, 3, 4, -2, 0, 1, 3, 5, -11, 6, 7, 0, 4};

    int arrSize = sizeof(arr) / sizeof(arr[0]);

    ArrayInfo arrayInfo = q1Function(arr, arrSize, _aligned_malloc);

    printf("Average: %f, Max: %f, Min: %f\n", arrayInfo.avg, arrayInfo.max, arrayInfo.min);
    printf("Even array (Length: %d): [", arrayInfo.arrEvenLen);
    for(int i = 0; i < arrayInfo.arrEvenLen; i++) printf("%f, ", arrayInfo.arrEven[i]);
    printf("]\n");

}