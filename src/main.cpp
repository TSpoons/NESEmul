#include <MOS6502.h>
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
    ifstream romFile;
	romFile.open("../ROMS/nestest.nes", ios::binary);
	if (!romFile) {
		cout << "File not opened!";
	}
	else {
		// cout << "File opened successfully!" << "\n";
	}
    MOS6502 CPU = MOS6502();
    CPU.init(romFile);
	CPU.execute();
}