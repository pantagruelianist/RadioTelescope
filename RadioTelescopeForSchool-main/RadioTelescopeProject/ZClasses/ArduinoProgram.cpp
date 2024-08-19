#ifdef DOXYGEN_SHOULD_SKIP_THIS

#include <AccelStepper.h> //need this for stepper motor control. 

//this program runs on ArduinoIDE for the Arduino control... 
//I had originally planned for an SDR control but I just couldn't hack it. I couldn't figure out the SDR in time... We will be within the 950MHz - 2150MHz range... using an NSendato 
//satellite finder. Ran wire from speaker to A0... Hypothetically, in this range, I should be able to "see" the moon. Sun shows up regardless. 

//by Chris 

class Scanner
{
public:
    Scanner() : stepperX(AccelStepper::DRIVER, 2, 5), stepperY(AccelStepper::DRIVER, 3, 6), scanningInProgress(false) {} //init the steppers... 

    void setup() //learned quickly about a frustrating fact regarding stepper motors... 
    {

        Serial.begin(9600); //matches ArduinoProgram.cpp, we're good... 

        //to start, we need to be gentle with these values due to torque and power concerns related to the motor. The dish antenna is around 13 pounds... 
        stepperX.setMaxSpeed(1);  //one step per second... 
        stepperX.setAcceleration(1); //one step per second per second... 

        stepperY.setMaxSpeed(1);  
        stepperY.setAcceleration(1);
    }

    void processSensorData()
    {
        int sensorValue = analogRead(A0); //this is an arduino specific function that allows one to read from the analog pin, A0 where the satellite finder's buzzer is run into... 
        int normalizedValue = map(sensorValue, 0, 1023, 0, 255); //normalize the value for the image maker... 
        sendSensorDataToComputer(normalizedValue);//sends info down serial port. 
        delay(100);  //just wait for data fidelity... I don't know what good protocol is for waits or what sorts of flags I should be sending between or before messages... probably would have been a good idea... 
        //we have a good limit of time with the antenna having to wait five to two seconds before turning again... 
    }

    void sendSensorDataToComputer(int data) 
    {
        Serial.println(data); //send the int back to computer for file interp... 
    }

    void sendStartToComputer() 
    {
        Serial.println("START"); //send this flag to let the computer know it's time to party... 
        
    }

    void sendEndToComputer() 
    {
        Serial.println("END");//send this flag to let the computer know the file is completed, so go ahead and close the file... 
        //no need to wait since we're just done. 
    }

    bool moveStepperMotors(int xDegrees, int yDegrees) 
    {
        scanningInProgress = true; //just to make sure no new commands come down the pipe. 
        sendStartToComputer();//fire the start... need to let the program running on the computer that it's gonna send new data... 
        delay(100); //give time for the program on the computer to get it's bearings... In lieu of state checks that need to be sent back down line from computer we'll just do some waiting...

        int originalYPosition = stepperY.currentPosition(); //original positions for later.. 
        int originalXPosition = stepperX.currentPosition();

        // Move along the Y-axis
        for (int y = 1; y <= yDegrees; ++y)
        {

            for (int x = 0; x <= xDegrees; ++x) //scan along x... 
            {
                // Move the stepper motor to the specified X degrees
                stepperX.moveTo(x);

                // Wait for the X motor to reach its position
                while (stepperX.distanceToGo() != 0) 
                {
                    stepperX.run();
                    processSensorData();  
                    delay(2000);// wait because I am afraid of the inertia.... guessing 2 seconds is safe... 
                }

                //take it back home... 
                stepperX.moveTo(originalXPosition);
                while (stepperX.distanceToGo() != 0)
                {
                    stepperX.run();
                    delay(5000);// wait because I am afraid of the inertia.... guessing 5 seconds is safe here... 
                }
                stepperX.setCurrentPosition(0);
            }

            // Increment the Y motor by 1 degree and move to the next Y position
            stepperY.move(1);

            // Wait for the Y motor to reach its position
            while (stepperY.distanceToGo() != 0) 
            {
                stepperY.run();
                processSensorData();  // Continue processing sensor data during movement
                delay(1000);// wait because I am afraid of the inertia....
            }

           
            stepperY.setCurrentPosition(0); //reset... 
        }

        //Reset Y to original position after all is said and done... 
        stepperY.moveTo(originalYPosition); //we saved this at the start... 
        while (stepperY.distanceToGo() != 0) //run until it goes home... 
        {
            stepperY.run();//go... 
            delay(1000);// wait because I am afraid of the inertia....
        }
        stepperY.setCurrentPosition(0);//reset position now that we've set the Y position back to start.... 
       
        sendEndToComputer();  // Send "END" signal at the end
        scanningInProgress = false; //all done, we can go at it again. 
        
        return true;//all done... 
    }
    void listenForCommands()
    {
       
        if (scanningInProgress) //is it scanning? 
        {
            return; //yeah end it if it is... take no info in... no need to trigger anything further... 
        }

        if (Serial.available() > 0) //if the serial is valid... 
        {
            
            String command = Serial.readStringUntil('\n'); //different from 

            
            if (command.startsWith("SCAN_")) //for the scan command... 
            {
               
                int underscore1 = command.indexOf('_'); //xval... 
                int underscore2 = command.indexOf('_', underscore1 + 1);//yval... 

                if (underscore1 >= 0 && underscore2 >= 0) //check if the values are valid... cannot have this go "down" as it would break the dang assembly... 
                {
                    String xDegreesStr = command.substring(underscore1 + 1, underscore2);
                    String yDegreesStr = command.substring(underscore2 + 1);

                    //recast to int.... 
                    int xDegrees = xDegreesStr.toInt();
                    int yDegrees = yDegreesStr.toInt();

                  
                    Serial.println("SCAN COMMAND");
                    if (moveStepperMotors(xDegrees, yDegrees)) //gotta make sure its' true... 
                    {
                        Serial.println("Scan Done, sent to PC...");//all done! 
                    }
                }
            }
        }
    }

private:
    AccelStepper stepperX; //stepper motor obj for x axis... 
    AccelStepper stepperY; //stepper motor obj for y axis... 
    bool scanningInProgress; //whether or not the scan is going... if it's going we need to limit input from user... 
};


Scanner scanner; //make a new scanner on runtime... 

void setup() //entry method of sorts... works like OnStart() in Unity C#... 
{
    scanner.setup(); //fire that bad boy up... 
}

void loop() //just an update loop... 
{

    scanner.listenForCommands();//listen for new commands... 

    delay(100); //don't have enough arduino knowledge to know how much it can do per second... let's be safe and cut it there... 
}

#endif