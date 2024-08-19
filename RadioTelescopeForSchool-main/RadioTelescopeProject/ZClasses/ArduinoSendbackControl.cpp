#include <windows.h>
#include <iostream>

class ArduinoSendbackControl
{
public:
    HANDLE serialHandle;

    // Constructor
    ArduinoSendbackControl(const char* portName) : serialHandle(INVALID_HANDLE_VALUE)
    {
        ArduinoSetup(portName);
    }

    // Destructor
    ~ArduinoSendbackControl()
    {
        ArduinoSendbackOver();
    }

    void ArduinoSetup(const char* portName)
    {
        serialHandle = CreateFileA(portName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        DCB dcbSerialParams = { 0 };
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        dcbSerialParams.BaudRate = CBR_9600;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
    }

    void ArduinoSendbackOver()
    {
        if (serialHandle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(serialHandle);
            serialHandle = INVALID_HANDLE_VALUE; // Set to invalid value after closing
        }
    }

    void sendPointData(int X, int Y)
    {
        if (serialHandle == INVALID_HANDLE_VALUE) //this will pop if you don't have anything connected... 
        {
            std::cerr << "Error: no serial connection to Arduino. Consult ArduinoIDE" << std::endl;
            return;//testing something.. trying to narrow down this null pointer return on the write file... 
        }

        char buffer[20];
        sprintf_s(buffer, sizeof(buffer), "SCAN_%d_%d", X, Y);

        DWORD bytesWritten;
        if (!WriteFile(serialHandle, buffer, strlen(buffer), &bytesWritten, NULL))
        {
            std::cerr << "Error writing to serial port. Consult ArduinoIDE: " << GetLastError() << std::endl;
            //throwing back another error to narrow down what's going on... 
        }
    }
};

