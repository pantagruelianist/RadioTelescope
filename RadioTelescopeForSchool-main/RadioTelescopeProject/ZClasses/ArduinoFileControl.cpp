#ifdef DOXYGEN_SHOULD_SKIP_THIS

#include <iostream>
#include <fstream>
#include <Windows.h>//need this for serial connect.... 

using namespace std;

//asynchronous program for taking Arduino output from antenna... puts a user definted width(int)*height(int) product of values set between 0-255 into a text file to be interpreted by 
//ImageCompiler class... 

//by Chris... 
void createNewFile(ofstream& file)
{
    //fire a timestamp... 
    time_t t = time(0);
    tm now;
    localtime_s(&now, &t);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "data_%Y%m%d%H%M%S.txt", &now);

   
    file.open(buffer); //open the file... 
}

int main()
{
    cout << "File authoring program now running." << endl;//debug purposes... 
    const char* portName = "COM2"; //seems to work given that the IDE makes a new Serial port for the sake of this... Shows up on Device Manager. Have no idea how to force this on someone else's computer... 
  
    HANDLE hSerial = CreateFileA(portName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0); //serial communication function... I'm opening an existing one, we're just reading it... 

    
    DCB dcbSerialParams = { 0 }; //Device Control Block... this is for controlling and configuring serial communications... null init for setting up... 
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams); //we'll use dcbSerialParams.. And we'll define them below... 
 

    dcbSerialParams.BaudRate = CBR_9600;  //default rates... double check in the ArduinoProgram... 
    dcbSerialParams.ByteSize = 8;  //max character size... 
    dcbSerialParams.StopBits = ONESTOPBIT; //handles end flag on the packet... 
    dcbSerialParams.Parity = NOPARITY;// no need for parity checking since I think it's clear to me when looking at the .txt file if there's an issue... 

    bool receivedData = false; //tells us if data is recieved... 
    bool toggle = false;       //toggle for blink... 

    ofstream outputFile; //new file... 

    try //going for a clean error handling... 
    {
        while (true)//unprotected while but I don't really need this to stop at any point frankly... 
        {
           //start reading... 
            char buffer[256];
            DWORD bytesRead;
            ReadFile(hSerial, buffer, sizeof(buffer), &bytesRead, nullptr);//first arg is the serial pipe we setup earlier... buffer is the size of the area we'll hold what comes from the pipe...
            //size of is the size of the max number of bytes to be read... 
            //&bytesRead is a pointer that holds the number of bytes read... After this call it'll hold the number read from pipe... 
            //no need for async ops since we have such a long time between sent information... nearly 5 to 2 seconds per input... nullptr it... 

            
            buffer[bytesRead] = '\0'; //clear it... 

            
           

            if (string(buffer).find("START") != string::npos) //new file starts now... checks if the entry is empty first though... string::npos indicates "no position"... 
            {
                
                createNewFile(outputFile);//boom timestamped file... 
            }
            else if (string(buffer).find("END") != string::npos)//ends the file... means we're done and there's a file ready for the GUI to put on the screen! 
            {
                //this comes before the .is_open() case because we don't want END added to the file lol... 
                outputFile.close();//we're done with it, close it... 
            }
            else if (outputFile.is_open())
            {
                
                outputFile << buffer << endl;//add the data to the file during the scan... 
            }

            // Output received data or a placeholder message if no data is received
            if (bytesRead > 0)
            {
                cout << "Received: " << buffer << endl;
                receivedData = true; // Set the flag if data is received
            }
            else
            {
                if (receivedData)
                {
                    if (toggle)
                    {
                        cout << "Received: None." << endl;
                    }
                    else
                    {
                        cout << endl; // Empty line for blinking
                    }
                    toggle = !toggle; // Toggle the flag
                }
                else
                {
                    cout << "Will update when the device sends data. Please check if the Arduino is connected and your COM2 is configured to it." << endl;
                }

                Sleep(1000); // Sleep for 1 second before the next check
            }
        }
    }
    catch (const exception& e)
    {
        cerr << "Invalid input" << endl; //thought I'd have time to play with exceptions but must move... 
    }

   
    CloseHandle(hSerial);//we're done with this throw it out. 

    return 0;
}

#endif