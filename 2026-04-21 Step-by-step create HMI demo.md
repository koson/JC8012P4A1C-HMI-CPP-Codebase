
``` powershell 

PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> git branch --show-current
main
PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> git checkout -b AppDemoScreen
Switched to a new branch 'AppDemoScreen'
PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> git branch --show-current    
AppDemoScreen

# สร้างผิด ไปสร้างจาก main ต้องเอาใหม่

PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> git branch --show-current
AppDemoScreen
PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> git checkout App-MenuDemo
Switched to branch 'App-MenuDemo'
Your branch is up to date with 'origin/App-MenuDemo'.
PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> git branch 
* App-MenuDemo
  AppDemoScreen
  DEV
  main
PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> git branch -D AppDemoScreen
Deleted branch AppDemoScreen (was 3c7b490).
PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> git branch                 
* App-MenuDemo
  DEV
  main
PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> git checkout -b Demo-ColorWheel
Switched to a new branch 'Demo-ColorWheel'
PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> git branch --show-current      
Demo-ColorWheel
  
PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> ls main/cpp_version

PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> ls main/cpp_version


    Directory: D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase\main\cpp_version


Mode                 LastWriteTime         Length Name                                                                                   
----                 -------------         ------ ----                                                                                   
-a----         4/21/2026  10:51 PM           6158 DemoManager.cpp   # <--- มีไฟล์นี้
-a----         1/13/2026   5:03 PM          10474 example_cpp.cpp                                                                        -a----         4/21/2026  10:51 PM          12994 MenuDemoApplication.cpp # <--- มีไฟล์นี้
-a----         1/13/2026   5:03 PM           1020 README.md                                                                              
-a----         1/15/2026   8:50 AM           6136 screenshot.cpp                                                                         
-a----         4/21/2026  10:58 AM           7134 SystemManager.cpp                                                                      
-a----         4/21/2026  10:50 AM          11953 VPIApplication.cpp  

PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> git log --oneline -5
2a46497 (HEAD -> Demo-ColorWheel, origin/App-MenuDemo, App-MenuDemo) Add DemoManager and MenuDemoApplication for LVGL demo gallery
bcb949f feat: Refactor VPI HMI application structure
ac56f27 (origin/DEV, DEV) Add support for LVGL avoid tear modes and screenshot feature
PS D:\GitHubRepos\__AES\__ESP32_HMI_IPS_CODEBASE\JC8012P4A1C-HMI-CPP-Codebase> 
```