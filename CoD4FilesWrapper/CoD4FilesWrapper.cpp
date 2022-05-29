// CoD4FilesWrapper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <filesystem>
#include <vector>
#include <map>
#include <regex>
#include <AtlBase.h>
#include <atlconv.h>
#include <typeinfo>
#include <locale>
#include <codecvt>
#include <Windows.h>


///////// < Global Variables //////////

int nCodePage = GetACP();

bool brushFlag = false;
bool entityFlag = false;
bool modelFlag = false;
bool prefabFlag = false;
bool specialFlag = false;

bool xmodelFlag = false;
bool materialFlag = false;

std::string flagName = "";

std::vector<std::string> textures;
std::map<std::string, bool> texturesMap;

std::vector<std::string> entities;
std::map<std::string, bool> entitiesMap;

std::vector<std::string> models;
std::map<std::string, bool> modelsMap;

std::map<std::string, std::string> customXmodelsMap;
std::map<std::string, std::string> customMaterialsMap;

std::map<std::string, std::string> convertcacheIwiMap;
std::map<std::string, std::vector<std::string>> duplicateTextureMaterialsArray;

std::map<std::string, bool> stockTexturesMap;
std::map<std::string, bool> stockSourceDataFilesMap;
std::map<std::string, bool> xmodelStockPathFilesMap;
std::map<std::string, bool> fxFilesMap;

std::vector<std::string> errLog;

std::vector<std::string> customTextures;
std::map<std::string, bool> customTexturesMap;

std::map<std::filesystem::path, std::map<std::string, std::filesystem::path>> specialCustomTexturesMap;

std::string cod4RootDirectory;
int cod4RootDirectoryLength;
std::map<std::string, std::map<std::string, std::filesystem::path>> colSpecNormMap;

std::filesystem::path mapFile;

std::map<std::filesystem::path, std::map<std::filesystem::path, std::string>> textureFilePath;

std::vector<std::filesystem::path> gdtFilePath;

std::vector<std::string> allExtensions = { ".jpg", ".dds", ".tga", ".jpeg" };

std::map <std::string, int> _CSVprepareSwitchCase{ {"fx", 1}, {"rawfile", 2}, {"material", 3}, {"xmodel", 4}};

//1 == StockTextures, 2 == stockSourceFiles, 3 == xmodelStockFiles,, 4 == fxFiles
std::map<std::string, int> startFilesMap{ {"--iwi--", 1}, {"--gdt--", 2}, {"--xmodel--", 3}, {"--fx--", 4} };
std::vector<std::string> specularTexturesArray;
std::map<std::string, int> materialMapTypes{ {"colorMap",1},{"detailMap",2},{"normalMap",3},{"specularMap",4} };
std::map<std::string, std::string> textureMapTypeMap;
std::vector<std::string> textureMapTypeFileArray;

std::map<std::string, bool> xmodelPartsMap{ {"raw/xmodelsurfs/", true}, {"raw/xmodel/", false}, {"raw/xmodelparts/", true},
    {"assettgtcache/raw/raw/xmodel/", false}, {"assettgtcache/raw/raw/xmodelparts/", true}, {"assettgtcache/raw/raw/xmodelsurfs/", true} };

std::map < int, int > includeCharactersFromTo{ {32,57}, {65,90}, {95,122}, {126,126}};
std::vector<int> includeCharactersArray;

std::map <std::string, std::string> textureMapTypeFromMaterialMap;

//////////////////////////////////////////////////
//          All Files Global Vars               //
//////////////////////////////////////////////////
std::map<std::string, bool> allMapFiles;        //
std::map<std::string, bool> allCSVFiles;        //
std::map<std::string, bool> allFXFiles;         //
std::map<std::string, bool> allGDTFiles;        //
std::map<std::string, bool> allMaterialFiles;   //
std::map<std::string, bool> allGscFiles;        //
std::map<std::string, bool> allExtraFiles;      //
int allTextureAndGdtFilePathsMaxLength;         //
//////////////////////////////////////////////////

//////////////////////////////////////////////////
//          All Optional Global Vars            //
//////////////////////////////////////////////////
std::map<std::string, bool> imageFiles;         //
std::map<std::string, bool> materialFiles;      //
std::map<std::string, bool> materialPropsFiles; //
std::map<std::string, bool> xmodelFiles;        //
std::map<std::string, bool> iwiFiles;           //
int optionalFilePathsMaxLength;                 //
//////////////////////////////////////////////////





///////// Global Variables /> /////////


///////// < Functions/////////
void assignCharactersArray() {
    for (auto& [from, to] : includeCharactersFromTo) {
        for (int i = from; i < to + 1; i++) {
            includeCharactersArray.push_back(i);
        }
    }
}

bool add2Map(std::map<std::string, bool>& addToMap, std::string& addString, bool addValue = true, bool isOptional = false) {
    if (!addToMap.contains(addString)) {
        addToMap[addString] = addValue;
        if (addString.length() > allTextureAndGdtFilePathsMaxLength && !isOptional)
            allTextureAndGdtFilePathsMaxLength = addString.length();
        if (addString.length() > optionalFilePathsMaxLength && isOptional) optionalFilePathsMaxLength = addString.length();
        return true;
    }
    return false;
}

bool add2AllFiles(std::string addToGlobalMap , std::string addString, bool addValue = true) {
    std::map<std::string, int> prepare4SwitchCase{ {"map", 1}, {"csv", 2}, {"fx", 3}, {"gdt", 4}, {"material", 5}, {"gsc", 6}, {"extra", 7}};
    switch (prepare4SwitchCase[addToGlobalMap])
    {
    case 1:
        add2Map(allMapFiles, addString, &addValue);
        break;
    case 2:
        add2Map(allCSVFiles, addString, &addValue);
        break;
    case 3:
        add2Map(allFXFiles, addString, &addValue);
        break;
    case 4:
        add2Map(allGDTFiles, addString, &addValue);
        break;
    case 5:
        add2Map(allMaterialFiles, addString, &addValue);
        break;
    case 6:
        add2Map(allGscFiles, addString, &addValue);
        break;
    case 7:
        add2Map(allExtraFiles, addString, &addValue);
        break;

    }
    return false;
}


bool add2OptionalFiles(std::string addToGlobalMap, std::string addString, bool addValue = true) {
    std::map<std::string, int> prepare4SwitchCase{ {"image", 1}, {"materials", 2},{"material-props", 3}, {"xmodel", 4}, {"iwi-files", 5}};
    switch (prepare4SwitchCase[addToGlobalMap])
    {
    case 1:
        add2Map(imageFiles, addString, &addValue, true);
        break;
    case 2:
        add2Map(materialFiles, addString, &addValue, true);
        break;
    case 3:
        add2Map(materialPropsFiles, addString, &addValue, true);
        break;
    case 4:
        add2Map(xmodelFiles, addString, &addValue, true);
        break;
    case 5:
        add2Map(iwiFiles, addString, &addValue, true);
        break;
    }

    return false;
}


inline void trim(std::string& str)
{
    str.erase(str.find_last_not_of(' ') + 1);           //suffixing spaces
    str.erase(0, str.find_first_not_of(' '));           //prefixing spaces
}

void reassignValues()
{
    brushFlag = false;
    entityFlag = false;
    modelFlag = false;
    prefabFlag = false;
    specialFlag = false;
    flagName = "";
}

namespace String
{
    bool str_contains(std::string haystack, std::string needle)
    {
        return haystack.find(needle) != std::string::npos;
    }
    //const std::string& value, const std::vector<std::string>& array
    bool in_array(const std::string& value, const std::vector<std::string>& array)
    {
        return std::find(array.begin(), array.end(), value) != array.end();
    }

    std::string lowerCase(std::string str) {
        std::string output = "";
        for (auto& c : str) { output += tolower(c); }
        return output;
    }

    std::string swiftRegex(std::string examineString, std::string regexString, int resultNumber) {
        std::smatch matchResults;
        try
        {
            std::regex modelRegex(regexString, std::regex_constants::icase);
            std::regex_search(examineString, matchResults, modelRegex);
        }
        catch (std::exception& e)
        {
            std::cout << "\n" << e.what() << std::endl;
        }
       
        return matchResults.str(resultNumber);



    }
    std::string replace(std::string str, std::string substr1, std::string substr2)
    {
        for (size_t index = str.find(substr1, 0); index != std::string::npos && substr1.length(); index = str.find(substr1, index + substr2.length()))
            str.replace(index, substr1.length(), substr2);
        return str;
    }

    std::vector<std::string> split(std::string strLine){
        std::istringstream iss(strLine);
        std::vector<std::string> result;
        for (std::string s; iss >> s;)
            result.push_back(s);
        return result;
    }
}

namespace Path {
    std::filesystem::path array2Path(std::vector<std::filesystem::path> myArray, int from, int to, std::string separator) {
        //if (to>myArray.size() || from>myArray.size() || to<0 || from<0) return "";
        std::string myString;
        for (int i = from; i < to; i++) {
            if (i == to - 1) {
                myString += myArray[i].string();
            }
            else {
                myString += myArray[i].string() + separator;
            }
        }
        std::filesystem::path returnPath(myString);
        return returnPath;
    }
}

namespace Map {
    std::map<std::string, bool> array2map(std::vector<std::string> myVector) {
        std::map<std::string, bool> returnArray;
        for (auto& element : myVector) {
            returnArray[element] = true;
        }
        return returnArray;
    }
}

void addTextures(std::string textureName) {
    textureName = String::lowerCase(textureName);
    if (!texturesMap.contains(textureName)) {
        texturesMap[textureName] = true;
        textures.push_back(textureName);
    }
}

void addEntities(std::string entityName) {
    if (String::str_contains(entityName, ".map")) {
        if (!entitiesMap.contains(entityName)) {
            entitiesMap[entityName] = true;
            entities.push_back(entityName);
            return;
        }
    }
    else {
        if (!modelsMap.contains(entityName)) {
            modelsMap[entityName] = true;
            models.push_back(entityName);
            return;
        }
    }
}


void brushAssignTextures(std::string line, bool resetValue = false) {
    if (String::str_contains(line, "curve") || String::str_contains(line, "mesh")) {
        specialFlag = true;
        return;
    }

    if (specialFlag) {
        addTextures(line);
        specialFlag = false;
        brushFlag = false;
        return;
    }
    else {
        if (!String::str_contains(line, "(")) return;
        std::string textureName = String::swiftRegex(line, "\\s*([(].*[)])\\s(\\w*)", 2);
        addTextures(textureName);
    }

    if (line == "}") {
        //if (resetValue) reassignValues(); else brushFlag = false;
        brushFlag = false;
        specialFlag = false;
        return;
    }
}

void entityAssign(std::string line) {
    if (line == "}" && !brushFlag) {
        reassignValues();
        return;
    }
    if (String::str_contains(line, "brush")) {
        brushFlag = true;
        return;
    }

    if (brushFlag) {
        brushAssignTextures(line, false);
        return;
    }

    if (String::str_contains(line, "\"model\"")) {
        std::string entityName = String::swiftRegex(line, "\\W*(\\w*)\\W*(.*)\"\\W*", 2);
        addEntities(entityName);
        return;
    };

}

void readMapFile(std::filesystem::path filePath) {
    add2AllFiles("map", filePath.make_preferred().string());
    std::ifstream file(filePath);
    std::cout << "Reading " << filePath.string();
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            trim(line);
            if ((line.length() <= 2 && line != "}") || String::str_contains(line, "toolFlags") || String::str_contains(line, "content")) continue;
            if (String::str_contains(line, "brush")) {
                brushFlag = true;
                continue;
            }
            if (String::str_contains(line, "entity") && !String::str_contains(line, "entity 0")) {
                reassignValues();
                entityFlag = true;
                continue;
            }
            if (brushFlag && !entityFlag) {
                brushAssignTextures(line, true);
            }
            if (entityFlag) {
                entityAssign(line);
            }
        }
        std::cout << " DONE" << std::endl;
        file.close();
        reassignValues();

    }
}

void readGDT(std::string gdtString) {
    std::filesystem::path gdtPath(gdtString);
    std::cout << "Reading " << gdtPath.make_preferred().string();
    std::ifstream file(gdtPath);
    std::filesystem::path colorMapName;
    if (file.is_open()) {
        std::string line;
        std::map<std::string, std::filesystem::path> tempMap;
        std::map<std::filesystem::path, std::string> tempGdtMap;
        while (std::getline(file, line)) {
            trim(line);
            if (line.length() <= 5 && !String::str_contains(line, "}")) continue;
            if (String::str_contains(line, "\"xmodel.gdf\"")) {
                flagName = String::swiftRegex(line, "\"([^\"]*)\"\\W*(.*)\"", 1);
                xmodelFlag = true;
                //std::cout << line << std::endl;
                continue;
            }
            if (String::str_contains(line, "\"material.gdf\"")) {
                flagName = String::swiftRegex(line, "\"([^\"]*)\"\\W*(.*)\"", 1);
                materialFlag = true;
                //std::cout << line << std::endl;
                continue;
            }
            if (xmodelFlag) {
                if (String::str_contains(line, "}")) {
                    xmodelFlag = false;
                    flagName = "";
                    continue;
                }
                if (String::str_contains(line, "\"filename\"")) {
                    std::string modelPath = String::swiftRegex(line, "\"([^\"]*)\"\\W*(.*)\"", 2);
                    customXmodelsMap[flagName] = modelPath;
                    //std::cout << line << std::endl;
                }
            }
            if (materialFlag) {
                if (String::str_contains(line, "}")) {
                    if (!specialCustomTexturesMap.contains(colorMapName)) {
                        specialCustomTexturesMap[colorMapName] = tempMap;
                    }
                    if (flagName != colorMapName.stem().string()) {
                        duplicateTextureMaterialsArray[colorMapName.stem().string()].push_back(flagName);
                    }
                    materialFlag = false;
                    flagName = "";
                    colorMapName = "";
                    tempMap.clear();
                    continue;
                }
                if (String::str_contains(line, "\"colorMap\"")) {
                    std::string materialName = String::swiftRegex(line, "\"([^\"]*)\"\\W*(.*)\"", 2);
                    std::string removeSeparator = "\\\\";
                    materialName = String::replace(materialName, removeSeparator, "/");
                    std::filesystem::path materialPath(cod4RootDirectory + materialName);
                    std::filesystem::path textureFileName (String::lowerCase(materialPath.string()));
                    if (!textureFilePath.contains(textureFileName.string()) && materialPath != "") {
                        tempGdtMap[materialPath] = gdtPath.string();
                        textureFilePath[textureFileName] = tempGdtMap;
                        tempGdtMap.clear();
                    }

                    customMaterialsMap[flagName] = textureFileName.stem().string();
                    colorMapName = textureFileName.string();
                    //std::cout << line << std::endl;
                }
                else if (String::str_contains(line, "\"detailMap\"") || String::str_contains(line, "\"normalMap\"") ||
                    String::str_contains(line, "\"specColorMap\"") ||
                    String::str_contains(line, "\"cosinePowerMap\"")) {
                    std::regex modelRegex("\"([^\"]*)\"\\W*(.*)\"", std::regex_constants::icase);
                    std::smatch matchResults;
                    std::regex_search(line, matchResults, modelRegex);
                    std::string mapType = String::lowerCase(matchResults.str(1));
                    std::string regexString = matchResults.str(2);
                    std::string removeSeparator = "\\\\";
                    regexString = String::replace(regexString, removeSeparator, "/");
                    std::filesystem::path fullMaterialPath("");
                    if(regexString != "") fullMaterialPath = (cod4RootDirectory + regexString);
                    tempMap[mapType] = fullMaterialPath;
                    std::filesystem::path tempString = String::lowerCase(fullMaterialPath.string());
                    if (!textureFilePath.contains(tempString) && fullMaterialPath != "") {
                        tempGdtMap[fullMaterialPath] = gdtPath.string();
                        textureFilePath[tempString] = tempGdtMap;
                        tempGdtMap.clear();
                    }
                    /*
                    if (fullMaterialPath.has_filename() && fullMaterialPath.has_extension()) {
                        std::cout << "\nFlagname: " << flagName << ", =>" << colorMapName << " ==>> " << mapType << ": " << fullMaterialPath.filename().stem().string();
                    }
                    */
                }
            }
        }
        std::cout << " DONE" << std::endl;;
        file.close();
        materialFlag = false;
        xmodelFlag = false;
    }
}


void readMaterialCSV(std::filesystem::path filePath, bool isMissingFromGdt = false) {
    char c;
    std::string removeSeparator = "\\\\";
    filePath = String::replace(filePath.string(), removeSeparator, "/");
    std::cout << "Reading " << filePath.make_preferred().string() << " material file for Textures";
    if( !isMissingFromGdt ) add2OptionalFiles("materials", filePath.string());
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    int counter = -1;
    std::string textureName;
    std::filesystem::path texturePath;
    std::streampos size = file.tellg();
    std::string line;
    bool foundFlag = false;
    if (file.is_open()) {
        for (int i = 1; i <= size; i++) {
            file.seekg(-i, std::ios::end);
            file.get(c);
            //std::cout << "C in int: " << int(c) <<" :: Char: " << c << "\n";
            if (int(c) > 30 && int(c) < 128 || int(c) == 0) {
                if (int(c) == 0) {
                    line += " ";
                    continue;
                }
                std::string s(1, c);
                line += s;
            }

        }
    }
    else {
        std::cout << "\n---Failed to open " << filePath.string() << "---\n";
        file.close();
        return;
    }
    std::reverse(line.begin(), line.end());
    //if (filePath.filename().string() == "mtl_plr_dnf_duke_glasses_lens") std::cout << line << "\n";
    //std::cout << line << "\n";
    std::regex modelRegex( "(?:" + filePath.filename().string() + "\\s)(.*)", std::regex_constants::icase);
    std::smatch matchResults;
    std::regex_search(line, matchResults, modelRegex);
    std::vector<std::string> arr = String::split(matchResults.str(1));
    if (!isMissingFromGdt) {
        if (!String::in_array(filePath.filename().string(), textureMapTypeFileArray)) textureMapTypeFileArray.push_back(filePath.filename().string());
        for (int i = 0; i < arr.size(); i++) {
            if (String::lowerCase(arr[i]) == "colortint" || String::lowerCase(arr[i]) == "envmapparms") break;
            if (String::lowerCase(arr[i]) == "colormap" && i != 0) {
                if (!textureMapTypeMap.contains(arr[i - 1])) textureMapTypeMap[arr[i - 1]] = arr[i];
            }
            else if (String::lowerCase(arr[i]) == "detailmap" || String::lowerCase(arr[i]) == "specularmap" || String::lowerCase(arr[i]) == "normalmap") {
                if (i + 1 != arr.size() && !textureMapTypeMap.contains(arr[i + 1])) textureMapTypeMap[arr[i + 1]] = arr[i];
            }
            //std::cout <<i << ": " << arr[i] << "\n";
        }
        //std::cout << matchResults.str(1) << "\n";
    }
    else {
        for (int i = 0; i < arr.size(); i++) {
            if (String::lowerCase(arr[i]) == "colortint" || String::lowerCase(arr[i]) == "envmapparms") break;
            if (String::lowerCase(arr[i]) == "colormap" && i != 0) {
                if (!textureMapTypeFromMaterialMap.contains(arr[i - 1])) textureMapTypeFromMaterialMap[arr[i - 1]] = arr[i];
            }
            else if (String::lowerCase(arr[i]) == "detailmap" || String::lowerCase(arr[i]) == "specularmap" || String::lowerCase(arr[i]) == "normalmap") {
                if (i + 1 != arr.size() && !textureMapTypeFromMaterialMap.contains(arr[i + 1])) textureMapTypeFromMaterialMap[arr[i + 1]] = arr[i];
            }
            //std::cout <<i << ": " << arr[i] << "\n";
        }
    }
    std::cout << " DONE" << std::endl;
    file.close();
}


void readCSV(std::filesystem::path csvPath) {

    std::string removeSeparator = "\\\\";
    csvPath = String::replace(csvPath.string(), removeSeparator, "/");
    add2AllFiles("csv", csvPath.make_preferred().string());
    std::cout << "Reading " << csvPath.make_preferred().string() << "\n";
    std::ifstream file(csvPath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            trim(line);
            //std::cout << line << "\n";
            std::regex modelRegex("\\W*(\\w*),(.*)", std::regex_constants::icase);
            std::smatch matchResults;
            std::regex_search(line, matchResults, modelRegex);
            //{"fx", 1}, {"rawfile", 2}
            std::map<std::string, std::string> materialFiles = { {"raw/material_properties/",""}, {"raw/materials/",""}, {"raw/images/",".iwi"} };
            switch (_CSVprepareSwitchCase[matchResults.str(1)]) {
            case 1:
                if (std::filesystem::exists(cod4RootDirectory + "raw/fx/" + matchResults.str(2) + ".efx") && !fxFilesMap.contains(matchResults.str(2) + ".efx")) {
                    add2AllFiles("fx", cod4RootDirectory + "raw/fx/" + matchResults.str(2) + ".efx");
                }
                break;
            case 2:
                if (std::filesystem::exists(cod4RootDirectory + "raw/" + matchResults.str(2))) {
                    add2AllFiles("gsc", cod4RootDirectory + "raw/" + matchResults.str(2));
                }
                break;
            case 3:
                for (auto& [path, ext] : materialFiles) {
                    std::filesystem::path myPath = cod4RootDirectory + path + matchResults.str(2) + ext;
                    //std::cout << myPath.make_preferred().string() << "\n";
                    if (std::filesystem::exists(myPath.make_preferred().string()) && path != "raw/materials/") {
                        if (path == "raw/images/") {
                            add2OptionalFiles("iwi-files", myPath.make_preferred().string());
                        }
                        else {
                            add2OptionalFiles("material-props", myPath.make_preferred().string());
                        }
                    }
                    if (path == "raw/materials/" ) {
                        readMaterialCSV(myPath.make_preferred().string());
                    }
                }
                break;
            case 4:
                if (std::filesystem::exists(cod4RootDirectory + "raw/xmodel/" + matchResults.str(2))) {
                    if (xmodelStockPathFilesMap.contains(matchResults.str(2))) {
                        continue;
                    }
                    add2OptionalFiles("xmodel", cod4RootDirectory + "raw/xmodel/" + matchResults.str(2));
                }
                break;
            }
            
        }
        std::cout << " DONE" << std::endl;
        file.close();
    }
}



//1 == StockTextures, 2 == stockSourceFiles, 3 == xmodelStockFiles, 4 == fxFiles
bool readTxtFile(std::filesystem::path txtPath) {
    std::ifstream file(txtPath);
    std::cout << "Reading " << txtPath.string();
    if (!std::filesystem::exists(txtPath)) {
        std::cout << "\n" << txtPath.filename().string() << " doesn't exist!\nGet working version from my GitHub!\nGitHub: https://github.com/skazalien/Cod4FilesWrapper \nExiting program." << std::endl;
        return false;
    }
    int fileType = 0;

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line == "") continue;
            for (auto& [k, v] : startFilesMap) {
                if (line == k) {
                    fileType = v;
                    break;
                }
                if (line == "--end--") fileType == -1;
            }
            switch (fileType)
            {
            case 1:
                stockTexturesMap[line] = true;
                break;
            case 2:
                stockSourceDataFilesMap[line] = true;
                break;
            case 3:
                xmodelStockPathFilesMap[line] = true;
                break;
            case 4:
                fxFilesMap[line] = true;
                break;
            }
        }
        std::cout << " DONE" << std::endl;
        file.close();
    }
    return true;
}

void readFile(std::filesystem::path filePath, std::string texture) {
    std::cout << "Reading " << filePath.make_preferred().string() << " convertCache file";
    add2AllFiles("material", filePath.make_preferred().string());
    std::ifstream file(filePath, std::ios::binary);
    std::string textureName;
    std::filesystem::path texturePath;
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            //std::cout << "Line " << counter << ": " << line << "\n";
            std::regex modelRegex("\\W*([\\w\\\\|\\/]*)[\\\\](\\w*)", std::regex_constants::icase);
            std::smatch matchResults;
            std::regex_search(line, matchResults, modelRegex);
            texturePath = matchResults.str(1) + "/" + matchResults.str(2);
            textureName = texturePath.stem().string();
            if (textureName == "") {
                //std::cout << "\nTexture: " << texture << " filepath doesn't exist!\nAdding it to errLog!" << std::endl;
                errLog.push_back(texture);
            }
            else {
                addTextures(textureName);
                convertcacheIwiMap[texture] = textureName;
                //std::cout << "\nConvertCache Name: [" << filePath.filename().stem().string() << "]:" << "Texture Name: [" << textureName << "]" << std::endl;
            }
            break;
        }
        std::cout << " DONE" << std::endl;
        file.close();
        reassignValues();
        return;
    }
    else {
        std::cout << "File: " << filePath.filename().string() << " doesn't exist!\n Adding it to errLog!" << std::endl;
    }
}

bool copyFile(std::string fileOriginString, bool isOptional = false) {
    std::filesystem::path fileParentRoot(fileOriginString);
    std::filesystem::path fileOrigin(cod4RootDirectory + fileParentRoot.string());
    std::filesystem::path targetOrigin(cod4RootDirectory + "z_wrapped_maps/" + mapFile.stem().string() + "/" + fileParentRoot.parent_path().string());
    if(isOptional) targetOrigin = cod4RootDirectory + "z_wrapped_maps/" + mapFile.stem().string() + "_OPTIONAL/" + fileParentRoot.parent_path().string();
    
    /*
    
    if (isSpecular) {
        std::cout << "FileOrigin: " << fileOrigin << "\n";
    }
    */

    if (std::filesystem::exists(fileOrigin)) {
        try
        {
            if (!std::filesystem::exists(targetOrigin)) {
                std::filesystem::create_directories(targetOrigin);
            }
            const auto copyOptions = std::filesystem::copy_options::update_existing;
            std::filesystem::copy(fileOrigin, targetOrigin, copyOptions);
            return true;
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
            return false;
        }
    }
    return false;
}

void readXmodelBackwards(std::string xmodelString) {
    char c;
    std::string removeSeparator = "\\\\";
    xmodelString = String::replace(xmodelString, removeSeparator, "/");
    std::filesystem::path xmodelPath(cod4RootDirectory + "model_export/" + xmodelString);
    add2AllFiles("xmodel", xmodelPath.make_preferred().string());
    std::cout << "Reading " << xmodelPath.make_preferred().string();

    std::ifstream myFile(xmodelPath, std::ios::ate);
    std::streampos size = myFile.tellg();
    if (myFile.is_open()) {
        std::string line;
        for (int i = 1; i <= size; i++) {
            myFile.seekg(-i, std::ios::end);
            myFile.get(c);
            std::string s(1, c);
            if (s != "\n") {
                line += s;
            }
            else {
                if (String::str_contains(line, "LAIRETAM")) {
                    std::reverse(line.begin(), line.end());
                    std::string xmodelTexture = String::swiftRegex(line, "\\s*(MATERIAL [0-9]+)\\W*(\\w+)\\W*(\\w*)\\W*(\\w*)", 4);
                    addTextures(xmodelTexture);
                    //std::cout << line << std::endl;
                    if (String::str_contains(line, "MATERIAL 0")) break;
                }
                line = "";
            }
        }
    }
    else {
        std::cout << "Failed to open " << xmodelString << "\n";
    }
    std::cout << " DONE" << std::endl;
    myFile.close();
}

/*
void readXmodelFile(std::string xmodelString) {
    char c;
    std::string removeSeparator = "\\\\";
    xmodelString = String::replace(xmodelString, removeSeparator, "/");
    //std::filesystem::path xmodelPath(cod4RootDirectory + xmodelString);
    std::filesystem::path xmodelPath(xmodelString);
    allTextureAndGdtFilePaths[xmodelPath.make_preferred().string()] = true;
    std::cout << "Reading " << xmodelPath.make_preferred().string();

    std::ifstream myFile(xmodelPath, std::ios::ate | std::ios::binary);
    std::streampos size = myFile.tellg();
    std::map<std::string, int> texturesString;
    std::string line;
    bool foundNewLineFlag = false;
    if (myFile.is_open()) {
        for (int i = 1; i <= size; i++) {
            myFile.seekg(-i, std::ios::end);
            myFile.get(c);
            std::string s(1, c);
            if (s != "\n" || int(c) != 10) {
                if (String::in_array(int(c), includeCharactersArray)) {
                    line += s;
                }
                else if (int(c) >= 0 && int(c) < 32) {
                    //they are in reverse order
                    switch (int(c))
                    {
                    case 0:
                        line += "<LUN>";
                        break;
                    case 1:
                        line += "<HOS>";
                        break;
                    case 2:
                        line += "<XTS>";
                        break;
                    case 4:
                        line += "<TOE>";
                        break;
                    case 7:
                        line += "<LEB>";
                        break;
                    }

                }
                else {
                    if (line.length() > 0) {
                        if(int(line.back()) != int(" ")) line += " ";
                    } 
                }
            }
            else {
                foundNewLineFlag = true;
                break;
            }
            
        }
    }
    else {
        std::cout << "Failed to open " << xmodelString << "\n";
    }
    //std::reverse(line.begin(), line.end());

    std::cout << "String: " << line << "\n";
    //std::string textures = String::swiftRegex(line, ".*(?:>NUL<){4}>SOH<(?:>NUL<){5}(.*)>NUL<", 1);
    //if (textures == "") {
    //    std::cout << "String: " << line << "\n";
    //}
    //else {
    //    std::cout << textures << "\n";
    //}

    std::cout << " DONE" << std::endl;
    myFile.close();
}
*/


void echoLine(const int &maxLength, char &typeChar, std::string type = "", int sizeOfType = 0) {
    if(type == "") std::cout << std::string(1, typeChar) << std::string(maxLength, char(32)) << std::string(1, typeChar) << "\n";
    else {
        //+ 14 = 2 a végén, material.length() = 8, 2xSpace = 4
        int longestWordLength = 23;
        int mA = floor((longestWordLength - 2 - type.length()) / 2);
        int mB = longestWordLength - mA -2 - type.length();



        std::string sizeString = "  Size:  " + std::to_string(sizeOfType);



        //TODO PRINT OUT CORRECTLY
        std::cout
            << std::string(longestWordLength, typeChar) << std::string(maxLength - longestWordLength+1, char(32)) << std::string(1, typeChar) << "\n"
            << std::string(1, typeChar) << std::string(mA, char(32)) << type <<  std::string(mB, char(32)) << std::string(1, typeChar)
            << sizeString << std::string(maxLength - (longestWordLength + sizeString.length()) + 1, char(32)) << std::string(1, typeChar) << "\n"
            << std::string(longestWordLength, typeChar) << std::string(maxLength - longestWordLength+1, char(32)) << std::string(1, typeChar) << "\n";
    }
}



/////////Functions /> /////////

int main()
{
    setlocale(LC_ALL, "");
    SetConsoleOutputCP(nCodePage);
    SetConsoleCP(nCodePage);
    std::string currentWorkingDirectory = std::filesystem::current_path().string();
    std::filesystem::path stockFiles(currentWorkingDirectory + "/stockFiles.txt");

    assignCharactersArray();

    //readFileForSpecular("D:/G/Call of Duty 4/raw/materials/purple2alt");


    //std::string mapfilePath = "D:/G/Call of Duty 4/map_source/mp_stalker_v2.map";

    std::string mapfilePath;
    std::cout << "CoD4 Files Wrapper v1.1 (c) skazy\n" << std::endl;
    std::cout << "Input your .map file location, in the form of:";
    std::cout << "\nC:/Program Files (x86)/Activision/Call of Duty 4 - Modern Warfare/map_source/mapFileName.map" << std::endl;
    std::cout << "Separators can be either / or \\\n\nInput >> ";
    std::getline(std::cin, mapfilePath);
    std::cout << "The given path: " << mapfilePath << std::endl;
    while (true) {
        if (!std::filesystem::exists(mapfilePath)) {
            std::cout << "File doesn't exist...\nInput >> " << std::endl;
            std::getline(std::cin, mapfilePath);
            continue;
        }
        else if (String::lowerCase(String::swiftRegex(mapfilePath, "\\W*(\\w*[\\\\|\\/]*)\\.(\\w*)", 2)) != "map") {
            std::cout << "Wrong File Extension!\nInput >> " << std::endl;
            std::getline(std::cin, mapfilePath);
            continue;
        }
        else {
            std::cout << "File exists and has .map extension!\nContinue..." << std::endl;
            break;
        }
    }
    mapFile = String::lowerCase(mapfilePath);
    std::vector<std::filesystem::path> directoryArray;
    &mapFile.make_preferred();
    for (auto i = mapFile.begin(); i != mapFile.end(); i++) {
        if (*i == "\\" || *i == "/") continue;
        directoryArray.push_back(*i);
    }

    std::filesystem::path potentialRoot = Path::array2Path(directoryArray, 0, directoryArray.size() - 2, "/");

    bool restartLoop = false;
    while (true) {
        if (restartLoop) {
            std::string potentialRootString;
            std::cout << "\nInput >> ";
            std::getline(std::cin, potentialRootString);
            std::filesystem::path potentialPath(potentialRootString);
            if (!std::filesystem::exists(potentialPath)) {
                std::cout << potentialPath.string() << " doesn't exist..." << std::endl;
                continue;
            }
            directoryArray.clear();
            for (auto i = potentialPath.begin(); i != potentialPath.end(); i++) {
                if (*i == "\\" || *i == "/") continue;
                directoryArray.push_back(*i);
            }
            potentialRoot = Path::array2Path(directoryArray, 0, directoryArray.size(), "/");
            restartLoop = false;
            continue;

        }
        if (std::filesystem::exists(potentialRoot / "map_source") && std::filesystem::exists(potentialRoot / "miles")
            && std::filesystem::exists(potentialRoot / "main")
            && std::filesystem::exists(potentialRoot / "iw3mp.exe")) {
            cod4RootDirectory = potentialRoot.string() + "/";
            cod4RootDirectoryLength = cod4RootDirectory.length();
            break;
        }
        else {
            std::cout << "File is not from map_source folder!" << std::endl;
            std::cout << "Define Cod4 Root Directory!\nExample: C:\\X\\Y\\Z\\Call of Duty 4 - Modern Warfare" << std::endl;
            restartLoop = true;
        }
    }

    std::cout << "Your Cod4 Directory: " << cod4RootDirectory << std::endl;
    std::string sourceDataPath = cod4RootDirectory + "source_data";


    if (!readTxtFile(stockFiles)) {
        std::cout << "\nType 1, then Enter to open GitHub Repository,\nElse Exit:\n";
        std::string endString;
        std::getline(std::cin, endString);
        if (endString == "1") {
            ShellExecute(NULL, NULL, "https://github.com/skazalien/Cod4FilesWrapper", NULL, NULL, SW_SHOWNORMAL);
        }
        return EXIT_FAILURE;
    }



    std::vector<std::string> allGdtFiles;
    for (const auto& entry : std::filesystem::directory_iterator(sourceDataPath))
    {
        if (entry.is_directory() || stockSourceDataFilesMap.contains(entry.path().stem().string())) continue;
        allGdtFiles.push_back(entry.path().string());
        //std::cout << entry << std::endl;
    }

    std::cout << "Reading Main mapfile: " << mapFile.filename() << std::endl;
    readMapFile(mapFile);
    readCSV(cod4RootDirectory + "zone_source/" + mapFile.stem().string() + ".csv");
    /*
    for (auto& [k,v] : textureMapTypeMap) {
        //TODO: REST
        if (v == "file") {
            std::cout << "File: " << k << "\n";
        }
        else {
            std::cout << v << " : " << k << "\n";
        }
    }

    for (auto& mFile : textureMapTypeFileArray) {
        std::cout << "File: " << mFile << "\n";
    }
    */
    /*
    
    std::string xModelFileFilePath = "D:/G/Call of Duty 4/raw/xmodel";
    int cooo = 0;
    for (const auto& entry : std::filesystem::directory_iterator(xModelFileFilePath))
    {
        if (cooo > 20) break;
        cooo += 1;
        if (entry.is_directory()) continue;
        std::cout << entry << ":\n";
        readXmodelFile(entry.path().string());
    }
    */
    
    int counter = 0;
    while (counter != entities.size()) {
        std::filesystem::path entityPath(cod4RootDirectory + "map_source/" + entities[counter]);
        readMapFile(entityPath.make_preferred());
        counter += 1;
    }

    for (auto& gdtfile : allGdtFiles) {
        readGDT(gdtfile);
    }
   

    for (auto& model : models) {
        if (customXmodelsMap.contains(model)) {
            readXmodelBackwards(customXmodelsMap[model]);
        }
    }

    std::vector<std::string> stockTexturesArray;
    int i = 0;
    while (i != textures.size()) {
        std::string texture = textures[i];
        if (customMaterialsMap.contains(textures[i]) && customMaterialsMap[textures[i]] != "") {
            texture = customMaterialsMap[textures[i]];
        }
        i += 1;
        if (stockTexturesMap.contains(texture)) {
            stockTexturesArray.push_back(texture);
            //std::cout << " << Texture: [[" << texture << "]] is a stock texture!" << std::endl;
            continue;
        }
        std::filesystem::path iwiPath(cod4RootDirectory + "raw/images/" + texture + ".iwi");
        if (!std::filesystem::exists(iwiPath)) {
            //std::cout << "\nFile: " << iwiPath.filename().string() << " doesn't exist...\nThe file is most likely a material.\nLooking for convertcache." << std::endl;
            std::filesystem::path convertCacheMaterialPath(cod4RootDirectory + "convertcache/raw/materials/" + texture);
            if (std::filesystem::exists(convertCacheMaterialPath)) {
                readFile(convertCacheMaterialPath, texture);
            }
            else {
                std::cout << ">> Texture: {" << texture << "} doesn't exist!\nAdding it to errLog." << std::endl;
                errLog.push_back(texture);
            }

        }
        else {
            //std::cout << " >> Texture: [[" << texture << "]] is a valid custom texture!" << std::endl;
            if (!customTexturesMap.contains(texture)) {
                customTexturesMap[texture] = true;
                customTextures.push_back(texture);
            }
        }
    }
    /*
    for (auto& texture : stockTexturesArray) {
        std::cout << ">> Texture: ((" << texture << ")) is a stock texture!" << std::endl;
    }
    */

    std::cout << "\n" << std::endl;
    std::map<std::string, std::filesystem::path> colSpecNormArray;

    for (auto& [kulcs, ertek] : specialCustomTexturesMap) {
        if (customTexturesMap.contains(kulcs.stem().string())) {
            //std::cout << "Texture: " << kulcs << "\n";
            for (auto& [k, v] : ertek) {
                if (v == "") continue;
                //std::cout << k << " : " << v << "\n";
                v = String::lowerCase(v.string());
                /*if (!String::in_array(v, colSpecNormArray) && String::lowerCase(k) == "speccolormap")*/ 
                if(String::lowerCase(k) != "colormap") {
                    //std::cout << kulcs << "::" << k << "::" << v << "\n";
                    if (v.stem().string() == "" || String::lowerCase(v.stem().string()) == "white") continue;
                    colSpecNormArray[k] = v;
                    continue;
                }
                if (!String::in_array(v.string(), customTextures)) {
                    customTextures.push_back(v.string());
                    customTexturesMap[v.string()] = true;
                    continue;
                }
            }
            if (colSpecNormArray.size() > 0) {
                colSpecNormMap[kulcs.stem().string()] = colSpecNormArray;
                colSpecNormArray.clear();
            }
            //std::cout << "\n";
        }
    }

    std::sort(customTextures.begin(), customTextures.end());
    /*
    for (int i = 0; i < colSpecNormArray.size(); i++) {
        //std::cout << colSpecNormArray[i] << "\n";
        for (const auto& entry : std::filesystem::directory_iterator(cod4RootDirectory + "raw/images"))
        {
            if (!String::str_contains(entry.path().filename().string(), "~")) continue;
            if (String::str_contains(entry.path().filename().string(), "~" + colSpecNormArray[i] + "-")) {
                customTextures.push_back(entry.path().stem().string());
                customTexturesMap[colSpecNormArray[i]] = true;
                break;
            }
        }
    }
    */


    //TextureName --> [TextureFilePath, gdtFilePath]
    //TODO Store it as : TextureName => { gdtFileName, {col,norm: filePath} }

    for (auto& [k, v] : customTexturesMap) {
        if (duplicateTextureMaterialsArray.contains(k)) {
            for (int i = 0; i < duplicateTextureMaterialsArray[k].size(); i++) {
                //std::cout << "Map uses: " << duplicateTextureMaterialsArray[k][i] << " materialFile!\n";
                convertcacheIwiMap[duplicateTextureMaterialsArray[k][i]] = k;
            }
        }
    }

    std::map<std::string, std::string> convertCacheIwiMapReverse;

    for (auto& [k, v] : convertcacheIwiMap) {
        convertCacheIwiMapReverse[v] = k;
    }

    for (auto& [kulcs, ertek] : colSpecNormMap) {
        //std::cout << "SpecOrNormalMap >> Texture: " << kulcs << "\n";
        for (auto& [k,v] : ertek) {
            //std::cout << "V>>" << k << ": J>>" << v << "\n";
            if (k != "speccolormap") {
                add2OptionalFiles("image", v.string());
                if (!customTexturesMap.contains(v.stem().string())) {
                    customTextures.push_back(v.stem().string());
                    customTexturesMap[v.stem().string()] = true;
                }
            }
            else if(k == "speccolormap") {
                //std::cout << "Key: " << k << ", Value: " << v << "\n";
                std::filesystem::path filePath(cod4RootDirectory + "raw/materials/" + kulcs);
                if (!std::filesystem::exists(filePath)) {
                    filePath = cod4RootDirectory + "raw/materials/" + convertCacheIwiMapReverse[kulcs];
                }
                //std::cout << "FileNameConvertCache: " << filePath.string() << "\n";
                readMaterialCSV(filePath,true);
                add2OptionalFiles("image",v.string());
                
            }
        }
    }

    std::map<std::string, bool> wasInGDT;
    std::map<std::string, bool> inGdt;

    for (auto& [kulcs, ertek] : textureFilePath) {
        inGdt[kulcs.stem().string()] = true;
        //std::cout << "Kulcs: " << kulcs.stem().string() << "\n";
        if (customTexturesMap.contains(kulcs.stem().string()) || convertcacheIwiMap.contains(kulcs.stem().string())) {
            for (auto& [k, v] : ertek) {
                add2OptionalFiles("image",k.string());
                add2AllFiles("gdt", v);

                //std::cout << "K: " << k << " : " << "V: " << v << " :\n";
            }
            //std::cout << "\n";
        }
    }

    for (auto& [k, v] : customTexturesMap) {
        if (!inGdt.contains(k)) {
            std::cout << "File: \n <<" << k << ">> doesn't exist in any GDT files!\n";
            std::string myPath = cod4RootDirectory + "raw/materials/" + k;
            readMaterialCSV(myPath, true);
        }
    }

    for (auto& [k, v] : textureMapTypeFromMaterialMap) {
        if (String::lowerCase(v) == "specularmap") {
            specularTexturesArray.push_back(k);
        }
        else {
            if (!customTexturesMap.contains(k)) {
                customTexturesMap[k] = true;
                customTextures.push_back(k);
            }
        }
    }

    std::map<std::string, int> extraCSVMap{ {"raw/maps/", 0}, {"assettgtcache/raw/raw/maps/", 0}, {"zone_source/english/assetlist/", 1},
       {"zone_source/english/assetinfo/", 2}, {"zone_source/", 1} };

    for (auto& [k, v] : extraCSVMap) {
        std::string myString = k + mapFile.stem().string();
        if (std::filesystem::exists(cod4RootDirectory + myString + ".csv")) {
            copyFile(myString + ".csv");
            add2AllFiles("csv", cod4RootDirectory + myString + ".csv");
        }
        switch (v) {
        case 1:
            if (std::filesystem::exists(cod4RootDirectory + myString + "_load.csv")) {
                copyFile(myString + "_load.csv");
                add2AllFiles("csv", cod4RootDirectory + myString + "_load.csv");
            }
            break;
        case 2:
            if (std::filesystem::exists(cod4RootDirectory + myString + "_load.csv")) {
                copyFile(myString + "_load.csv");
                add2AllFiles("csv", cod4RootDirectory + myString + "_load.csv");
            }
            if (std::filesystem::exists(cod4RootDirectory + myString + "_load_total.csv")) {
                copyFile(myString + "_load_total.csv");
                add2AllFiles("csv", cod4RootDirectory + myString + "_load_total.csv");
            }


            break;

        }
    }

    std::map<std::string, std::string> extraFiles{ {"bin/CoD4CompileTools", "settings"} };

    for (auto& [k, v] : extraFiles) {
        if (std::filesystem::exists(cod4RootDirectory + k + "/" + mapFile.stem().string() + "." + v)) {
            copyFile(k + "/" + mapFile.stem().string() + "." + v);
            add2AllFiles("extra", cod4RootDirectory + k + "/" + mapFile.stem().string() + "." + v);
        }
    }

    
//####################################################################################################################################################################################
                                                                        //Print out all Files it collected//
                                                                        ////////////////////////////////////
    int extra = 0;
    char borderChar = char(35);
    char spaceChar = char(32);

    std::string tab = "          ";
    std::string printLine = "FilePath: ";
    const int maxLength = allTextureAndGdtFilePathsMaxLength + printLine.length() + 2*tab.length();
    std::string headLine = "  All Files  ";

    std::map < std::string, bool > customMap = Map::array2map(customTextures);
    std::map < std::string, bool > specMap = Map::array2map(specularTexturesArray);
    std::map < std::string, bool > customModelMap;

    for (auto& [k,v] : modelsMap) {
        if (!xmodelStockPathFilesMap.contains(k) && !customModelMap.contains(k)) {
            customModelMap[k] = true;
        }
    }


    std::map<std::map<std::string, bool>, std::string> accumulatedFilesMap{
        {allMapFiles, "map"}, {allCSVFiles, "csv"}, {allFXFiles, "fx"}, {allGDTFiles, "gdt"},
        {customMap, "textures"}, {specMap, "specular-textures"}, {customModelMap, "xmodel"},
        {allGscFiles, "gsc"}, {allExtraFiles, "extra"} 
    };

    std::map<std::string, bool> exclusionMaps{ {"textures", true}, {"specular-textures", true}, {"xmodel", true } };


    int maxMapSize = 0;
    for (auto& [k, v] : accumulatedFilesMap) {
        maxMapSize += k.size();
    }
    std::string sizeString = "  Size:  " + std::to_string(maxMapSize) + std::string(2, spaceChar);
    std::cout << "\n" << std::string(headLine.length() + 3 + sizeString.length(), borderChar) << "\n"
        << std::string(1, borderChar) << headLine << std::string(1, borderChar) << sizeString << std::string(1, borderChar) << "\n"
        << std::string(maxLength+2, borderChar) << "\n"
        << std::string(1, borderChar) << std::string(maxLength, spaceChar) << std::string(1, borderChar) << "\n";


    for (auto & [subMap, type]: accumulatedFilesMap) {
        echoLine(maxLength, borderChar);
        echoLine(maxLength, borderChar, type, subMap.size());
        echoLine(maxLength, borderChar);
        for (auto& [k, v] : subMap) {
            std::filesystem::path myTemp(k);
            int stringLength;
            if (!exclusionMaps.contains(type)) stringLength = myTemp.make_preferred().string().length() + printLine.length() + 2 * tab.length();
            else stringLength = myTemp.make_preferred().string().length() + 2 * tab.length();
            std::cout
                << std::string(1, borderChar)
                << tab;
            if(!exclusionMaps.contains(type)) std::cout<< printLine;
            std::cout
                << myTemp.make_preferred().string()
                << std::string(maxLength - stringLength, spaceChar)
                << tab
                << std::string(1, borderChar) << "\n";

            if (myTemp.extension().string() == ".gdt") {
                if (std::filesystem::exists(cod4RootDirectory + "assetsrccache/raw/source_data/" + myTemp.filename().string())) {
                    copyFile("assetsrccache/raw/source_data/" + myTemp.filename().string());
                    extra += 1;
                }
            }
            if(!exclusionMaps.contains(type)) copyFile(myTemp.string().substr(cod4RootDirectoryLength));
            //std::cout << "subStrPath: " << myTemp.string().substr(cod4RootDirectoryLength) << "\n\n";
        }
    }

    echoLine(maxLength, borderChar);
    echoLine(maxLength, borderChar, "material-iwi", convertcacheIwiMap.size());
    echoLine(maxLength, borderChar);
    int longestCacheFile  = 0;
    for (auto& [k, v] : convertcacheIwiMap) {
        if (k.length() > longestCacheFile) longestCacheFile = k.length();
    }
    std::string firstLine = tab + "Material: " + std::string(longestCacheFile, char(32)) + "Textures: ";
    std::cout << std::string(1, borderChar) << firstLine << std::string(maxLength - firstLine.length(), char(32))  << std::string(1, borderChar) << "\n";
    echoLine(maxLength, borderChar);
    for (auto& [k, v] : convertcacheIwiMap) {
        std::string myLine = tab + k + std::string(longestCacheFile - k.length(), char(32)) + tab + v;
        std::cout
            << std::string(1, borderChar) << myLine
            << std::string(maxLength - myLine.length(), spaceChar)
            << std::string(1, borderChar) << "\n";
    }

    std::cout << std::string(1, borderChar) << std::string(maxLength, spaceChar) << std::string(1, borderChar) << "\n"
        << std::string(maxLength+2, borderChar) << "\n";

//####################################################################################################################################################################################
// 
// 
//####################################################################################################################################################################################
                                                                        //Print out all Optional Files it collected//
                                                                        /////////////////////////////////////////////
    if (optionalFilePathsMaxLength == 0) optionalFilePathsMaxLength = 20;
    const int maxLengthOptional = optionalFilePathsMaxLength + printLine.length() + 2 * tab.length();
    std::string headLineOptional = "  All Optional Files  ";
    
    for (auto& [k,v] : textureMapTypeMap) {
        std::filesystem::path pathLine( cod4RootDirectory + "raw/images/" + k + ".iwi");
        if(!iwiFiles.contains(pathLine.make_preferred().string())) iwiFiles[pathLine.make_preferred().string()] = true;
    }


    std::map<std::map<std::string, bool>, std::string> accumulatedOptionalFilesMap{
        {imageFiles, "image"}, {materialFiles, "materials"}, {materialPropsFiles, "material-props"}, {xmodelFiles, "xmodel"}, {iwiFiles, "iwi-files"}
    };

    int maxMapOptionalSize = 0;
    for (auto& [k, v] : accumulatedOptionalFilesMap) {
        maxMapOptionalSize += k.size();
    }



    std::string sizeStringOptional = "  Size:  " + std::to_string(maxMapOptionalSize) + std::string(2, spaceChar);
    std::cout << "\n" << std::string(headLineOptional.length() + 3 + sizeStringOptional.length(), borderChar) << "\n"
        << std::string(1, borderChar) << headLineOptional << std::string(1, borderChar) << sizeStringOptional << std::string(1, borderChar) << "\n"
        << std::string(maxLengthOptional + 2, borderChar) << "\n"
        << std::string(1, borderChar) << std::string(maxLengthOptional, spaceChar) << std::string(1, borderChar) << "\n";

    for (auto& [subMap, type] : accumulatedOptionalFilesMap) {
        if (subMap.size() > 0) {
            echoLine(maxLengthOptional, borderChar);
            echoLine(maxLengthOptional, borderChar, type, subMap.size());
            echoLine(maxLengthOptional, borderChar);
            for (auto& [k, v] : subMap) {
                std::filesystem::path myTemp(k);
                int stringLength = myTemp.make_preferred().string().length() + printLine.length() + 2 * tab.length();
                std::cout
                    << std::string(1, borderChar)
                    << tab
                    << printLine
                    << myTemp.make_preferred().string()
                    << std::string(maxLengthOptional - stringLength, spaceChar)
                    << tab
                    << std::string(1, borderChar);


                if (!copyFile(myTemp.string().substr(cod4RootDirectoryLength), true)) std::cout << " FAILED";
                else {
                    std::string model = myTemp.filename().string();
                    if (type == "xmodel") {
                        //std::cout << model << "\n";
                        if (std::filesystem::exists(".errlog")) {
                            copyFile("export_errors/models/" + myTemp.parent_path().string() + model + ".errlog", true);
                        }
                        for (auto& [k, v] : xmodelPartsMap) {

                            for (int i = 0; i < 4; i++) {
                                if (xmodelStockPathFilesMap.contains(model)) {
                                    break;
                                }
                                if (v == false) {
                                    copyFile(k + model, true);
                                    break;
                                }
                                else {
                                    copyFile(k + model + std::to_string(i), true);
                                }

                            }

                        }

                    }

                }
                std::cout << "\n";
                //std::cout << "subStrPath: " << myTemp.string().substr(cod4RootDirectoryLength) << "\n\n";
            }
       }
    }

    std::cout << std::string(1, borderChar) << std::string(maxLengthOptional, spaceChar) << std::string(1, borderChar) << "\n"
        << std::string(maxLengthOptional + 2, borderChar) << "\n";

    //####################################################################################################################################################################################

    std::cout << "Copying all Optional Files... DONE\n";

    std::cout << "Copying Specular Textures...";
    for (int i = 0; i < specularTexturesArray.size(); i++) {
        copyFile("raw/images/" + specularTexturesArray[i] + ".iwi");
        copyFile("assettgtcache/raw/raw/images/" + specularTexturesArray[i] + ".iwi");
        copyFile("convertcache/raw/images/" + specularTexturesArray[i]);
    }
    std::cout << " DONE";

    std::cout << "\nCopying Custom Textures...";
    for (int i = 0; i < customTextures.size(); i++) {
        copyFile("assettgtcache/raw/raw/material_properties/" + customTextures[i]);
        copyFile("assettgtcache/raw/raw/materials/" + customTextures[i]);
        copyFile("assettgtcache/raw/raw/images/" + customTextures[i] + ".iwi");
        copyFile("raw/material_properties/" + customTextures[i]);
        copyFile("raw/materials/" + customTextures[i]);
        copyFile("raw/images/" + customTextures[i] + ".iwi");
        copyFile("convertcache/raw/images/" + customTextures[i]);
        copyFile("convertcache/raw/materials/" + customTextures[i]);
    }
    std::cout << " DONE\n";

    std::cout << "Copying Material Files...";
    for (auto& [k, v] : convertcacheIwiMap) {
        //std::cout << ">> Texture: [[" << k << "]] is a valid custom texture under [[" << v << "]] name!" << std::endl;
        copyFile("assettgtcache/raw/raw/material_properties/" + k);
        copyFile("assettgtcache/raw/raw/materials/" + k);
        copyFile("raw/material_properties/" + k);
        copyFile("raw/materials/" + k);
        copyFile("convertcache/raw/materials/" + k);
    }
    //std::cout << "ConvertCache - IWI Pair Size: " << convertcacheIwiMap.size();
    std::cout << "... DONE\n";



    std::string result = "";
    std::cout << "Copying xModels...";
    for (auto& [model, path] : modelsMap) {
        if (std::filesystem::exists(cod4RootDirectory + "export_errors/models/" + model + ".errlog")) {
            copyFile("export_errors/models/" + model + ".errlog");
        }
        //std::cout << "Model: {{" << model << "}}";

        for (auto& [k, v] : xmodelPartsMap) {

            for (int i = 0; i < 4; i++) {
                if (xmodelStockPathFilesMap.contains(model)) {
                    //if (result == "") result = " is a Stock Model! Not copying...\n";
                    break;
                }
                //if (result == "") result = " is a Custom Model! Copying...\n";
                if (v == false) {
                    copyFile(k + model);
                    break;
                }
                else {
                    copyFile(k + model + std::to_string(i));
                }

            }

        }
        //std::cout << result;
        //result = "";
    }
    std::cout << " DONE\n";

    if (errLog.size() > 0) {
        for (auto& texture : errLog) {
            std::cout << ">> Texture {{" << texture << "}} has a mismatch!" << std::endl;
        }
    }

    std::cout << "\nStock Texture Count: " << stockTexturesArray.size() << std::endl;
    std::cout << "Custom Texture Count: " << customTextures.size() << std::endl;
    std::cout << "Gross Texture Count(valid, invalid) => (" << customTextures.size() + stockTexturesArray.size() << ";" << errLog.size() << ")" << std::endl;
    std::cout << "Press any key to exit...";
    std::cin.get();
    return EXIT_SUCCESS;
}
