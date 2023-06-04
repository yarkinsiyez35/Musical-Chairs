#include <iostream>
#include "IntQueueHW6.h"
#include <mutex>
#include <chrono>
#include <ctime>
#include <vector>
#include <thread>
#include <iomanip>


using namespace std;

mutex Mutex;                //global mutex that will take care of dequeue operations

struct Player{
    int id;                 //this is the id of each player
    bool chair;              //this will indicate if player sat on a chair
    Player()            //default constructor
    {}
    Player(int i, int c): id(i), chair(c)           //parametric constructor
    {}
};

vector<Player> createVector(const int & numberOfPlayers)                //creates a vector to store the players
{
    vector<Player> players;
    for (int i= 0; i< numberOfPlayers; i++)
    {
        Player p(i,true);                   //creates the object
        players.push_back(p);                   //adds the object to the vector
    }
    return players;                     //returns the vector
}

vector<Player> eliminatePlayer(vector<Player> & player)               //this will be another vector without the chairless player
{
    vector<Player> newPlayer;
    for (int i =0; i< player.size(); i++)               //finds the standing player
    {
        if (player[i].chair)                      //if the player has the chair, he will be included in the new vector
        {
            newPlayer.push_back(player[i]);         //adds the player to the new vector
        }
    }
    return newPlayer;                                   //returns the new vector
}


void tryToDeque(IntQueueHW6 &queue, Player &player, vector<int> & order, struct tm * ptm)
{
    this_thread::sleep_until(chrono::system_clock::from_time_t(mktime(ptm)));
    Mutex.lock();                   //locks the mutex
    if(!queue.isEmpty())            //if there are still elements
    {
        int dummy;                  //dummy variable for .dequeue() to work
        queue.dequeue(dummy);           //dequeues an element
        cout << "Player " << player.id << " captured a chair at " <<  put_time(ptm,"%X") << "." << endl;
        order.push_back(player.id);                         //stores the order of each player
        Mutex.unlock();             //unlocks the mutex for other threads
    }
    else                            //fails to dequeue
    {
        player.chair= false;           //sets variable as false because the player will be eliminated
        cout << "Player " << player.id << " couldn't capture a chair." << endl;
        Mutex.unlock();             //unlocks the mutex for other threads
    }
}


void PrintOrder(const vector<int> & order)
{
    cout << "Remaining players are as follows: ";
    for (int i= 0; i< order.size(); i++)                //prints each remaining player
    {
        cout << order[i] << " ";
    }
    cout << endl << endl;
}

int main() {
    /*welcome message*/
    cout << "Welcome to Musical Chairs game!\nEnter the number of players in the game:\n";
    int numberOfPlayers;
    cin >> numberOfPlayers;
    cout << "Game Start!\n\n";

    /*creating a vector to store the players*/
    vector<Player> players = createVector(numberOfPlayers);

    int winnerID= -1;           //dummy value, it will be updated in the loop

    while (numberOfPlayers > 1)            //game loop
    {
        time_t tt = chrono::system_clock::to_time_t (chrono::system_clock::now());  //gets the current time
        struct tm *ptm = localtime(&tt);
        cout << "Time is now " << put_time(ptm,"%X") << endl;
        if (ptm->tm_sec >57)            //caution against seconds overflow
        {
            if (ptm->tm_min == 59)           //caution against minutes overflow
            {
                ptm->tm_hour++;
                ptm->tm_min = 0;
            }
            else
            {
                ptm->tm_min++;
            }
            ptm->tm_sec = (ptm->tm_sec + 2) %60;
        }
        else
        {
            ptm->tm_sec += 2;
        }

        /*creating the queue*/
        IntQueueHW6 queue(numberOfPlayers-1);

        /*creating a vector to store the order of finishes*/
        vector<int> order;

        /*adding chairs*/
        for (int i=0; i< numberOfPlayers-1; i++)
        {
            queue.enqueue(i);
        }

        /*creating a thread array*/
        thread *tarr = new thread[numberOfPlayers];                 //a thread array with numberOfPlayers many slots is created
        for (int i= 0; i< numberOfPlayers; i++)
        {
            tarr[i] = thread(&tryToDeque, ref(queue), ref(players[i]), ref(order), ptm);             //race begins
        }

        /*joining all the threads*/
        for (int i= 0; i<numberOfPlayers; i++)
        {
            tarr[i].join();                                                         //joins each thread
        }

        /*Display*/
        PrintOrder(order);                  //prints the order of the players

        /*editing for the next iteration*/
        players = eliminatePlayer(players);         //creates the new player vector
        numberOfPlayers--;                  //decrements the number of players
        delete[] tarr;              //deletes the dynamic thread array
        winnerID = order[0];                            //at the end of the loop, first index of the order will be the winner
    }

    cout << "Game Over!" << endl;
    cout << "Winner is Player " << winnerID << "!" << endl;

    return 0;
}
