<a name="HOLTop" />
# Creating a new ANGLE project using a Visual Studio Template #

---

<a name="Exercise1" />
### Exercise 1: Creating a new ANGLE project using a Visual Studio Template ###

In this exercise, you will use Visual Studio 2015 to create a new ANGLE project using the installed ANGLE project templates. You will also build and run the new project.

#### Updating your Win32 Project Properties ###

1. Right-click on your Visual Studio 2015 project and select **Properties**.

	![Project Properties](Images/using.png?raw=true "Project Properties")

	_Project Properties_

1. Select the **XAML App for OpenGL ES (Windows Universal)** template from **Visual C++ | Windows | Universal** to start a new solution.

	![Selecting the ANGLE XAML template](../../Images/ex1-new-angle-project.PNG?raw=true "Selecting the ANGLE XAML template")

	_Selecting the ANGLE XAML template_

1. Name the solution **Breakout** and save it in the **CodeLabs/Workshops/Games/Module3-ANGLE/Source/Ex1/Begin** folder. Click on **OK** to create the solution.

1. Select **Debug x64** from the Project Configuration and Platform dropdowns.

	![Configuring the build target](../../Images/ex2-debug-x64.PNG?raw=true "Configuring the build target")

	_Configuring the build target_

1. Press the **F5** key to build and run your app. After the build completes, your app should look like the following.

	![Sample ANGLE app](../../Images/ex1-sample-angle-app.PNG?raw=true "Sample ANGLE app")

	_Sample ANGLE app_

Congratulations. You have created your first Windows 10 UWP ANGLE app.

#### Next ####

- Continue on to [Exercise 2: Integrating your game code with ANGLE](../../Source/Ex2/README.md)
- Return to [Start](../../README.md)