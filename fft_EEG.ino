// define variables
#include "arduinoFFT.h"
 
#define SAMPLES 128             //Must be a power of 2
#define SAMPLING_FREQUENCY 1000 //Hz, must be less than 10000 due to ADC
 
arduinoFFT FFT = arduinoFFT();
 
unsigned int sampling_period_us;
unsigned long microseconds;
 
 double vReal[SAMPLES];
 double vImag[SAMPLES];
 

#define START_BYTE 0xAA
#define RAW_WAVE_16BIT 0x80
#define ASIC_EEG_POWER_INT 0x83
#define ATTENTION_VALUE 0x04
#define MEDITATION_VALUE 0x05
#define POOR_QUALITY 0x02

uint8_t plength;
int16_t rawEEG16Bit,rawdata;
uint32_t delta, theta, lowAlpha, highAlpha, lowBeta, highBeta, lowGamma, midGamma;
int j=0;

byte calculatedchecksum, checksum;
byte payloaddata[170], attention=0, meditation=0, blink_strength=0, poor=0;

void setup(){
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  Serial.begin(57600);    // should be this value
}

void loop(){
 


 for(int i=0; i<SAMPLES; i++)
    {
        microseconds = micros();    //Overflows after around 70 minutes!
     
        vReal[i] = Readpacket();
        vImag[i] = 0;
     
        while(micros() < (microseconds + sampling_period_us)){
        }
    }
 
    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
 
    /*PRINT RESULTS*/
    //Serial.println(peak);     //Print out what frequency is the most dominant.
 
    for(int i=0; i<(SAMPLES/2); i++)
    {
        /*View all these three lines in serial terminal to see which frequencies has which amplitudes*/
         
        //Serial.print((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES, 1);
        //Serial.print(" ");
       // Serial.println(vReal[i], 1);    //View only this line in serial plotter to visualize the bins  //    fft
    }
    float x_v_a = 0;
    float x_v_b = 0;

  for(int i=7; i<12; i++)
  {
    x_v_a = x_v_a + (vReal[i]*vReal[i])/5;
  }

  for(int i=13; i<30; i++)
  {
    x_v_b = x_v_b + (vReal[i]*vReal[i])/5;
  }



if(x_v_a > x_v_b && x_v_b > 4000000 && x_v_a >4000000)
{
   digitalWrite(13, HIGH);
   digitalWrite(12, LOW);
}

if(x_v_a < x_v_b && x_v_b > 4000000 && x_v_a >4000000)
{
   digitalWrite(13, LOW);
   digitalWrite(12, HIGH);
}

if(x_v_b < 4000000 && x_v_a < 4000000)
{
   digitalWrite(13, LOW);
   digitalWrite(12, LOW);
}
  //Serial.println(x_v_b);
  //Serial.println(x_v_a);
 
    delay(1000);  //Repeat the process every second OR:
//    while(1);       //Run code once






}

// Extract and calculate raw EEG and ASIC EEG power
int16_t CalculateRawEEGnPower(){
  
  for(int i = 0; i < plength; i++){
      if(payloaddata[i] == RAW_WAVE_16BIT)
      {
          rawEEG16Bit = Bytes2IntConverter(payloaddata[i+2],payloaddata[i+3]);                      
          
      }
  }
//Serial.println(rawEEG16Bit);
  return rawEEG16Bit;
}


int16_t Bytes2IntConverter(byte value1, byte value2){
  uint16_t value = value1*256 + value2;
  int16_t valueReturn=0;
  if (value > 32768)
    valueReturn = value - 65536;
  else
    valueReturn = value;
    
   return valueReturn;
}


byte readserial(){
  byte packet;
  while(!Serial.available());
  packet = Serial.read();
  return packet;
}

int16_t Readpacket()
{
   if(readserial() == START_BYTE)
    if(readserial() == START_BYTE){
      // Ignore date if its length > 169
      plength = readserial();
      if(plength > 169)
      return 0;

      // Check validity of data using checksum byte
      calculatedchecksum = 0;
      for(int i = 0; i < plength; i++){
        payloaddata[i] = readserial();
        calculatedchecksum += payloaddata[i];
       }
       checksum = readserial();
       calculatedchecksum = 255 - calculatedchecksum;

       // Extract RawEEG and Power
       if(checksum == calculatedchecksum)
       {
         rawdata = CalculateRawEEGnPower();
//         Serial.println(rawdata, DEC);
       }
    }

    return rawdata;

}
