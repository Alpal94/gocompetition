## Go competition server

This open source software is designed to host a compeition, where compeditors design AI's for the board game Go.  The server code handles everything that is required to run the competition, and includes an API for the AI's to use in order to communicate to the server.  The server is currently under construction and is not ready yet.  API Documentation can be found here: http://gocompetition.hopto.org/html/annotated.html

## Client API

The Client API is written in C++.  It is intended that there will be API's for Java, PHP and Python and more.  API Documentation: http://gocompetition.hopto.org/html/classAPI.html 

## Server running instructions
# Apache and PHP
Install apache and PHP and put everything in the WEB folder into Apaches root folder.
# Mongo 
Install and setup mongodb.  Change passwords and usernames in files to your password and username, including port number of mongo db.  Installation of rockmongo is recommended for Mongo GUI.
# Mongo C library
Install the mongo C library: http://mongoc.org/
