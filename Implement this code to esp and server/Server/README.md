# How to Add the Server Fix Files to Your SlimeVR Project

This guide will help you place and integrate the `data_processor` fix into your SlimeVR Server project. No advanced coding experience is needed.

---

## 📁 Step 1: Locate Your SlimeVR Server Project Folder

Find the folder where all your SlimeVR server-side code is stored.  
It might be named something like:

```
SlimeVR-Server
```

---

## 📂 Step 2: Create a Folder Named `Server`

Inside your main project folder, create a new folder named:

```
Server
```

This keeps the new files organized and separate from other parts of the project.

> **Example Path:**  
> If your project is located at:
> ```
> C:\Projects\SlimeVR-Server
> ```
> You should create:
> ```
> C:\Projects\SlimeVR-Server\Server
> ```

---

## 📄 Step 3: Add the Provided Files

Inside the new `Server` folder, create and save the following four files:

- `data_processor.h`
- `data_processor.cpp`
- `changelog.md`
- `README.md`

You can copy the content of each file from the provided source and paste it into a new file using a text editor such as **Notepad** or **Visual Studio Code**.

---

## 🛠 Step 4: Integrate the Fix into Your Server Code

The purpose of these files is to **correct the sensor's acceleration axes** before they're used in orientation calculations.

In your server’s code, locate the section where sensor data is received (usually as raw acceleration data).  
Before using that data in calculations or sending it to the client, call this function:

```cpp
DataProcessor::correctAcceleration(rawAccelData);
```

If you're not sure where this goes:
- Ask a developer or maintainer for help.
- Refer to the documentation for where IMU data is processed.

This step ensures the axes are correctly aligned before any further processing.

---

## 🧪 Step 5: Build or Compile the Server

Once the files are added and the correction function is called in your code:

- Rebuild or compile the SlimeVR Server project.
- You can usually do this using a "Build" button in your IDE (e.g., Visual Studio), or by running a build command if using command-line tools.

---

✅ That’s it!  
The server now has a proper axis correction step, improving accuracy for sensor fusion.

Need help? Reach out to your team or post on the [SlimeVR GitHub Discussions](https://github.com/SlimeVR/SlimeVR-Server/discussions).
