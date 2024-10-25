/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/

/********************************************************************
-usb_descriptors.c-
-------------------------------------------------------------------
Filling in the descriptor values in the usb_descriptors.c file:
-------------------------------------------------------------------

[Device Descriptors]
The device descriptor is defined as a USB_DEVICE_DESCRIPTOR type.
This type is defined in usb_ch9.h  Each entry into this structure
needs to be the correct length for the data type of the entry.

[Configuration Descriptors]
The configuration descriptor was changed in v2.x from a structure
to a uint8_t array.  Given that the configuration is now a byte array
each byte of multi-byte fields must be listed individually.  This
means that for fields like the total size of the configuration where
the field is a 16-bit value "64,0," is the correct entry for a
configuration that is only 64 bytes long and not "64," which is one
too few bytes.

The configuration attribute must always have the _DEFAULT
definition at the minimum. Additional options can be ORed
to the _DEFAULT attribute. Available options are _SELF and _RWU.
These definitions are defined in the usb_device.h file. The
_SELF tells the USB host that this device is self-powered. The
_RWU tells the USB host that this device supports Remote Wakeup.

[Endpoint Descriptors]
Like the configuration descriptor, the endpoint descriptors were
changed in v2.x of the stack from a structure to a uint8_t array.  As
endpoint descriptors also has a field that are multi-byte entities,
please be sure to specify both bytes of the field.  For example, for
the endpoint size an endpoint that is 64 bytes needs to have the size
defined as "64,0," instead of "64,"

Take the following example:
    // Endpoint Descriptor //
    0x07,                       //the size of this descriptor //
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP02_IN,                   //EndpointAddress
    _INT,                       //Attributes
    0x08,0x00,                  //size (note: 2 bytes)
    0x02,                       //Interval

The first two parameters are self-explanatory. They specify the
length of this endpoint descriptor (7) and the descriptor type.
The next parameter identifies the endpoint, the definitions are
defined in usb_device.h and has the following naming
convention:
_EP<##>_<dir>
where ## is the endpoint number and dir is the direction of
transfer. The dir has the value of either 'OUT' or 'IN'.
The next parameter identifies the type of the endpoint. Available
options are _BULK, _INT, _ISO, and _CTRL. The _CTRL is not
typically used because the default control transfer endpoint is
not defined in the USB descriptors. When _ISO option is used,
addition options can be ORed to _ISO. Example:
_ISO|_AD|_FE
This describes the endpoint as an isochronous pipe with adaptive
and feedback attributes. See usb_device.h and the USB
specification for details. The next parameter defines the size of
the endpoint. The last parameter in the polling interval.

-------------------------------------------------------------------
Adding a USB String
-------------------------------------------------------------------
A string descriptor array should have the following format:

rom struct{byte bLength;byte bDscType;word string[size];}sdxxx={
sizeof(sdxxx),DSC_STR,<text>};

The above structure provides a means for the C compiler to
calculate the length of string descriptor sdxxx, where xxx is the
index number. The first two bytes of the descriptor are descriptor
length and type. The rest <text> are string texts which must be
in the unicode format. The unicode format is achieved by declaring
each character as a word type. The whole text string is declared
as a word array with the number of characters equals to <size>.
<size> has to be manually counted and entered into the array
declaration. Let's study this through an example:
if the string is "USB" , then the string descriptor should be:
(Using index 02)
rom struct{byte bLength;byte bDscType;word string[3];}sd002={
sizeof(sd002),DSC_STR,'U','S','B'};

A USB project may have multiple strings and the firmware supports
the management of multiple strings through a look-up table.
The look-up table is defined as:
rom const unsigned char *rom USB_SD_Ptr[]={&sd000,&sd001,&sd002};

The above declaration has 3 strings, sd000, sd001, and sd002.
Strings can be removed or added. sd000 is a specialized string
descriptor. It defines the language code, usually this is
US English (0x0409). The index of the string must match the index
position of the USB_SD_Ptr array, &sd000 must be in position
USB_SD_Ptr[0], &sd001 must be in position USB_SD_Ptr[1] and so on.
The look-up table USB_SD_Ptr is used by the get string handler
function.

-------------------------------------------------------------------

The look-up table scheme also applies to the configuration
descriptor. A USB device may have multiple configuration
descriptors, i.e. CFG01, CFG02, etc. To add a configuration
descriptor, user must implement a structure similar to CFG01.
The next step is to add the configuration descriptor name, i.e.
cfg01, cfg02,.., to the look-up table USB_CD_Ptr. USB_CD_Ptr[0]
is a dummy place holder since configuration 0 is the un-configured
state according to the definition in the USB specification.

********************************************************************/

/*********************************************************************
 * Descriptor specific type definitions are defined in:
 * usb_device.h
 *
 * Configuration options are defined in:
 * usb_config.h
 ********************************************************************/

#ifndef __USB_DESCRIPTORS_C
#define __USB_DESCRIPTORS_C

/** INCLUDES *******************************************************/
#include "usb.h"
#include "usb_device_hid.h"

/** CONSTANTS ******************************************************/
#if defined(__18CXX)
#pragma romdata
#endif

/* Device Descriptor */
const USB_DEVICE_DESCRIPTOR device_dsc=
{
    0x12,    				// Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,  // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    0x00,                   // Class Code
    0x00,                   // Subclass code
    0x00,                   // Protocol code
    USB_EP0_BUFF_SIZE,      // Max packet size for EP0, see usb_config.h
    MY_VID,                 // Vendor ID
    MY_PID,                 // Product ID: Mouse in a circle fw demo
    0x0001,                 // Device release number in BCD format
    0x01,                   // Manufacturer string index
    0x02,                   // Product string index
    0x00,                   // Device serial number string index
    0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
const uint8_t configDescriptor1[]={
    /* Configuration Descriptor */
    0x09,//sizeof(USB_CFG_DSC),    // Size of this descriptor in bytes
    USB_DESCRIPTOR_CONFIGURATION,  // CONFIGURATION descriptor type
    DESC_CONFIG_WORD(0x0022),   // Total length of data for this cfg
    1,                      // Number of interfaces in this cfg
    1,                      // Index value of this configuration
    0,                      // Configuration string index
    _DEFAULT | _SELF | _RWU,// Attributes, see usb_device.h
    50,                     // Max power consumption (2X mA)

    /* Interface Descriptor */
    0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,      // INTERFACE descriptor type
    0,                      // Interface Number
    0,                      // Alternate Setting Number
    1,                      // Number of endpoints in this intf
    HID_INTF,               // Class code
    0,					    // Subclass code
    0,    // Protocol code
    0,                      // Interface string index

    /* HID Class-Specific Descriptor */
    0x09,//sizeof(USB_HID_DSC)+3,    // Size of this descriptor in bytes
    DSC_HID,                // HID descriptor type
    DESC_CONFIG_WORD(0x0111),    	// HID Spec Release Number in BCD format (1.11)
    0x00,                   // Country Code (0x00 for Not supported)
    HID_NUM_OF_DSC,         // Number of class descriptors, see usbcfg.h
    DSC_RPT,                // Report descriptor type
    DESC_CONFIG_WORD(HID_RPT01_SIZE),   //sizeof(hid_rpt01),      // Size of the report descriptor

    /* Endpoint Descriptor */
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    HID_EP | _EP_IN,            //EndpointAddress
    _INTERRUPT,                 //Attributes
    DESC_CONFIG_WORD(64),       //size
    0x04                        //Interval in ms.  4ms = up to 250Hz update rate.
};


//Language code string descriptor
const struct{uint8_t bLength;uint8_t bDscType;uint16_t string[1];}sd000={
sizeof(sd000),USB_DESCRIPTOR_STRING,{0x0409
}};

//Manufacturer string descriptor
const struct{uint8_t bLength;uint8_t bDscType;uint16_t string[25];}sd001={
sizeof(sd001),USB_DESCRIPTOR_STRING,
{'M','i','c','r','o','c','h','i','p',' ',
'T','e','c','h','n','o','l','o','g','y',' ','I','n','c','.'
}};

//Product string descriptor
const struct{uint8_t bLength;uint8_t bDscType;uint16_t string[16];}sd002={
sizeof(sd002),USB_DESCRIPTOR_STRING,
{'H','I','D',' ','-',' ','T','o','u','c','h',' ','D','e','m','o'
}};

//Class specific descriptor - HID report descriptor:
//At a minimum a pen tablet digitizer or single touch finger input digitizer
//must support the following report information:
//X coordinate of contact point
//Y coordinate of contact point
//"Tip Switch" (ex: a button on the end of a pen, to be used for performing "left click" mouse like operations)
//"In-range" indicator: For example, if a magnetic input tablet is used, and a small
//		magnet is placed near the tip of the pen, the tablet could detect when the pen is
//		hovering over a location on the tablet, without requiring the user to depress the
//		tip switch in that location.
//Other input usages are optional.

//Note: In a real application, at a minimum, certain terms in the report descriptor
//(ex: PHYSICAL_MAXIMUM and LOGICAL_MAXIMUM for the X and Y coordinates) will need
//to be modified to match the characteristics of the actual application being
//developed.  See the HID1_11.pdf specifications regarding these terms.

const struct{uint8_t report[HID_RPT01_SIZE];}hid_rpt01={
	{
    0x05, 0x0d,             // USAGE_PAGE (Digitizers)
    0x09, 0x02,             // USAGE (Pen)
    0xa1, 0x01,             // COLLECTION (Application)
    0x85, 0x01, 			//   REPORT_ID (Pen)                	//To match this descriptor, byte[0] of IN packet should be = 0x01 always for this demo
    0x09, 0x20,             //   USAGE (Stylus)
    0xa1, 0x00,             //   COLLECTION (Physical)
    0x09, 0x42, 			//     USAGE (Tip Switch)           	//(byte[1] bit 0)
    0x09, 0x44, 			//     USAGE (Barrel Switch)        	//(byte[1] bit 1)
    0x09, 0x45, 			//     USAGE (Eraser Switch)        	//(byte[1] bit 2)
    0x09, 0x3c, 			//     USAGE (Invert)               	//(byte[1] bit 3)
    0x09, 0x32, 			//     USAGE (In Range)             	//(byte[1] bit 4)
    0x15, 0x00,             //     LOGICAL_MINIMUM (0)
    0x25, 0x01,             //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,             //     REPORT_SIZE (1)
    0x95, 0x05,             //     REPORT_COUNT (5)
    0x81, 0x02, 			//     INPUT (Data,Var,Abs)         	//Makes five, 1-bit IN packet fields (byte[1] bits 0-4)) for (USAGE) tip sw, barrel sw, invert sw, in range sw.  Send '1' here when switch is active.  Send '0' when switch not active.
    0x95, 0x0b, 			//     REPORT_COUNT (11)
    0x81, 0x03, 			//     INPUT (Cnst,Var,Abs)         	//Makes eleven, 1-bit IN packet fields (byte[1] bits 5-7, and byte[2] all bits) with no usage.  These are pad bits that don't contain useful data.
    0x05, 0x01,             //     USAGE_PAGE (Generic Desktop)
    0x26, 0xff, 0x7f,       //     LOGICAL_MAXIMUM (32767)
    0x75, 0x10,             //     REPORT_SIZE (16)
    0x95, 0x01,             //     REPORT_COUNT (1)
    0xa4,                   //     PUSH
    0x55, 0x0d,             //     UNIT_EXPONENT (-3)
    0x65, 0x33,             //     UNIT (Inch,EngLinear)        	//(10^-3 inches = 1/1000 of an inch = 1 mil)
    0x09, 0x30,  			//     USAGE (X)                    	//(byte[3] and byte[4])
    0x35, 0x00,             //     PHYSICAL_MINIMUM (0)
    0x46, 0x00, 0x00,       //     PHYSICAL_MAXIMUM (0)
    0x81, 0x02,  			//     INPUT (Data,Var,Abs)         	//Makes one, 16-bit IN packet field used for (USAGE) X-coordinate input info.  Valid values 0 to 32767.
    0x09, 0x31,  			//     USAGE (Y)                    	//(byte[5] and byte[6])
    0x46, 0x00, 0x00,  		//     PHYSICAL_MAXIMUM (0)
    0x81, 0x02,  			//     INPUT (Data,Var,Abs)         	//Makes one, 16-bit IN packet field used for (USAGE) Y-coordinate input info.  Valid values 0 to 32767.
    0xb4,                   //     POP
    0x05, 0x0d,             //     USAGE_PAGE (Digitizers)
    0x09, 0x30,  			//     USAGE (Tip Pressure)         	//(byte[7] and byte[8])
    0x81, 0x02,  			//     INPUT (Data,Var,Abs)         	//Makes one, 16-bit IN packet field used for (USAGE) tip pressure input info.  Valid values 0 to 32767
    0x09, 0x3d,  			//     USAGE (X Tilt)               	//(byte[9] and byte[10])
    0x09, 0x3e,  			//     USAGE (Y Tilt)               	//(byte[11] and byte[12])
    0x16, 0x01, 0x80,       //     LOGICAL_MINIMUM (-32767)
    0x95, 0x02,             //     REPORT_COUNT (2)
    0x81, 0x02,   			//     INPUT (Data,Var,Abs)         	//Makes two, 16-bit IN packet fields, used for (USAGE) X-Tilt and Y-Tilt. Valid values -32767 to 32767
    0xc0,                   //   END_COLLECTION
    0xc0                    // END_COLLECTION

//Based on the above report descriptor, HID input packets
//sent to the host on EP1 IN should be formatted as follows:

//byte[0] = 0x01 (Report ID, for this application, always = 0x01)
//byte[1] = contains bit fields for various input information typically generated by an input pen. '1' is the active value (ex: pressed), '0' is the non active value
//		bit0 = Tip switch. At the end of a pen input device would normally be a pressure senstive switch.  Asserting this performs an operation analogous to a "left click" on a mouse
//		bit1 = Barrel switch.
//		bit2 = Erasure switch.
//		bit3 = Invert
//		bit4 = In range indicator.
//		bit5 though bit 7 = Pad bits.  Values not used for anything.
//byte[2] = Pad byte.  Value not used for anything.
//byte[3] = X coordinate LSB value of contact point
//byte[4] = X coordinate MSB value of contact point
//byte[5] = Y coordinate LSB value of contact point
//byte[6] = Y coordinate MSB value of contact point
//byte[7] = Tip pressure LSB.  If the tip switch implemented (or finger contact surface) has analog sensing capability to detect relative amount of pressure, this can be used to indicate to the host how hard is the contact.
//byte[8] = Tip pressure MSB.
//byte[9] = X tilt LSB
//byte[10]= X tilt MSB
//byte[11]= Y tilt LSB
//byte[12]= Y tilt MSB

	}
};/* End Collection,End Collection            */

//Array of configuration descriptors
const uint8_t *const USB_CD_Ptr[]=
{
    (const uint8_t *const)&configDescriptor1
};

//Array of string descriptors
const uint8_t *const USB_SD_Ptr[]=
{
    (const uint8_t *const)&sd000,
    (const uint8_t *const)&sd001,
    (const uint8_t *const)&sd002
};

/** EOF usb_descriptors.c ***************************************************/

#endif
