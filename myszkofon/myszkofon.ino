/*
 * ALL FREQUENCIES IN kHz!!!
 * 
 MISO: pin D12
 SCK: pin D13
*/

#include <arduinoFFT.h>
#include <SPI.h>

#define CS PA_4 //A3 on board
#define DAC PA_5 //A4 on board

#define SAMPLES 128          //Must be a power of 2
#define SAMPLING_FREQUENCY 200 

#define CUTOFF_LOW 25
#define CUTOFF_HIGH 50
#define SHIFT_TO_LOW 1
 
arduinoFFT FFT = arduinoFFT();

inline unsigned int freq_to_ind(unsigned int frq) {

  return (frq * (SAMPLES - 1)) / SAMPLING_FREQUENCY;

}
 
unsigned int sampling_period_us=1000/SAMPLING_FREQUENCY;
unsigned long microseconds;

unsigned int cut_low_index = freq_to_ind(CUTOFF_LOW);
unsigned int cut_high_index = freq_to_ind(CUTOFF_HIGH);
unsigned int new_low_index = freq_to_ind(SHIFT_TO_LOW);
unsigned int shifted_freqs = cut_high_index - cut_low_index;
 
double vReal[SAMPLES];
double vImag[SAMPLES];

double vReal_pb[SAMPLES];
double vImag_pb[SAMPLES];

double *cut_low_real = vReal + cut_low_index;
double *cut_low_imag = vImag + cut_high_index;

double *new_low_real = vReal_pb + new_low_index;
double *new_low_imag = vImag_pb + new_low_index;

void setup() {
  //Serial.begin(9600);
  SPI.begin(CS);
  SPI.setDataMode(SPI_MODE0); // configuration of SPI communication in mode 0
  SPI.setClockDivider(72); // configuration of clock at 1MHz
  pinMode(CS, OUTPUT);
  pinMode(DAC, OUTPUT);
  analogWriteResolution(12);
}

void loop() {
  
  /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
        microseconds = micros();    //Overflows after around 70 minutes!
     
        vReal[i] = MIC3_getSound();
        vImag[i] = 0;

        analogWrite(DAC, vReal_pb[i]);

        vReal_pb[i] = 0;
        vImag_pb[i] = 0;
     
        while(micros() < (microseconds + sampling_period_us)) ;
        
    }
 
    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);

    /* ??? */
    for (unsigned int i=0;i<shifted_freqs;i++) {
      new_low_real[i] = cut_low_real[i];
      new_low_imag[i] = cut_low_imag[i];
    }

    /* PROFIT! */
    FFT.Windowing(vReal_pb, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_REVERSE);
    FFT.Compute(vReal_pb, vImag_pb, SAMPLES, FFT_REVERSE);
    
    
}

inline int MIC3_getSound(void) {
  digitalWrite(CS, LOW);  //activate chip select
  int sound = SPI.transfer(0) | (SPI.transfer(0) << 8); //reconstruct 12-bit data
  digitalWrite(CS, HIGH); //deactivate chip select
  return sound;
}
