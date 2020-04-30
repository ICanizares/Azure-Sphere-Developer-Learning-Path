# Lab 4: Integrating FreeRTOS with Azure Sphere Inter-Core Messaging

<!-- ![](resources/azure-sphere-iot-central-banner.png) -->

---

|Author|[Dave Glover](https://developer.microsoft.com/en-us/advocates/dave-glover?WT.mc_id=github-blog-dglover), Microsoft Cloud Developer Advocate, [@dglover](https://twitter.com/dglover) |
|:----|:---|
|Source Code | https://github.com/gloveboxes/Azure-Sphere-Learning-Path.git |
|Date| January  2020|

---

## Azure Sphere Learning Path

Each module assumes you have completed the previous module.

[Home](https://gloveboxes.github.io/Azure-Sphere-Learning-Path/)

* Lab 0: [Introduction Azure Sphere and Lab Set Up](https://github.com/gloveboxes/Azure-Sphere-Learning-Path/tree/master/Lab%200%20-%20Introduction%20and%20Lab%20Set%20Up)
* Lab 1: [Build your first Azure Sphere Application with Visual Studio](https://github.com/gloveboxes/Azure-Sphere-Learning-Path/tree/master/Lab%201%20-%20Build%20your%20first%20Azure%20Sphere%20Application%20with%20Visual%20Studio)
* Lab 2: [Send Telemetry from an Azure Sphere to Azure IoT Central](https://github.com/gloveboxes/Azure-Sphere-Learning-Path/tree/master/Lab%202%20-%20Send%20Telemetry%20from%20an%20Azure%20Sphere%20to%20Azure%20IoT%20Central)
* Lab 3: [Control an Azure Sphere with Device Twins and Direct Methods](https://github.com/gloveboxes/Azure-Sphere-Learning-Path/tree/master/Lab%203%20-%20Control%20an%20Azure%20Sphere%20with%20Device%20Twins%20and%20Direct%20Methods)
* Lab 4: [Integrating FreeRTOS with Azure Sphere Inter-Core Messaging](https://github.com/gloveboxes/Azure-Sphere-Learning-Path/tree/master/Lab%204%20-%20Integrating%20FreeRTOS%20with%20Azure%20Sphere%20Inter-Core%20Messaging)
* Lab 5: [Automating Azure Sphere Registration with Azure IoT Central](https://github.com/gloveboxes/Azure-Sphere-Learning-Path/tree/master/Lab%205%20-%20Automating%20Azure%20Sphere%20Registration%20with%20Azure%20IoT%20Central)

---

## Prerequisites

This lab assumes you have completed [Lab 2: Send Telemetry from an Azure Sphere to Azure IoT Central](https://github.com/gloveboxes/Azure-Sphere-Learning-Path/tree/master/Lab%202%20-%20Send%20Telemetry%20from%20an%20Azure%20Sphere%20to%20Azure%20IoT%20Central). You will have created an Azure IoT Central application, connected Azure IoT Central to your Azure Sphere Tenant and you have configured the **app_manifest.json** for the Azure Device Provisioning Service.

You will need to **copy** and **paste** the Lab 2 **app_manifest.json** you created to this labs **app_manifest.json** file.

---

## Problems Addressed in this Tutorial

1. Internet enabling an existing FreeRTOS application
2. End to end security for telemetry and device control
3. Streamlining device provisioning

## What you will learn

1. How to run a **FreeRTOS** Real-Time application on Azure Sphere and integrate with Azure IoT.
2. How to create an Azure IoT Central Application.
3. How to integrate an [Azure Sphere](https://azure.microsoft.com/services/azure-sphere/?WT.mc_id=github-blog-dglover) application with [Azure IoT Central](https://azure.microsoft.com/services/iot-central/?WT.mc_id=github-blog-dglover).
4. How to securely control an Azure Sphere with Azure IoT Central Device **[Settings](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-device-twins?WT.mc_id=github-blog-dglover)** and **[Commands](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-direct-methods?WT.mc_id=github-blog-dglover)**.

If unfamiliar with Azure Sphere development then review the [Create a Secure Azure Sphere App using the Grove Shield Sensor Kit](https://github.com/gloveboxes/Create-a-Secure-Azure-Sphere-App-using-the-Grove-Shield-Sensor-Kit) tutorial before starting this tutorial.

---

## Azure Sphere Solution Architecture

There are **two** applications deployed to the Azure Sphere.

1. The first application is a **High-Level** *Linux* application running on the **Cortex A7** core. It is responsible for sending temperature and humidity data to Azure IoT Central, processing Digital Twin and Direct Method messages from Azure IoT Central, and finally, passing on **inter-core** messages from the *FreeRTOS* application running on the Real-Time core to Azure IoT Central.
1. The second is a **Real-Time** *FreeRTOS* application running in the **Cortex M4**. It runs several FreeRTOS Tasks. The first task is to blink an LED, the second is to monitor for button presses, and the third is to send **inter-core** messages to the **High-Level** application whenever the button is pressed. **Note**, the FreeRTOS application running on the Real-Time core cannot connect directly to the network.

![](resources/azure-sphere-application-architecture.png)

---

## Tutorial Overview

1. Set up your development environment.
2. Deploy your first FreeRTOS Application to Azure Sphere.
3. Create an Azure IoT Central Application (Free trial).
4. Connect Azure IoT Central to your Azure Sphere Tenant.
5. Deploy an Azure IoT Central application to Azure Sphere.

---

## Ensure you have cloned the Azure Sphere Learning Path source code

```bash
git clone https://github.com/gloveboxes/Azure-Sphere-Learning-Path.git
```

---

## Open Lab 4

### Enable Real Time Core Debugging

Run the **Azure Sphere Developer Command Prompt** as **Administrator**.

Run the following command to install the required USB drivers to support real-time core debugging.

```bash
azsphere.exe device enable-development -r
```

### Step 1: Start Visual Studio 2019

![](resources/visual-studio-open-local-folder.png)

### Step 2: Open the lab project

1. Click **Open a local folder**
2. Navigate to the folder you cloned **Azure Sphere Learning Path** into.
3. Double click to open the **Lab 4 - Integrating FreeRTOS with Azure Sphere Inter-Core Messaging** folder
4. Double click to open the **azure-sphere-rtcore-freertos** folder
5. Click **Select Folder** button to open the project

---

## Deploy your first FreeRTOS Application to Azure Sphere

1. Set the startup configuration. Select the **ARM-Debug** configuration and the **GDB Debugger (RTCore)** startup item.

    ![](resources/azure-sphere-rtcore-startup-config.png)
3. Press <kbd>**F5**</kbd>, this will start the build, deploy, attach debugger process. The leftmost **blue LED** on the Azure Sphere will start **blinking**.
5. Press **Button A** on the Azure Sphere to change the blink rate. 
6. You can **Remote Debug** the FreeRTOS application running on Azure Sphere Cortex M4 Core. 
    1. From Visual Studio, open the FreeRTOS application **main.c** file.
    2. Set a [Visual Studio Breakpoint](https://docs.microsoft.com/en-us/visualstudio/debugger/using-breakpoints?view=vs-2019) in the **ButtonTask** function on the line that reads ```bool pressed = !newState;```.
    3. Press **Button A** on the Azure Sphere, Visual Studio will halt the execution of the FreeRTOS application and you can step through the code. Pretty darn neat!

### Understanding the Real-Time Core Security

Applications on the Azure Sphere are locked down by default. You need to grant capabilities to the application.

From Visual Studio open the **app_manifest.json** file.

**Observe**:

1. GPIO Capabilities: This application uses two GPIO pins. Pins 10, and 12.
2. Allowed Application Connections: This is the ID of the High-Level application that this application will be partnered with. It is required for inter-core communications.

```json
{
  "SchemaVersion": 1,
  "Name": "GPIO_RTApp_MT3620_BareMetal",
  "ComponentId": "6583cf17-d321-4d72-8283-0b7c5b56442b",
  "EntryPoint": "/bin/app",
  "CmdArgs": [],
  "Capabilities": {
    "Gpio": [ 10, 12 ],
    "AllowedApplicationConnections": [ "25025d2c-66da-4448-bae1-ac26fcdd3627" ]
  },
  "ApplicationType": "RealTimeCapable"
}

```

### Declaring the Partner Application

In the **launch.js.json** file, you need to declare the ID of the High-Level Application that this Real-Time application will be communicating with.

```json
{
    ...
    "configurations": [
        ...
        "partnerComponents": [ "25025d2c-66da-4448-bae1-ac26fcdd3627" ]
    ]
}
```

---

## Deploy the High Level Application

### Step 1: Open the High-Level Application with Visual Studio 2019

1. Start Visual Studio 2019, select **Open a local folder**, navigate to the Azure Sphere tutorial project folder you cloned from GitHub, then open the **azure-sphere-hlcore-iot-central** project.
2. Open the **app_manifest.json** file

### Step 2: Azure IoT Central Connection Information

If a lab Azure Sphere that you did not claim in to your tenant then **set the connection string** as per Lab 2.

If the Azure Sphere is a device you claimed into your own Azure Tenant the copy the contents of the **app_manifest.json** you created in *Lab 2* to this labs (*Lab 4*) **app_manifest.json** file.

### Step 3: Configure Inter-Core Communications

The Azure Sphere High-Level Application requires **inter-core** communications and this configured.

For this sample this has already been set to the Component ID of the Real-Time core application.

### Step 4:  Visual Studio App Deployment Settings

Before building the application with Visual Studio ensure ARM-Debug and GDB Debugger (HLCore) options are selected.

![](resources/visual-studio-start-config.png)

### Step 5: Build, Deploy and start Debugging

To start the build, deploy and debug process either click the Visual Studio **Start Selected Item** icon or press <kbd>**F5**</kbd>. To Build and deploy without attaching the debugger, press <kbd>**Ctrl+F5**</kbd>.

![](resources/visual-studio-start-debug.png)

---

## Test the Solution

Now you have both the FreeRTOS and the High-Level applications running there are things to test:

1. Press **Button A** on the Azure Sphere. The FreeRTOS application will change the blink rate of the leftmost blue LED on the Azure Sphere and will send an inter-core message to the High-Level application which in turn will toggled the relay pin.

2. Observer the **3rd LED from the left** on the Azure Sphere. It should blink green every 10 seconds as telemetry is sent to Azure IoT Central.

3. The **Visual Studio debugger** should still be connected to the High-Level application. Set a [Visual Studio Breakpoint](https://docs.microsoft.com/en-us/visualstudio/debugger/using-breakpoints?view=vs-2019) in the **InterCoreHandler** function in the **main.c** file. Press **Button A** on the Azure Sphere, the debugger will stop code execution at the breakpoint. You can now step through the code as well as display/change variables.

---

## Azure IoT Central Integration

Now the application is running on the Azure Sphere switch across to Azure IoT Central, select the **Devices** tab, the **Azure Sphere** template, then the device you created. You may have to wait a miniute or two before the telemetry is displayed in the **Overview** tab.

![](resources/iot-central-display-events.png)

---

## Finished 完了 fertig finito ख़त्म होना terminado

Congratulations you have finished lab 4.

![](resources/finished.jpg)

---

## Appendix

### Learn about Azure Sphere

1. [Azure Sphere Documentation](https://docs.microsoft.com/en-au/azure-sphere/)
1. Using Yocto to Build an IoT OS Targeting a Crossover SoC. [Video](https://www.youtube.com/watch?v=-T7Et5qfqQQ), and [Slides](https://static.sched.com/hosted_files/ossna19/91/Crossover_ELC2019.pdf)
2. [Anatomy of a secured MCU](https://azure.microsoft.com/en-au/blog/anatomy-of-a-secured-mcu/)
3. [Azure Sphere’s customized Linux-based OS](https://azure.microsoft.com/en-au/blog/azure-sphere-s-customized-linux-based-os/)
4. [Tech Communities Blog](https://techcommunity.microsoft.com/t5/internet-of-things/bg-p/IoTBlog)
5. The [Azure IoT Central Sample](https://github.com/Azure/azure-sphere-samples/blob/master/Samples/AzureIoT/IoTCentral.md)

