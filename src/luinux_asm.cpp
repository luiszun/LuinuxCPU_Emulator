#include "assembler.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: luinuxasm <input_file> <output_file>";
        return -1;
    }

    try
    {
        Assembler asmObj(std::string{argv[1]}, std::string{argv[2]});
        auto binProgram = asmObj.AssembleFile();
        asmObj.WriteBinaryFile(binProgram);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}