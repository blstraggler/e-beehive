// connected beehive v04
// 6 parameters are transmitted and stored on the cloud (keen_IO)
// internal temperature  (probe borrowed on an old laptop batteries) : A0
// external temperature: probe DHT11 : D2 
// hygrometry : probe DHT11 : D2
// luminosity : LED used as sensor : between A1 and A2
// sound : microphpne electret : A4
// weight : differential amplifier ADC HX711 on the output of a low price scale : D0 and D1 
// use of function publish with webhook declared towards keen_IO
// python software for data retrieval and plotting
// TBA = TO  BE ADAPTED 

// Libraries :

#include "math.h"
//  HX711
// This #include statement was automatically added by the Particle IDE.
#include "HX711ADC/HX711ADC.h"
HX711ADC *scale = NULL;

//  DHT11
// This #include statement was automatically added by the Particle IDE.
#include "PietteTech_DHT/PietteTech_DHT.h"
#define DHTPIN 2 // 
#define DHTTYPE DHT11 // DHT 11
//declaration
void dht_wrapper(); // must be declared before the lib initialization
// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

unsigned long trefresh = 60000UL;  //   period of upload on cloud
int calibration_factor = -2188; // calibration of the scale 
long offs = 7826236 ; // offset
int i;
int nmes=100; // number of measurements at each cycle 
int temp, hygro, lumi, zzz ;
double dtemp=24., dhygro=99., dlumi=70., dzzz=55., dpoids=60., dtempext=888.; // parameters initialization
unsigned long lastTime = 0UL;
char publishString[240]; // string of published data


void dht_wrapper() {
    DHT.isrCallback();
}


void temp_probe() // internal temperature 
{
    int ctemp = 0;
    for(i=0 ; i<nmes ; i++){
      delay(1000/nmes) ;    
      ctemp += analogRead(A0) ;
    }
    temp = ctemp / nmes ;
   // dtemp =  temp ; //92.7735 - temp*0.0210861;
   //    dtemp = -6.70820 + (0.00187501 + 3.94358e-06*temp)*temp;
   dtemp = 135. * log10(temp/1088.) ; // TBA
}

void hygro_probe() // sonde DHT11 : hygrometry and external temperature 
{
    float chygro = 0.;
    float ctempext = 0.;
    for(i=0 ; i<nmes ; i++){
      delay(1000/nmes) ;    
      float h = DHT.readHumidity();
      float t = DHT.readTemperature();
      chygro += h ;
      ctempext +=t ;
      
    }
    dhygro = chygro / nmes ; // moyenne 
    dtempext = ctempext / nmes ;
}

void lumi_probe() // luminosity
{
   int j;

    pinMode(3,OUTPUT);
    pinMode(4,OUTPUT);
    digitalWrite(3,HIGH);
    digitalWrite(4,LOW);
    delay(500);

    pinMode(3,INPUT);

     // Count how long it takes the diode to bleed back down to a logic zero
    for ( j = 0; j < 30000; j++) {
      if ( digitalRead(3)==0) break;
    }
    dlumi = j ; ///// to be calibrated */
//    dlumi = float(analogRead(A2)-analogRead(A1)) ; // LED between A2 & A1 

}

void zzz_probe() // read electret n-times ->  variance 
{
    double s = 0;
    double s2 = 0 ;
    int ech ;
    for(i=0 ; i<nmes ; i++){
      delay(1000/nmes) ;  
      ech = analogRead(A4) ;
      s +=  ech ;
      s2 += ech*ech ;
    }
    dzzz = sqrt(s2 / nmes - (s/nmes)*(s/nmes)) ; // variance = noise level

}

void poids_probe() // read scale  ADC 
{

  dpoids = scale->get_units(100);
 // Serial.printf("Reading: %f hg",dpoids);
 // Serial.printf(" calibration_factor: %i", calibration_factor);
 // Serial.println();
}

int ruche(String command) // for possible command
{
  
  if(command == "on")
  {
    //swith on
    digitalWrite(D3, HIGH);
    digitalWrite(D7, HIGH);
    return 1;
  }
  else if(command == "off")
  {
    digitalWrite(D3, LOW);
    digitalWrite(D7, LOW);
    return 0;
  }
  return 0;
} 


void setup() {
  Particle.function("ruche", ruche);
  Particle.variable("ruche_temp",&dtemp, DOUBLE);
  Particle.variable("ruche_hygro",&dhygro, DOUBLE);
  Particle.variable("ruche_tempext",&dtempext, DOUBLE);
  Particle.variable("ruche_poids",&dpoids, DOUBLE);
  Particle.variable("ruche_lumi",&dlumi, DOUBLE);
  Particle.variable("ruche_zzz",&dzzz, DOUBLE);
    
  scale = new HX711ADC(D0, D1);
  scale->set_scale();
  scale->tare();    //Reset the scale to 0
  long zero_factor = scale->read_average(); //Get a baseline reading
  scale->set_offset(offs); 
  scale->set_scale(calibration_factor); //Adjust to this calibration factor
  delay(2000);
}
void loop() { // main loop for reading / publish 

  temp_probe();
  hygro_probe();
  poids_probe();
  zzz_probe();
  lumi_probe();
 //   delay(2000);
  unsigned long now = millis();
    //Every 15 seconds publish uptime
  if (now-lastTime>trefresh) {
        lastTime = now;
        // now is in milliseconds
        sprintf(publishString,"temp: %f tempext: %f poids: %f hygro: %f lum: %f activ: %f",dtemp, dtempext, dpoids, dhygro, dlumi, dzzz);
        Particle.publish("keen_io",publishString,60);
    }    
}