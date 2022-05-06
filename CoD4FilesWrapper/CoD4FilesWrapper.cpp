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

std::map<std::string, std::map<std::string, std::string>> specialCustomTexturesMap;

std::string cod4RootDirectory;
int cod4RootDirectoryLength;
std::map<std::string, std::string> colSpecNormMap;

std::filesystem::path mapFile;

std::map<std::string, std::map<std::filesystem::path, std::string>> textureFilePath;

std::vector<std::filesystem::path> gdtFilePath;

std::vector<std::string> allExtensions = { ".jpg", ".dds", ".tga", ".jpeg" };

std::map<std::string, bool> allTextureAndGdtFilePaths;

std::map <std::string, int> _CSVprepareSwitchCase{ {"fx", 1}, {"rawfile", 2} };

//1 == StockTextures, 2 == stockSourceFiles, 3 == xmodelStockFiles,, 4 == fxFiles
std::map<std::string, int> startFilesMap{ {"--iwi--", 1}, {"--gdt--", 2}, {"--xmodel--", 3}, {"--fx--", 4} };


///////// Global Variables /> /////////


///////// < Functions/////////


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
        std::regex modelRegex(regexString, std::regex_constants::icase);
        std::smatch matchResults;
        std::regex_search(examineString, matchResults, modelRegex);
        return matchResults.str(resultNumber);
    }
    std::string replace(std::string str, std::string substr1, std::string substr2)
    {
        for (size_t index = str.find(substr1, 0); index != std::string::npos && substr1.length(); index = str.find(substr1, index + substr2.length()))
            str.replace(index, substr1.length(), substr2);
        return str;
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
    allTextureAndGdtFilePaths[filePath.make_preferred().string()] = true;
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
    std::string colorMapName;
    if (file.is_open()) {
        std::string line;
        std::map<std::string, std::string> tempMap;
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
                    if (flagName != colorMapName) {
                        duplicateTextureMaterialsArray[colorMapName].push_back(flagName);
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
                    std::string textureFileName = String::lowerCase(materialPath.stem().string());
                    if (!textureFilePath.contains(textureFileName) && materialPath != "") {
                        tempGdtMap[materialPath] = gdtPath.string();
                        textureFilePath[textureFileName] = tempGdtMap;
                        tempGdtMap.clear();
                    }

                    customMaterialsMap[flagName] = textureFileName;
                    colorMapName = textureFileName;
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
                    std::filesystem::path fullMaterialPath(cod4RootDirectory + regexString);
                    tempMap[mapType] = fullMaterialPath.stem().string();
                    std::string tempString = String::lowerCase(fullMaterialPath.stem().string());
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


void readCSV(std::filesystem::path csvPath) {

    std::string removeSeparator = "\\\\";
    csvPath = String::replace(csvPath.string(), removeSeparator, "/");
    allTextureAndGdtFilePaths[csvPath.make_preferred().string()] = true;
    std::cout << "Reading " << csvPath.make_preferred().string();
    std::ifstream file(csvPath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            trim(line);
            std::regex modelRegex("\\W*(\\w*),(.*)", std::regex_constants::icase);
            std::smatch matchResults;
            std::regex_search(line, matchResults, modelRegex);
            switch (_CSVprepareSwitchCase[matchResults.str(1)]) {
            case 1:
                if (std::filesystem::exists(cod4RootDirectory + "raw/fx/" + matchResults.str(2) + ".efx") && !fxFilesMap.contains(matchResults.str(2) + ".efx")) {
                    allTextureAndGdtFilePaths[cod4RootDirectory + "raw/fx/" + matchResults.str(2) + ".efx"];
                }
                break;
            case 2:
                if (std::filesystem::exists(cod4RootDirectory + "raw/" + matchResults.str(2))) {
                    allTextureAndGdtFilePaths[cod4RootDirectory + "raw/" + matchResults.str(2)];
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

void readFile(std::filesystem::path filePath, std::string texture, bool searchSpecular = false) {
    std::cout << "Reading " << filePath.make_preferred().string() << " convertCache file";
    allTextureAndGdtFilePaths[filePath.make_preferred().string()] = true;
    std::ifstream file(filePath, std::ios::binary);
    int counter = -1;
    std::string textureName;
    std::filesystem::path texturePath;
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            counter += 1;
            //std::cout << "Line " << counter << ": " << line << "\n";
            if (counter == 0) {
                std::regex modelRegex("\\W*([\\w\\\\|\\/]*)[\\\\](\\w*)", std::regex_constants::icase);
                std::smatch matchResults;
                std::regex_search(line, matchResults, modelRegex);
                texturePath = matchResults.str(1) + "/" + matchResults.str(2);
                textureName = texturePath.stem().string();
                if (!searchSpecular) {
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
            }
            else {
                std::cout << "Line " << counter << ": " << line << "\n";
                if (!String::str_contains(line, "-rgb&$") || !String::str_contains(line, "$white")) continue;
                                        //((?:~\\w*-)(?:[\\w\\-\\_\\&\\$]*white-l[\\-0-9]*))
                                        //((?:~\w*-)(?:[\w\-\_\&\$]*[\-0-9\~\w]*))
                std::regex modelRegex("(~.*[0-9])", std::regex_constants::icase);
                std::smatch matchResults;
                std::regex_search(line, matchResults, modelRegex);
                std::string foundSpecular = matchResults.str(1);
                //std::cout << "Line " << counter << ": " << foundSpecular << "\n";
                customTextures.push_back(foundSpecular);
                customTexturesMap[textureName] = true;
                break;

            }

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

void readFileForSpecular(std::filesystem::path filePath) {
    char c;
    std::cout << "Reading " << filePath.make_preferred().string() << " convertCache file\n";
    allTextureAndGdtFilePaths[filePath.make_preferred().string()] = true;
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
            std::string s(1, c);
            line += s;

        }
    }
    else {
        std::cout << "Failed to open " << filePath.string() << "\n";
    }
    std::reverse(line.begin(), line.end());
    //std::cout << line << "\n";
    std::regex modelRegex("(~.*[0-9])", std::regex_constants::icase);
    std::smatch matchResults;
    std::regex_search(line, matchResults, modelRegex);
    std::string foundSpecular = matchResults.str(1);
    std::cout << "Line " << counter << ": " << foundSpecular << "\n";
    customTextures.push_back(foundSpecular);
    customTexturesMap[textureName] = true;

    std::cout << " DONE" << std::endl;
    file.close();
}




void copyFile(std::string fileOriginString) {
    std::filesystem::path fileParentRoot(fileOriginString);
    std::filesystem::path fileOrigin(cod4RootDirectory + fileParentRoot.string());
    std::filesystem::path targetOrigin(cod4RootDirectory + "z_wrapped_maps/" + mapFile.stem().string() + "/" + fileParentRoot.parent_path().string());
    if (String::in_array(fileParentRoot.extension().string(), allExtensions)) {
        targetOrigin = cod4RootDirectory + "z_wrapped_maps/" + mapFile.stem().string() + "_TEXTURES/" + fileParentRoot.parent_path().string();
    }
    if (std::filesystem::exists(fileOrigin)) {
        try
        {
            if (!std::filesystem::exists(targetOrigin)) {
                std::filesystem::create_directories(targetOrigin);
            }
            const auto copyOptions = std::filesystem::copy_options::update_existing;
            std::filesystem::copy(fileOrigin, targetOrigin, copyOptions);
        }
        catch (std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
}

void readXmodelBackwards(std::string xmodelString) {
    char c;
    std::string removeSeparator = "\\\\";
    xmodelString = String::replace(xmodelString, removeSeparator, "/");
    std::filesystem::path xmodelPath(cod4RootDirectory + "model_export/" + xmodelString);
    allTextureAndGdtFilePaths[xmodelPath.make_preferred().string()] = true;
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


/////////Functions /> /////////

int main()
{
    setlocale(LC_ALL, "");
    SetConsoleOutputCP(nCodePage);
    SetConsoleCP(nCodePage);
    std::string currentWorkingDirectory = std::filesystem::current_path().string();
    std::filesystem::path stockFiles(currentWorkingDirectory + "/stockFiles.txt");
    //std::string mapfilePath = "D:/G/Call of Duty 4/map_source/mp_stalker_v2.map";

    std::string mapfilePath;
    std::cout << "CoD4 Files Wrapper v1.0 (c) skazy\n" << std::endl;
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
                std::cout << ">> Texture: " << texture << " doesn't exist!\nAdding it to errLog." << std::endl;
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
    std::vector<std::string> colSpecNormArray;

    for (auto& [kulcs, ertek] : specialCustomTexturesMap) {
        if (customTexturesMap.contains(kulcs)) {
            //std::cout << "Texture: " << kulcs << "\n";
            for (auto& [k, v] : ertek) {
                if (v == "") continue;
                //std::cout << k << " : " << v << "\n";
                v = String::lowerCase(v);
                if (!String::in_array(v, colSpecNormArray) && String::lowerCase(k) == "speccolormap") {
                    //std::cout << kulcs << "::" << k << "::" << v << "\n";
                    colSpecNormArray.push_back(v);
                    colSpecNormMap[kulcs] = v;
                    continue;
                }
                if (!String::in_array(v, customTextures)) {
                    customTextures.push_back(v);
                    continue;
                }
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



    for (auto& [kulcs, ertek] : textureFilePath) {
        //std::cout << kulcs << " :\n";
        if (customTexturesMap.contains(kulcs) || convertcacheIwiMap.contains(kulcs)) {
            for (auto& [k, v] : ertek) {
                if (!allTextureAndGdtFilePaths.contains(k.string())) {
                    allTextureAndGdtFilePaths[k.string()] = true;
                }
                if (!allTextureAndGdtFilePaths.contains(v)) {
                    allTextureAndGdtFilePaths[v] = true;
                }
                //std::cout << k << " : " << v << " :\n";
            }
        }
    }
    for (auto& [k, v] : allTextureAndGdtFilePaths) {
        std::filesystem::path myTemp(k);
        std::cout << "FilePath: " << myTemp.make_preferred().string() << "\n";
        if (myTemp.extension().string() == ".gdt") copyFile("assetsrccache/raw/source_data/" + myTemp.filename().string());
        copyFile(myTemp.string().substr(cod4RootDirectoryLength));
        //std::cout << "subStrPath: " << myTemp.string().substr(cod4RootDirectoryLength) << "\n\n";
    }

    std::cout << "allTextureAndGdtFilePaths size: " << allTextureAndGdtFilePaths.size() << "\n";

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

    for (auto& [k, v] : colSpecNormMap) {
        std::filesystem::path filePath(cod4RootDirectory + "convertcache/raw/materials/" + k);
        if (!std::filesystem::exists(filePath)) {
            filePath = cod4RootDirectory + "convertcache/raw/materials/" + convertCacheIwiMapReverse[k];
        }
        //std::cout << filePath.string() << "\n";
        readFileForSpecular(filePath);
    }

    for (int i = 0; i < customTextures.size(); i++) {
        std::cout << ">> Texture: [[" << customTextures[i] << "]] is a valid custom texture!" << std::endl;
        copyFile("assettgtcache/raw/raw/material_properties/" + customTextures[i]);
        copyFile("assettgtcache/raw/raw/materials/" + customTextures[i]);
        copyFile("assettgtcache/raw/raw/images/" + customTextures[i] + ".iwi");
        copyFile("raw/material_properties/" + customTextures[i]);
        copyFile("raw/materials/" + customTextures[i]);
        copyFile("raw/images/" + customTextures[i] + ".iwi");
        copyFile("convertcache/raw/images/" + customTextures[i]);
        copyFile("convertcache/raw/materials/" + customTextures[i]);
    }



    for (auto& [k, v] : convertcacheIwiMap) {
        std::cout << ">> Texture: [[" << k << "]] is a valid custom texture under [[" << v << "]] name!" << std::endl;
        copyFile("assettgtcache/raw/raw/material_properties/" + k);
        copyFile("assettgtcache/raw/raw/materials/" + k);
        copyFile("raw/material_properties/" + k);
        copyFile("raw/materials/" + k);
        copyFile("convertcache/raw/materials/" + k);
    }
    std::cout << "ConvertCache - IWI Pair Size: " << convertcacheIwiMap.size() << "\n" << std::endl;

    std::map<std::string, bool> xmodelPartsMap{ {"raw/xmodelsurfs/", true}, {"raw/xmodel/", false}, {"raw/xmodelparts/", true},
        {"assettgtcache/raw/raw/xmodel/", false}, {"assettgtcache/raw/raw/xmodelparts/", true}, {"assettgtcache/raw/raw/xmodelsurfs/", true} };

    std::string result = "";
    for (auto& [model, path] : modelsMap) {
        if (std::filesystem::exists(cod4RootDirectory + "export_errors/models/" + model + ".errlog")) {
            copyFile("export_errors/models/" + model + ".errlog");
        }
        std::cout << "Model: {{" << model << "}}";

        for (auto& [k, v] : xmodelPartsMap) {

            for (int i = 0; i < 4; i++) {
                if (xmodelStockPathFilesMap.contains(model)) {
                    if (result == "") result = " is a Stock Model! Not copying...\n";
                    break;
                }
                if (result == "") result = " is a Custom Model! Copying...\n";
                if (v == false) {
                    copyFile(k + model);
                    break;
                }
                else {
                    copyFile(k + model + std::to_string(i));
                }

            }

        }
        std::cout << result;
        result = "";
    }

    std::map<std::string, int> extraCSVMap{ {"raw/maps/", 0}, {"assettgtcache/raw/raw/maps/", 0}, {"zone_source/english/assetlist/", 1},
        {"zone_source/english/assetinfo/", 2}, {"zone_source/", 1} };

    for (auto& [k, v] : extraCSVMap) {
        copyFile(k + mapFile.stem().string() + ".csv");
        switch (v) {
        case 1:
            copyFile(k + mapFile.stem().string() + "_load.csv");
            break;
        case 2:
            copyFile(k + mapFile.stem().string() + "_load.csv");
            copyFile(k + mapFile.stem().string() + "_load_total.csv");
            break;

        }
    }
    std::map<std::string, std::string> extraFiles{ {"bin/CoD4CompileTools", "settings"} };

    for (auto& [k, v] : extraFiles) {
        copyFile(k + "/" + mapFile.stem().string() + "." + v);
    }

    for (auto& texture : errLog) {
        std::cout << ">> Texture {{" << texture << "}} has a mismatch!" << std::endl;
    }

    std::cout << "\nStock Texture Count: " << stockTexturesArray.size() << std::endl;
    std::cout << "Custom Texture Count: " << customTextures.size() << std::endl;
    std::cout << "Gross Texture Count(valid, invalid) => (" << customTextures.size() + stockTexturesArray.size() << ";" << errLog.size() << ")" << std::endl;
    std::cout << "Press any key to exit...";
    std::cin.get();
    return EXIT_SUCCESS;
}