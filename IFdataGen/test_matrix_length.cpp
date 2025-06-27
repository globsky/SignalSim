#include <iostream>
#include <cstring>
#include "src/BCNav1Bit.cpp"
int main() {
    std::cout << "B1CMatrixGen2 length: " << strlen(BCNav1Bit::B1CMatrixGen2) << std::endl;
    std::cout << "B1CMatrixGen3 length: " << strlen(BCNav1Bit::B1CMatrixGen3) << std::endl;
    std::cout << "Expected B1CMatrixGen2 length: " << (100*100) << std::endl;
    std::cout << "Expected B1CMatrixGen3 length: " << (44*44) << std::endl;
    return 0;
}
