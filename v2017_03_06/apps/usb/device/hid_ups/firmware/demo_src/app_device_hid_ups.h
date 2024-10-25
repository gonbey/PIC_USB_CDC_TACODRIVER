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

/*********************************************************************
* Function: void APP_DeviceHIDUPSInitialize(void);
*
* Overview: Initializes the demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceHIDUPSInitialize();

/*********************************************************************
* Function: void APP_DeviceHIDUPSStart(void);
*
* Overview: Starts running the demo.
*
* PreCondition: The device should be configured into the configuration
*   that contains the custome HID interface.  The APP_DeviceHIDUPSInitialize()
*   function should also have been called before calling this function.
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceHIDUPSStart();

/*********************************************************************
* Function: void APP_DeviceHIDUPSTasks(void);
*
* Overview: Keeps the demo running.
*
* PreCondition: The demo should have been initialized and started via
*   the APP_DeviceHIDUPSInitialize() and APP_DeviceHIDUPSStart() demos
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceHIDUPSTasks();

/*********************************************************************
* Function: void APP_DeviceHIDUPSSOFHandler(void);
*
* Overview: Handles SOF based events for this demo
*
* PreCondition: The demo should have been initialized and started via
*   the APP_DeviceHIDUPSInitialize() and APP_DeviceHIDUPSStart() demos
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceHIDUPSSOFHandler();
