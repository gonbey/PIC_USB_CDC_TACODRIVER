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
* Function: void APP_DeviceHIDDigitizerInitialize(void);
*
* Overview: Initializes the Custom HID demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceHIDDigitizerInitialize();


/*********************************************************************
* Function: void APP_DeviceHIDDigitizerTasks(void);
*
* Overview: Keeps the Custom HID demo running.
*
* PreCondition: The demo should have been initialized via
*   the APP_DeviceHIDDigitizerInitialize() function
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceHIDDigitizerTasks();

/*********************************************************************
* Function: void APP_DeviceHIDDigitizerSOFHandler(void);
*
* Overview: Handles SOF based events for this demo
*
* PreCondition: The demo should have been initialized and started via
*   the APP_DeviceHIDDigitizerInitialize() and APP_DeviceHIDDigitizerStart() demos
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceHIDDigitizerSOFHandler();
