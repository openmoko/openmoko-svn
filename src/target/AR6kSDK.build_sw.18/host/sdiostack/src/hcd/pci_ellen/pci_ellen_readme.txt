Codetelligence Embedded SDIO Stack Host Controller Driver Read Me.
Copyright 2005, Atheros Communications, Inc.

Tokyo Electron PCI Ellen SD/SDIO Host Controller Driver

General Notes:
-  SDIO Standard Host 1.0 compliant.
-  SD 1 and 4 bit modes, up to 24 Mhz Clock rates.
-  SDIO IRQ detection for 1,4 bit modes.
-  Programmed I/O mode (hardware does not implement DMA).
-  Card detect via slot mechanical switch
    
Linux Notes:

Module Parameters:
   
    "debug" =  set module debug level (default = 4).
    Module Debug Level:      Description of Kernel Prints:
           7                   Setup/Initialization
           8                   Card Insertion (if mechanical card switch implemented)
           9                   Processing bus requests w/ Data
          10                   Processing for all bus requests.
          11                   Configuration Requests.
          12                   SDIO controller IRQ processing
          13                   Clock Control
          14                   SDIO Card Interrupt  



          

