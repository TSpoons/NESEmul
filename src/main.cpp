#include <Controller.h>
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
    ifstream romFile;
	romFile.open("../ROMS/Super-Mario-Bros.nes", ios::binary);
	if (!romFile) {
		cout << "File not opened!";
	}
	else {
		// cout << "File opened successfully!" << "\n";
	}
    Controller controller = Controller(romFile);
    controller.run();
}