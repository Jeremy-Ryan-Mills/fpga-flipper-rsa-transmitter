Author: Jeremy Mills
Date: 1/7/2025

My FPGA/Flipper RSA Transmitter is a personal project that intends to send encrypted 8 byte messages using the Flipper Zero's sub GHz radio. There are multiple parts to this project, including an app on the Flipper
zero that reads the encrypted message from the Flipper's GPIO pins using a custom communication protocol and transmits the data on a specified MHz frequency. The encrypted message comes from the SRT-96B-MEZ-FPGA
(Shiratech 96Board Mezzanine FPGA board) with an Intel Altera Max 10 FPGA. The FPGA will have code that encrypts a message that is received through specified pins (with the same communication protocol), and then
encrypted with randomly generated 64 bit RSA keys. Testing and debugging is done with Arduino UNO.

This is a personal project that I pursued to learn System Verilog, basic RF concepts, and learning the Flipper Zero firmware and hardware abstraction to build my own apps.
