# NFC2
For the new improved NFC reader

Provided to support the hardware provided by Bostin Technology

Firstly download the github repository onto your Raspnberry Pi, then follow the steps below to build and run the demo software.

To build the application run the following commands

1. Download and install cmake

        apt-get install cmake
        
2. Build make lists

        cd NFC2/rfal_v1.3.0/build
        cmake ..

    Note the double ".." which forces it to start in the rfal... directory

3. Build the application

        make

4. Run the Application

    Change to the applications directory
    
        cd applications/
        
    Then
    
        sudo ./nfcPoller

For more information, please refer to www.cogniot.eu
