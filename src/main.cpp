#include <Controller.h>
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
    ifstream romFile;
	romFile.open("ROMS/snake.bin", ios::binary);
	if (!romFile) {
		cout << "File not opened!";
		exit(1);
	}
	else {
		cout << "File opened successfully!" << "\n";
	}
    Controller controller = Controller(romFile);
    controller.run();
}