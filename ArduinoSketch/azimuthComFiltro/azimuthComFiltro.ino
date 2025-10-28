/*
===============================================================================================================
QMC5883LCompass.h Library Azimuth Example Sketch
Learn more at [https://github.com/mprograms/QMC5883Compas]
===============================================================================================================
v0.3 - June 12, 2019
Written by MRPrograms 
Github: [https://github.com/mprograms/]

Release under the GNU General Public License v3
[https://www.gnu.org/licenses/gpl-3.0.en.html]
===============================================================================================================
*/
#include <QMC5883LCompass.h>
#include <SimpleKalmanFilter.h>

QMC5883LCompass compass;
SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.1);

void setup() {
  Serial.begin(115200);
  compass.init();
  
}

void loop() {
  int a;
  
  // Read compass values
  compass.read();

  // Return Azimuth reading
  a = compass.getAzimuth();
  int b = simpleKalmanFilter.updateEstimate(a);
  Serial.print("A:");
  Serial.print(a);
  Serial.print(", B:");
  Serial.print(b);
  Serial.println();
  
  delay(100);
}
