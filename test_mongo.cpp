#include<iostream>
#include<stdio.h>
#include "mongo.h"
using namespace std;

int main() {
	MONGO mongo;
	mongo.init();
	
	mongo.delete_collection("live_teams");	
}
