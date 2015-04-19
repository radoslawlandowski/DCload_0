#include <Encoder.h>
#include <MCP3424.h>
#include <Wire.h>
#include <TimerOne.h>

 MCP3424 MCP(0x68);   // Declaration of MCP3424 with Address of I2C
 Encoder knobRight(9, 8);

 long Voltage;
 long Buf[5];
 long positionLeft  = 1;
 long positionRight = 1;

class mode /* Does the appropriate calculations according to the mode that is set. There's no method for constant current mode, because this function is held by external hardware */
{
  private:
    static const float maxEncoderSetting = 65535.0;
    static const float maxResistance = 1000.0;
    static const float maxPower = 50.0;
    static const char stepIncrement = 1;
    static const float refVoltage = 1.1;
    static const int ADCrange = 1023; // all of the five above describe parameteres of this device
    static const float voltDivRatio = 22.38;
    float voltage, current, resistance, power;
    float setCurrent, setResistance, setPower;

  public:
    void updateReadings()
    {
      current =  (analogRead(A0)*refVoltage)/ADCrange; //analog input for measuring total voltage
      voltage = (analogRead(A2)*refVoltage*voltDivRatio)/ADCrange;

      resistance = voltage/current;
      power = voltage*current;
    }
    float getPower()
    {
      updateReadings();
      return power;
    }
    float getVoltage()
    {
      updateReadings();
      return voltage;
    }
    float getCurrent()
    {
      updateReadings();
      return current;
    }
    float getResistance()
    {
      updateReadings();
      return resistance;
    }

    float constantPower(long encoderSetting)
    {
      updateReadings();

      float setPower = (float)maxPower*(encoderSetting/5000.0);

      if(power < setPower)
      {
        OCR1A = OCR1A + stepIncrement;
      }
      else
      {
        if(OCR1A < 16) OCR1A = 24;
        OCR1A = OCR1A - stepIncrement;
      }
     return power;
    }

    int constantResistance(long encoderSetting)
    {
       updateReadings();

       float setResistance = maxResistance*(encoderSetting/maxEncoderSetting);

       if(resistance > setResistance)
       {
          OCR1A = OCR1A + stepIncrement;
       }
       else
       {
          OCR1A = OCR1A - stepIncrement;
       }
       return resistance;
    }
};

 float queueFunc(long data, long *buffer, char sizeOfIt)
{
	int aux = 1;

	for(aux ; aux < sizeOfIt  ; aux++)
	{
		buffer[sizeOfIt - aux] = buffer[sizeOfIt - aux - 1];
	}
    buffer[0] = data;

    long sum = 0;
    aux = 0;

	for(aux ; aux < sizeOfIt ; aux++)
    {
        sum += buffer[aux];
    }

    return sum/sizeOfIt;

}

char setParameters()
{
	char pressedKey = 0;
	char aux = 1;
        char aux2 = 0;
	long multiplication = 1;
        long positionRight = 1;
        long newRight = 1;
        long myVal = 0;

	while(aux)
	{
		if(!digitalRead(2))
		{
                    delay(200);
                    multiplication *= 10;
                }
		if(!digitalRead(3))
		{
                    delay(200);
                    if(multiplication !=1)
	            multiplication /= 10;
		}
                newRight = knobRight.read();
                float pos;
                if (newRight != positionRight)
                {
                    aux2++;

                    if(newRight > positionRight)
                    {
                       pos = 1;
                    }
                    else
                    {
                       pos = -1;
                    }
                    positionRight = newRight;
                    if(aux2 == 4)
                    {
                      myVal += multiplication*pos;
                      aux2 = 0;
                    }
                }
          }

        return 0;
}


void setup()
{
  MCP.Configuration(2,18,0,1);
  Serial.begin(9600);
  Serial.println("DC Electronic Load:");

  DDRB |= (1 << DDB1) | (1 << DDB2);
  TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
  TCCR1B = (1 << WGM12) | (1 << WGM13) |(1 << CS10);
  OCR1A = 0;
  ICR1 = 0xffff;
  OCR1A = 0x0000;

  pinMode(A5, INPUT);
  pinMode(A4, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
}


void loop()
{
  int auxiliary = 0;
  int myVal = 0;
  float final = 0;

  while(1){
    long newLeft, newRight;
    while(digitalRead(A5))
    {
      OCR1A = 0;
      newRight = knobRight.read();
      if (newRight != positionRight)
      {
          positionRight = newRight;
          if(positionRight <= 0) positionRight = 0;
          myVal = positionRight/4;
          final = (float)myVal/100;
      }
      currentSet.changeValue(final);
    }

    myMode.constantPower(myVal);
  }
}
