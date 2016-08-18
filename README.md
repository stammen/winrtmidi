# Adding the WinRTMidi DLL to your Win32 Project #

---
### Requirements ###

1. Visual Studio 2015 (Update 3 recommended) with **Universal Windows App Development Tools and Windows 10 Tools and SDKs** [installed](https://msdn.microsoft.com/en-us/library/e2h7fzkw.aspx)


### Updating your Win32 Project Properties ###

1. Right-click on your Visual Studio 2015 application project and select **Properties**.

	![Project Properties](Images/properties.png?raw=true "Project Properties")

	_Project Properties_

1. Select the **General** Property. Set the **Target Platform Version** property to 10.0.10586.0:

	![Required General Properties](Images/version.png?raw=true "Required General Properties")

	_Required General Properties_
	
1. Select the **C++ | General** Property. Add the following paths to the **Additional #using Directories** property:

	C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcpackages;C:\Program Files (x86)\Windows Kits\10\UnionMetadata

	You will also need to enable the **Consume Windows Runtime Extension (Yes/ZW)** property.

	![Required C++ General Properties](Images/using.png?raw=true "Required C++ General Properties")

	_Required C++ General Properties_

1. Select the **C++ | Code Generation** Property. Disable the **Enable Minimal Rebuild** property.

	![Required C++ Code Generation Properties](Images/gm.png?raw=true "Required C++ Code Generation Properties")

	_Required C++ Code Generation Properties_
	
### Updating your Win32 main() function ###

Note: This is still under investigation for running the app on Windows 7.

You have several options. We are still investigating which is the best option.

1. Change your main() function to:

    ````C++
	int main(Platform::Array<Platform::String^>^ args) 
    ````

1. Decorate your main() function with [MTAThread]

    ````C++
	#include <wrl\wrappers\corewrappers.h>
	
	using namespace Platform;
	using namespace Microsoft::WRL::Wrappers;

	[MTAThread]
	int main()
	{
		// Initialize the Windows Runtime.
		RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
	````