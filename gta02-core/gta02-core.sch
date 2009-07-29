EESchema Schematic File Version 2  date Wed 29 Jul 2009 21:06:16 BST
LIBS:power,device,conn,gta02-core
EELAYER 24  0
EELAYER END
$Descr A3 16535 11700
Sheet 1 12
Title "GTA02-CORE"
Date "20 jun 2009"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Notes 9050 5000 0    60   ~ 0
x
Text Notes 9050 4850 0    60   ~ 0
79xx
Text Notes 9050 4700 0    60   ~ 0
77xx
Text Notes 7100 4700 0    60   ~ 0
76xx
Text Notes 5100 4850 0    60   ~ 0
U1501
Text Notes 5100 4700 0    60   ~ 0
75xx
Text Notes 3100 4850 0    60   ~ 0
U1501
Text Notes 3100 4700 0    60   ~ 0
60xx
Text Notes 1100 5150 0    60   ~ 0
15xx
Text Notes 1100 5000 0    60   ~ 0
78xx
Text Notes 1100 4850 0    60   ~ 0
47xx
Text Notes 1100 4700 0    60   ~ 0
46xx
Text Notes 11000 2500 0    60   ~ 0
R1918
Wire Notes Line
	12350 10200 12350 6150
Wire Wire Line
	13450 8800 13650 8800
Wire Wire Line
	13450 8950 13650 8950
Wire Wire Line
	13450 8500 13650 8500
Wire Wire Line
	13450 8650 13650 8650
Wire Notes Line
	12350 10200 15000 10200
Wire Notes Line
	12350 6150 15000 6150
Wire Wire Line
	13450 9550 13650 9550
Wire Wire Line
	13450 9250 13650 9250
Wire Wire Line
	13450 8050 13650 8050
Wire Wire Line
	13650 7600 13450 7600
Wire Wire Line
	13650 7000 13450 7000
Wire Wire Line
	13650 7150 13450 7150
Wire Wire Line
	13650 6850 12850 6850
Wire Wire Line
	13650 7300 13450 7300
Wire Wire Line
	12850 6850 12850 6700
Wire Wire Line
	13450 7750 13650 7750
Wire Wire Line
	13450 8200 13650 8200
Wire Wire Line
	13450 9400 13650 9400
Wire Wire Line
	13450 9700 13650 9700
Wire Notes Line
	15000 6150 15000 10200
Text Notes 11000 2350 0    60   ~ 0
U1501
Text Notes 11000 2200 0    60   ~ 0
49xx
Text Notes 9050 2800 0    60   ~ 0
U1501
Text Notes 9050 2650 0    60   ~ 0
44xx
Text Notes 9050 2500 0    60   ~ 0
43xx
Text Notes 9050 2350 0    60   ~ 0
41xx
Text Notes 9050 2200 0    60   ~ 0
30xx
Text Notes 7100 2350 0    60   ~ 0
U1501
Text Notes 5100 2350 0    60   ~ 0
R2504
Text Notes 1100 2350 0    60   ~ 0
R2502
Text Notes 7100 2200 0    60   ~ 0
22xx
Text Notes 5100 2200 0    60   ~ 0
17xx
Text Notes 3100 2200 0    60   ~ 0
15xx
Text Notes 1100 2200 0    60   ~ 0
15xx
NoConn ~ 13450 8950
NoConn ~ 13450 8800
Text GLabel 13650 8950 2    60   Input ~ 0
MODEM_RX
Text GLabel 13650 8800 2    60   Output ~ 0
MODEM_TX
NoConn ~ 13450 8650
NoConn ~ 13450 8500
Text GLabel 13650 8650 2    60   Output ~ 0
MODEM_RTS
Text GLabel 13650 8500 2    60   Input ~ 0
MODEM_CTS
Text Notes 12450 6300 0    60   ~ 0
GSM labels unused while calypso status is unresolved
NoConn ~ 13450 9700
NoConn ~ 13450 9550
NoConn ~ 13450 9400
NoConn ~ 13450 9250
Text GLabel 13650 9700 2    60   Input ~ 0
MICIN
Text GLabel 13650 9550 2    60   Input ~ 0
MICIP
Text GLabel 13650 9400 2    60   Output ~ 0
EARN
Text GLabel 13650 9250 2    60   Output ~ 0
EARP
NoConn ~ 13450 8200
NoConn ~ 13450 8050
Text GLabel 13650 8200 2    60   Input ~ 0
MODEM_ON
Text GLabel 13650 8050 2    60   Input ~ 0
MODEM_RST
NoConn ~ 13450 7750
Text GLabel 13650 7750 2    60   Input ~ 0
DL_GSM
NoConn ~ 13450 7600
Text GLabel 13650 7600 2    60   Input ~ 0
INTO
NoConn ~ 13450 7300
NoConn ~ 13450 7150
NoConn ~ 13450 7000
$Comp
L PWR_FLAG #FLG?
U 1 1 4A5C71E1
P 12850 6700
F 0 "#FLG?" H 12850 6970 30  0001 C CNN
F 1 "PWR_FLAG" H 12850 6930 30  0000 C CNN
	1    12850 6700
	1    0    0    -1  
$EndComp
Text GLabel 13650 7150 2    60   Output ~ 0
SIM_CLK
Text GLabel 13650 7000 2    60   Output ~ 0
SIM_RST
Text GLabel 13650 6850 2    60   Output ~ 0
SIM_V
Text GLabel 13650 7300 2    60   BiDi ~ 0
SIM_IO
$Sheet
S 5000 4500 1550 1500
U 4A4B8F03
F0 "SD-SIM" 60
F1 "sd-sim.sch" 60
$EndSheet
$Sheet
S 3000 2000 1500 1500
U 4A45E033
F0 "CPU-POWER" 60
F1 "cpu-power.sch" 60
$EndSheet
$Sheet
S 3000 4500 1500 1500
U 4A434A5F
F0 "LCM" 60
F1 "lcm.sch" 60
$EndSheet
$Sheet
S 8950 4500 1500 1500
U 4A424951
F0 "BT" 60
F1 "bt.sch" 60
$EndSheet
$Sheet
S 1000 2000 1500 1500
U 4A0AE5B6
F0 "CPU" 60
F1 "cpu.sch" 60
$EndSheet
$Sheet
S 7000 2000 1500 1500
U 4A0AE69D
F0 "MEMORY" 60
F1 "memory.sch" 60
$EndSheet
$Sheet
S 1000 4500 1500 1500
U 4A0AE5F3
F0 "IO" 60
F1 "io.sch" 60
$EndSheet
$Sheet
S 5000 2000 1500 1500
U 4A0AE5D4
F0 "PMU" 60
F1 "pmu.sch" 60
$EndSheet
$Sheet
S 8950 2000 1500 1500
U 4A0AE60B
F0 "AUDIO" 60
F1 "audio.sch" 60
$EndSheet
$Sheet
S 7000 4500 1500 1500
U 4A3D08B2
F0 "GPS" 60
F1 "gps.sch" 60
$EndSheet
$Sheet
S 10900 2000 1500 1500
U 4A3C2B62
F0 "USB" 60
F1 "usb.sch" 60
$EndSheet
$EndSCHEMATC
