﻿// libCallCmd.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "process.hpp"
int main()
{
    Process process;
    thread thread([&]() {
        
        process.open(".\\winBin\\tubepaw.exe ewogICAiZG93bmxvYWRlciI6ewogICAgICAiYWRkX3BsYXlsaXN0X2luZGV4IjoiZmFsc2UiLAogICAgICAicmVhZF9jb29raWUiOiJmYWxzZSIsCiAgICAgICJzYXZlX2lkMyI6InRydWUiLAogICAgICAic2F2ZV9wYXRoIjoiLi92aWRlb3MiLAogICAgICAic3VidGl0bGUiOiJ6aC1IYW5zIiwKCSJzbmlmZl9vbmx5IjogImZhbHNlIiwKICAgICAgInVybCI6ICJodHRwczovL3d3dy55b3V0dWJlLmNvbS93YXRjaD92PUd3QjNHZFBBUmx3IgogICB9LAogICAiZmZtcGVnX2xvY2F0aW9uIjoiLi93aW5CaW4vIgp9Cg==", [](const char* bytes) {
            cout << "Output from stdout: " << string(bytes) << endl;
            });
        });
    thread.detach();
    process.kill();
    Sleep(10000);
     return 0;
}