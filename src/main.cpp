#include "main.h"
#include "raylib.h"
#define NLOHMANN_JSON_DISABLE_CONSTEXPR
#include "json.hpp"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <thread>
#include <random>
#include <sstream>
#include <iomanip>

#include "registry.hpp"
#include "messageBox.h"

using json = nlohmann::json;
using namespace std;

const string SCRAP_MECHANIC_STEAM_ID = "387990";

constexpr auto SCREEN_WIDTH  = 1280;
constexpr auto SCREEN_HEIGHT = 720;

struct ThreadInfo {
    string &userFolder;//should be the whole thing up to chellanges
    string &workshopFolder;
    string &challengeDIR;
    json &challengePackJson;
    json &challengePackDescription;
    int *packIndex;
    int *progress;
    int * maxxProgress;
};

string getSteamLibraryLocation();
void extractionThreadFunction(ThreadInfo);

int main() {

    //Step 1 get the app data users folder
    const char * appDataCStr = getenv("appdata");
    string ScrapMechanicAppdataFolder;
    if (appDataCStr != nullptr) {
        ScrapMechanicAppdataFolder = appDataCStr;
        ScrapMechanicAppdataFolder += R"(\Axolot Games\Scrap Mechanic\User\)";
    } else {
        cerr << "Unable to access Appdata folder!"<<endl;
        showMessageBox("Unable to access Appdata folder!","It no work!");
        return EXIT_FAILURE;
    }

    cout <<"App data folder DIR: "<< ScrapMechanicAppdataFolder << endl;

    //Step 2 check with steam to see where scrapmahcanic is installed
    string steamLibLocation = getSteamLibraryLocation();
    string scrapMechanicWorkShopConentLocation = steamLibLocation +R"(\steamapps\workshop\content\)"+SCRAP_MECHANIC_STEAM_ID+"\\";
    cout << "Steam library containing Scrap Mechanic: " << steamLibLocation << endl;


    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Scrap Mechanic Challenge Extractor");
    SetTargetFPS(60);

    //Step 3 look through each workshop entry and collect the ones that are chellange packs
    int numberFiles = 0;
    char ** contentFiles = GetDirectoryFiles(scrapMechanicWorkShopConentLocation.c_str(),&numberFiles);
    vector<string> contentFiless;
    for (int i=0;i<numberFiles;i++) {
        contentFiless.emplace_back(contentFiles[i]);
    }
    ClearDirectoryFiles();//this is the free operation for the dir files

    vector<string> challengePacks;
    //check if this workshop item is a Challenge pack
    for (const auto &dir : contentFiless) {
        string challengePackFile = scrapMechanicWorkShopConentLocation + dir+"\\challengePack.json";
        if (FileExists(challengePackFile.c_str())) {
            challengePacks.push_back(dir);
        }
    }

    //Display the Loading screen
    BeginDrawing();
    ClearBackground(DARKGRAY);
    DrawText("Loading",SCREEN_WIDTH/2,SCREEN_HEIGHT/2,40,WHITE);
    EndDrawing();
    WindowShouldClose();
    BeginDrawing();
    ClearBackground(DARKGRAY);
    DrawText("Loading",SCREEN_WIDTH/2,SCREEN_HEIGHT/2,40,WHITE);
    EndDrawing();

    //Load all the json files for ease of use later
    vector<json> challengePackJsons;
    vector<json> challengePackDescriptions;
    vector<Texture2D> packPreviews;
    for (const auto &dir : challengePacks) {
        ifstream packFile(scrapMechanicWorkShopConentLocation + dir+"\\challengePack.json");
        ifstream descriptionFile(scrapMechanicWorkShopConentLocation + dir+"\\description.json");
        json a = json::parse(packFile);
        json b = json::parse(descriptionFile);


        challengePackJsons.push_back(a);
        challengePackDescriptions.push_back(b);

        Image img = LoadImage((scrapMechanicWorkShopConentLocation + dir+"\\preview.jpg").c_str());
        ImageResize(&img,240,135);
        packPreviews.push_back(LoadTextureFromImage(img));
    }

    cout << "Found the following workshop chellange packs:" << endl;
    for (const auto& pack:challengePackDescriptions) {
        cout << pack["name"] << endl;
    }

    vector<string> users;
    contentFiles = GetDirectoryFiles(ScrapMechanicAppdataFolder.c_str(),&numberFiles);
    for (int i=0;i<numberFiles;i++) {
        string user (contentFiles[i]);
        if (user != "." && user != "..") {
            users.emplace_back(user);
        }
    }

    int scrollValue = 0;

    int extractingIndex = -1;
    int extractProgress = 0;
    int extractMaxProgress = 1;
    thread extractingThread;
    bool extracting = false;
    bool userSelected = false;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        if (userSelected) {
            DrawRectangle(0,26-scrollValue,SCREEN_WIDTH,2,BLACK);
            for (size_t i=0;i<challengePacks.size();i++) {
                DrawTexture(packPreviews[i],30,30+140*(int)i-scrollValue,WHITE);//pack preview image
                DrawRectangle(0,166+140*(int)i-scrollValue,SCREEN_WIDTH,2,BLACK);//black line under the image
                string description = challengePackDescriptions[i]["name"];
                DrawText(description.c_str(),280,35+140*(int)i-scrollValue,30,WHITE);
                DrawRectangle(970,35+140*(int)i-scrollValue,300,120,GREEN);//the EXtract button
                DrawRectangleLinesEx({970,35.0f+140.0f* (float)i-(float)scrollValue,300,120},5,LIME);
                DrawText("Extract",1060,80+140*(int)i-scrollValue,30,WHITE);
            }

            if (extracting) {
                DrawRectangle(100,100,SCREEN_WIDTH-200,SCREEN_HEIGHT-200,ORANGE);
                DrawText("Extracting",SCREEN_WIDTH/2 - MeasureText("Extracting",50)/2,300,50,BLACK);
                DrawRectangleLinesEx({300,400,SCREEN_WIDTH-600,100},3,BLACK);
                float progress = (float)extractProgress / (float)extractMaxProgress;
                int progressWidth = (int)((SCREEN_WIDTH-600)*progress);
                DrawRectangle(300,400,progressWidth,100,BLACK);
            }
        } else {
            scrollValue = 0;
            DrawText("SelectUser",20,20,50,WHITE);
            for (int i=0;i<users.size();i++) {
                DrawRectangle(40,85+50*i,600,35,{ 168, 166, 0, 255 });
                DrawText(users[i].c_str(),50,90+50*i,30,WHITE);
                DrawRectangleLinesEx({40,(float)(85+50*i),600,35},2,ORANGE);
            }
        }

        EndDrawing();

        scrollValue -= GetMouseWheelMove() *15;
        scrollValue = min(scrollValue,140*(int)(challengePacks.size()-1));
        scrollValue = max(scrollValue,0);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePos = GetMousePosition();

            if (extractingIndex == -1 && userSelected) {
                if (mousePos.x >= 970 && mousePos.x <= 1270) {
                    for (size_t i=0;i<challengePacks.size();i++) {
                        int yTop = 35+140*(int)i-scrollValue;
                        if ((int)mousePos.y >= yTop && (int)mousePos.y <= yTop + 120) {
                            //set extracting number
                            extractingIndex = (int)i;
                            extracting = true;
                            ThreadInfo info = {
                                ScrapMechanicAppdataFolder,
                                scrapMechanicWorkShopConentLocation,
                                challengePacks[i],
                                challengePackJsons[i],
                                challengePackDescriptions[i],
                                &extractingIndex,
                                &extractProgress,
                                &extractMaxProgress
                            };
                            extractProgress = 0;
                            cout << "Beginning extraction" << endl;
                            extractingThread = thread(extractionThreadFunction,info);
                            break;
                        }
                    }
                }
            }
            if (!userSelected) {
                // 40,85+50*i,600,35
                for (int i=0;i<users.size();i++) {
                    if (mousePos.x >= 40 && mousePos.x <= 640 && mousePos.y >= 85.0f+50.0f*(float)i && mousePos.y <= 85.0f+50.0f*(float)i+35) {
                        userSelected = true;
                        ScrapMechanicAppdataFolder+=users[i]+"\\Challenges";
                        break;
                    }
                }
            }
        }

        if (extracting && extractingIndex == -1) {
            extractingThread.join();//collect the thread resources
            extracting = false;
            cout << "extraction finsihed" << endl;
        }

    }

    for (auto texture: packPreviews) {
        UnloadTexture(texture);
    }

    CloseWindow();
    return 0;
}

string trimString(const string& str) {
    const auto start = str.find_first_not_of(" \t\n\r\f\v");
    const auto end   = str.find_last_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    return str.substr(start, end - start + 1);
}

// ReSharper disable once CppDFAConstantFunctionResult
string getSteamLibraryLocation() {
    //Read the registry key HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Valve\Steam InstallPath
    string steamInstallPath = readRegistryKey(R"(HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Valve\Steam\InstallPath)");
    //in the steam apps folder read the libraryfolders.vfd file
    ifstream libFoldersVDF(steamInstallPath+"\\steamapps\\libraryfolders.vdf");
    //parse though that to see what library scrap mechanic is installed in
    string libPath = "ERROR";
    bool inLib = false;
    bool lastLineNumbers = false;
    bool inApps = false;
    bool success = false;
    string line;

    while (!libFoldersVDF.eof()) {
        getline(libFoldersVDF,line);
        //preform a trim because apparently c++ has no built-in function for that
        line = trimString(line);
        if (!inLib) {
            if (line[0] == '{' && lastLineNumbers) {
                inLib = true;
                lastLineNumbers = false;
            } else if (regex_match(line,regex(R"(\"\d+\")"))) {
                lastLineNumbers = true;
            } else {
                lastLineNumbers = false;
            }
        } else if (!inApps) {
            if (line.starts_with("\"path\"")) {
                string containsPath = line.substr(6);
                containsPath = trimString(containsPath);
                containsPath = containsPath.substr(1, containsPath.length()-2);
                libPath = regex_replace(containsPath, regex(R"(\\\\)"),"\\");
            } else if (line.starts_with("\"apps\"")) {
                inApps = true;
            } else if (line.starts_with("}")) {
                inLib = false;
            }
        } else {
            if (line.starts_with("}")) {
                inApps = false;
            }
            if (line.starts_with("\""+SCRAP_MECHANIC_STEAM_ID+"\"")) {
                success = true;
                break;
            }
        }

    }
    libFoldersVDF.close();
    if (!success) {
        cerr << "Unable to find Scrap Mechanic in installed steam games!" << endl;
        showMessageBox("Unable to find Scrap Mechanic in inallted Steam games!","Can't find it");
        throw runtime_error("Unable to find Scrap Mechanic in installed steam games!");
    }

    return libPath;
}

string generate_uuid_v4() {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_int_distribution<> dis(0, 15);
    static uniform_int_distribution<> dis2(8, 11);

    stringstream ss;
    ss << hex;

    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4"; // UUID version 4
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << dis2(gen); // UUID variant
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);

    return ss.str();
}

void extractChallenge(const string &challengeSourceFolder, string &userContentFolder, json &packJson, json packDescription, const int levelNumber, string &uuid) {
    string destUUID = generate_uuid_v4();//create a new UUID for the destination challenge

    cout << "Extracting Challenge level "<< uuid << " to " << destUUID << endl;

    //make the destination directory
    if (MakeDirectory((userContentFolder+"\\"+destUUID).c_str())) {
        showMessageBox("Failed to create desintatoin challenge folder!","A failure!");
        throw runtime_error("Failed to create Destination challenge folder! "+destUUID);
    }

    //get all the files that need to be coppied
    int numFilesToCopy = 0;
    char ** filesToCopy = GetDirectoryFiles(challengeSourceFolder.c_str(), &numFilesToCopy);
    //copy the files to the dest folder
    for (int i = 0; i < numFilesToCopy; i++) {
        string file(filesToCopy[i]);
        if (file != "." && file != "..") {
            int result = FileCopy((challengeSourceFolder+"\\"+file).c_str(),(userContentFolder+"\\"+destUUID+"\\"+file).c_str());
            if (result != 0) {
                cerr << "WARNING: Failed to copy: " << file << endl;
            }
        }

    }

    int levelIndex = 0;
    string name;
    string description;

    //find the level data from the pack data file
    auto levelList = packJson["levelList"];
    for (int j=0;j<levelList.size();j++) {
        json rawLevelData= levelList[j];
        if (rawLevelData["uuid"] == uuid) {
            name = rawLevelData["name"];
            description = rawLevelData["description"];
            levelIndex = j;
            json usefulData = rawLevelData["data"];
            //make a copy of this json object
            json usefulData2 = json::parse(usefulData.dump());
            //remove the uuid from level creations
            json levelCreations = usefulData2["levelCreations"];
            for (int i=0;i<levelCreations.size();i++) {//TODO this but for inital creations
                string blueprintFile = levelCreations[i];
                // cout << "BPF before: " << blueprintFile <<" after:";
                blueprintFile = regex_replace(blueprintFile,regex("\\/.+?\\/"),"/");
                // cout << blueprintFile << endl;
                levelCreations[i] = blueprintFile;
            }
            usefulData2["levelCreations"] = levelCreations;
            if (!usefulData2["startCreations"].is_null()) {

            }

            json challengeLevel;
            challengeLevel["data"] = usefulData2;
            ofstream challengeLevelOut(userContentFolder+"\\"+destUUID+"\\challengeLevel.json");
            challengeLevelOut << challengeLevel;
            challengeLevelOut.close();
            break;

        }
    }
    //create the description file
    packDescription["type"] = "Challenge Level";
    string chalNumber = " [";
    chalNumber += std::to_string(levelIndex) + "] [Extracted by: Scrap Mechanic Challenge Extractor]";
    packDescription["description"] = description + chalNumber ;
    packDescription["localId"] = generate_uuid_v4();
    packDescription["name"] = name;
    packDescription.erase("fileId");
    ofstream descriptionOut(userContentFolder+"\\"+destUUID+"\\description.json");
    descriptionOut << packDescription;
    descriptionOut.close();

}

void extractionThreadFunction(ThreadInfo info) {
    cout << "extracting " << info.challengePackDescription["name"] << endl;
    int challengeFileCount = 0;
    //get the uuid folders of the challenges
    char ** rawChallengeFiles = GetDirectoryFiles((info.workshopFolder+info.challengeDIR).c_str(),&challengeFileCount);
    vector<string> challenges;
    for (int i=0;i<challengeFileCount;i++) {
        string challenge(rawChallengeFiles[i]);
        if (challenge != "." && challenge!= "..")
        if (DirectoryExists((info.workshopFolder+info.challengeDIR+"\\"+challenge).c_str())) {
            challenges.push_back(challenge);
        }
    }
    *info.progress = 0;
    *info.maxxProgress = (int)challenges.size();
    for (int i=0;i<challenges.size();i++) {
        string &challenge(challenges[i]);
        extractChallenge(info.workshopFolder+info.challengeDIR+"\\"+challenge,info.userFolder,info.challengePackJson,info.challengePackDescription,i,challenge);
        *info.progress = *info.maxxProgress+1;
    }
    //end of thread, set the pack index back to -1 so the main thread knows this thread is done
    *info.packIndex = -1;

}