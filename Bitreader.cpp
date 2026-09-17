// Bitreader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>0
#include <fstream>
#include <bitset> // Required for std::bitset

int main()
{
    std::ifstream file("test.txt", std::ios::binary);
    if (!file) return 1;

    char byte;
    while (file.get(byte)) {
        // Convert the byte into an 8-bit array-like structure
        std::bitset<8> bits(byte);

        // You can print the whole byte in binary easily:
        std::cout << "Byte in binary: " << bits << std::endl;

        // Or you can access individual bits like an array.
        // NOTE: bits[0] is the Least Significant Bit (the rightmost bit).
        // bits[7] is the Most Significant Bit (the leftmost bit).
        //std::cout << "Bit 7 (MSB): " << bits[7] << std::endl;
        //std::cout << "Bit 0 (LSB): " << bits[0] << std::endl;
        for (int bit_index = 7; bit_index >= 0; bit_index--)
        {
            std::cout << bits[bit_index];
        }
        std::cout << std::endl;
    }

    std::cin.get();
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
