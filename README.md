# Course Project Repository

The goal of this study project:

* Implement basic networking in C (client to server and vice-versa)
* Create game logic and multiplayer functionality
* Build a decent-looking GUI
* Work with WSL, learn to work unix/linux

## Running

This project uses make as the build system. I recommend building it in a linux based system or WSL.

* Open the project root in terminal
* make adjustments under `testserv` and `testclient` to modiufy host and port according to your needs
* Run `make all`
* Run `make testserv` to start server
* Run `make testclient` for each client you want to launch
* Follow the instructions on the client screen

## Improvements needed

* Improve collision system (so that users cannot walk through walls)