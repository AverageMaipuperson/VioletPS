#pragma once
#include "../KittyMemory/KittyMemory/MemoryPatch.hpp"
#include "../KittyMemory/KittyMemory/KittyMemory.hpp"
// #include "../KittyMemory/KittyMemory/MemoryPatch.cpp"

class PatchManager {
private:
    std::vector<MemoryPatch> patches;
public:
    void addPatch(const char *libraryName, uintptr_t address,std::string hex){
		KittyMemory::ProcMap map = KittyMemory::getMaps(libraryName);
        patches.push_back(MemoryPatch::createWithHex(map,address,hex));
    }

    void Modify(){
        for(int k = 0; k < patches.size(); k++){
            patches[k].Modify();
        }
    }

    void Restore(){
        for(int k = 0; k < patches.size(); k++){
            patches[k].Restore();
        }
    }

};