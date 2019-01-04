# iCode NFC Reader

The purpose of the software is to demonstrate the capabilities and provide some working software as a platform for starting project development. Although the software is written with the iCode NFC-V tags in mind, the libraries provided by ST Microelectronics will work with all common NFC tag types.
The sample software is built on the ST25R3911B HF RFID Reader IC  rfal libraries provided by ST Microelectronics (https://www.st.com/en/nfc/st25r3911b.html#sw-tools-scroll).

# iCode NFC READER
The iCode NFC Reader uses the ST25R3911BNFC / HF RFID Reader to communicate with the iCode tags.
Using the supplied software the following functions are available, each command being accessed by the letter.
•	a – Scan for Available Cards
•	s – Scan for specific card type
•	m – Example Read card memory (ST Example) 
•	v – Read Block Zero from first NFC-V tag found
•	w – Write to Block Zero on the first NFC-V tag
•	e – Exit Program
Note 1: Options a, s and m are based on the examples provided by ST.

For more information about these commands and all of the functionality of the Ls.1 reader please refer to the datasheet which can be found at http://cogniot.eu/wp/nfc-reader/ .

# INSTALLATION INSTRUCTIONS
To install the software, follow these simple steps:-
1)	Upgrade and Update the operating system
a)	sudo apt-get update
b)	sudo apt-get upgrade
2)	Using Raspberry Pi Configuration, disable the shell and kernel from using the serial port.
a)	From the Menu, select Preferences, Raspberry Pi Configuration
b)	On the interfaces tab, set Serial to ‘Disabled’
c)	On the interfaces tab, set SPi to enabled
3)	git clone https://github.com/CognIot/iCode.git
4)	cd iCode/iCodeDemo/build
5)	sudo apt-get install cmake
6)	cmake ..
a)	NOTE: The 2 full stops are important
7)	make
8)	cd applications
9)	sudo ./exampleNfC


