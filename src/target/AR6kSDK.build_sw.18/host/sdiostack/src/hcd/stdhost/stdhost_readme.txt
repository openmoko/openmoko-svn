Codetelligence Embedded SDIO Stack Host Controller Driver Read Me.
Copyright 2006, Atheros Communications, Inc.

Standard Host Controller Driver Core

This sample driver source (stdhost) provides reference code for a
SDIO Standard Host 2.0 compliant controller.  The source code is designed to be compiled 
within a bus-specific implementation (i.e PCI).  The bus implementation is responsible for
allocating I/O, interrupt and DMA resources.  The reference code may contain OS-specific helper
functions that may handle common resource allocations (i.e. DMA, I/O Work items).
Refer to the pci_std sample to view how the reference code is used on some OS implementations. 

The core provides the following features:

   1. Host Controller Driver interface to the SDIO stack (request and configuration processing).
   2. Abstraction of interrupt handling, deferred work and card insertion/removal.
   3. OS-specific implementation of Direct Memory Access (DMA) (see pci_std reference).
   4. OS-specific management of multiple slot instances (see pci_std reference).



          

