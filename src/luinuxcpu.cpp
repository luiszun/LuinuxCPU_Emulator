#include "processor.h"

using NVMem = NVMemory<uint16_t>;

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: luinuxcpu <program_binary_file> <nvram_file> " << std::endl;
        return -1;
    }
    try
    {

        NVMem programMemory(0x10000, std::string(argv[1]));
        std::shared_ptr<NVMem> nvram = std::make_shared<NVMem>(0x10000, std::string(argv[2]));

        Processor cpu(programMemory, nvram);
        cpu.ExecuteAll();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }

    return 0;
}