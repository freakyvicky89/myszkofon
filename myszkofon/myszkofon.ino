/*
 CS: pin 7
 MISO: pin 12
 SCK: pin 13
*/

#include <arduinoFFT.h>
#include <SPI.h>

#define CS 7
#define AUDIO_OUT 3 

#define SAMPLES 128          //Must be a power of 2
#define SAMPLING_FREQUENCY 200000 
 
arduinoFFT FFT = arduinoFFT();
 
unsigned int sampling_period_us;
unsigned long microseconds;
 
double vReal[SAMPLES];
double vImag[SAMPLES];

void setup() {
  Serial.begin(9600);
  SPI.begin();
  SPI.setDataMode(SPI_MODE0); // configuration of SPI communication in mode 0
  SPI.setClockDivider(SPI_CLOCK_DIV16); // configuration of clock at 1MHz
  pinMode(CS, OUTPUT);
}

void loop() {
  /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
        microseconds = micros();    //Overflows after around 70 minutes!
     
        vReal[i] = MIC3_getSound();
        vImag[i] = 0;
     
        while(micros() < (microseconds + sampling_period_us)){
        }
    }
 
    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

    double *mouse_freqs = vReal+100;
    double max_mouse_intens;
    max_mouse_intens = 0.0;

    for(byte idx = 0; idx < 10; idx++) {
      max_mouse_intens = mouse_freqs[idx] > max_mouse_intens ? mouse_freqs[idx] : max_mouse_intens;
    }

    if (max_mouse_intens > 3000.0) {
      double peak = FFT.MajorPeak(mouse_freqs, SAMPLES, SAMPLING_FREQUENCY);
      int frq = (int) (peak/20.0);
      if (frq < 5000) {
        tone(AUDIO_OUT, frq);
        Serial.println(max_mouse_intens);
      } else {
        noTone(AUDIO_OUT);
      }
    } else {
      noTone(AUDIO_OUT);
    }
    
    
    /*PRINT RESULTS*/
    
}

int MIC3_getSound(void) {
  digitalWrite(CS, LOW);  //activate chip select
  int sound = SPI.transfer(0) | (SPI.transfer(0) << 8); //reconstruct 12-bit data
  digitalWrite(CS, HIGH); //deactivate chip select
  return sound;
}
